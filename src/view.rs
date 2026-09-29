//! The viewport: perspective/orthographic projection, view directions and
//! the mapping from the active camera to Bevy's camera.

use bevy::prelude::*;
use bevy::render::camera::ScalingMode;

use crate::camera::{CameraManager, CameraRig};
use crate::settings::Settings;

/// Which way the world is viewed.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ViewType {
    Perspective,
    Front,
    Back,
    Left,
    Right,
    Top,
    Bottom,
}

impl ViewType {
    /// Menu caption of the view.
    pub fn caption(self) -> &'static str {
        match self {
            Self::Perspective => "3d",
            Self::Front => "Front",
            Self::Back => "Back",
            Self::Left => "Left",
            Self::Right => "Right",
            Self::Top => "Top",
            Self::Bottom => "Bottom",
        }
    }

    pub const ALL: [ViewType; 7] =
        [Self::Perspective, Self::Front, Self::Back, Self::Left, Self::Right, Self::Top, Self::Bottom];
}

/// Distance from the eye to the look-at point in orthographic views.
const ORTHO_LOOK_DISTANCE: f32 = 6.0;
/// How far the orthographic cameras start from the origin.
const ORTHO_START_DISTANCE: f32 = 1000.0;

/// Marks the camera that renders the world to the window.
#[derive(Component)]
pub struct MainCamera;

#[derive(Resource)]
pub struct ViewPort {
    pub view_type: ViewType,
    /// Vertical field of view in degrees (perspective).
    pub fov: f32,
    /// Orthographic zoom; larger shows more.
    pub zoom_level: f32,
    /// Index into [`CameraManager::cameras`] of the camera being viewed.
    pub camera: usize,
    near: f32,
    far: f32,
    /// Where each orthographic camera last was, so switching back restores it.
    ortho_pos: [Vec3; 6],
    saved_perspective: Option<(Vec3, f64, f64)>,
}

impl Default for ViewPort {
    fn default() -> Self {
        Self {
            view_type: ViewType::Perspective,
            fov: 45.0,
            zoom_level: 100.0,
            camera: 0,
            near: 1.0,
            far: 8000.0,
            ortho_pos: [
                Vec3::new(0.0, 0.0, ORTHO_START_DISTANCE),
                Vec3::new(0.0, 0.0, -ORTHO_START_DISTANCE),
                Vec3::new(-ORTHO_START_DISTANCE, 0.0, 0.0),
                Vec3::new(ORTHO_START_DISTANCE, 0.0, 0.0),
                Vec3::new(0.0, ORTHO_START_DISTANCE, 0.0),
                Vec3::new(0.0, -ORTHO_START_DISTANCE, 0.0),
            ],
            saved_perspective: None,
        }
    }
}

impl ViewPort {
    pub fn is_perspective(&self) -> bool {
        self.view_type == ViewType::Perspective
    }

    fn ortho_index(view: ViewType) -> Option<usize> {
        match view {
            ViewType::Perspective => None,
            ViewType::Front => Some(0),
            ViewType::Back => Some(1),
            ViewType::Left => Some(2),
            ViewType::Right => Some(3),
            ViewType::Top => Some(4),
            ViewType::Bottom => Some(5),
        }
    }

    /// Unit direction the camera looks along in the current view.
    pub fn view_direction(&self, rig: &CameraRig) -> Vec3 {
        match self.view_type {
            ViewType::Perspective => rig.direction(),
            ViewType::Front => Vec3::NEG_Z,
            ViewType::Back => Vec3::Z,
            ViewType::Left => Vec3::X,
            ViewType::Right => Vec3::NEG_X,
            ViewType::Top => Vec3::NEG_Y,
            ViewType::Bottom => Vec3::Y,
        }
    }

    /// Up vector used when looking along a view direction.
    fn up_vector(view: ViewType) -> Vec3 {
        match view {
            ViewType::Top => Vec3::X,
            ViewType::Bottom => Vec3::NEG_X,
            _ => Vec3::Y,
        }
    }

    /// Switches to `view`, remembering the free camera when leaving it, and
    /// picks a grid spacing that suits the view.
    pub fn set_view(&mut self, view: ViewType, rig: &mut CameraRig, settings: &mut Settings) {
        if view == self.view_type {
            return;
        }
        if self.is_perspective() {
            self.saved_perspective = Some((rig.pos, rig.theta, rig.phi));
        }

        match Self::ortho_index(view) {
            None => {
                if let Some((pos, theta, phi)) = self.saved_perspective.take() {
                    (rig.pos, rig.theta, rig.phi) = (pos, theta, phi);
                }
                settings.grid.set_spacing(3, 100);
            }
            Some(i) => {
                rig.pos = self.ortho_pos[i];
                settings.grid.set_spacing(if matches!(view, ViewType::Top | ViewType::Bottom) { 1 } else { 2 }, 64);
            }
        }
        rig.up = Self::up_vector(view);
        self.view_type = view;
    }

    /// Remembers where an orthographic camera is as it is panned.
    pub fn remember_ortho_position(&mut self, rig: &CameraRig) {
        if let Some(i) = Self::ortho_index(self.view_type) {
            self.ortho_pos[i] = rig.pos;
        }
    }

    /// Where the camera is and what it looks at.
    pub fn eye_and_target(&self, rig: &CameraRig) -> (Vec3, Vec3) {
        let distance = if self.is_perspective() { 1.0 } else { ORTHO_LOOK_DISTANCE };
        (rig.pos, rig.pos + self.view_direction(rig) * distance)
    }

    pub fn zoom_in(&mut self, amount: f32) {
        self.zoom_level = (self.zoom_level - amount).max(1.0);
    }

    pub fn zoom_out(&mut self, amount: f32) {
        self.zoom_level += amount;
    }

    /// A percentage of the current zoom, used to scale panning speed.
    pub fn zoom_percent(&self, percent: f32) -> f32 {
        self.zoom_level * percent / 100.0
    }

    /// Mouse wheel: change field of view, or zoom in orthographic views.
    pub fn wheel(&mut self, notches: f32) {
        if self.is_perspective() {
            self.fov = (self.fov - notches).clamp(1.0, 170.0);
        } else {
            let amount = notches * self.zoom_percent(25.0);
            self.zoom_in(amount);
        }
    }

    pub fn projection(&self) -> Projection {
        if self.is_perspective() {
            Projection::Perspective(PerspectiveProjection {
                fov: self.fov.to_radians(),
                near: self.near,
                far: self.far,
                ..default()
            })
        } else {
            Projection::Orthographic(OrthographicProjection {
                near: 0.5,
                far: self.far,
                // One world unit spans `scale` pixels: at the default zoom of
                // 100 the view is 0.1 world units per pixel.
                scaling_mode: ScalingMode::WindowSize,
                scale: self.zoom_level / 1000.0,
                ..OrthographicProjection::default_3d()
            })
        }
    }
}

/// Positions Bevy's main camera from the active camera and view type.
pub fn sync_main_camera(
    mut view: ResMut<ViewPort>,
    cameras: Res<CameraManager>,
    mut camera: Query<(&mut Transform, &mut Projection), With<MainCamera>>,
) {
    let Ok((mut transform, mut projection)) = camera.get_single_mut() else { return };
    let Some(rig) = cameras.cameras.get(view.camera) else { return };

    view.remember_ortho_position(rig);
    let (eye, target) = view.eye_and_target(rig);
    *transform = Transform::from_translation(eye).looking_at(target, rig.up);

    let wanted = view.projection();
    if !projection_matches(&projection, &wanted) {
        *projection = wanted;
    }
}

fn projection_matches(a: &Projection, b: &Projection) -> bool {
    match (a, b) {
        (Projection::Perspective(a), Projection::Perspective(b)) => a.fov == b.fov && a.near == b.near && a.far == b.far,
        (Projection::Orthographic(a), Projection::Orthographic(b)) => a.scale == b.scale,
        _ => false,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn setup() -> (ViewPort, CameraRig, Settings) {
        (ViewPort::default(), CameraRig::new("w", Vec3::new(3.0, 4.0, 5.0)), Settings::default())
    }

    #[test]
    fn leaving_and_returning_to_perspective_restores_the_camera() {
        let (mut view, mut rig, mut settings) = setup();
        rig.theta = 1.2;
        view.set_view(ViewType::Top, &mut rig, &mut settings);
        assert_eq!(rig.pos, Vec3::new(0.0, 1000.0, 0.0));
        assert_eq!(rig.up, Vec3::X);
        rig.theta = 2.0;
        view.set_view(ViewType::Perspective, &mut rig, &mut settings);
        assert_eq!(rig.pos, Vec3::new(3.0, 4.0, 5.0));
        assert_eq!(rig.theta, 1.2);
        assert_eq!(rig.up, Vec3::Y);
    }

    #[test]
    fn switching_between_orthographic_views_keeps_the_saved_free_camera() {
        let (mut view, mut rig, mut settings) = setup();
        view.set_view(ViewType::Front, &mut rig, &mut settings);
        view.set_view(ViewType::Left, &mut rig, &mut settings);
        view.set_view(ViewType::Perspective, &mut rig, &mut settings);
        assert_eq!(rig.pos, Vec3::new(3.0, 4.0, 5.0));
    }

    #[test]
    fn panned_orthographic_position_is_remembered_per_view() {
        let (mut view, mut rig, mut settings) = setup();
        view.set_view(ViewType::Front, &mut rig, &mut settings);
        rig.pos.x = 42.0;
        view.remember_ortho_position(&rig);
        view.set_view(ViewType::Back, &mut rig, &mut settings);
        view.set_view(ViewType::Front, &mut rig, &mut settings);
        assert_eq!(rig.pos.x, 42.0);
    }

    #[test]
    fn orthographic_views_look_down_their_axis() {
        let (mut view, mut rig, mut settings) = setup();
        view.set_view(ViewType::Front, &mut rig, &mut settings);
        let (eye, target) = view.eye_and_target(&rig);
        assert_eq!(target - eye, Vec3::new(0.0, 0.0, -6.0));
    }

    #[test]
    fn grid_spacing_follows_the_view() {
        let (mut view, mut rig, mut settings) = setup();
        view.set_view(ViewType::Top, &mut rig, &mut settings);
        assert_eq!((settings.grid.thickness, settings.grid.spacing), (1, 64));
        view.set_view(ViewType::Perspective, &mut rig, &mut settings);
        assert_eq!((settings.grid.thickness, settings.grid.spacing), (3, 100));
    }

    #[test]
    fn wheel_changes_fov_in_perspective_and_zoom_in_ortho() {
        let (mut view, mut rig, mut settings) = setup();
        view.wheel(1.0);
        assert_eq!(view.fov, 44.0);
        for _ in 0..500 {
            view.wheel(1.0);
        }
        assert_eq!(view.fov, 1.0);
        view.set_view(ViewType::Front, &mut rig, &mut settings);
        view.wheel(1.0);
        assert_eq!(view.zoom_level, 75.0);
        view.wheel(-1.0);
        assert!(view.zoom_level > 75.0);
    }
}
