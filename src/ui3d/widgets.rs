//! Windows, buttons, text boxes and list boxes.

use bevy::input::ButtonState;
use bevy::input::keyboard::{Key, KeyboardInput};
use bevy::input::mouse::MouseWheel;
use bevy::prelude::*;
use bevy::ui::FocusPolicy;

use super::theme;
use super::actions::{Action, UiAction};
use super::panel::PanelMaterial;
use super::{UiRoot, UiState};
use crate::touch::Pointer;

/// What a window is for. Some kinds only display things and do not stop the
/// world from receiving input.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum WindowKind {
    FileOpen,
    ObjectProp,
    Message,
    Confirm,
    NetConnect,
    AttachPhysics,
    WorldForce,
    /// Picture-in-picture camera view.
    Video,
}

impl WindowKind {
    pub fn blocks_world(self) -> bool {
        !matches!(self, Self::Message | Self::Video)
    }
}

#[derive(Component)]
pub struct Window3d {
    pub kind: WindowKind,
    /// Action for the Enter key.
    pub default_action: Option<Action>,
    /// Action for the Escape key.
    pub cancel_action: Option<Action>,
}

#[derive(Clone, Copy, Debug)]
pub struct WindowDrag {
    window: Entity,
    grab: Vec2,
}

#[derive(Component)]
pub struct TitleBar;

/// Looks up a named text box inside a window.
#[derive(Component, Clone, Copy, PartialEq, Eq, Debug)]
pub struct Field(pub &'static str);

/// A single-line text input.
#[derive(Component, Default, Debug)]
pub struct TextBox {
    pub text: String,
    /// Cursor position in characters.
    pub cursor: usize,
    pub read_only: bool,
    /// Only digits, '.' and '-' are accepted.
    pub digits_only: bool,
    pub password: bool,
    /// Drawn with a frame and fill even though it is read-only (the file
    /// dialog's path box was an ordinary text box in the original).
    pub framed: bool,
    /// Tab order within the window.
    pub order: usize,
}

impl TextBox {
    pub fn set_text(&mut self, text: impl Into<String>) {
        self.text = text.into();
        self.cursor = self.text.chars().count();
    }

    fn byte_index(&self, chars: usize) -> usize {
        self.text.char_indices().nth(chars).map_or(self.text.len(), |(i, _)| i)
    }

    /// Types a character, honouring the digits-only restriction.
    pub fn insert(&mut self, c: char) {
        if self.read_only || c.is_control() || (self.digits_only && !(c.is_ascii_digit() || c == '.' || c == '-')) {
            return;
        }
        let at = self.byte_index(self.cursor);
        self.text.insert(at, c);
        self.cursor += 1;
    }

    pub fn backspace(&mut self) {
        if self.read_only || self.cursor == 0 {
            return;
        }
        let at = self.byte_index(self.cursor - 1);
        self.text.remove(at);
        self.cursor -= 1;
    }

    pub fn delete(&mut self) {
        if self.read_only || self.cursor >= self.text.chars().count() {
            return;
        }
        let at = self.byte_index(self.cursor);
        self.text.remove(at);
    }

    pub fn move_cursor(&mut self, by: isize) {
        let len = self.text.chars().count() as isize;
        self.cursor = (self.cursor as isize + by).clamp(0, len) as usize;
    }

    /// The text as shown, with a cursor if focused.
    pub fn display(&self, focused: bool) -> String {
        let shown: Vec<char> = if self.password { self.text.chars().map(|_| '*').collect() } else { self.text.chars().collect() };
        let mut s: String = shown.iter().collect();
        if focused && !self.read_only {
            let at = shown.iter().take(self.cursor).map(|c| c.len_utf8()).sum();
            s.insert(at, '|');
        }
        s
    }

    pub fn parse_f32(&self) -> f32 {
        self.text.trim().parse().unwrap_or(0.0)
    }

    pub fn parse_f64(&self) -> f64 {
        self.text.trim().parse().unwrap_or(0.0)
    }
}

#[derive(Component)]
pub struct TextBoxLabel;

#[derive(Component)]
pub struct UiButton {
    pub action: Action,
    armed: bool,
}

/// A scrolling list of strings with a single selection.
#[derive(Component, Default, Debug)]
pub struct ListBox {
    pub items: Vec<String>,
    pub selected: Option<usize>,
    pub offset: usize,
    pub rows: usize,
    /// Height of each row in UI units.
    pub row_height: f32,
    /// Double-click does nothing (used for the applied-forces list).
    pub click_lock: bool,
    /// Action fired by a double click.
    pub activate: Option<Action>,
    last_click: Option<(usize, f32)>,
}

impl ListBox {
    pub fn new(rows: usize) -> Self {
        Self { rows, row_height: theme::ROW_HEIGHT, ..default() }
    }

    /// As many rows as fit in `height`, finger-sized on touch screens.
    pub fn fitting(height: f32, touch: bool) -> Self {
        let row_height = if touch { theme::TOUCH_ROW_HEIGHT } else { theme::ROW_HEIGHT };
        Self { rows: ((height - 4.0) / row_height).floor().max(1.0) as usize, row_height, ..default() }
    }

    pub fn selected_item(&self) -> Option<&str> {
        self.items.get(self.selected?).map(String::as_str)
    }

    pub fn set_items(&mut self, items: Vec<String>) {
        self.items = items;
        self.selected = None;
        self.offset = 0;
    }

    pub fn scroll(&mut self, notches: i32) {
        let max = self.items.len().saturating_sub(self.rows.saturating_sub(1));
        self.offset = (self.offset as i32 + notches).clamp(0, max as i32) as usize;
    }

    fn click(&mut self, row: usize, now: f32) -> bool {
        let index = self.offset + row;
        if index >= self.items.len() {
            return false;
        }
        // Two clicks on the same row within 1.5 seconds activate it.
        let double = matches!(self.last_click, Some((i, t)) if i == index && now - t <= 1.5);
        self.selected = Some(index);
        self.last_click = if double { None } else { Some((index, now)) };
        double
    }
}

#[derive(Component)]
struct ListRow(usize);

/// Fired when a list row is double-clicked.
#[derive(Event)]
pub struct ListActivated {
    pub list: Entity,
}

pub struct WidgetsPlugin;

impl Plugin for WidgetsPlugin {
    fn build(&self, app: &mut App) {
        app.add_event::<ListActivated>()
            .add_plugins(UiMaterialPlugin::<PanelMaterial>::default())
            .add_systems(PostUpdate, sync_panels)
            .add_systems(
            Update,
            (
                focus_windows,
                drag_windows,
                button_clicks,
                window_focus_colors,
                text_box_focus,
                keyboard_input,
                list_clicks,
                list_scroll,
                refresh_text_boxes,
                refresh_lists,
                refit_windows,
            )
                .chain(),
        );
    }
}

pub struct WindowSpec<'a> {
    pub title: &'a str,
    pub size: Vec2,
    pub pos: Vec2,
    pub kind: WindowKind,
    pub default_action: Option<Action>,
    pub cancel_action: Option<Action>,
}

/// The desired look of a node; turned into a [`PanelMaterial`] by
/// [`sync_panels`] (and updated in place when it changes).
#[derive(Component, Clone, Debug)]
pub struct Panel(pub PanelMaterial);

/// Size of a window's cut corners, as in the original:
/// `curveRad = (width / 10 + height / 10) / 2`.
pub fn curve_radius(size: Vec2) -> f32 {
    // Integer arithmetic, as in the C++.
    (((size.x / 10.0).floor() + (size.y / 10.0).floor()) / 2.0).floor()
}

#[derive(Component)]
pub struct WindowCaption;

/// The UI font at the original's size, with lines no taller than the font
/// (the original placed text on 16-18px rows).
pub fn ui_font() -> TextFont {
    TextFont { font_size: theme::FONT_SIZE, ..default() }
}

fn window_panel(size: Vec2, focused: bool) -> PanelMaterial {
    let curve = curve_radius(size);
    PanelMaterial::flat(if focused { theme::WINDOW_FOCUSED } else { theme::WINDOW })
        .rounded([0.0, 0.0, curve, curve])
        .bordered(theme::WINDOW_OUTLINE, 1.0)
}

/// Spawns a window and returns `(window, content)`. Children of `content`
/// are positioned with absolute window-relative pixel coordinates.
///
/// Drawn like the original `Window3d`: a flat light-grey body (brighter
/// when focused) with square top corners, rounded-off bottom corners and a
/// one-pixel shadow outline; the caption sits in a white-to-grey tab inset
/// from the top edge; a magenta close button at the top right.
pub fn spawn_window(commands: &mut Commands, camera: Entity, ui: &mut UiState, spec: WindowSpec) -> (Entity, Entity) {
    ui.next_z += 1;
    let z = 100 + ui.next_z;
    let menu_tab = if ui.touch_mode { Vec2::new(theme::TOUCH_MENU_TAB_WIDTH, theme::TOUCH_MENU_HEIGHT) } else { Vec2::new(theme::MENU_TAB_WIDTH, theme::MENU_HEIGHT) };
    let pos = fit_on_screen(spec.pos, spec.size, ui.screen, menu_tab);
    let curve = curve_radius(spec.size);
    let window = commands
        .spawn((
            Window3d { kind: spec.kind, default_action: spec.default_action, cancel_action: spec.cancel_action },
            UiRoot,
            Node {
                position_type: PositionType::Absolute,
                left: Val::Px(pos.x),
                top: Val::Px(pos.y),
                width: Val::Px(spec.size.x),
                height: Val::Px(spec.size.y),
                ..default()
            },
            Panel(window_panel(spec.size, true)),
            Interaction::default(),
            FocusPolicy::Block,
            GlobalZIndex(z),
            TargetCamera(camera),
        ))
        .id();
    ui.focused_window = Some(window);

    let content = commands.spawn(Node { position_type: PositionType::Absolute, width: Val::Percent(100.0), height: Val::Percent(100.0), ..default() }).id();
    // The drag area is the top strip; the visible tab spans from one corner
    // curve to the other, one pixel down, 19 pixels tall.
    let title_bar = commands
        .spawn((
            TitleBar,
            Interaction::default(),
            Node { width: Val::Percent(100.0), height: Val::Px(theme::TITLE_HEIGHT), position_type: PositionType::Absolute, ..default() },
        ))
        .with_children(|bar| {
            bar.spawn((
                Node {
                    position_type: PositionType::Absolute,
                    left: Val::Px(curve),
                    top: Val::Px(1.0),
                    width: Val::Px((spec.size.x - 2.0 * curve).max(0.0)),
                    height: Val::Px(19.0),
                    justify_content: JustifyContent::Center,
                    align_items: AlignItems::Center,
                    ..default()
                },
                Panel(PanelMaterial::gradient(theme::TITLE_TOP, theme::TITLE_BOTTOM).rounded([0.0, 0.0, 19.0, 19.0])),
            ))
            .with_children(|tab| {
                tab.spawn((WindowCaption, Text::new(spec.title), ui_font(), TextColor(theme::TEXT), TextLayout::new_with_no_wrap()));
            });
        })
        .id();
    let close = spawn_close_button(commands, spec.size);
    commands.entity(window).add_children(&[content, title_bar, close]);
    (window, content)
}

/// The original close button: `curveRad * 2 / 3` wide, 20 tall, at
/// `width - 30`, dark magenta to magenta with a red X.
fn spawn_close_button(commands: &mut Commands, window: Vec2) -> Entity {
    let width = (curve_radius(window) * 2.0 / 3.0).floor().max(12.0);
    commands
        .spawn((
            UiButton { action: Action::CloseWindow, armed: false },
            Interaction::default(),
            FocusPolicy::Block,
            Node {
                position_type: PositionType::Absolute,
                left: Val::Px(window.x - 30.0),
                top: Val::Px(0.0),
                width: Val::Px(width),
                height: Val::Px(20.0),
                justify_content: JustifyContent::Center,
                align_items: AlignItems::Center,
                ..default()
            },
            Panel(PanelMaterial::gradient(theme::CLOSE_TOP, theme::CLOSE_BOTTOM)),
        ))
        .with_children(|b| {
            b.spawn((Text::new("X"), ui_font(), TextColor(theme::CLOSE_TEXT)));
        })
        .id()
}

/// Moves a window so as much of it as possible is on screen (the title bar
/// always is, so it can be dragged), and out from under the menu tab in the
/// top-left corner.
pub fn fit_on_screen(pos: Vec2, size: Vec2, screen: Vec2, menu_tab: Vec2) -> Vec2 {
    if screen.cmple(Vec2::ZERO).any() {
        return pos;
    }
    let mut pos = pos.min(screen - size).max(Vec2::ZERO);
    if pos.y < menu_tab.y && pos.x < menu_tab.x {
        pos.x = menu_tab.x.min((screen.x - size.x).max(0.0));
    }
    pos
}

/// Message text: dark, one line per 16 pixels (the original's
/// Message/Confirm text starts at `(10, height / 2)`).
pub fn spawn_label(commands: &mut Commands, parent: Entity, text: &str, pos: Vec2) -> Entity {
    let label = commands
        .spawn(Node { position_type: PositionType::Absolute, left: Val::Px(pos.x), top: Val::Px(pos.y), flex_direction: FlexDirection::Column, ..default() })
        .with_children(|lines| {
            for line in text.lines() {
                lines.spawn((
                    Text::new(line),
                    ui_font(),
                    TextColor(theme::TEXT),
                    TextLayout::new_with_no_wrap(),
                    Node { height: Val::Px(16.0), ..default() },
                ));
            }
        })
        .id();
    commands.entity(parent).add_child(label);
    label
}

/// Spawns a button positioned in its parent (parent set by the caller).
///
/// The original `OK`-type button: a narrow block (1/7 of the width), a
/// 5-pixel gap and the main block with the caption, both graded from light
/// to darker grey. No border, no hover highlight. Captions longer than the
/// block simply overflow, as they did.
pub fn spawn_button(commands: &mut Commands, caption: &str, action: Action, size: Vec2, pos: Vec2) -> Entity {
    let small = (size.x / 7.0).floor();
    let gap = 5.0;
    let main = size.x - small - gap;
    commands
        .spawn((
            UiButton { action, armed: false },
            Interaction::default(),
            FocusPolicy::Block,
            Node { position_type: PositionType::Absolute, left: Val::Px(pos.x), top: Val::Px(pos.y), width: Val::Px(size.x), height: Val::Px(size.y), ..default() },
        ))
        .with_children(|b| {
            let block = PanelMaterial::gradient(theme::BUTTON_TOP, theme::BUTTON_BOTTOM);
            b.spawn((
                Node { position_type: PositionType::Absolute, left: Val::Px(0.0), width: Val::Px(small), height: Val::Percent(100.0), ..default() },
                Panel(block.clone()),
            ));
            b.spawn((
                Node {
                    position_type: PositionType::Absolute,
                    left: Val::Px(small + gap),
                    width: Val::Px(main),
                    height: Val::Percent(100.0),
                    justify_content: JustifyContent::Center,
                    align_items: AlignItems::Center,
                    overflow: Overflow::visible(),
                    ..default()
                },
                Panel(block),
            ))
            .with_children(|m| {
                m.spawn((UiButtonCaption, Text::new(caption), ui_font(), TextColor(theme::TEXT), TextLayout::new_with_no_wrap()));
            });
        })
        .id()
}

/// The caption of a button (so it can be changed after spawning).
#[derive(Component)]
pub struct UiButtonCaption;

pub fn add_button(commands: &mut Commands, parent: Entity, caption: &str, action: Action, size: Vec2, pos: Vec2) -> Entity {
    let button = spawn_button(commands, caption, action, size, pos);
    commands.entity(parent).add_child(button);
    button
}

/// The red bar cursor of a focused text box.
#[derive(Component)]
struct TextCursor;

/// The original `TextBox3d`. Editable boxes have a two-pixel frame (blue
/// when focused, grey otherwise) around a grey fill (lighter when focused);
/// read-only ones are bare text. Text is blue either way.
pub fn spawn_text_box(commands: &mut Commands, parent: Entity, field: Option<Field>, text_box: TextBox, size: Vec2, pos: Vec2) -> Entity {
    let boxed = !text_box.read_only || text_box.framed;
    let mut entity = commands.spawn((
        text_box,
        Interaction::default(),
        Node {
            position_type: PositionType::Absolute,
            // The frame is drawn two pixels outside the box, as in the original.
            left: Val::Px(pos.x - if boxed { 2.0 } else { 0.0 }),
            top: Val::Px(pos.y - if boxed { 2.0 } else { 0.0 }),
            width: Val::Px(size.x + if boxed { 4.0 } else { 0.0 }),
            height: Val::Px(size.y + if boxed { 4.0 } else { 0.0 }),
            align_items: AlignItems::Center,
            padding: UiRect::left(Val::Px(4.0)),
            border: UiRect::all(Val::Px(if boxed { 2.0 } else { 0.0 })),
            overflow: Overflow::clip(),
            ..default()
        },
        BackgroundColor(if boxed { theme::FIELD_FILL } else { Color::NONE }),
        BorderColor(theme::FIELD_FRAME),
    ));
    if let Some(field) = field {
        entity.insert(field);
    }
    let id = entity
        .with_children(|b| {
            b.spawn((TextBoxLabel, Text::new(""), ui_font(), TextColor(theme::FIELD_TEXT), TextLayout::new_with_no_wrap()));
            b.spawn((
                TextCursor,
                Node { position_type: PositionType::Absolute, left: Val::Px(3.0), top: Val::Px(2.0), width: Val::Px(2.0), height: Val::Px((size.y - 4.0).max(2.0)), ..default() },
                BackgroundColor(theme::FIELD_CURSOR),
                Visibility::Hidden,
            ));
        })
        .id();
    commands.entity(parent).add_child(id);
    id
}

/// A read-only text box (the original's labels were read-only text boxes).
pub fn add_read_only(commands: &mut Commands, parent: Entity, text: &str, size: Vec2, pos: Vec2) -> Entity {
    let mut tb = TextBox { read_only: true, ..default() };
    tb.set_text(text);
    spawn_text_box(commands, parent, None, tb, size, pos)
}

#[derive(Component)]
struct ListThumb;

/// The original `ListBox3d`: a dark two-pixel frame around a light grey
/// list, a cyan band on the selected row, folders in blue, and a scroll
/// strip on the right (up button, track with a dark thumb, down button).
pub fn spawn_list_box(commands: &mut Commands, parent: Entity, list: ListBox, size: Vec2, pos: Vec2) -> Entity {
    let (rows, row_height) = (list.rows, list.row_height);
    let id = commands
        .spawn((
            list,
            Node {
                position_type: PositionType::Absolute,
                left: Val::Px(pos.x - 2.0),
                top: Val::Px(pos.y - 2.0),
                width: Val::Px(size.x + 4.0),
                height: Val::Px(size.y + 4.0),
                flex_direction: FlexDirection::Column,
                overflow: Overflow::clip(),
                border: UiRect::all(Val::Px(2.0)),
                ..default()
            },
            BackgroundColor(theme::LIST),
            BorderColor(theme::LIST_FRAME),
        ))
        .with_children(|b| {
            for row in 0..rows {
                b.spawn((
                    ListRow(row),
                    Interaction::default(),
                    Node {
                        height: Val::Px(row_height),
                        width: Val::Px(size.x - 20.0),
                        padding: UiRect::left(Val::Px(5.0)),
                        align_items: AlignItems::Center,
                        ..default()
                    },
                    BackgroundColor(Color::NONE),
                ))
                .with_children(|r| {
                    r.spawn((Text::new(""), ui_font(), TextColor(theme::TEXT), TextLayout::new_with_no_wrap()));
                });
            }
            // Scroll strip.
            let strip = |top: f32, height: f32| Node {
                position_type: PositionType::Absolute,
                left: Val::Px(size.x - 20.0),
                top: Val::Px(top),
                width: Val::Px(20.0),
                height: Val::Px(height),
                justify_content: JustifyContent::Center,
                align_items: AlignItems::Center,
                ..default()
            };
            b.spawn((strip(20.0, (size.y - 40.0).max(0.0)), BackgroundColor(theme::SCROLL_TRACK))).with_children(|t| {
                t.spawn((
                    ListThumb,
                    Node { position_type: PositionType::Absolute, left: Val::Px(2.0), top: Val::Px(0.0), width: Val::Px(16.0), height: Val::Px(4.0), ..default() },
                    BackgroundColor(theme::SCROLL_THUMB),
                ));
            });
            for (up, top) in [(true, 0.0), (false, size.y - 20.0)] {
                let (a, z) = if up { (theme::SCROLL_DARK, theme::SCROLL_LIGHT) } else { (theme::SCROLL_LIGHT, theme::SCROLL_DARK) };
                b.spawn((
                    UiButton { action: Action::ScrollList(if up { -1 } else { 1 }), armed: false },
                    Interaction::default(),
                    FocusPolicy::Block,
                    strip(top, 20.0),
                    Panel(PanelMaterial::gradient(a, z)),
                ))
                .with_children(|arrow| {
                    // A 10x10 square with two corners cut away is a triangle.
                    let corners = if up { [5.0, 5.0, 0.0, 0.0] } else { [0.0, 0.0, 5.0, 5.0] };
                    arrow.spawn((
                        Node { width: Val::Px(10.0), height: Val::Px(10.0), ..default() },
                        Panel(PanelMaterial::flat(theme::SCROLL_THUMB).chamfered(corners)),
                    ));
                });
            }
        })
        .id();
    commands.entity(parent).add_child(id);
    id
}

/// The window containing `entity`.
pub fn owning_window(mut entity: Entity, parents: &Query<&Parent>, windows: &Query<&Window3d>) -> Option<Entity> {
    loop {
        if windows.contains(entity) {
            return Some(entity);
        }
        entity = parents.get(entity).ok()?.get();
    }
}

/// Raises a window when any part of it is pressed.
fn focus_windows(mut ui: ResMut<UiState>, windows: Query<(Entity, &Interaction), (With<Window3d>, Changed<Interaction>)>, mut z: Query<&mut GlobalZIndex, With<Window3d>>) {
    for (entity, interaction) in &windows {
        if *interaction == Interaction::Pressed && ui.focused_window != Some(entity) {
            ui.focused_window = Some(entity);
            ui.next_z += 1;
            if let Ok(mut index) = z.get_mut(entity) {
                index.0 = 100 + ui.next_z;
            }
        }
    }
}

fn drag_windows(
    mut ui: ResMut<UiState>,
    pointer: Res<Pointer>,
    scale: Res<UiScale>,
    title_bars: Query<(&Interaction, &Parent), With<TitleBar>>,
    mut nodes: Query<&mut Node, With<Window3d>>,
) {
    // Window positions are in UI units.
    let Some(position) = pointer.position.map(|p| p / scale.0) else { return };
    if !pointer.pressed {
        ui.drag = None;
    }
    if ui.drag.is_none() {
        for (interaction, parent) in &title_bars {
            if *interaction == Interaction::Pressed {
                if let Ok(node) = nodes.get(parent.get()) {
                    let (Val::Px(x), Val::Px(y)) = (node.left, node.top) else { continue };
                    ui.drag = Some(WindowDrag { window: parent.get(), grab: position - Vec2::new(x, y) });
                }
            }
        }
    }
    if let Some(drag) = ui.drag {
        if let Ok(mut node) = nodes.get_mut(drag.window) {
            let pos = position - drag.grab;
            node.left = Val::Px(pos.x);
            node.top = Val::Px(pos.y.max(0.0));
        }
    }
}

/// Buttons fire when the pointer is released after pressing them.
fn button_clicks(
    pointer: Res<Pointer>,
    mut buttons: Query<(Entity, &Interaction, &mut UiButton)>,
    mut actions: EventWriter<UiAction>,
    parents: Query<&Parent>,
    windows: Query<&Window3d>,
    mut fired: Local<Vec<(Entity, Action)>>,
) {
    fired.clear();
    for (entity, interaction, mut button) in &mut buttons {
        if *interaction == Interaction::Pressed && pointer.pressed {
            button.armed = true;
        }
        if button.armed && pointer.just_released && !pointer.long_pressed {
            // A finger lifting reports no hover, so trust the press; with a
            // mouse, releasing off the button cancels.
            if pointer.touch || *interaction != Interaction::None {
                fired.push((entity, button.action.clone()));
            }
            button.armed = false;
        } else if !pointer.pressed {
            button.armed = false;
        }
    }
    for (entity, action) in fired.drain(..) {
        actions.send(UiAction { action, window: owning_window(entity, &parents, &windows) });
    }
}

/// Keeps windows on screen when the screen changes size (window resize,
/// phone rotation, touch mode changing the UI scale).
fn refit_windows(ui: Res<UiState>, mut last: Local<Vec2>, mut windows: Query<&mut Node, With<Window3d>>) {
    if *last == ui.screen {
        return;
    }
    *last = ui.screen;
    let menu_tab = if ui.touch_mode { Vec2::new(theme::TOUCH_MENU_TAB_WIDTH, theme::TOUCH_MENU_HEIGHT) } else { Vec2::new(theme::MENU_TAB_WIDTH, theme::MENU_HEIGHT) };
    for mut node in &mut windows {
        let (Val::Px(x), Val::Px(y), Val::Px(w), Val::Px(h)) = (node.left, node.top, node.width, node.height) else { continue };
        let fitted = fit_on_screen(Vec2::new(x, y), Vec2::new(w, h), ui.screen, menu_tab);
        if fitted != Vec2::new(x, y) {
            node.left = Val::Px(fitted.x);
            node.top = Val::Px(fitted.y);
        }
    }
}

/// Creates or updates the material for every [`Panel`].
pub fn sync_panels(
    mut commands: Commands,
    mut materials: ResMut<Assets<PanelMaterial>>,
    added: Query<(Entity, &Panel), Added<Panel>>,
    changed: Query<(&Panel, &MaterialNode<PanelMaterial>), Changed<Panel>>,
) {
    for (entity, panel) in &added {
        commands.entity(entity).insert(MaterialNode(materials.add(panel.0.clone())));
    }
    for (panel, node) in &changed {
        if let Some(material) = materials.get_mut(&node.0) {
            *material = panel.0.clone();
        }
    }
}

/// The focused window is drawn brighter.
fn window_focus_colors(ui: Res<UiState>, mut windows: Query<(Entity, &Node, &mut Panel), With<Window3d>>) {
    for (entity, node, mut panel) in &mut windows {
        let (Val::Px(w), Val::Px(h)) = (node.width, node.height) else { continue };
        let wanted = window_panel(Vec2::new(w, h), ui.focused_window == Some(entity));
        if panel.0.top != wanted.top {
            panel.0 = wanted;
        }
    }
}

fn text_box_focus(mut ui: ResMut<UiState>, boxes: Query<(Entity, &Interaction, &TextBox), Changed<Interaction>>) {
    for (entity, interaction, text_box) in &boxes {
        if *interaction == Interaction::Pressed && !text_box.read_only {
            ui.text_focus = Some(entity);
        }
    }
}

/// Typing, tabbing, Enter and Escape for the focused window.
#[allow(clippy::too_many_arguments)]
fn keyboard_input(
    mut events: EventReader<KeyboardInput>,
    keys: Res<ButtonInput<KeyCode>>,
    mut ui: ResMut<UiState>,
    mut boxes: Query<(Entity, &mut TextBox)>,
    parents: Query<&Parent>,
    windows: Query<&Window3d>,
    mut actions: EventWriter<UiAction>,
) {
    let Some(window) = ui.focused_window.filter(|w| windows.contains(*w)) else {
        events.clear();
        return;
    };
    let shift = keys.pressed(KeyCode::ShiftLeft) || keys.pressed(KeyCode::ShiftRight);

    for event in events.read() {
        if event.state != ButtonState::Pressed {
            continue;
        }
        let focused = ui.text_focus;
        match &event.logical_key {
            Key::Enter => {
                if let Ok(w) = windows.get(window) {
                    if let Some(action) = w.default_action.clone() {
                        actions.send(UiAction { action, window: Some(window) });
                    }
                }
            }
            Key::Escape => {
                if let Ok(w) = windows.get(window) {
                    if let Some(action) = w.cancel_action.clone() {
                        actions.send(UiAction { action, window: Some(window) });
                    }
                }
            }
            Key::Tab | Key::ArrowUp | Key::ArrowDown => {
                let backwards = matches!(event.logical_key, Key::ArrowUp) || (matches!(event.logical_key, Key::Tab) && shift);
                let mut order: Vec<(usize, Entity)> = boxes
                    .iter()
                    .filter(|(e, t)| !t.read_only && owning_window(*e, &parents, &windows) == Some(window))
                    .map(|(e, t)| (t.order, e))
                    .collect();
                order.sort_unstable();
                if !order.is_empty() {
                    let current = order.iter().position(|(_, e)| Some(*e) == focused);
                    let next = match (current, backwards) {
                        (None, _) => 0,
                        (Some(i), false) => (i + 1) % order.len(),
                        (Some(i), true) => (i + order.len() - 1) % order.len(),
                    };
                    ui.text_focus = Some(order[next].1);
                }
            }
            key => {
                let Some(mut text_box) = focused.and_then(|f| boxes.get_mut(f).ok()).map(|(_, t)| t) else { continue };
                match key {
                    Key::Backspace => text_box.backspace(),
                    Key::Delete => text_box.delete(),
                    Key::ArrowLeft => text_box.move_cursor(-1),
                    Key::ArrowRight => text_box.move_cursor(1),
                    Key::Space => text_box.insert(' '),
                    Key::Character(s) => s.chars().for_each(|c| text_box.insert(c)),
                    _ => {}
                }
            }
        }
    }
}

fn refresh_text_boxes(
    ui: Res<UiState>,
    boxes: Query<(Entity, &TextBox, &Children)>,
    mut labels: Query<&mut Text, With<TextBoxLabel>>,
    mut cursors: Query<(&mut Node, &mut Visibility), With<TextCursor>>,
    mut frames: Query<(&mut BorderColor, &mut BackgroundColor)>,
) {
    for (entity, text_box, children) in &boxes {
        let focused = ui.text_focus == Some(entity);
        let editing = focused && !text_box.read_only;
        for &child in children {
            if let Ok(mut label) = labels.get_mut(child) {
                let shown = text_box.display(false);
                if label.0 != shown {
                    label.0 = shown;
                }
            }
            if let Ok((mut node, mut visibility)) = cursors.get_mut(child) {
                // The original's red bar: one character cell per position.
                node.left = Val::Px(3.0 + text_box.cursor as f32 * theme::CHAR_WIDTH);
                *visibility = if editing { Visibility::Inherited } else { Visibility::Hidden };
            }
        }
        if (!text_box.read_only || text_box.framed) && let Ok((mut frame, mut fill)) = frames.get_mut(entity) {
            let (f, b) = if editing { (theme::FIELD_FRAME_FOCUSED, theme::FIELD_FILL_FOCUSED) } else { (theme::FIELD_FRAME, theme::FIELD_FILL) };
            if frame.0 != f {
                frame.0 = f;
            }
            if fill.0 != b {
                fill.0 = b;
            }
        }
    }
}

fn list_clicks(
    time: Res<Time>,
    pointer: Res<Pointer>,
    rows: Query<(&ListRow, &Interaction, &Parent)>,
    mut lists: Query<&mut ListBox>,
    mut activated: EventWriter<ListActivated>,
) {
    if !pointer.just_pressed {
        return;
    }
    for (row, interaction, parent) in &rows {
        if *interaction == Interaction::Pressed {
            if let Ok(mut list) = lists.get_mut(parent.get()) {
                if list.click(row.0, time.elapsed_secs()) && !list.click_lock {
                    activated.send(ListActivated { list: parent.get() });
                }
            }
        }
    }
}

/// The wheel scrolls the first list in the window under the pointer.
fn list_scroll(
    mut wheel: EventReader<MouseWheel>,
    windows: Query<(Entity, &Interaction), With<Window3d>>,
    parents: Query<&Parent>,
    window_query: Query<&Window3d>,
    mut lists: Query<(Entity, &mut ListBox)>,
) {
    let notches = crate::input::wheel_notches(&mut wheel);
    if notches == 0.0 {
        return;
    }
    let Some(target) = windows.iter().find(|(_, i)| **i != Interaction::None).map(|(e, _)| e) else { return };
    if let Some((_, mut list)) = lists.iter_mut().find(|(e, _)| owning_window(*e, &parents, &window_query) == Some(target)) {
        list.scroll(-(notches.signum() as i32));
    }
}

fn refresh_lists(
    lists: Query<(&ListBox, &Children), Changed<ListBox>>,
    rows: Query<(&ListRow, &Children)>,
    mut texts: Query<(&mut Text, &mut TextColor)>,
    mut backgrounds: Query<&mut BackgroundColor>,
    row_entities: Query<Entity, With<ListRow>>,
    tracks: Query<(&Children, &ComputedNode)>,
    mut thumbs: Query<&mut Node, With<ListThumb>>,
) {
    for (list, children) in &lists {
        // The thumb marks how far down the list is scrolled.
        let max = list.items.len().saturating_sub(list.rows.saturating_sub(1)).max(1) as f32;
        let fraction = (list.offset as f32 / max).clamp(0.0, 1.0);
        for &child in children {
            if let Ok((track_children, track)) = tracks.get(child) {
                let height = track.size().y * track.inverse_scale_factor();
                for &t in track_children {
                    if let Ok(mut node) = thumbs.get_mut(t) {
                        node.top = Val::Px(fraction * (height - 4.0).max(0.0));
                    }
                }
            }
        }
        for &row_entity in children {
            if !row_entities.contains(row_entity) {
                continue;
            }
            let Ok((row, row_children)) = rows.get(row_entity) else { continue };
            let index = list.offset + row.0;
            let item = list.items.get(index);
            if let Ok(mut background) = backgrounds.get_mut(row_entity) {
                background.0 = if list.selected == Some(index) && item.is_some() { theme::LIST_SELECTED } else { Color::NONE };
            }
            for &child in row_children {
                if let Ok((mut text, mut color)) = texts.get_mut(child) {
                    text.0 = item.cloned().unwrap_or_default();
                    color.0 = if item.is_some_and(|i| i.starts_with('<')) { theme::FOLDER } else { theme::TEXT };
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn typed(text: &str) -> TextBox {
        let mut t = TextBox::default();
        text.chars().for_each(|c| t.insert(c));
        t
    }

    #[test]
    fn typing_and_editing_respect_the_cursor() {
        let mut t = typed("helo");
        t.move_cursor(-1);
        t.insert('l');
        assert_eq!(t.text, "hello");
        t.backspace();
        assert_eq!(t.text, "helo");
        t.move_cursor(-10);
        t.delete();
        assert_eq!(t.text, "elo");
        t.backspace();
        assert_eq!(t.text, "elo");
    }

    #[test]
    fn digit_boxes_only_take_numbers() {
        let mut t = TextBox { digits_only: true, ..default() };
        "a1b.5-c".chars().for_each(|c| t.insert(c));
        assert_eq!(t.text, "1.5-");
        assert_eq!(t.parse_f32(), 0.0);
        t.set_text("-2.5");
        assert_eq!(t.parse_f32(), -2.5);
    }

    #[test]
    fn read_only_boxes_ignore_edits() {
        let mut t = TextBox { read_only: true, ..default() };
        t.set_text("fixed");
        t.insert('x');
        t.backspace();
        assert_eq!(t.text, "fixed");
    }

    #[test]
    fn cursor_is_shown_only_when_focused_and_passwords_are_masked() {
        let mut t = typed("ab");
        t.move_cursor(-1);
        assert_eq!(t.display(true), "a|b");
        assert_eq!(t.display(false), "ab");
        t.password = true;
        assert_eq!(t.display(true), "*|*");
    }

    #[test]
    fn multibyte_characters_do_not_break_editing() {
        let mut t = typed("aé");
        t.backspace();
        assert_eq!(t.text, "a");
    }

    #[test]
    fn list_double_click_needs_two_clicks_on_one_row() {
        let mut list = ListBox::new(5);
        list.set_items(vec!["a".into(), "b".into(), "c".into()]);
        assert!(!list.click(1, 0.0));
        assert_eq!(list.selected_item(), Some("b"));
        assert!(list.click(1, 1.0));
        // Clicking past the last item selects nothing.
        assert!(!list.click(4, 2.0));
        // A slow second click does not count.
        assert!(!list.click(0, 3.0));
        assert!(!list.click(0, 6.0));
    }

    #[test]
    fn touch_lists_have_finger_sized_rows() {
        let desk = ListBox::fitting(280.0, false);
        let touch = ListBox::fitting(280.0, true);
        assert_eq!(desk.row_height, theme::ROW_HEIGHT);
        assert!(touch.row_height >= 30.0);
        assert!(touch.rows < desk.rows && touch.rows as f32 * touch.row_height <= 280.0);
    }

    #[test]
    fn windows_are_moved_onto_the_screen() {
        let phone = Vec2::new(915.0, 412.0);
        // The 400-tall file dialog at (50, 50) would hang off the bottom.
        // ...and is slid right so the menu tab does not cover its title.
        let tab = Vec2::new(96.0, 32.0);
        assert_eq!(fit_on_screen(Vec2::new(50.0, 50.0), Vec2::new(350.0, 400.0), phone, tab), Vec2::new(96.0, 12.0));
        // Taller than the screen: pinned to the top so the title bar shows.
        assert_eq!(fit_on_screen(Vec2::new(75.0, 75.0), Vec2::new(600.0, 550.0), phone, tab).y, 0.0);
        assert_eq!(fit_on_screen(Vec2::new(10.0, 60.0), Vec2::new(100.0, 100.0), phone, tab), Vec2::new(10.0, 60.0));
    }

    #[test]
    fn list_scrolling_stays_in_range() {
        let mut list = ListBox::new(3);
        list.set_items((0..10).map(|i| i.to_string()).collect());
        list.scroll(-1);
        assert_eq!(list.offset, 0);
        for _ in 0..50 {
            list.scroll(1);
        }
        assert_eq!(list.offset, 8);
    }
}
