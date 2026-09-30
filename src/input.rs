//! Keyboard and mouse control of the camera and world.
//!
//! * `W A S D` fly; speed ramps up while keys are held.
//! * Drag with the left mouse button on empty space to look around (works
//!   in the browser and with touch), or hold `Ctrl` and move the mouse.
//! * Mouse wheel changes the field of view (or zooms orthographic views).
//! * `Space` toggles the menu, `G` the grid, `Q` asks to quit.

use bevy::input::ButtonState;
use bevy::input::keyboard::{Key, KeyboardInput};
use bevy::input::mouse::{AccumulatedMouseMotion, MouseScrollUnit, MouseWheel};

use crate::touch::{Pointer, TAP_SLOP, TouchInput};
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

/// A drag that started on empty space (not on the UI or a manipulator
/// handle) turns the camera. Moves under a few pixels still count as a click
/// or tap, so selecting objects is unaffected.
#[allow(clippy::too_many_arguments)]
fn mouse_look(
    motion: Res<AccumulatedMouseMotion>,
    pointer: Res<Pointer>,
    touch: Res<TouchInput>,
    settings: Res<Settings>,
    ui: Res<UiState>,
    manip: Res<Manipulator>,
    keys: Res<ButtonInput<KeyCode>>,
    view: Res<ViewPort>,
    mut armed: Local<bool>,
    mut cameras: ResMut<CameraManager>,
) {
    if pointer.just_pressed {
        *armed = !ui.active && !pointer.pressed_on_ui;
    }
    if !pointer.pressed || manip.is_dragging() {
        *armed = false;
    }

    let mut delta = Vec2::ZERO;
    if looking(&settings, &ui, &manip, &keys) {
        // Locked pointer (desktop) reports raw motion; unlocked (web) moves the cursor.
        delta = if motion.delta != Vec2::ZERO { motion.delta } else { pointer.delta };
    } else if *armed && pointer.travel > TAP_SLOP {
        delta = pointer.delta;
    }
    if !ui.active {
        delta += touch.look;
    }
    if delta != Vec2::ZERO {
        if let Some(rig) = cameras.cameras.get_mut(view.camera) {
            rig.look(delta.x, delta.y);
        }
    }
}

/// Mouse wheel movement in notches. Browsers and touchpads report pixels
/// (about 100 per notch) rather than lines.
pub fn wheel_notches(events: &mut EventReader<MouseWheel>) -> f32 {
    events
        .read()
        .map(|w| match w.unit {
            MouseScrollUnit::Line => w.y,
            MouseScrollUnit::Pixel => w.y / 100.0,
        })
        .sum()
}

fn wheel_zoom(mut wheel: EventReader<MouseWheel>, ui: Res<UiState>, mut view: ResMut<ViewPort>) {
    let notches = wheel_notches(&mut wheel);
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
    /// Analog stick: x right, y forward, length up to 1.
    pub stick: Vec2,
}

impl FlyInput {
    fn any(&self) -> bool {
        self.forward || self.back || self.left || self.right || self.stick.length() > 0.05
    }

    /// Forward/right amounts from keys and stick, each in -1..=1.
    fn axes(&self) -> Vec2 {
        let keys = Vec2::new(f32::from(self.right) - f32::from(self.left), f32::from(self.forward) - f32::from(self.back));
        (keys + self.stick).clamp(Vec2::NEG_ONE, Vec2::ONE)
    }
}

/// Port of the original `HandleKeys`: `mov` is the (unit) view direction.
pub fn fly_step(input: &FlyInput, view: &ViewPort, settings: &mut Settings, mov: Vec3, up: Vec3, dt: f32) -> Vec3 {
    let axes = input.axes();
    let strafe_left = Vec3::new(mov.z, 0.0, -mov.x);
    let mut force = Vec3::ZERO;
    if view.is_perspective() {
        if !input.any() {
            settings.current_speed = 0.0;
            return Vec3::ZERO;
        }
        let start = settings.current_speed.max(settings.min_player_speed);
        settings.current_speed = (start + settings.accel_speed * dt).min(settings.max_player_speed);
        force = mov * axes.y - strafe_left * axes.x;
        // Full speed for keys (normalised) or a fully pushed stick.
        force = force.normalize_or_zero() * axes.length().min(1.0) * settings.current_speed;
    } else {
        let pan = view.zoom_percent(10.0);
        let flat = matches!(view.view_type, ViewType::Top | ViewType::Bottom);
        force += up * pan * axes.y;
        force -= if flat { Vec3::new(0.0, 0.0, -pan) } else { strafe_left * pan } * axes.x;
        // Panning covers the visible width (zoom / 1000 units per pixel) in
        // a couple of seconds.
        force *= 8.0;
    }
    force * dt
}

fn fly_camera(
    time: Res<Time>,
    touch: Res<TouchInput>,
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
        stick: touch.stick,
    };
    let step = fly_step(&input, &view, &mut settings, view.view_direction(rig), rig.up, time.delta_secs());
    rig.pos += step;
    // The free camera lives inside the holodeck. (Orthographic views look
    // at the room from outside, so they are not fenced in.)
    if view.is_perspective() {
        rig.pos = settings.grid.keep_inside(rig.pos);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const FORWARD: FlyInput = FlyInput { forward: true, back: false, left: false, right: false, stick: Vec2::ZERO };
    const NONE: FlyInput = FlyInput { forward: false, back: false, left: false, right: false, stick: Vec2::ZERO };

    #[test]
    fn flying_forward_accelerates_along_the_view_direction() {
        let view = ViewPort::default();
        let mut settings = Settings::default();
        // Moves at a useful speed straight away (not from a standstill).
        let first = fly_step(&FORWARD, &view, &mut settings, Vec3::Z, Vec3::Y, 0.1);
        assert!(first.z >= settings.min_player_speed * 0.1);
        let mut previous = first.z;
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
        let left = FlyInput { forward: false, back: false, left: true, right: false, stick: Vec2::ZERO };
        // Facing +Z, the viewer's left is +X.
        let step = fly_step(&left, &view, &mut settings, Vec3::Z, Vec3::Y, 0.1);
        assert!(step.x > 0.0 && step.z.abs() < 1e-6);
    }

    #[test]
    fn diagonal_movement_is_not_faster_than_straight() {
        let view = ViewPort::default();
        let both = FlyInput { forward: true, back: false, left: true, right: false, stick: Vec2::ZERO };
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
    fn crossing_an_object_takes_a_fraction_of_a_second_and_the_room_several() {
        let view = ViewPort::default();
        let mut settings = Settings::default();
        let mut travelled = 0.0;
        let mut t = 0.0;
        while travelled < 600.0 {
            travelled += fly_step(&FORWARD, &view, &mut settings, Vec3::Z, Vec3::Y, 1.0 / 60.0).z;
            t += 1.0 / 60.0;
        }
        assert!((8.0..20.0).contains(&t), "crossing the 600-unit room took {t}s");
        let mut fresh = Settings::default();
        let first_second: f32 = (0..60).map(|_| fly_step(&FORWARD, &view, &mut fresh, Vec3::Z, Vec3::Y, 1.0 / 60.0).z).sum();
        assert!((15.0..45.0).contains(&first_second), "first second covers {first_second} units");
    }

    #[test]
    fn half_pushed_stick_flies_at_half_speed() {
        let view = ViewPort::default();
        let half = FlyInput { forward: false, back: false, left: false, right: false, stick: Vec2::new(0.0, 0.5) };
        let mut a = Settings::default();
        let mut b = Settings::default();
        let s = fly_step(&half, &view, &mut a, Vec3::Z, Vec3::Y, 0.1);
        let k = fly_step(&FORWARD, &view, &mut b, Vec3::Z, Vec3::Y, 0.1);
        assert!((s.z - k.z * 0.5).abs() < 1e-6 && s.z > 0.0);
    }

    #[test]
    fn orthographic_views_pan_by_the_zoom() {
        let mut view = ViewPort::default();
        let mut settings = Settings::default();
        let mut rig = crate::camera::CameraRig::new("t", Vec3::ZERO);
        view.set_view(ViewType::Front, &mut rig, &mut settings);
        let step = fly_step(&FORWARD, &view, &mut settings, Vec3::NEG_Z, Vec3::Y, 1.0);
        assert!((step - Vec3::new(0.0, 80.0, 0.0)).length() < 1e-4, "{step:?}");
        // 80 units a second, with 128 units visible across the window.
        view.zoom_level = 200.0;
        let bigger = fly_step(&FORWARD, &view, &mut settings, Vec3::NEG_Z, Vec3::Y, 1.0);
        assert!(bigger.y > step.y);
    }
}
