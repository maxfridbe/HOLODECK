//! Windows, buttons, text boxes and list boxes.

use bevy::input::ButtonState;
use bevy::input::keyboard::{Key, KeyboardInput};
use bevy::input::mouse::MouseWheel;
use bevy::prelude::*;
use bevy::ui::FocusPolicy;

use super::theme;
use super::actions::{Action, UiAction};
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
    /// Double-click does nothing (used for the applied-forces list).
    pub click_lock: bool,
    /// Action fired by a double click.
    pub activate: Option<Action>,
    last_click: Option<(usize, f32)>,
}

impl ListBox {
    pub fn new(rows: usize) -> Self {
        Self { rows, ..default() }
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
        app.add_event::<ListActivated>().add_systems(
            Update,
            (
                focus_windows,
                drag_windows,
                button_clicks,
                button_colors,
                text_box_focus,
                keyboard_input,
                list_clicks,
                list_scroll,
                refresh_text_boxes,
                refresh_lists,
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

/// Spawns a window and returns `(window, content)`. Children of `content`
/// are positioned with absolute window-relative pixel coordinates.
pub fn spawn_window(commands: &mut Commands, camera: Entity, ui: &mut UiState, spec: WindowSpec) -> (Entity, Entity) {
    ui.next_z += 1;
    let z = 100 + ui.next_z;
    let window = commands
        .spawn((
            Window3d { kind: spec.kind, default_action: spec.default_action, cancel_action: spec.cancel_action },
            UiRoot,
            Node {
                position_type: PositionType::Absolute,
                left: Val::Px(spec.pos.x),
                top: Val::Px(spec.pos.y),
                width: Val::Px(spec.size.x),
                height: Val::Px(spec.size.y),
                border: UiRect::all(Val::Px(2.0)),
                ..default()
            },
            BackgroundColor(theme::WINDOW),
            BorderColor(theme::WINDOW_BORDER),
            BorderRadius::all(Val::Px(6.0)),
            BoxShadow {
                color: Color::srgba(0.0, 0.0, 0.0, 0.5),
                x_offset: Val::Px(5.0),
                y_offset: Val::Px(5.0),
                spread_radius: Val::Px(0.0),
                blur_radius: Val::Px(6.0),
            },
            Interaction::default(),
            FocusPolicy::Block,
            GlobalZIndex(z),
            TargetCamera(camera),
        ))
        .id();
    ui.focused_window = Some(window);

    let content = commands.spawn(Node { position_type: PositionType::Absolute, width: Val::Percent(100.0), height: Val::Percent(100.0), ..default() }).id();
    let title_bar = commands
        .spawn((
            TitleBar,
            Interaction::default(),
            Node {
                width: Val::Percent(100.0),
                height: Val::Px(theme::TITLE_HEIGHT),
                position_type: PositionType::Absolute,
                align_items: AlignItems::Center,
                padding: UiRect::horizontal(Val::Px(8.0)),
                ..default()
            },
            BackgroundColor(theme::TITLE_BAR),
            BorderRadius::top(Val::Px(4.0)),
        ))
        .with_children(|bar| {
            bar.spawn((Text::new(spec.title), TextFont { font_size: theme::FONT_SIZE, ..default() }, TextColor(theme::TITLE_TEXT)));
        })
        .id();
    let close = spawn_button(commands, "X", Action::CloseWindow, Vec2::new(28.0, theme::TITLE_HEIGHT - 4.0), Vec2::new(spec.size.x - 34.0, 0.0));
    commands.entity(window).add_children(&[content, title_bar, close]);
    (window, content)
}

pub fn spawn_label(commands: &mut Commands, parent: Entity, text: &str, pos: Vec2) -> Entity {
    let label = commands
        .spawn((
            Text::new(text),
            TextFont { font_size: theme::FONT_SIZE, ..default() },
            TextColor(theme::TEXT),
            Node { position_type: PositionType::Absolute, left: Val::Px(pos.x), top: Val::Px(pos.y), ..default() },
        ))
        .id();
    commands.entity(parent).add_child(label);
    label
}

/// Spawns a button positioned in its parent (parent set by the caller).
pub fn spawn_button(commands: &mut Commands, caption: &str, action: Action, size: Vec2, pos: Vec2) -> Entity {
    commands
        .spawn((
            UiButton { action, armed: false },
            Interaction::default(),
            FocusPolicy::Block,
            Node {
                position_type: PositionType::Absolute,
                left: Val::Px(pos.x),
                top: Val::Px(pos.y),
                width: Val::Px(size.x),
                height: Val::Px(size.y),
                justify_content: JustifyContent::Center,
                align_items: AlignItems::Center,
                border: UiRect::all(Val::Px(1.0)),
                ..default()
            },
            BackgroundColor(theme::BUTTON),
            BorderColor(theme::FIELD_BORDER),
            BorderRadius::all(Val::Px(3.0)),
        ))
        .with_children(|b| {
            b.spawn((Text::new(caption), TextFont { font_size: theme::FONT_SIZE, ..default() }, TextColor(theme::TEXT)));
        })
        .id()
}

pub fn add_button(commands: &mut Commands, parent: Entity, caption: &str, action: Action, size: Vec2, pos: Vec2) -> Entity {
    let button = spawn_button(commands, caption, action, size, pos);
    commands.entity(parent).add_child(button);
    button
}

pub fn spawn_text_box(commands: &mut Commands, parent: Entity, field: Option<Field>, text_box: TextBox, size: Vec2, pos: Vec2) -> Entity {
    let read_only = text_box.read_only;
    let mut entity = commands.spawn((
        text_box,
        Interaction::default(),
        Node {
            position_type: PositionType::Absolute,
            left: Val::Px(pos.x),
            top: Val::Px(pos.y),
            width: Val::Px(size.x),
            height: Val::Px(size.y),
            align_items: AlignItems::Center,
            padding: UiRect::horizontal(Val::Px(4.0)),
            border: UiRect::all(Val::Px(if read_only { 0.0 } else { 1.0 })),
            overflow: Overflow::clip(),
            ..default()
        },
        BackgroundColor(if read_only { Color::NONE } else { theme::FIELD }),
        BorderColor(theme::FIELD_BORDER),
    ));
    if let Some(field) = field {
        entity.insert(field);
    }
    let id = entity
        .with_children(|b| {
            b.spawn((TextBoxLabel, Text::new(""), TextFont { font_size: theme::FONT_SIZE, ..default() }, TextColor(theme::TEXT)));
        })
        .id();
    commands.entity(parent).add_child(id);
    id
}

/// A read-only label styled as a text box (kept for parity with the layouts).
pub fn add_read_only(commands: &mut Commands, parent: Entity, text: &str, size: Vec2, pos: Vec2) -> Entity {
    let mut tb = TextBox { read_only: true, ..default() };
    tb.set_text(text);
    spawn_text_box(commands, parent, None, tb, size, pos)
}

pub fn spawn_list_box(commands: &mut Commands, parent: Entity, list: ListBox, size: Vec2, pos: Vec2) -> Entity {
    let rows = list.rows;
    let id = commands
        .spawn((
            list,
            Node {
                position_type: PositionType::Absolute,
                left: Val::Px(pos.x),
                top: Val::Px(pos.y),
                width: Val::Px(size.x),
                height: Val::Px(size.y),
                flex_direction: FlexDirection::Column,
                overflow: Overflow::clip(),
                border: UiRect::all(Val::Px(2.0)),
                ..default()
            },
            BackgroundColor(theme::LIST),
            BorderColor(Color::srgb_u8(54, 54, 54)),
        ))
        .with_children(|b| {
            for row in 0..rows {
                b.spawn((
                    ListRow(row),
                    Interaction::default(),
                    Node { height: Val::Px(theme::ROW_HEIGHT), width: Val::Percent(100.0), padding: UiRect::horizontal(Val::Px(4.0)), ..default() },
                    BackgroundColor(Color::NONE),
                ))
                .with_children(|r| {
                    r.spawn((Text::new(""), TextFont { font_size: theme::FONT_SIZE - 1.0, ..default() }, TextColor(theme::TEXT)));
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

fn button_colors(mut buttons: Query<(&Interaction, &mut BackgroundColor), (With<UiButton>, Changed<Interaction>)>) {
    for (interaction, mut color) in &mut buttons {
        color.0 = match interaction {
            Interaction::Pressed => theme::BUTTON_PRESSED,
            Interaction::Hovered => theme::BUTTON_HOVER,
            Interaction::None => theme::BUTTON,
        };
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
    boxes: Query<(Entity, &TextBox, &Children, Option<&Interaction>)>,
    mut labels: Query<&mut Text, With<TextBoxLabel>>,
    mut borders: Query<&mut BorderColor>,
) {
    for (entity, text_box, children, _) in &boxes {
        let focused = ui.text_focus == Some(entity);
        for &child in children {
            if let Ok(mut label) = labels.get_mut(child) {
                let shown = text_box.display(focused);
                if label.0 != shown {
                    label.0 = shown;
                }
            }
        }
        if let Ok(mut border) = borders.get_mut(entity) {
            border.0 = if focused { theme::FIELD_FOCUS } else { theme::FIELD_BORDER };
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
) {
    for (list, children) in &lists {
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
