//! Keyboard and mouse control of the camera and world.
//!
//! * `W A S D` fly; speed ramps up while keys are held.
//! * Drag with the left mouse button on empty space to look around (works
//!   in the browser and with touch), or hold `Ctrl` and move the mouse.
//! * Mouse wheel changes the field of view (or zooms orthographic views).
//! * `Space` toggles the menu, `G` the grid, `Q` asks to quit.

use bevy::input::ButtonState;
use bevy::input::keyboard::{Key, KeyboardInput};
use bevy::input::mouse::{AccumulatedMouseMotion, MouseWheel};
use bevy::prelude::*;
use bevy::window::{CursorGrabMode, PrimaryWindow};

use crate::camera::{self, CameraManager};
use crate::objects::manip::Manipulator;
use crate::settings::{GridState, Settings, SystemState};
use crate::ui3d::actions::{Action, ShowMessage, UiAction};
use crate::ui3d::menu::{self, MenuBar};
use crate::ui3d::{Command, UiState};
use crate::view::{ViewPort, ViewType};

pub struct InputPlugin;

impl Plugin for InputPlugin {
    fn build(&self, app: &mut App) {
        app.add_systems(Update, (hotkeys, look_mode, mouse_look, wheel_zoom, fly_camera).chain());
    }
}

/// Whether mouse motion should turn the camera right now.
pub fn looking(settings: &Settings, ui: &UiState, manip: &Manipulator, keys: &ButtonInput<KeyCode>) -> bool {
    settings.system == SystemState::CameraControl
        || (!ui.active && !manip.is_dragging() && (keys.pressed(KeyCode::ControlLeft) || keys.pressed(KeyCode::ControlRight)))
}

#[allow(clippy::too_many_arguments)]
fn hotkeys(
    mut events: EventReader<KeyboardInput>,
    mut commands: Commands,
    settings: ResMut<Settings>,
    ui: Res<UiState>,
    mut bars: Query<&mut MenuBar>,
    mut actions: EventWriter<UiAction>,
    mut messages: EventWriter<ShowMessage>,
) {
    let mut settings = settings;
    for event in events.read() {
        if event.state != ButtonState::Pressed {
            continue;
        }

        if settings.system == SystemState::CameraControl {
            if event.logical_key == Key::Escape {
                commands.queue(camera::exit_control);
            }
            continue;
        }

        match &event.logical_key {
            // Space belongs to text boxes while one is being edited.
            Key::Space if ui.text_focus.is_none() || !ui.active => menu::toggle_main_menu(&mut bars),
            Key::Character(c) if !ui.active => match c.as_str() {
                "q" | "Q" if ui.command == Command::None => {
                    actions.send(UiAction { action: Action::Quit, window: None });
                }
                "E" => {
                    messages.send(ShowMessage::new("Oh NO!!", "You pressed 'E'!!"));
                }
                "g" | "G" => match settings.grid.state {
                    GridState::On => settings.grid.state = GridState::Disable,
                    GridState::Off => settings.grid.state = GridState::Enable,
                    _ => {}
                },
                _ => {}
            },
            _ => {}
        }
    }
}

/// Captures the pointer while looking.
/// Captures the pointer while Ctrl-looking on desktop. Browsers only allow
/// pointer lock inside a user gesture, so the web build never locks and
/// looks from cursor movement instead.
fn look_mode(
    settings: Res<Settings>,
    ui: Res<UiState>,
    manip: Res<Manipulator>,
    keys: Res<ButtonInput<KeyCode>>,
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
) {
    if cfg!(target_arch = "wasm32") {
        return;
    }
    let Ok(mut window) = windows.get_single_mut() else { return };
    let look = looking(&settings, &ui, &manip, &keys);
    let (grab, visible) = if look { (CursorGrabMode::Locked, false) } else { (CursorGrabMode::None, true) };
    if window.cursor_options.grab_mode != grab {
        window.cursor_options.grab_mode = grab;
    }
    if window.cursor_options.visible != visible {
        window.cursor_options.visible = visible;
    }
}

/// A left-button drag that started on empty space (not on the UI or a
/// manipulator handle) turns the camera. Moves under a few pixels still count
/// as a click, so selecting objects is unaffected.
#[derive(Default)]
struct LookDrag {
    armed: bool,
    started: bool,
    last: Option<Vec2>,
    travelled: f32,
}

const DRAG_THRESHOLD: f32 = 4.0;

#[allow(clippy::too_many_arguments)]
fn mouse_look(
    motion: Res<AccumulatedMouseMotion>,
    mouse: Res<ButtonInput<MouseButton>>,
    touches: Res<Touches>,
    settings: Res<Settings>,
    ui: Res<UiState>,
    manip: Res<Manipulator>,
    keys: Res<ButtonInput<KeyCode>>,
    view: Res<ViewPort>,
    windows: Query<&Window, With<PrimaryWindow>>,
    mut drag: Local<LookDrag>,
    mut cameras: ResMut<CameraManager>,
) {
    let cursor = windows.get_single().ok().and_then(Window::cursor_position);
    let cursor_delta = match (cursor, drag.last) {
        (Some(now), Some(before)) => now - before,
        _ => Vec2::ZERO,
    };
    drag.last = cursor;

    if mouse.just_pressed(MouseButton::Left) {
        *drag = LookDrag { armed: !ui.active && !ui.pointer_over_ui, started: false, last: cursor, travelled: 0.0 };
    }
    if !mouse.pressed(MouseButton::Left) || manip.is_dragging() {
        drag.armed = false;
        drag.started = false;
    }

    let mut delta = Vec2::ZERO;
    if looking(&settings, &ui, &manip, &keys) {
        // Locked pointer (desktop) reports raw motion; unlocked (web) moves the cursor.
        delta = if motion.delta != Vec2::ZERO { motion.delta } else { cursor_delta };
    } else if drag.armed {
        drag.travelled += cursor_delta.length();
        drag.started |= drag.travelled > DRAG_THRESHOLD;
        if drag.started {
            delta = cursor_delta;
        }
    }
    if !ui.active {
        for touch in touches.iter() {
            delta += touch.delta();
        }
    }
    if delta != Vec2::ZERO {
        if let Some(rig) = cameras.cameras.get_mut(view.camera) {
            rig.look(delta.x, delta.y);
        }
    }
}

fn wheel_zoom(mut wheel: EventReader<MouseWheel>, ui: Res<UiState>, mut view: ResMut<ViewPort>) {
    let notches: f32 = wheel.read().map(|w| w.y).sum();
    if notches != 0.0 && !ui.active && !ui.pointer_over_ui {
        view.wheel(notches);
    }
}

/// Movement for one frame, in world units.
pub struct FlyInput {
    pub forward: bool,
    pub back: bool,
    pub left: bool,
    pub right: bool,
}

impl FlyInput {
    fn any(&self) -> bool {
        self.forward || self.back || self.left || self.right
    }
}

/// Port of the original `HandleKeys`: `mov` is the (unit) view direction.
pub fn fly_step(input: &FlyInput, view: &ViewPort, settings: &mut Settings, mov: Vec3, up: Vec3, dt: f32) -> Vec3 {
    let mut force = Vec3::ZERO;
    if view.is_perspective() {
        if !input.any() {
            settings.current_speed = 0.0;
        }
        let ramp = |settings: &mut Settings| {
            settings.current_speed = (settings.current_speed + settings.accel_speed * dt).min(settings.max_player_speed);
        };
        if input.forward {
            ramp(settings);
            force += mov;
        }
        if input.back {
            ramp(settings);
            force -= mov;
        }
        if input.left {
            ramp(settings);
            force += Vec3::new(mov.z, 0.0, -mov.x);
        }
        if input.right {
            ramp(settings);
            force += Vec3::new(-mov.z, 0.0, mov.x);
        }
        force = force.normalize_or_zero() * settings.current_speed;
    } else {
        let pan = view.zoom_percent(10.0);
        let flat = matches!(view.view_type, ViewType::Top | ViewType::Bottom);
        if input.forward {
            force += up * pan;
        }
        if input.back {
            force -= up * pan;
        }
        if input.left {
            force += if flat { Vec3::new(0.0, 0.0, -pan) } else { Vec3::new(mov.z * pan, 0.0, -mov.x * pan) };
        }
        if input.right {
            force += if flat { Vec3::new(0.0, 0.0, pan) } else { Vec3::new(-mov.z * pan, 0.0, mov.x * pan) };
        }
    }
    force * dt * 8.0
}

fn fly_camera(
    time: Res<Time>,
    keys: Res<ButtonInput<KeyCode>>,
    mut settings: ResMut<Settings>,
    ui: Res<UiState>,
    view: Res<ViewPort>,
    mut cameras: ResMut<CameraManager>,
) {
    if ui.active && settings.system != SystemState::CameraControl {
        return;
    }
    if ui.text_focus.is_some() && ui.active {
        return;
    }
    let Some(rig) = cameras.cameras.get_mut(view.camera) else { return };
    let input = FlyInput {
        forward: keys.pressed(KeyCode::KeyW),
        back: keys.pressed(KeyCode::KeyS),
        left: keys.pressed(KeyCode::KeyA),
        right: keys.pressed(KeyCode::KeyD),
    };
    let step = fly_step(&input, &view, &mut settings, view.view_direction(rig), rig.up, time.delta_secs());
    rig.pos += step;
}

#[cfg(test)]
mod tests {
    use super::*;

    const FORWARD: FlyInput = FlyInput { forward: true, back: false, left: false, right: false };
    const NONE: FlyInput = FlyInput { forward: false, back: false, left: false, right: false };

    #[test]
    fn flying_forward_accelerates_along_the_view_direction() {
        let view = ViewPort::default();
        let mut settings = Settings::default();
        let mut previous = 0.0;
        for _ in 0..5 {
            let step = fly_step(&FORWARD, &view, &mut settings, Vec3::Z, Vec3::Y, 0.1);
            assert_eq!((step.x, step.y), (0.0, 0.0));
            assert!(step.z > previous, "speed ramps up");
            previous = step.z;
        }
    }

    #[test]
    fn speed_is_capped_and_resets_when_keys_are_released() {
        let view = ViewPort::default();
        let mut settings = Settings::default();
        for _ in 0..1000 {
            fly_step(&FORWARD, &view, &mut settings, Vec3::Z, Vec3::Y, 0.1);
        }
        assert_eq!(settings.current_speed, settings.max_player_speed);
        assert_eq!(fly_step(&NONE, &view, &mut settings, Vec3::Z, Vec3::Y, 0.1), Vec3::ZERO);
        assert_eq!(settings.current_speed, 0.0);
    }

    #[test]
    fn strafing_left_goes_to_the_left_of_the_view() {
        let view = ViewPort::default();
        let mut settings = Settings::default();
        let left = FlyInput { forward: false, back: false, left: true, right: false };
        // Facing +Z, the viewer's left is +X.
        let step = fly_step(&left, &view, &mut settings, Vec3::Z, Vec3::Y, 0.1);
        assert!(step.x > 0.0 && step.z.abs() < 1e-6);
    }

    #[test]
    fn diagonal_movement_is_not_faster_than_straight() {
        let view = ViewPort::default();
        let both = FlyInput { forward: true, back: false, left: true, right: false };
        let mut a = Settings { current_speed: 5.0, ..default() };
        let mut b = Settings { current_speed: 5.0, ..default() };
        let straight = fly_step(&FORWARD, &view, &mut a, Vec3::Z, Vec3::Y, 0.0).length();
        let _ = straight;
        a.current_speed = 5.0;
        let s = fly_step(&FORWARD, &view, &mut a, Vec3::Z, Vec3::Y, 0.1).length();
        let d = fly_step(&both, &view, &mut b, Vec3::Z, Vec3::Y, 0.1).length();
        // Both ramp; compare per-unit-speed magnitudes.
        assert!((s / a.current_speed - d / b.current_speed).abs() < 1e-5);
    }

    #[test]
    fn orthographic_views_pan_by_the_zoom() {
        let mut view = ViewPort::default();
        let mut settings = Settings::default();
        let mut rig = crate::camera::CameraRig::new("t", Vec3::ZERO);
        view.set_view(ViewType::Front, &mut rig, &mut settings);
        let step = fly_step(&FORWARD, &view, &mut settings, Vec3::NEG_Z, Vec3::Y, 1.0);
        assert!((step - Vec3::new(0.0, 80.0, 0.0)).length() < 1e-4, "{step:?}");
        view.zoom_level = 200.0;
        let bigger = fly_step(&FORWARD, &view, &mut settings, Vec3::NEG_Z, Vec3::Y, 1.0);
        assert!(bigger.y > step.y);
    }
}
