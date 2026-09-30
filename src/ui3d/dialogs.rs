//! Builders for the simulator's windows, laid out exactly as the original
//! (`ui3d/window3d.cpp`, `MenuHandler.cpp`, `userinterface.cpp`): same window
//! sizes and positions, same controls at the same window-relative pixel
//! coordinates, same captions.

use std::path::PathBuf;

use bevy::prelude::*;

use super::actions::Action;
use super::filebrowser::{self, FileBrowser};
use super::widgets::*;
use super::{Command, UiState};
use crate::settings::GridColorScheme;

/// The original OK/Cancel button size.
const BUTTON: Vec2 = Vec2::new(85.0, 18.0);

/// Common arguments for spawning UI.
pub struct Ctx<'a, 'w, 's> {
    pub commands: &'a mut Commands<'w, 's>,
    pub camera: Entity,
    pub ui: &'a mut UiState,
}

impl Ctx<'_, '_, '_> {
    fn window(&mut self, title: &str, pos: Vec2, size: Vec2, kind: WindowKind, default: Option<Action>, cancel: Option<Action>) -> (Entity, Entity) {
        spawn_window(self.commands, self.camera, self.ui, WindowSpec { title, size, pos, kind, default_action: default, cancel_action: cancel })
    }

    fn label(&mut self, content: Entity, text: &str, x: f32, y: f32, w: f32, h: f32) -> Entity {
        add_read_only(self.commands, content, text, Vec2::new(w, h), Vec2::new(x, y))
    }

    fn field(&mut self, content: Entity, name: &'static str, text_box: TextBox, x: f32, y: f32, w: f32, h: f32) -> Entity {
        spawn_text_box(self.commands, content, Some(Field(name)), text_box, Vec2::new(w, h), Vec2::new(x, y))
    }
}

/// Where the original put Message/Confirm text: 10 pixels in, starting at
/// half the window height (that was the text baseline).
fn message_position(size: Vec2) -> Vec2 {
    Vec2::new(10.0, size.y / 2.0 - 14.0)
}

/// `AddMessageBox`: 400x220 at (250, 250), OK button at (w/2, 5h/7).
pub fn message_box(ctx: &mut Ctx, title: &str, text: &str) {
    let size = Vec2::new(400.0, 220.0);
    let (_, content) = ctx.window(title, Vec2::new(250.0, 250.0), size, WindowKind::Message, Some(Action::CloseWindow), Some(Action::CloseWindow));
    spawn_label(ctx.commands, content, text, message_position(size));
    add_button(ctx.commands, content, "OK", Action::CloseWindow, BUTTON, Vec2::new((size.x / 2.0).floor(), (size.y * 5.0 / 7.0).floor()));
}

/// `AddConfirmWindow`: 400x220 at (30, 30). The original placed the buttons
/// with the window's own position added in (`x + w/2 - w/4`, `y + 5h/7`),
/// so they sit at (130, 187) and (230, 187).
pub fn confirm(ctx: &mut Ctx, title: &str, text: &str, ok: &str, cancel: &str) -> (Entity, Entity) {
    let (pos, size) = (Vec2::new(30.0, 30.0), Vec2::new(400.0, 220.0));
    let (window, content) = ctx.window(title, pos, size, WindowKind::Confirm, Some(Action::ConfirmOk), Some(Action::CloseWindow));
    if !text.is_empty() {
        spawn_label(ctx.commands, content, text, message_position(size));
    }
    let buttons_y = pos.y + (size.y * 5.0 / 7.0).floor();
    add_button(ctx.commands, content, ok, Action::ConfirmOk, BUTTON, Vec2::new(pos.x + size.x / 2.0 - size.x / 4.0, buttons_y));
    add_button(ctx.commands, content, cancel, Action::CloseWindow, BUTTON, Vec2::new(pos.x + size.x / 2.0, buttons_y));
    (window, content)
}

/// Settings > Grid: a confirm window with a prompt and two scheme buttons
/// (captions spelled as in the original).
pub fn grid_settings(ctx: &mut Ctx) {
    let (_, content) = confirm(ctx, "Grid", "", "Ok", "Cancel");
    ctx.label(content, "Please select a color scheme:", 30.0, 30.0, 325.0, 20.0);
    add_button(ctx.commands, content, "White Backround", Action::GridScheme(GridColorScheme::White), BUTTON, Vec2::new(60.0, 60.0));
    add_button(ctx.commands, content, "Black Backround", Action::GridScheme(GridColorScheme::Black), BUTTON, Vec2::new(60.0, 80.0));
}

/// A list box with the path box above it and the file name box below it,
/// as the original `ListBox3d` constructor created them.
fn list_with_boxes(ctx: &mut Ctx, content: Entity, list: ListBox, pos: Vec2, size: Vec2, path: &str) -> (Entity, Entity) {
    let mut path_box = TextBox { read_only: true, framed: true, ..default() };
    path_box.set_text(path);
    ctx.field(content, "path", path_box, pos.x, pos.y - 20.0, size.x, 20.0);
    let list = spawn_list_box(ctx.commands, content, list, size, pos);
    let name = ctx.field(content, "filename", TextBox::default(), pos.x, pos.y + size.y, size.x, 25.0);
    (list, name)
}

/// `FileOpen` window: 350x400 at (50, 50), list at (50, 50) 250 x (h-125),
/// OK button (captioned with the title) at (w/2 - 80, h - curve - 5).
pub fn file_dialog(ctx: &mut Ctx, title: &str, purpose: Command, ok_caption: &str) {
    let size = Vec2::new(350.0, 400.0);
    let (_, content) = ctx.window(title, Vec2::new(50.0, 50.0), size, WindowKind::FileOpen, Some(Action::FileOk), Some(Action::CloseWindow));

    let browser = FileBrowser { dir: filebrowser::start_dir(), purpose };
    let list_size = Vec2::new(250.0, size.y - 125.0);
    let mut list = ListBox::fitting(list_size.y, ctx.ui.touch_mode);
    list.activate = Some(Action::FileOk);
    filebrowser::populate(&browser, &mut list);
    let caption = filebrowser::path_caption(browser.dir.as_deref());
    let (list_entity, name_box) = list_with_boxes(ctx, content, list, Vec2::new(50.0, 50.0), list_size, &caption);
    ctx.commands.entity(list_entity).insert(browser);
    ctx.ui.text_focus = Some(name_box);

    let curve = curve_radius(size);
    add_button(ctx.commands, content, ok_caption, Action::FileOk, BUTTON, Vec2::new(size.x / 2.0 - 80.0, size.y - curve - 5.0));
}

/// What the properties dialog shows about an object.
pub struct ObjectValues {
    pub name: String,
    pub comments: String,
    pub scale: Vec3,
    pub translation: Vec3,
}

fn number(value: f32) -> String {
    format!("{value}")
}

/// `ObjectProp` window: 550x350 at (50, 50). Its caption becomes the
/// object's name once the object's information arrives, as in the original.
pub fn object_properties(ctx: &mut Ctx, values: &ObjectValues) {
    let size = Vec2::new(550.0, 350.0);
    let (_, content) = ctx.window(&values.name, Vec2::new(50.0, 50.0), size, WindowKind::ObjectProp, Some(Action::ApplyTransform), Some(Action::CloseWindow));
    ctx.label(content, "Object Information", 200.0, 30.0, 200.0, 18.0);
    ctx.label(content, "Name:", 15.0, 55.0, 100.0, 18.0);
    ctx.label(content, "Caption:", 15.0, 75.0, 100.0, 18.0);
    ctx.label(content, &values.name, 150.0, 55.0, 400.0, 20.0);
    ctx.label(content, &values.comments, 150.0, 75.0, 400.0, 20.0);

    ctx.label(content, "Transformations", 200.0, 150.0, 200.0, 18.0);
    let columns = [
        ("Translate", 15.0, 100.0, 40.0, ["xTrans", "yTrans", "zTrans"], values.translation),
        ("Scale", 200.0, 75.0, 225.0, ["xScale", "yScale", "zScale"], values.scale),
        ("Rotate By", 385.0, 75.0, 410.0, ["xRotate", "yRotate", "zRotate"], Vec3::ZERO),
    ];
    let mut order = 0;
    for (title, label_x, title_width, box_x, fields, initial) in columns {
        ctx.label(content, title, label_x, 175.0, title_width, 18.0);
        for (i, (axis, field)) in ["X:", "Y:", "Z:"].into_iter().zip(fields).enumerate() {
            let y = 200.0 + 25.0 * i as f32;
            ctx.label(content, axis, label_x, y, 15.0, 18.0);
            let mut tb = TextBox { digits_only: true, order, ..default() };
            order += 1;
            tb.set_text(number(initial[i]));
            ctx.field(content, field, tb, box_x, y, 103.0, 18.0);
        }
    }
    add_button(ctx.commands, content, "Apply", Action::ApplyTransform, BUTTON, Vec2::new(size.x / 2.0 - 50.0, size.y - 65.0));
    add_button(ctx.commands, content, "Done", Action::CloseWindow, BUTTON, Vec2::new(size.x / 2.0 - 50.0, size.y - 25.0));
}

/// `NetConnect` window: 550x250 at (75, 75), OK at (3w/4 + 2, h - curve - 5).
pub fn net_connect(ctx: &mut Ctx, ip: &str, port: &str) {
    let size = Vec2::new(550.0, 250.0);
    let (_, content) = ctx.window("Network", Vec2::new(75.0, 75.0), size, WindowKind::NetConnect, Some(Action::NetConnect), Some(Action::CloseWindow));
    ctx.label(content, "Connect To Server:", 15.0, 30.0, 250.0, 18.0);
    ctx.label(content, "IP:", 15.0, 70.0, 50.0, 18.0);
    ctx.label(content, "Port:", 300.0, 70.0, 100.0, 18.0);
    ctx.label(content, "Username:", 15.0, 115.0, 100.0, 18.0);
    ctx.label(content, "Password:", 15.0, 150.0, 100.0, 18.0);

    let text = |text: &str, order, password| {
        let mut tb = TextBox { order, password, ..default() };
        tb.set_text(text);
        tb
    };
    let ip_box = ctx.field(content, "ip", text(ip, 0, false), 100.0, 70.0, 175.0, 18.0);
    ctx.field(content, "port", text(port, 1, false), 400.0, 70.0, 100.0, 18.0);
    ctx.field(content, "user", text("", 2, false), 150.0, 115.0, 200.0, 18.0);
    ctx.field(content, "pass", text("", 3, true), 150.0, 150.0, 200.0, 18.0);
    ctx.ui.text_focus = Some(ip_box);

    let curve = curve_radius(size);
    add_button(ctx.commands, content, "OK", Action::NetConnect, BUTTON, Vec2::new(size.x * 3.0 / 4.0 + 2.0, size.y - curve - 5.0));
}

/// `AddObjectPhysics` window: 600x550 at (75, 75) with the available and
/// applied force lists and the move buttons between them.
pub fn attach_physics(ctx: &mut Ctx, available: Vec<String>, applied: Vec<String>) {
    let size = Vec2::new(600.0, 550.0);
    let (_, content) = ctx.window("Attach Physics", Vec2::new(75.0, 75.0), size, WindowKind::AttachPhysics, None, Some(Action::CloseWindow));
    let list_size = Vec2::new(200.0, 350.0);

    let mut left = ListBox::fitting(list_size.y, ctx.ui.touch_mode);
    left.items = available;
    left.activate = Some(Action::AddForce);
    let (left, _) = list_with_boxes(ctx, content, left, Vec2::new(30.0, 75.0), list_size, "");
    ctx.commands.entity(left).insert(Field("forces"));

    let mut right = ListBox::fitting(list_size.y, ctx.ui.touch_mode);
    right.items = applied;
    right.click_lock = true;
    let (right, _) = list_with_boxes(ctx, content, right, Vec2::new(375.0, 75.0), list_size, "");
    ctx.commands.entity(right).insert(Field("applied"));

    add_button(ctx.commands, content, " >>  ", Action::AddForce, BUTTON, Vec2::new(260.0, 215.0));
    add_button(ctx.commands, content, " <<  ", Action::RemoveForce, BUTTON, Vec2::new(260.0, 275.0));
    add_button(ctx.commands, content, "Close", Action::CloseWindow, BUTTON, Vec2::new(260.0, 480.0));
}

/// Scene > Add World Force: 600x550 at (75, 75), built as in
/// `EventHandlers::AddWorldForcesMenu`.
pub fn world_force(ctx: &mut Ctx) {
    let size = Vec2::new(600.0, 550.0);
    let (_, content) = ctx.window("World Forces", Vec2::new(75.0, 75.0), size, WindowKind::WorldForce, Some(Action::AddWorldForce), Some(Action::CloseWindow));
    let (line1, line2, line4) = (70.0, 110.0, 230.0);
    ctx.label(content, "Force Direction", 30.0, 30.0, 200.0, 20.0);
    let mut first = None;
    for (i, (field, x)) in [("vX", 50.0), ("vY", 150.0), ("vZ", 250.0)].into_iter().enumerate() {
        let id = ctx.field(content, field, TextBox { digits_only: true, order: i, ..default() }, x, line1, 60.0, 20.0);
        first.get_or_insert(id);
    }
    ctx.label(content, "X:", 15.0, line1, 20.0, 20.0);
    ctx.label(content, "Y:", 115.0, line1, 20.0, 20.0);
    ctx.label(content, "Z:", 215.0, line1, 20.0, 20.0);
    ctx.label(content, "Duration:", 30.0, line2, 150.0, 20.0);
    ctx.field(content, "duration", TextBox { digits_only: true, order: 3, ..default() }, 30.0, line2 + 30.0, 50.0, 20.0);
    ctx.label(content, "Name:", 30.0, line4, 150.0, 20.0);
    ctx.field(content, "name", TextBox { order: 4, ..default() }, 30.0, line4 + 30.0, 50.0, 20.0);
    ctx.ui.text_focus = first;
    add_button(ctx.commands, content, "OK", Action::AddWorldForce, BUTTON, Vec2::new(240.0, 180.0));
}

/// `AddVideoWindow` + `QuickCamView`: a 260x260 window captioned "Video
/// Camera" at (30, 30) with the 200x200 picture 30 pixels in.
pub fn video_window(ctx: &mut Ctx, image: Handle<Image>) -> Entity {
    let size = Vec2::new(260.0, 260.0);
    let (window, content) = ctx.window("Video Camera", Vec2::new(30.0, 30.0), size, WindowKind::Video, None, Some(Action::CloseWindow));
    let picture = ctx
        .commands
        .spawn((
            ImageNode::new(image),
            Node { position_type: PositionType::Absolute, left: Val::Px(30.0), top: Val::Px(30.0), width: Val::Px(200.0), height: Val::Px(200.0), ..default() },
        ))
        .id();
    ctx.commands.entity(content).add_child(picture);
    window
}

/// The directory a file dialog result lives in, and the chosen name.
pub fn chosen_file(dir: Option<PathBuf>, name: &str) -> Option<PathBuf> {
    let name = name.trim();
    if name.is_empty() {
        return None;
    }
    Some(dir.map_or_else(|| PathBuf::from(name), |d| d.join(name)))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn chosen_file_joins_directory_and_name() {
        assert_eq!(chosen_file(Some(PathBuf::from("/a")), " x.world "), Some(PathBuf::from("/a/x.world")));
        assert_eq!(chosen_file(None, "demo.world"), Some(PathBuf::from("demo.world")));
        assert_eq!(chosen_file(Some(PathBuf::from("/a")), "  "), None);
    }
}
