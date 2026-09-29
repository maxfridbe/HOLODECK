//! Selecting objects and manipulator handles with the mouse.

use bevy::prelude::*;
use bevy::window::PrimaryWindow;

use super::format::ModelData;
use super::manip::{Layout, Manipulator};
use super::model::ModelGeometry;
use crate::settings::{Settings, SystemState};
use crate::ui3d::UiState;
use crate::view::MainCamera;

/// What the user has selected.
#[derive(Resource, Default, Debug)]
pub struct Selection {
    /// The selected model (a scene object or a camera model).
    pub entity: Option<Entity>,
    /// The last click landed on a manipulator handle rather than an object.
    pub on_handle: bool,
}

impl Selection {
    pub fn clear(&mut self) {
        self.entity = None;
        self.on_handle = false;
    }
}

/// Distance along the ray to a triangle, if it hits (either side).
/// Möller–Trumore.
pub fn ray_triangle(origin: Vec3, dir: Vec3, a: Vec3, b: Vec3, c: Vec3) -> Option<f32> {
    let (e1, e2) = (b - a, c - a);
    let p = dir.cross(e2);
    let det = e1.dot(p);
    if det.abs() < 1e-9 {
        return None;
    }
    let inv = 1.0 / det;
    let t_vec = origin - a;
    let u = t_vec.dot(p) * inv;
    if !(0.0..=1.0).contains(&u) {
        return None;
    }
    let q = t_vec.cross(e1);
    let v = dir.dot(q) * inv;
    if v < 0.0 || u + v > 1.0 {
        return None;
    }
    let t = e2.dot(q) * inv;
    (t > 0.0).then_some(t)
}

/// Distance along a world-space ray to the nearest triangle of a model
/// placed with `transform`.
pub fn ray_model(origin: Vec3, dir: Vec3, transform: &Transform, model: &ModelData) -> Option<f32> {
    // Work in model space; the ray parameter is unchanged by the affine map.
    let inverse = transform.compute_matrix().inverse();
    let (o, d) = (inverse.transform_point3(origin), inverse.transform_vector3(dir));
    if !inverse.is_finite() {
        return None;
    }

    // Cheap rejection against the model's box first.
    let bounds = model.bounding_box;
    if !ray_hits_box(o, d, bounds.min, bounds.max) {
        return None;
    }

    model
        .meshes()
        .iter()
        .flat_map(|mesh| mesh.vertices.chunks_exact(3))
        .filter_map(|t| ray_triangle(o, d, t[0].position, t[1].position, t[2].position))
        .min_by(f32::total_cmp)
}

fn ray_hits_box(o: Vec3, d: Vec3, min: Vec3, max: Vec3) -> bool {
    let inv = Vec3::ONE / d;
    let (t1, t2) = ((min - o) * inv, (max - o) * inv);
    let near = t1.min(t2).max_element();
    let far = t1.max(t2).min_element();
    far >= near.max(0.0) || d.abs().min_element() == 0.0 && far >= 0.0
}

/// The world ray under the cursor.
fn cursor_ray(window: &Window, camera: &Camera, transform: &Transform) -> Option<Ray3d> {
    let cursor = window.cursor_position()?;
    camera.viewport_to_world(&GlobalTransform::from(*transform), cursor).ok()
}

/// The cursor ray of the main camera (shared with manipulator dragging).
pub fn main_ray(window: &Window, camera: &Camera, transform: &Transform) -> Option<(Vec3, Vec3)> {
    cursor_ray(window, camera, transform).map(|r| (r.origin, *r.direction))
}

/// Left click: grab a manipulator handle if one is under the cursor,
/// otherwise select the frontmost object (or nothing).
#[allow(clippy::too_many_arguments)]
pub fn select_on_click(
    mouse: Res<ButtonInput<MouseButton>>,
    settings: Res<Settings>,
    ui: Res<UiState>,
    windows: Query<&Window, With<PrimaryWindow>>,
    camera: Query<(&Camera, &Transform), With<MainCamera>>,
    models: Query<(Entity, &Transform, &ModelGeometry, &Visibility), Without<MainCamera>>,
    mut selection: ResMut<Selection>,
    mut manip: ResMut<Manipulator>,
) {
    let clicked = mouse.just_pressed(MouseButton::Left) && !mouse.just_pressed(MouseButton::Right) && !mouse.just_pressed(MouseButton::Middle);
    if !clicked || ui.active || ui.pointer_over_ui || settings.system == SystemState::CameraControl {
        return;
    }
    let (Ok(window), Ok((camera, camera_transform))) = (windows.get_single(), camera.get_single()) else { return };
    let Some((origin, dir)) = main_ray(window, camera, camera_transform) else { return };

    // Handles win over objects regardless of depth.
    if let (Some(kind), Some(entity)) = (manip.kind, selection.entity) {
        if let Ok((_, transform, geometry, _)) = models.get(entity) {
            let layout = Layout::compute(kind, &geometry.data.bounding_box, transform, camera_transform.translation);
            if let Some((handle, _)) = super::manip::pick_handle(kind, &layout, origin, dir) {
                selection.on_handle = true;
                manip.begin_drag(handle, &layout, origin, dir);
                return;
            }
        }
    }

    let nearest = models
        .iter()
        .filter(|(.., visibility)| **visibility != Visibility::Hidden)
        .filter_map(|(entity, transform, geometry, _)| Some((entity, ray_model(origin, dir, transform, &geometry.data)?)))
        .min_by(|a, b| a.1.total_cmp(&b.1));

    match nearest {
        Some((entity, _)) => {
            selection.entity = Some(entity);
            selection.on_handle = false;
        }
        None => {
            selection.clear();
            manip.hide();
        }
    }
}

/// Continues or ends a handle drag while the left button is held.
pub fn drag_handle(
    mouse: Res<ButtonInput<MouseButton>>,
    windows: Query<&Window, With<PrimaryWindow>>,
    camera: Query<(&Camera, &Transform), With<MainCamera>>,
    selection: Res<Selection>,
    mut manip: ResMut<Manipulator>,
    mut targets: Query<(&mut Transform, &ModelGeometry), Without<MainCamera>>,
) {
    if !manip.is_dragging() {
        return;
    }
    if !mouse.pressed(MouseButton::Left) {
        manip.end_drag();
        return;
    }
    let (Ok(window), Ok((camera, camera_transform))) = (windows.get_single(), camera.get_single()) else { return };
    let (Some((origin, dir)), Some(entity)) = (main_ray(window, camera, camera_transform), selection.entity) else { return };
    if let Ok((mut transform, geometry)) = targets.get_mut(entity) {
        manip.drag_to(&mut transform, &geometry.data.bounding_box, origin, dir);
    } else {
        manip.end_drag();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::objects::format;

    #[test]
    fn ray_hits_a_triangle_from_either_side() {
        let (a, b, c) = (Vec3::new(-1.0, -1.0, 0.0), Vec3::new(1.0, -1.0, 0.0), Vec3::new(0.0, 1.0, 0.0));
        assert_eq!(ray_triangle(Vec3::new(0.0, 0.0, 5.0), Vec3::NEG_Z, a, b, c), Some(5.0));
        assert_eq!(ray_triangle(Vec3::new(0.0, 0.0, -5.0), Vec3::Z, a, b, c), Some(5.0));
        assert_eq!(ray_triangle(Vec3::new(3.0, 0.0, 5.0), Vec3::NEG_Z, a, b, c), None);
        // Behind the ray origin is not a hit.
        assert_eq!(ray_triangle(Vec3::new(0.0, 0.0, 5.0), Vec3::Z, a, b, c), None);
    }

    fn cone() -> ModelData {
        format::parse(crate::data::model("cone").unwrap()).unwrap()
    }

    #[test]
    fn a_ray_through_a_model_hits_it_and_one_beside_it_does_not() {
        let model = cone();
        let center = model.bounding_box.center();
        let hit = ray_model(center + Vec3::new(0.0, 0.0, 1000.0), Vec3::NEG_Z, &Transform::IDENTITY, &model);
        assert!(hit.is_some());
        let miss = ray_model(center + Vec3::new(10_000.0, 0.0, 1000.0), Vec3::NEG_Z, &Transform::IDENTITY, &model);
        assert!(miss.is_none());
    }

    #[test]
    fn picking_respects_the_objects_transform() {
        let model = cone();
        let center = model.bounding_box.center();
        let moved = Transform::from_xyz(500.0, 0.0, 0.0);
        let at_old_spot = ray_model(center + Vec3::new(0.0, 0.0, 1000.0), Vec3::NEG_Z, &moved, &model);
        let at_new_spot = ray_model(center + Vec3::new(500.0, 0.0, 1000.0), Vec3::NEG_Z, &moved, &model);
        assert!(at_old_spot.is_none());
        assert!(at_new_spot.is_some());
    }

    #[test]
    fn scaled_models_report_world_distance() {
        let model = cone();
        let center = model.bounding_box.center();
        let t = Transform::from_scale(Vec3::splat(2.0));
        // Ray in world space starts 1000 units in front of the scaled centre.
        let d = ray_model(t.transform_point(center) + Vec3::new(0.0, 0.0, 1000.0), Vec3::NEG_Z, &t, &model).unwrap();
        assert!(d < 1000.0 && d > 0.0);
    }
}
