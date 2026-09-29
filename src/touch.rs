//! Touch support, in two parts.
//!
//! **The pointer.** Everything that used to read the left mouse button
//! (menus, dialog buttons, list boxes, selecting objects, dragging handles)
//! reads [`Pointer`] instead, which is driven by the mouse *or* by a finger:
//! a tap is a click, a drag is a drag, and holding still for a moment is a
//! right-click (opens the context menu).
//!
//! **On-screen controls** (as in DarkMessenger): a floating fly stick that
//! appears under the thumb near the bottom-left corner, drag anywhere else to look, and buttons on the right
//! for the menu and the grid. They show on Android by default and anywhere
//! as soon as the screen is touched, and hide when a key is pressed.

use bevy::prelude::*;
use bevy::ui::{CalculatedClip, FocusPolicy, UiStack};
use bevy::utils::HashMap;
use bevy::window::PrimaryWindow;

use crate::settings::GridState;
use crate::settings::Settings;
use crate::ui3d::UiState;
use crate::ui3d::menu::{self, MenuBar};

/// Holding a finger still this long opens the context menu.
const LONG_PRESS_SECS: f32 = 0.55;
/// Movement (logical px) below which a press still counts as a tap.
pub const TAP_SLOP: f32 = 8.0;

/// Radius of the stick's travel, in logical pixels.
const STICK_RADIUS: f32 = 64.0;
const STICK_KNOB: f32 = 56.0;
/// Where the stick rests when nobody is touching it (from bottom-left).
const STICK_HOME: Vec2 = Vec2::new(40.0 + STICK_RADIUS, 40.0 + STICK_RADIUS);
/// Touches starting within this distance of the stick's resting place drive
/// it. (Kept tight so objects and handles near the corner stay grabbable.)
const STICK_ZONE_RADIUS: f32 = STICK_RADIUS * 2.2;

/// The primary pointer: the mouse, or the finger doing mouse duties.
#[derive(Resource, Default, Debug)]
pub struct Pointer {
    /// Position in logical window pixels (origin top-left).
    pub position: Option<Vec2>,
    pub pressed: bool,
    pub just_pressed: bool,
    pub just_released: bool,
    /// Right-click, or a long press on a touch screen.
    pub secondary: bool,
    /// Distance travelled since the press.
    pub travel: f32,
    /// Movement since last frame.
    pub delta: Vec2,
    /// The current (or last) press is a finger.
    pub touch: bool,
    /// The press became a long press; its release is not a tap.
    pub long_pressed: bool,
    /// The current (or last) press started on the UI, so it is not meant
    /// for the 3D world.
    pub pressed_on_ui: bool,
    pressed_at: f32,
    finger: Option<u64>,
}

impl Pointer {
    /// A press and release without dragging: a click or a tap.
    pub fn tapped(&self) -> bool {
        self.just_released && self.travel <= TAP_SLOP && !self.long_pressed
    }
}

/// Fly input from the stick (x right, y forward) and look input from
/// extra fingers, for this frame.
#[derive(Resource, Default, Debug)]
pub struct TouchInput {
    pub stick: Vec2,
    pub look: Vec2,
}

/// Whether the on-screen controls are showing.
#[derive(Resource)]
pub struct TouchControls {
    pub visible: bool,
}

#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
enum Button {
    Menu,
    Grid,
}

impl Button {
    const ALL: [Button; 2] = [Button::Menu, Button::Grid];

    /// Diameter and centre offset from the bottom-right corner.
    fn layout(self) -> (f32, Vec2) {
        match self {
            Button::Menu => (96.0, Vec2::new(36.0 + 48.0, 40.0 + 48.0)),
            Button::Grid => (68.0, Vec2::new(44.0 + 34.0, 170.0 + 34.0)),
        }
    }

    fn label(self) -> &'static str {
        match self {
            Button::Menu => "MENU",
            Button::Grid => "GRID",
        }
    }

    fn center(self, window: Vec2) -> Vec2 {
        let (_, offset) = self.layout();
        window - offset
    }

    fn hit(self, window: Vec2, point: Vec2) -> bool {
        point.distance(self.center(window)) < self.layout().0 * 0.6
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
enum Role {
    Pointer,
    Stick(Vec2),
    Look,
    Button(Button),
}

pub fn in_stick_zone(window: Vec2, point: Vec2) -> bool {
    point.distance(Vec2::new(STICK_HOME.x, window.y - STICK_HOME.y)) < STICK_ZONE_RADIUS
}

#[derive(Component)]
struct TouchRoot;
#[derive(Component)]
struct StickBase;
#[derive(Component)]
struct StickKnob;
#[derive(Component)]
struct ButtonNode(Button);

pub struct TouchPlugin;

impl Plugin for TouchPlugin {
    fn build(&self, app: &mut App) {
        app.insert_resource(TouchControls { visible: cfg!(any(target_os = "android", target_os = "ios")) })
            .init_resource::<Pointer>()
            .init_resource::<TouchInput>()
            .add_systems(Startup, spawn_ui.after(crate::spawn_main_camera))
            // After bevy_ui has worked out what is under the pointer.
            .add_systems(PreUpdate, (update_pointer, touch_interaction).chain().after(bevy::ui::UiSystem::Focus))
            .add_systems(Update, (detect_input_device, press_buttons, update_ui));
    }
}

/// The UI nodes a hit test needs.
type HitNodes<'w, 's> = Query<
    'w,
    's,
    (&'static ComputedNode, &'static GlobalTransform, &'static ViewVisibility, Option<&'static CalculatedClip>, Option<&'static FocusPolicy>, Has<Interaction>),
>;

/// The front-most interactive node at a logical window position, following
/// bevy_ui's rules (visible area only, `FocusPolicy::Block` stops the search).
fn ui_hit(point: Vec2, scale: f32, stack: &UiStack, nodes: &HitNodes) -> Option<Entity> {
    let point = point * scale;
    for &entity in stack.uinodes.iter().rev() {
        let Ok((node, transform, visible, clip, focus, interactive)) = nodes.get(entity) else { continue };
        if !visible.get() || node.size().cmple(Vec2::ZERO).any() {
            continue;
        }
        let rect = Rect::from_center_size(transform.translation().truncate(), node.size());
        let rect = clip.map_or(rect, |c| rect.intersect(c.clip));
        if !rect.contains(point) {
            continue;
        }
        if interactive {
            return Some(entity);
        }
        if focus == Some(&FocusPolicy::Block) {
            return None;
        }
    }
    None
}

/// bevy_ui hit-tests the mouse cursor in preference to touches, so while a
/// finger is the pointer, redo `Interaction` from the finger's position.
fn touch_interaction(
    pointer: Res<Pointer>,
    windows: Query<&Window, With<PrimaryWindow>>,
    stack: Res<UiStack>,
    nodes: HitNodes,
    mut interactions: Query<(Entity, &mut Interaction)>,
) {
    let (true, Some(position), Ok(window)) = (pointer.touch && (pointer.pressed || pointer.just_released), pointer.position, windows.get_single()) else {
        return;
    };
    let hit = if pointer.pressed { ui_hit(position, window.scale_factor(), &stack, &nodes) } else { None };
    for (entity, mut interaction) in &mut interactions {
        let want = if Some(entity) == hit { Interaction::Pressed } else { Interaction::None };
        interaction.set_if_neq(want);
    }
}

#[allow(clippy::too_many_arguments)]
fn update_pointer(
    time: Res<Time>,
    mouse: Res<ButtonInput<MouseButton>>,
    touches: Res<Touches>,
    windows: Query<&Window, With<PrimaryWindow>>,
    stack: Res<UiStack>,
    nodes: HitNodes,
    ui: Res<UiState>,
    ui_scale: Res<UiScale>,
    mut pointer: ResMut<Pointer>,
    mut input: ResMut<TouchInput>,
    mut roles: Local<HashMap<u64, Role>>,
) {
    let Ok(window) = windows.get_single() else { return };
    // The on-screen controls are laid out in UI units.
    let k = ui_scale.0;
    let size = Vec2::new(window.width(), window.height()) / k;
    let now = time.elapsed_secs();
    let before = pointer.position;
    pointer.just_pressed = false;
    pointer.just_released = false;
    pointer.secondary = false;
    *input = TouchInput::default();

    // Give each new finger a job.
    for touch in touches.iter_just_pressed() {
        let over = ui_hit(touch.position(), window.scale_factor(), &stack, &nodes).is_some();
        let p = touch.position() / k;
        let pointer_busy = roles.values().any(|r| *r == Role::Pointer);
        let role = if over || ui.active {
            if pointer_busy { Role::Look } else { Role::Pointer }
        } else if let Some(b) = Button::ALL.into_iter().find(|b| b.hit(size, p)) {
            Role::Button(b)
        } else if in_stick_zone(size, p) && !roles.values().any(|r| matches!(r, Role::Stick(_))) {
            Role::Stick(p)
        } else if pointer_busy {
            Role::Look
        } else {
            Role::Pointer
        };
        roles.insert(touch.id(), role);
    }

    for touch in touches.iter() {
        match roles.get(&touch.id()) {
            Some(Role::Stick(origin)) => {
                let v = ((touch.position() / k - *origin) / STICK_RADIUS).clamp_length_max(1.0);
                input.stick = Vec2::new(v.x, -v.y);
            }
            Some(Role::Look) => input.look += touch.delta(),
            _ => {}
        }
    }

    // The finger acting as the pointer: the one we were tracking (possibly
    // lifted this frame), else a newly assigned one.
    let tracked = pointer.finger.filter(|id| touches.get_pressed(*id).is_some() || touches.just_released(*id));
    let finger = tracked.or_else(|| roles.iter().find(|(id, r)| **r == Role::Pointer && touches.get_pressed(**id).is_some()).map(|(id, _)| *id));

    match finger {
        Some(id) => {
            let touch = touches.get_pressed(id).or_else(|| touches.get_released(id)).expect("pressed or just released");
            if pointer.finger != Some(id) {
                pointer.finger = Some(id);
                pointer.touch = true;
                pointer.just_pressed = true;
                pointer.pressed_at = now;
                pointer.travel = 0.0;
                pointer.long_pressed = false;
            }
            let down = touches.get_pressed(id).is_some();
            pointer.pressed = down;
            pointer.just_released = !down;
            pointer.position = Some(touch.position());
            if !down {
                pointer.finger = None;
            }
        }
        None => {
            // The mouse.
            pointer.position = window.cursor_position().or(pointer.position);
            pointer.pressed = mouse.pressed(MouseButton::Left);
            pointer.just_pressed = mouse.just_pressed(MouseButton::Left);
            pointer.just_released = mouse.just_released(MouseButton::Left);
            pointer.secondary = mouse.just_pressed(MouseButton::Right);
            if pointer.just_pressed {
                pointer.touch = false;
                pointer.travel = 0.0;
                pointer.long_pressed = false;
                pointer.pressed_at = now;
            }
        }
    }

    pointer.delta = match (pointer.position, before) {
        (Some(a), Some(b)) if !pointer.just_pressed => a - b,
        _ => Vec2::ZERO,
    };
    if pointer.pressed || pointer.just_released {
        pointer.travel += pointer.delta.length();
    }

    if pointer.just_pressed {
        pointer.pressed_on_ui = pointer.position.is_some_and(|p| ui_hit(p, window.scale_factor(), &stack, &nodes).is_some());
    }

    // Long press = right click.
    if pointer.touch && pointer.pressed && !pointer.long_pressed && pointer.travel <= TAP_SLOP && now - pointer.pressed_at >= LONG_PRESS_SECS {
        pointer.long_pressed = true;
        pointer.secondary = true;
    }

    roles.retain(|id, _| touches.get_pressed(*id).is_some());
}

fn press_buttons(
    touches: Res<Touches>,
    controls: Res<TouchControls>,
    ui: Res<UiState>,
    ui_scale: Res<UiScale>,
    windows: Query<&Window, With<PrimaryWindow>>,
    mut settings: ResMut<Settings>,
    mut bars: Query<&mut MenuBar>,
) {
    if !controls.visible || ui.active && !bars.iter().any(MenuBar::is_active) {
        return;
    }
    let Ok(window) = windows.get_single() else { return };
    let size = Vec2::new(window.width(), window.height()) / ui_scale.0;
    for touch in touches.iter_just_pressed() {
        match Button::ALL.into_iter().find(|b| b.hit(size, touch.position() / ui_scale.0)) {
            Some(Button::Menu) => menu::toggle_main_menu(&mut bars),
            Some(Button::Grid) => match settings.grid.state {
                GridState::On => settings.grid.state = GridState::Disable,
                GridState::Off => settings.grid.state = GridState::Enable,
                _ => {}
            },
            None => {}
        }
    }
}

fn circle(size: f32) -> Node {
    Node {
        position_type: PositionType::Absolute,
        width: Val::Px(size),
        height: Val::Px(size),
        border: UiRect::all(Val::Px(2.0)),
        justify_content: JustifyContent::Center,
        align_items: AlignItems::Center,
        ..default()
    }
}

fn spawn_ui(mut commands: Commands) {
    let ring = BorderColor(Color::srgba(1.0, 0.95, 0.4, 0.45));
    let fill = BackgroundColor(Color::srgba(0.25, 0.3, 0.55, 0.25));
    commands
        .spawn((
            TouchRoot,
            Node { position_type: PositionType::Absolute, width: Val::Percent(100.0), height: Val::Percent(100.0), ..default() },
            Visibility::Hidden,
            GlobalZIndex(50),
            PickingBehavior::IGNORE,
        ))
        .with_children(|root| {
            let d = STICK_RADIUS * 2.0;
            root.spawn((
                StickBase,
                Node { left: Val::Px(STICK_HOME.x - STICK_RADIUS), bottom: Val::Px(STICK_HOME.y - STICK_RADIUS), ..circle(d) },
                ring,
                fill,
                BorderRadius::MAX,
            ))
            .with_child((
                StickKnob,
                Node { left: Val::Px(STICK_RADIUS - STICK_KNOB / 2.0 - 2.0), top: Val::Px(STICK_RADIUS - STICK_KNOB / 2.0 - 2.0), ..circle(STICK_KNOB) },
                BorderColor(Color::srgba(1.0, 1.0, 0.8, 0.7)),
                BackgroundColor(Color::srgba(0.45, 0.55, 0.9, 0.45)),
                BorderRadius::MAX,
            ));
            for button in Button::ALL {
                let (size, offset) = button.layout();
                root.spawn((
                    ButtonNode(button),
                    Node { right: Val::Px(offset.x - size / 2.0), bottom: Val::Px(offset.y - size / 2.0), ..circle(size) },
                    ring,
                    fill,
                    BorderRadius::MAX,
                ))
                .with_child((
                    Text::new(button.label()),
                    TextFont { font_size: 16.0, ..default() },
                    TextColor(Color::srgba(1.0, 1.0, 0.85, 0.9)),
                ));
            }
        });
}

/// A key press hides the touch UI; touching shows it.
fn detect_input_device(keys: Res<ButtonInput<KeyCode>>, touches: Res<Touches>, mut controls: ResMut<TouchControls>) {
    let visible = if touches.iter_just_pressed().next().is_some() {
        true
    } else if keys.get_just_pressed().next().is_some() {
        false
    } else {
        controls.visible
    };
    if controls.visible != visible {
        controls.visible = visible;
    }
}

#[allow(clippy::too_many_arguments)]
fn update_ui(
    controls: Res<TouchControls>,
    touches: Res<Touches>,
    ui_scale: Res<UiScale>,
    windows: Query<&Window, With<PrimaryWindow>>,
    mut root: Query<&mut Visibility, With<TouchRoot>>,
    mut base: Query<&mut Node, (With<StickBase>, Without<StickKnob>)>,
    mut knob: Query<&mut Node, (With<StickKnob>, Without<StickBase>)>,
    mut buttons: Query<(&ButtonNode, &mut BackgroundColor)>,
) {
    let show = controls.visible;
    if let Ok(mut v) = root.get_single_mut() {
        let want = if show { Visibility::Inherited } else { Visibility::Hidden };
        if *v != want {
            *v = want;
        }
    }
    if !show {
        return;
    }
    let Ok(window) = windows.get_single() else { return };
    let k = ui_scale.0;
    let size = Vec2::new(window.width(), window.height()) / k;

    let stick = touches
        .iter()
        .filter(|t| in_stick_zone(size, t.start_position() / k) && !Button::ALL.iter().any(|b| b.hit(size, t.start_position() / k)))
        .min_by_key(|t| t.id());
    let (origin, offset) = match stick {
        Some(t) => (t.start_position() / k, ((t.position() - t.start_position()) / k).clamp_length_max(STICK_RADIUS)),
        None => (Vec2::new(STICK_HOME.x, size.y - STICK_HOME.y), Vec2::ZERO),
    };
    if let Ok(mut node) = base.get_single_mut() {
        node.left = Val::Px(origin.x - STICK_RADIUS);
        node.bottom = Val::Px(size.y - origin.y - STICK_RADIUS);
    }
    if let Ok(mut node) = knob.get_single_mut() {
        let corner = STICK_RADIUS - STICK_KNOB / 2.0 - 2.0;
        node.left = Val::Px(corner + offset.x);
        node.top = Val::Px(corner + offset.y);
    }
    for (button, mut bg) in &mut buttons {
        let held = touches.iter().any(|t| button.0.hit(size, t.start_position() / k));
        bg.0 = Color::srgba(0.25, 0.3, 0.55, if held { 0.55 } else { 0.25 });
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn stick_zone_is_the_bottom_left_corner() {
        let w = Vec2::new(1000.0, 600.0);
        assert!(in_stick_zone(w, Vec2::new(100.0, 550.0)));
        assert!(in_stick_zone(w, Vec2::new(200.0, 450.0)));
        assert!(!in_stick_zone(w, Vec2::new(100.0, 100.0)), "top-left belongs to the menu");
        assert!(!in_stick_zone(w, Vec2::new(420.0, 520.0)), "the rest of the screen stays grabbable");
    }

    #[test]
    fn buttons_sit_bottom_right_and_are_forgiving() {
        let w = Vec2::new(1000.0, 600.0);
        let c = Button::Menu.center(w);
        assert!(c.x > 850.0 && c.y > 450.0);
        assert!(Button::Menu.hit(w, c + Vec2::new(50.0, 0.0)));
        assert!(!Button::Menu.hit(w, c + Vec2::new(120.0, 0.0)));
        assert!(!Button::Grid.hit(w, c));
    }

    #[test]
    fn a_tap_is_a_short_still_press() {
        let p = Pointer { just_released: true, travel: 3.0, ..default() };
        assert!(p.tapped());
        assert!(!Pointer { just_released: true, travel: 30.0, ..default() }.tapped());
        assert!(!Pointer { just_released: true, long_pressed: true, ..default() }.tapped());
    }
}
