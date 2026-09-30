//! The main menu bar and right-click context menus.
//!
//! A menu bar is a row of headers; hovering a header drops down its items.
//! The main bar starts minimised as a small tab, and slides out when
//! toggled with Space. Context menus slide out at the cursor and vanish when
//! dismissed.

use bevy::prelude::*;
use bevy::ui::FocusPolicy;

use super::actions::{Action, UiAction};
use super::theme;
use super::panel::PanelMaterial;
use super::widgets::{Panel, ui_font};
use super::{UiRoot, UiState};
use crate::touch::Pointer;
use crate::objects::manip::ManipKind;
use crate::view::{MainCamera, ViewType};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum MenuState {
    Off,
    On,
    Engaging,
    Retracting,
    /// The main bar's collapsed tab.
    Min,
}

#[derive(Clone, Debug)]
pub struct MenuItem {
    pub caption: String,
    pub action: Action,
    pub enabled: bool,
    /// `Some` for items with a check mark.
    pub checked: Option<bool>,
}

impl MenuItem {
    fn new(caption: &str, action: Action) -> Self {
        Self { caption: caption.to_owned(), action, enabled: true, checked: None }
    }

    fn disabled(mut self) -> Self {
        self.enabled = false;
        self
    }

    fn checkable(mut self, checked: bool) -> Self {
        self.checked = Some(checked);
        self
    }

}

#[derive(Clone, Debug)]
pub struct MenuHeader {
    pub caption: String,
    pub items: Vec<MenuItem>,
}

/// Sizes of a menu bar, in UI units. Taller rows on touch screens; headers
/// narrow down so the whole bar fits on narrow (portrait) screens.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct MenuMetrics {
    pub row: f32,
    pub header: f32,
    pub tab: f32,
}

impl Default for MenuMetrics {
    fn default() -> Self {
        Self { row: theme::MENU_HEIGHT, header: theme::MENU_WIDTH, tab: theme::MENU_TAB_WIDTH }
    }
}

impl MenuMetrics {
    pub fn new(touch: bool, available_width: f32, headers: usize) -> Self {
        let (row, tab) = if touch { (theme::TOUCH_MENU_HEIGHT, theme::TOUCH_MENU_TAB_WIDTH) } else { (theme::MENU_HEIGHT, theme::MENU_TAB_WIDTH) };
        let header = (available_width / headers.max(1) as f32).clamp(theme::MIN_MENU_WIDTH, theme::MENU_WIDTH);
        Self { row, header, tab }
    }
}

#[derive(Component, Debug)]
pub struct MenuBar {
    pub metrics: MenuMetrics,
    pub headers: Vec<MenuHeader>,
    pub state: MenuState,
    /// Width revealed so far, in pixels.
    pub draw_place: f32,
    pub origin: Vec2,
    pub is_context: bool,
    /// The header whose items are showing.
    pub open: Option<usize>,
    /// The item under the pointer during the current press.
    pressed_item: Option<(usize, usize)>,
    /// Bumped whenever the entries change so the nodes get rebuilt.
    revision: u32,
    built: Option<(u32, MenuState, Option<usize>)>,
}

impl MenuBar {
    fn new(headers: Vec<MenuHeader>, origin: Vec2, is_context: bool) -> Self {
        Self {
            metrics: MenuMetrics::default(),
            headers,
            state: if is_context { MenuState::Engaging } else { MenuState::Min },
            draw_place: if is_context { 0.0 } else { theme::MENU_TAB_WIDTH },
            origin,
            is_context,
            // A context menu shows its first header's items straight away.
            open: is_context.then_some(0),
            pressed_item: None,
            revision: 0,
            built: None,
        }
    }

    /// True while the menu should capture the mouse.
    pub fn is_active(&self) -> bool {
        !matches!(self.state, MenuState::Off | MenuState::Min)
    }

    fn full_width(&self) -> f32 {
        self.metrics.header * self.headers.len() as f32
    }

    pub fn item_mut(&mut self, caption: &str) -> Option<&mut MenuItem> {
        self.revision += 1;
        self.headers.iter_mut().flat_map(|h| h.items.iter_mut()).find(|i| i.caption == caption)
    }

    pub fn set_checked(&mut self, caption: &str, checked: bool) {
        if let Some(item) = self.item_mut(caption) {
            item.checked = Some(checked);
        }
    }

    pub fn is_checked(&self, caption: &str) -> bool {
        self.headers.iter().flat_map(|h| &h.items).find(|i| i.caption == caption).and_then(|i| i.checked).unwrap_or(false)
    }

    /// Slides toward fully open or closed.
    fn advance(&mut self, dt: f32) {
        let speed = 512.0 * self.headers.len() as f32 * dt * if self.is_context { 1.0 } else { 2.5 };
        match self.state {
            MenuState::Engaging => {
                self.draw_place += speed;
                if self.draw_place >= self.full_width() {
                    self.draw_place = self.full_width();
                    self.state = MenuState::On;
                }
            }
            MenuState::Retracting => {
                self.draw_place -= speed;
                if self.draw_place <= 0.0 {
                    self.draw_place = 0.0;
                    self.state = if self.is_context { MenuState::Off } else { MenuState::Min };
                }
            }
            _ => {}
        }
    }
}

/// Identifies interactive nodes inside a menu.
#[derive(Component)]
pub struct MenuNode {
    header: usize,
    /// `None` for the header itself.
    item: Option<usize>,
    enabled: bool,
}

#[derive(Component)]
struct MinTab;

pub struct MenuPlugin;

impl Plugin for MenuPlugin {
    fn build(&self, app: &mut App) {
        app.add_systems(Update, (animate_menus, menu_input, rebuild_menus, hover_highlight).chain());
    }
}

fn item(caption: &str, action: Action) -> MenuItem {
    MenuItem::new(caption, action)
}

/// The view menu entries; kept so the check mark can follow the current view.
fn view_items() -> Vec<MenuItem> {
    ViewType::ALL.iter().map(|&v| item(v.caption(), Action::SetView(v)).checkable(v == ViewType::Perspective)).collect()
}

pub fn main_menu() -> Vec<MenuHeader> {
    let header = |caption: &str, items: Vec<MenuItem>| MenuHeader { caption: caption.to_owned(), items };
    let mut view = view_items();
    view.push(item(" ", Action::Nothing));
    view.push(item("HoloGrid", Action::ToggleHoloGrid).checkable(true));
    view.push(item("Wireframe", Action::ToggleWireframe));
    view.push(item("Inner Grid", Action::ToggleInnerGrid).checkable(false));

    vec![
        header("File", vec![item("Connect", Action::Connect), item("Exit", Action::Quit)]),
        header("Edit", vec![item("Duplicate", Action::Duplicate)]),
        header(
            "Settings",
            vec![item("Manipulator", Action::Nothing).disabled(), item("Grid", Action::ColorSettings), item("World", Action::Nothing).disabled()],
        ),
        header("View", view),
        header(
            "Scene",
            vec![
                item("New", Action::NewScene),
                item("Save", Action::SaveScene),
                item("Open", Action::LoadScene),
                item("Add World Force", Action::AddWorldForcesMenu),
            ],
        ),
        header("Model", vec![item("Load", Action::LoadModel), item("Unload", Action::UnloadModel)]),
    ]
}

/// What the right-click menu offers, depending on what is selected.
pub enum ContextTarget {
    Camera,
    Object,
    Manipulator,
    Nothing,
}

pub fn context_menu(target: ContextTarget) -> Vec<MenuHeader> {
    let header = |caption: &str, items: Vec<MenuItem>| MenuHeader { caption: caption.to_owned(), items };
    match target {
        ContextTarget::Camera => vec![header(
            "Camera",
            vec![
                item("Translate", Action::SetManip(ManipKind::Translate)),
                item("View", Action::LookFromCamera),
                item("Control", Action::ControlCamera),
            ],
        )],
        ContextTarget::Object => vec![header(
            "Object",
            vec![
                item("Translate", Action::SetManip(ManipKind::Translate)),
                item("Scale", Action::SetManip(ManipKind::Scale)),
                item("Rotate", Action::SetManip(ManipKind::Rotate)),
                item("Properties", Action::ObjectProperties),
                item("Modify Forces", Action::ModifyForces),
                item(" ", Action::Nothing),
                item("Duplicate", Action::Duplicate),
                item("Deselect", Action::Deselect),
                item(" ", Action::Nothing),
                item("Unload", Action::UnloadModel),
            ],
        )],
        ContextTarget::Manipulator => vec![header("Manipulator", vec![item("Done", Action::Deselect)])],
        ContextTarget::Nothing => vec![
            header("Scene", vec![item("Open", Action::LoadScene), item("Exit", Action::Quit)]),
            header("Model", vec![item("Load", Action::LoadModel)]),
        ],
    }
}

pub fn spawn_main_menu(mut commands: Commands, camera: Query<Entity, With<MainCamera>>) {
    let Ok(camera) = camera.get_single() else { return };
    spawn_bar(&mut commands, camera, MenuBar::new(main_menu(), Vec2::ZERO, false));
}

/// Opens a context menu at `at`, replacing any existing one.
pub fn spawn_context_menu(commands: &mut Commands, camera: Entity, at: Vec2, headers: Vec<MenuHeader>) {
    spawn_bar(commands, camera, MenuBar::new(headers, at, true));
}

fn spawn_bar(commands: &mut Commands, camera: Entity, bar: MenuBar) {
    let origin = bar.origin;
    commands.spawn((
        bar,
        UiRoot,
        Node {
            position_type: PositionType::Absolute,
            left: Val::Px(origin.x),
            top: Val::Px(origin.y),
            height: Val::Px(theme::MENU_HEIGHT),
            overflow: Overflow { x: OverflowAxis::Clip, y: OverflowAxis::Visible },
            ..default()
        },
        GlobalZIndex(1000),
        TargetCamera(camera),
    ));
}

fn animate_menus(time: Res<Time>, ui: Res<UiState>, mut commands: Commands, mut bars: Query<(Entity, &mut MenuBar, &mut Node)>) {
    for (entity, mut bar, mut node) in &mut bars {
        let metrics = MenuMetrics::new(ui.touch_mode, ui.screen.x - bar.origin.x, bar.headers.len());
        if bar.metrics != metrics {
            bar.metrics = metrics;
            bar.revision += 1;
            if bar.state == MenuState::On {
                bar.draw_place = bar.full_width();
            }
        }
        bar.advance(time.delta_secs());
        node.width = Val::Px(if bar.state == MenuState::Min { bar.metrics.tab } else { bar.draw_place });
        node.height = Val::Px(bar.metrics.row);
        // Clip only while sliding; once open, dropdowns may be wider than the bar.
        let overflow = if bar.state == MenuState::On { Overflow::visible() } else { Overflow { x: OverflowAxis::Clip, y: OverflowAxis::Visible } };
        if node.overflow != overflow {
            node.overflow = overflow;
        }
        if bar.state == MenuState::Off && bar.is_context {
            commands.entity(entity).despawn_recursive();
        }
    }
}

/// Hover, click and dismissal behaviour.
fn menu_input(
    pointer: Res<Pointer>,
    mut bars: Query<(&mut MenuBar, &Children)>,
    nodes: Query<(&MenuNode, &Interaction)>,
    tab: Query<&Interaction, With<MinTab>>,
    mut actions: EventWriter<UiAction>,
    children_q: Query<&Children>,
) {
    for (mut bar, children) in &mut bars {
        match bar.state {
            MenuState::Min => {
                if pointer.just_pressed && tab.iter().any(|i| *i != Interaction::None) {
                    bar.state = MenuState::Engaging;
                }
            }
            MenuState::On => {
                let mut hovered_header = None;
                let mut hovered_item = None;
                let mut stack: Vec<Entity> = children.to_vec();
                while let Some(e) = stack.pop() {
                    if let Ok((node, interaction)) = nodes.get(e) {
                        if *interaction != Interaction::None {
                            match node.item {
                                None => hovered_header = Some(node.header),
                                Some(i) => hovered_item = Some((node.header, i)),
                            }
                        }
                    }
                    if let Ok(c) = children_q.get(e) {
                        stack.extend(c.iter());
                    }
                }

                if let Some(h) = hovered_header.or(hovered_item.map(|(h, _)| h)) {
                    if bar.open != Some(h) {
                        bar.open = Some(h);
                    }
                }

                // A finger lifting leaves nothing hovered, so remember what
                // was under the pointer while it was down.
                if hovered_item.is_some() || pointer.just_pressed {
                    bar.pressed_item = hovered_item;
                }
                let over_menu = hovered_header.is_some() || hovered_item.is_some();
                if !over_menu && (pointer.just_pressed || pointer.secondary) {
                    bar.open = None;
                    bar.state = MenuState::Retracting;
                } else if pointer.just_released && !pointer.long_pressed {
                    // (Lifting the finger that long-pressed to open this menu is not a choice.)
                    if let Some((h, i)) = hovered_item.or(bar.pressed_item.take()) {
                        let entry = bar.headers[h].items[i].clone();
                        if entry.enabled && entry.action != Action::Nothing {
                            actions.send(UiAction { action: entry.action, window: None });
                            bar.open = None;
                            bar.state = MenuState::Retracting;
                        }
                    }
                }
            }
            _ => {}
        }
    }
}

/// Rebuilds a bar's nodes when its contents, state or open header change.
fn rebuild_menus(mut commands: Commands, mut bars: Query<(Entity, &mut MenuBar, Option<&Children>)>) {
    for (entity, mut bar, children) in &mut bars {
        // Only the collapsed/expanded distinction and open header change the nodes.
        let shape = match bar.state {
            MenuState::Min => MenuState::Min,
            _ => MenuState::On,
        };
        let signature = (bar.revision, shape, if bar.state == MenuState::On { bar.open } else { None });
        if bar.built == Some(signature) {
            continue;
        }
        bar.built = Some(signature);
        for &child in children.into_iter().flatten() {
            commands.entity(child).despawn_recursive();
        }

        let m = bar.metrics;
        commands.entity(entity).with_children(|root| {
            if shape == MenuState::Min {
                // The original's collapsed menu: a small white-to-grey tab.
                root.spawn((
                    MinTab,
                    Interaction::default(),
                    FocusPolicy::Block,
                    Node { width: Val::Px(m.tab), height: Val::Px(m.row), align_items: AlignItems::Center, padding: UiRect::left(Val::Px(10.0)), ..default() },
                    Panel(PanelMaterial::gradient(theme::MENU_ON_TOP, theme::MENU_ON_BOTTOM)),
                ))
                .with_children(|t| {
                    t.spawn((Text::new("Menu"), ui_font(), TextColor(theme::MENU_TEXT), TextLayout::new_with_no_wrap()));
                });
                return;
            }

            for (h, header) in bar.headers.iter().enumerate() {
                let is_open = signature.2 == Some(h);
                // About 10px per character at full size; shrink to fit narrow headers.
                let caption_size = theme::FONT_SIZE.min((m.header - 12.0) / header.caption.chars().count().max(1) as f32 / 0.6);
                root.spawn((
                    MenuNode { header: h, item: None, enabled: true },
                    Interaction::default(),
                    FocusPolicy::Block,
                    Node {
                        position_type: PositionType::Absolute,
                        left: Val::Px(h as f32 * m.header),
                        top: Val::Px(0.0),
                        width: Val::Px(m.header),
                        height: Val::Px(m.row),
                        overflow: Overflow::clip(),
                        align_items: AlignItems::Center,
                        padding: UiRect::left(Val::Px(10.0_f32.min(m.header / 12.0))),
                        ..default()
                    },
                    Panel(menu_panel(is_open, true)),
                ))
                .with_children(|n| {
                    n.spawn((
                        Text::new(header.caption.clone()),
                        TextFont { font_size: caption_size, ..default() },
                        TextColor(if is_open { theme::MENU_TEXT_SELECTED } else { theme::MENU_TEXT }),
                        TextLayout::new_with_no_wrap(),
                    ));
                });

                if !is_open {
                    continue;
                }
                // All items as wide as the longest caption, as in the original.
                let widest = header.items.iter().map(|i| i.caption.chars().count()).max().unwrap_or(0);
                let drop_width = theme::MENU_WIDTH.max((widest as f32 + 2.0) * 11.0);
                root.spawn(Node {
                    position_type: PositionType::Absolute,
                    left: Val::Px(h as f32 * m.header),
                    top: Val::Px(m.row),
                    width: Val::Px(drop_width),
                    flex_direction: FlexDirection::Column,
                    ..default()
                })
                .with_children(|list| {
                    for (i, entry) in header.items.iter().enumerate() {
                        list.spawn((
                            MenuNode { header: h, item: Some(i), enabled: entry.enabled },
                            Interaction::default(),
                            FocusPolicy::Block,
                            Node { height: Val::Px(m.row), align_items: AlignItems::Center, padding: UiRect::left(Val::Px(10.0)), ..default() },
                            Panel(menu_panel(false, entry.enabled)),
                        ))
                        .with_children(|n| {
                            if entry.checked == Some(true) {
                                // The original's check mark: a small green square.
                                n.spawn((
                                    Node {
                                        position_type: PositionType::Absolute,
                                        left: Val::Px(2.0),
                                        top: Val::Px((m.row - 8.0) / 2.0),
                                        width: Val::Px(8.0),
                                        height: Val::Px(8.0),
                                        ..default()
                                    },
                                    BackgroundColor(theme::MENU_CHECK),
                                ));
                            }
                            let color = if entry.enabled { theme::MENU_TEXT } else { theme::MENU_TEXT_DISABLED };
                            n.spawn((Text::new(entry.caption.clone()), ui_font(), TextColor(color), TextLayout::new_with_no_wrap()));
                        });
                    }
                });
            }
        });
    }
}

/// The original menu shading: the selected header or item uses the full
/// white-to-grey gradient, the others the same at half brightness, and
/// disabled items the darker "off" gradient.
fn menu_panel(selected: bool, enabled: bool) -> PanelMaterial {
    match (selected, enabled) {
        (_, false) => PanelMaterial::gradient(theme::MENU_OFF_TOP, theme::MENU_OFF_BOTTOM),
        (true, true) => PanelMaterial::gradient(theme::MENU_ON_TOP, theme::MENU_ON_BOTTOM),
        (false, true) => PanelMaterial::gradient(theme::MENU_TOP, theme::MENU_BOTTOM),
    }
}

/// Highlights the item under the pointer.
fn hover_highlight(mut nodes: Query<(&MenuNode, &Interaction, &Children, &mut Panel)>, mut texts: Query<&mut TextColor>) {
    for (node, interaction, children, mut panel) in &mut nodes {
        if node.item.is_none() {
            continue;
        }
        let lit = *interaction != Interaction::None && node.enabled;
        let wanted = menu_panel(lit, node.enabled);
        if panel.0.top != wanted.top {
            panel.0 = wanted;
        }
        for &child in children {
            if let Ok(mut text) = texts.get_mut(child) {
                let color = if lit {
                    theme::MENU_TEXT_SELECTED
                } else if node.enabled {
                    theme::MENU_TEXT
                } else {
                    theme::MENU_TEXT_DISABLED
                };
                if text.0 != color {
                    text.0 = color;
                }
            }
        }
    }
}

/// Mirrors the current view into the View menu's check marks.
pub fn check_view(bar: &mut MenuBar, current: ViewType) {
    for v in ViewType::ALL {
        bar.set_checked(v.caption(), v == current);
    }
}

/// Keeps the wireframe caption in sync with the setting.
pub fn sync_wireframe_caption(bar: &mut MenuBar, wireframe: bool) {
    let caption = if wireframe { "Textured" } else { "Wireframe" };
    for header in &mut bar.headers {
        for item in &mut header.items {
            if item.caption == "Wireframe" || item.caption == "Textured" {
                item.caption = caption.to_owned();
            }
        }
    }
    bar.revision += 1;
}

/// Space toggles the main menu.
pub fn toggle_main_menu(bars: &mut Query<&mut MenuBar>) {
    for mut bar in bars.iter_mut().filter(|b| !b.is_context) {
        bar.state = if bar.is_active() { MenuState::Retracting } else { MenuState::Engaging };
    }
}

/// Moves a menu's origin so the bar and its tallest dropdown fit in the window.
pub fn keep_on_screen(origin: Vec2, headers: &[MenuHeader], window: Vec2, metrics: MenuMetrics) -> Vec2 {
    let width = metrics.header * headers.len() as f32 + 60.0;
    let rows = headers.iter().map(|h| h.items.len()).max().unwrap_or(0) + 1;
    let height = metrics.row * rows as f32;
    Vec2::new(origin.x.min(window.x - width).max(0.0), origin.y.min(window.y - height).max(0.0))
}

/// Whether the context-menu spot is reserved for the main menu's tab.
pub fn in_menu_tab_area(pos: Vec2, metrics: MenuMetrics) -> bool {
    pos.x < metrics.tab.max(metrics.header) && pos.y < metrics.row
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn main_menu_has_the_original_layout() {
        let menu = main_menu();
        let names: Vec<_> = menu.iter().map(|h| h.caption.as_str()).collect();
        assert_eq!(names, ["File", "Edit", "Settings", "View", "Scene", "Model"]);
        let view = &menu[3];
        assert_eq!(view.items.len(), 11);
        assert_eq!(view.items[0].checked, Some(true));
        assert!(!menu[2].items[0].enabled, "Manipulator is greyed out");
    }

    #[test]
    fn view_check_marks_are_exclusive() {
        let mut bar = MenuBar::new(main_menu(), Vec2::ZERO, false);
        check_view(&mut bar, ViewType::Top);
        assert!(bar.is_checked("Top"));
        assert!(!bar.is_checked("3d"));
    }

    #[test]
    fn main_menu_slides_out_then_collapses_back_to_a_tab() {
        let mut bar = MenuBar::new(main_menu(), Vec2::ZERO, false);
        assert_eq!(bar.state, MenuState::Min);
        assert!(!bar.is_active());
        bar.state = MenuState::Engaging;
        for _ in 0..100 {
            bar.advance(0.02);
        }
        assert_eq!(bar.state, MenuState::On);
        assert_eq!(bar.draw_place, 6.0 * theme::MENU_WIDTH);
        bar.state = MenuState::Retracting;
        for _ in 0..100 {
            bar.advance(0.02);
        }
        assert_eq!(bar.state, MenuState::Min);
    }

    #[test]
    fn context_menus_start_with_their_first_dropdown_open() {
        assert_eq!(MenuBar::new(context_menu(ContextTarget::Object), Vec2::ZERO, true).open, Some(0));
        assert_eq!(MenuBar::new(main_menu(), Vec2::ZERO, false).open, None);
    }

    #[test]
    fn context_menus_turn_off_instead_of_minimising() {
        let mut bar = MenuBar::new(context_menu(ContextTarget::Object), Vec2::new(50.0, 50.0), true);
        assert_eq!(bar.state, MenuState::Engaging);
        for _ in 0..200 {
            bar.advance(0.02);
        }
        assert_eq!(bar.state, MenuState::On);
        bar.state = MenuState::Retracting;
        for _ in 0..200 {
            bar.advance(0.02);
        }
        assert_eq!(bar.state, MenuState::Off);
    }

    #[test]
    fn context_menu_contents_depend_on_what_is_selected() {
        assert_eq!(context_menu(ContextTarget::Camera)[0].items.len(), 3);
        assert_eq!(context_menu(ContextTarget::Object)[0].items.len(), 10);
        assert_eq!(context_menu(ContextTarget::Nothing).len(), 2);
        assert_eq!(context_menu(ContextTarget::Manipulator)[0].items[0].caption, "Done");
    }

    #[test]
    fn context_menus_are_kept_inside_the_window() {
        let headers = context_menu(ContextTarget::Object);
        let window = Vec2::new(1280.0, 720.0);
        let m = MenuMetrics::default();
        let at = keep_on_screen(Vec2::new(700.0, 500.0), &headers, window, m);
        assert!(at.y + theme::MENU_HEIGHT * 11.0 <= 720.0);
        assert_eq!(keep_on_screen(Vec2::new(10.0, 10.0), &headers, window, m), Vec2::new(10.0, 10.0));
        assert!(keep_on_screen(Vec2::new(1270.0, 10.0), &headers, window, m).x < 1270.0);
    }

    #[test]
    fn touch_menus_are_finger_sized_and_still_fit_a_phone() {
        let phone = Vec2::new(915.0, 412.0);
        let m = MenuMetrics::new(true, phone.x, 6);
        assert!(m.row >= 32.0);
        // The whole bar fits across, and the View menu (header + 11 items) fits down.
        assert!(m.header * 6.0 <= phone.x);
        assert!(m.row * 12.0 <= phone.y);
        // Portrait: headers narrow so all six are reachable.
        let portrait = MenuMetrics::new(true, 412.0, 6);
        assert!(portrait.header * 6.0 <= 412.0);
        // Desktop keeps the original compact look.
        assert_eq!(MenuMetrics::new(false, 1280.0, 6), MenuMetrics::default());
    }

    #[test]
    fn check_marks_are_state_not_part_of_the_caption() {
        let i = MenuItem::new("Grid", Action::Nothing).checkable(true);
        assert_eq!(i.caption, "Grid");
        assert_eq!(i.checked, Some(true));
    }
}
