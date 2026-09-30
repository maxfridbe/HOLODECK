//! Global, mutable world settings (the C++ `Settings` and `States` statics),
//! expressed as a Bevy resource.

use bevy::prelude::*;

/// High level mode of the simulator.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum SystemState {
    /// Normal editing: menus, selection and manipulators are live.
    Design,
    /// Mouse look and WASD drive a camera object; the UI is hidden.
    CameraControl,
}

/// State machine for the holographic grid box.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum GridState {
    /// Fully drawn.
    On,
    /// Not drawn (only the axis labels remain).
    Off,
    /// Animating from `On` to `Off`.
    Disable,
    /// Animating from `Off` to `On`.
    Enable,
}

/// The holographic grid: the room everything happens in.
///
/// Sized to the content: models are 2-25 units across and scenes spread over
/// roughly +-130 units, with the bottoms of objects near y = -5. (The
/// original's room was 1000 units a side with the floor 500 units below the
/// objects, so everything floated in the middle of a vast empty box.)
#[derive(Clone, Debug)]
pub struct GridSettings {
    pub state: GridState,
    /// Size of the room along x, y and z. It is centred on the origin
    /// horizontally and stands on the floor.
    pub width: f32,
    pub height: f32,
    pub depth: f32,
    /// Height of the floor.
    pub floor: f32,
    /// Distance between grid lines.
    pub spacing: f32,
    /// Thickness of each grid line.
    pub thickness: f32,
    pub scale: f32,
    /// 0..=scale; drives the grow/shrink animation.
    pub dynamic_scale: f32,
    /// Line colour (sRGB bytes).
    pub color: [u8; 3],
    /// Background (clear + box face) colour (sRGB bytes).
    pub back_color: [u8; 3],
}

impl Default for GridSettings {
    fn default() -> Self {
        Self {
            state: GridState::Enable,
            width: 600.0,
            height: 200.0,
            depth: 600.0,
            floor: -5.0,
            spacing: 10.0,
            thickness: 0.25,
            scale: 1.0,
            dynamic_scale: 0.0,
            color: [255, 255, 0],
            back_color: [0, 0, 0],
        }
    }
}

/// How far above the floor the viewer's eye starts.
pub const EYE_HEIGHT: f32 = 5.0;
/// The camera never goes lower than this above the floor.
pub const MIN_HEIGHT: f32 = 2.0;
/// How close the camera may get to the walls and ceiling: far enough that
/// the wall still reads as a grid rather than one line filling the view.
const WALL_MARGIN: f32 = 25.0;

/// Colour schemes selectable from the Grid settings dialog.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum GridColorScheme {
    White,
    Black,
}

impl GridSettings {
    pub fn set_spacing(&mut self, thickness: f32, spacing: f32) {
        self.thickness = thickness;
        self.spacing = spacing;
    }

    pub fn ceiling(&self) -> f32 {
        self.floor + self.height
    }

    /// Where the viewer's eye starts: a person's height above the floor.
    pub fn eye_height(&self) -> f32 {
        self.floor + EYE_HEIGHT
    }

    /// Keeps a camera inside the room: off the walls and ceiling, and never
    /// lower than a little above the floor.
    pub fn keep_inside(&self, pos: Vec3) -> Vec3 {
        let half = Vec3::new(self.width, 0.0, self.depth) / 2.0 - WALL_MARGIN;
        Vec3::new(
            pos.x.clamp(-half.x, half.x),
            pos.y.clamp(self.floor + MIN_HEIGHT, self.ceiling() - WALL_MARGIN),
            pos.z.clamp(-half.z, half.z),
        )
    }

    pub fn apply_scheme(&mut self, scheme: GridColorScheme) {
        match scheme {
            GridColorScheme::White => {
                self.back_color = [255, 255, 255];
                self.color = [128, 128, 128];
            }
            GridColorScheme::Black => {
                self.back_color = [0, 0, 0];
                self.color = [255, 255, 0];
            }
        }
    }

    pub fn line_color(&self) -> Color {
        Color::srgb_u8(self.color[0], self.color[1], self.color[2])
    }

    pub fn back_color(&self) -> Color {
        Color::srgb_u8(self.back_color[0], self.back_color[1], self.back_color[2])
    }
}

/// World settings shared by the view, input and UI layers.
#[derive(Resource, Debug)]
pub struct Settings {
    pub system: SystemState,
    /// State to return to when leaving camera control.
    pub old_system_state: SystemState,
    pub inner_grid: bool,
    pub wireframe_mode: bool,
    /// Fly speed in units per second: starts at `min_player_speed` and
    /// ramps up by `accel_speed` per second while moving, to the cap.
    pub min_player_speed: f32,
    pub max_player_speed: f32,
    pub accel_speed: f32,
    pub current_speed: f32,
    pub grid: GridSettings,
}

impl Default for Settings {
    fn default() -> Self {
        Self {
            system: SystemState::Design,
            old_system_state: SystemState::Design,
            inner_grid: false,
            wireframe_mode: false,
            min_player_speed: 15.0,
            max_player_speed: 60.0,
            accel_speed: 30.0,
            current_speed: 0.0,
            grid: GridSettings::default(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn the_room_contains_the_bundled_scenes() {
        let grid = GridSettings::default();
        // Scene objects sit within about +-130 units and bottom out near -5.
        assert!(grid.width / 2.0 > 130.0 && grid.depth / 2.0 > 270.0);
        assert!(grid.floor <= -4.3);
        // Grid cells are about the size of the objects, not ten times bigger.
        assert!(grid.spacing <= 10.0);
    }

    #[test]
    fn cameras_stay_inside_and_above_the_floor() {
        let grid = GridSettings::default();
        assert_eq!(grid.keep_inside(Vec3::new(0.0, -100.0, 0.0)).y, grid.floor + MIN_HEIGHT);
        assert_eq!(grid.keep_inside(Vec3::new(0.0, 1e6, 0.0)).y, grid.ceiling() - WALL_MARGIN);
        let far = grid.keep_inside(Vec3::new(1e6, 0.0, -1e6));
        assert_eq!((far.x, far.z), (300.0 - WALL_MARGIN, -300.0 + WALL_MARGIN));
        let inside = Vec3::new(10.0, 3.0, -20.0);
        assert_eq!(grid.keep_inside(inside), inside);
    }

    #[test]
    fn eye_height_is_a_bit_above_the_floor() {
        let grid = GridSettings::default();
        assert_eq!(grid.eye_height() - grid.floor, EYE_HEIGHT);
        assert_eq!(grid.keep_inside(Vec3::new(0.0, grid.eye_height(), 0.0)).y, grid.eye_height());
    }
}
