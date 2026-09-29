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

/// Look of the holographic grid.
#[derive(Clone, Debug)]
pub struct GridSettings {
    pub state: GridState,
    /// Extents of the grid box along each axis, centred on the origin.
    pub width: i32,
    pub height: i32,
    pub depth: i32,
    /// Distance between grid lines.
    pub spacing: i32,
    /// Thickness of each grid line.
    pub thickness: i32,
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
            width: 1000,
            height: 1000,
            depth: 1000,
            spacing: 100,
            thickness: 3,
            scale: 1.0,
            dynamic_scale: 0.0,
            color: [255, 255, 0],
            back_color: [0, 0, 0],
        }
    }
}

/// Colour schemes selectable from the Grid settings dialog.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum GridColorScheme {
    White,
    Black,
}

impl GridSettings {
    pub fn set_spacing(&mut self, thickness: i32, spacing: i32) {
        self.thickness = thickness;
        self.spacing = spacing;
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
    /// Camera fly speed ramps up by `accel_speed` per second to this cap.
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
            max_player_speed: 15.0,
            accel_speed: 1.0,
            current_speed: 0.0,
            grid: GridSettings::default(),
        }
    }
}
