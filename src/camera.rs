//! Camera state: the free-flying viewer, and camera objects placed in the
//! world that can be viewed from or taken control of.

use bevy::prelude::*;

use crate::math::{HALF_PI, spherical_direction};

/// A camera's position and orientation.
///
/// Orientation is kept as two angles rather than a quaternion because mouse
/// look adjusts them directly: `theta` is yaw about +Y (0 faces +Z) and
/// positive `phi` tilts the view downward.
#[derive(Clone, Debug)]
pub struct CameraRig {
    pub name: String,
    pub pos: Vec3,
    pub theta: f64,
    pub phi: f64,
    pub up: Vec3,
    /// The visible model of a placed camera; `None` for the world camera.
    pub model: Option<Entity>,
    /// Whether this camera's picture-in-picture view is showing.
    pub viewport_on: bool,
    /// The window showing this camera's picture, while it is open.
    pub video_window: Option<Entity>,
}

impl CameraRig {
    pub fn new(name: impl Into<String>, pos: Vec3) -> Self {
        Self {
            name: name.into(),
            pos,
            theta: 0.0,
            phi: 0.0,
            up: Vec3::Y,
            model: None,
            viewport_on: false,
            video_window: None,
        }
    }

    /// Unit vector this camera looks along in a free (perspective) view.
    pub fn direction(&self) -> Vec3 {
        spherical_direction(1.0, self.theta, self.phi + HALF_PI)
    }

    /// Applies mouse motion in device counts to the camera angles, clamping
    /// pitch just short of straight up or down.
    pub fn look(&mut self, rel_x: f32, rel_y: f32) {
        if rel_x == 0.0 && rel_y == 0.0 {
            return;
        }
        self.theta -= mouse_look_step(rel_x);
        self.phi += mouse_look_step(rel_y);
        self.phi = self.phi.clamp(-HALF_PI + 0.08, HALF_PI - 0.08);
    }

    /// Turns the camera to face `target`.
    pub fn look_at(&mut self, target: Vec3) {
        let dir = (target - self.pos).normalize_or_zero();
        if dir == Vec3::ZERO {
            return;
        }
        // Inverse of `direction()`: theta is the yaw from +Z, and positive phi
        // looks down.
        self.theta = f64::from(dir.x).atan2(f64::from(dir.z));
        self.phi = f64::from(-dir.y).asin().clamp(-HALF_PI + 0.08, HALF_PI - 0.08);
    }

    /// Orients a camera model to match this camera and moves it to its position.
    pub fn model_transform(&self) -> Transform {
        Transform {
            translation: self.pos,
            rotation: Quat::from_rotation_y(self.theta as f32) * Quat::from_rotation_x(self.phi as f32),
            scale: Vec3::ONE,
        }
    }
}

/// Radians of turn per pixel of mouse or finger movement (about 0.2 degrees).
///
/// The original divided raw DirectInput counts by a speed-dependent table,
/// which with pixel deltas turned up to a radian in a single frame.
pub const LOOK_RADIANS_PER_PIXEL: f64 = 0.0035;

pub fn mouse_look_step(rel: f32) -> f64 {
    f64::from(rel) * LOOK_RADIANS_PER_PIXEL
}

/// All cameras. `world` is the one the user flies around with; the others
/// are placed objects.
#[derive(Resource)]
pub struct CameraManager {
    pub cameras: Vec<CameraRig>,
    /// Camera whose model the user is currently controlling, if any.
    pub active: Option<usize>,
    /// Index of the free-flying world camera.
    pub world: usize,
}

impl Default for CameraManager {
    fn default() -> Self {
        // Standing at eye height in front of the origin, where scenes are
        // built, looking at it along +Z.
        let grid = crate::settings::GridSettings::default();
        let world_camera = CameraRig::new("World", Vec3::new(0.0, grid.eye_height(), -60.0));
        Self { cameras: vec![world_camera], active: None, world: 0 }
    }
}

impl CameraManager {
    pub fn is_camera_model(&self, entity: Entity) -> Option<usize> {
        self.cameras.iter().position(|c| c.model == Some(entity))
    }
}

/// Marks the model entity of a placed camera.
#[derive(Component)]
pub struct CameraObject {
    pub index: usize,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_camera_faces_positive_z() {
        let dir = CameraManager::default().cameras[0].direction();
        assert!((dir - Vec3::Z).length() < 1e-4, "{dir:?}");
    }

    #[test]
    fn moving_the_mouse_right_turns_right() {
        let mut rig = CameraRig::new("t", Vec3::ZERO);
        let before = rig.direction();
        rig.look(10.0, 0.0);
        // Facing +Z, right is -X.
        assert!(rig.direction().x < before.x);
    }

    #[test]
    fn moving_the_mouse_down_looks_down() {
        let mut rig = CameraRig::new("t", Vec3::ZERO);
        rig.look(0.0, 10.0);
        assert!(rig.direction().y < 0.0);
    }

    #[test]
    fn pitch_is_clamped_short_of_vertical() {
        let mut rig = CameraRig::new("t", Vec3::ZERO);
        for _ in 0..1000 {
            rig.look(0.0, 50.0);
        }
        assert!(rig.phi < HALF_PI);
        assert!(rig.direction().y > -1.0);
    }

    #[test]
    fn look_speed_is_proportional_to_movement() {
        // Linear: a fast swipe turns proportionally, not explosively.
        assert!((mouse_look_step(100.0) - 100.0 * mouse_look_step(1.0)).abs() < 1e-12);
        assert!(mouse_look_step(100.0) < 0.5, "a 100px swipe is well under 30 degrees");
        assert_eq!(mouse_look_step(-4.0), -mouse_look_step(4.0));
    }

    #[test]
    fn look_at_points_the_camera_at_the_target() {
        let mut rig = CameraRig::new("t", Vec3::new(35.0, 12.0, -5.0));
        rig.look_at(Vec3::ZERO);
        let expected = (Vec3::ZERO - rig.pos).normalize();
        assert!((rig.direction() - expected).length() < 1e-4, "{:?}", rig.direction());
    }

    #[test]
    fn camera_model_faces_the_viewing_direction() {
        let mut rig = CameraRig::new("t", Vec3::new(1.0, 2.0, 3.0));
        rig.theta = 0.7;
        rig.phi = 0.3;
        let t = rig.model_transform();
        assert_eq!(t.translation, rig.pos);
        assert!((t.rotation * Vec3::Z - rig.direction()).length() < 1e-4);
    }
}

/// Takes over a camera object: the view switches to it, its model is hidden
/// and the UI gets out of the way until Escape.
pub fn enter_control(world: &mut World, index: usize) {
    use crate::settings::{Settings, SystemState};
    use crate::view::ViewPort;

    let Some(model) = world.resource::<CameraManager>().cameras.get(index).and_then(|c| c.model) else { return };
    {
        let mut settings = world.resource_mut::<Settings>();
        settings.old_system_state = settings.system;
        settings.system = SystemState::CameraControl;
    }
    world.resource_mut::<CameraManager>().active = Some(index);
    world.resource_mut::<ViewPort>().camera = index;
    if let Some(mut visibility) = world.get_mut::<Visibility>(model) {
        *visibility = Visibility::Hidden;
    }
}

/// Returns from camera control: the camera's model is placed where the
/// camera ended up and the world camera takes over the view again.
pub fn exit_control(world: &mut World) {
    use crate::settings::Settings;
    use crate::view::ViewPort;

    let Some(index) = world.resource_mut::<CameraManager>().active.take() else { return };
    let (transform, model, world_camera) = {
        let cameras = world.resource::<CameraManager>();
        (cameras.cameras[index].model_transform(), cameras.cameras[index].model, cameras.world)
    };
    if let Some(model) = model {
        if let Some(mut t) = world.get_mut::<Transform>(model) {
            *t = transform;
        }
        if let Some(mut visibility) = world.get_mut::<Visibility>(model) {
            *visibility = Visibility::Inherited;
        }
    }
    world.resource_mut::<ViewPort>().camera = world_camera;
    let mut settings = world.resource_mut::<Settings>();
    settings.system = settings.old_system_state;
}

/// Placed cameras follow their models when the user drags them.
pub fn follow_models(mut cameras: ResMut<CameraManager>, models: Query<&Transform, With<CameraObject>>) {
    let active = cameras.active;
    for (index, rig) in cameras.cameras.iter_mut().enumerate() {
        if Some(index) == active {
            continue;
        }
        if let Some(t) = rig.model.and_then(|m| models.get(m).ok()) {
            if rig.pos != t.translation {
                rig.pos = t.translation;
            }
        }
    }
}

/// Creates the placed camera the simulator starts with.
pub fn spawn_placed_camera(world: &mut World, name: &str, pos: Vec3) -> Result<usize, crate::objects::model::ModelError> {
    use crate::objects::model;

    let bytes = crate::data::model("camera").ok_or_else(|| model::ModelError::NotFound("camera".into()))?;
    let index = world.resource::<CameraManager>().cameras.len();
    let mut rig = CameraRig::new(name, pos);
    // Placed cameras watch the middle of the room, where scenes are built.
    rig.look_at(Vec3::new(0.0, 5.0, -10.0));
    let layers = model::hidden_from_quick_camera(index.saturating_sub(1));
    let entity = model::spawn_from_bytes(world, "camera", bytes, rig.model_transform(), layers)?;
    world.entity_mut(entity).insert(CameraObject { index });
    rig.model = Some(entity);
    world.resource_mut::<CameraManager>().cameras.push(rig);
    Ok(index)
}
