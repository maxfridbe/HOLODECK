//! Builders for the simulator's windows. Layouts use window-relative pixel
//! coordinates from the original.

use std::path::PathBuf;

use bevy::prelude::*;

use super::actions::Action;
use super::filebrowser::{self, FileBrowser};
use super::widgets::*;
use super::{Command, UiState};
use crate::settings::GridColorScheme;

const BUTTON: Vec2 = Vec2::new(85.0, 22.0);

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
}

/// A box with a message and an OK button.
pub fn message_box(ctx: &mut Ctx, title: &str, text: &str) {
    let size = Vec2::new(400.0, 220.0);
    let (_, content) = ctx.window(title, Vec2::new(250.0, 250.0), size, WindowKind::Message, Some(Action::CloseWindow), Some(Action::CloseWindow));
    spawn_label(ctx.commands, content, text, Vec2::new(30.0, 60.0));
    add_button(ctx.commands, content, "OK", Action::CloseWindow, BUTTON, Vec2::new(size.x / 2.0 - BUTTON.x / 2.0, 160.0));
}

/// A question with accept and cancel buttons; accepting runs [`Action::ConfirmOk`].
pub fn confirm(ctx: &mut Ctx, title: &str, text: &str, ok: &str, cancel: &str) -> (Entity, Entity) {
    let size = Vec2::new(400.0, 220.0);
    let (window, content) = ctx.window(title, Vec2::new(30.0, 60.0), size, WindowKind::Confirm, Some(Action::ConfirmOk), Some(Action::CloseWindow));
    spawn_label(ctx.commands, content, text, Vec2::new(30.0, 60.0));
    add_button(ctx.commands, content, ok, Action::ConfirmOk, BUTTON, Vec2::new(100.0, 160.0));
    add_button(ctx.commands, content, cancel, Action::CloseWindow, BUTTON, Vec2::new(215.0, 160.0));
    (window, content)
}

/// The grid colour scheme picker.
pub fn grid_settings(ctx: &mut Ctx) {
    let size = Vec2::new(400.0, 220.0);
    let (_, content) = ctx.window("Grid", Vec2::new(30.0, 60.0), size, WindowKind::Confirm, None, Some(Action::CloseWindow));
    add_read_only(ctx.commands, content, "Please select a color scheme:", Vec2::new(325.0, 20.0), Vec2::new(30.0, 40.0));
    add_button(ctx.commands, content, "White Background", Action::GridScheme(GridColorScheme::White), Vec2::new(160.0, 24.0), Vec2::new(30.0, 80.0));
    add_button(ctx.commands, content, "Black Background", Action::GridScheme(GridColorScheme::Black), Vec2::new(160.0, 24.0), Vec2::new(30.0, 115.0));
    add_button(ctx.commands, content, "Cancel", Action::CloseWindow, BUTTON, Vec2::new(150.0, 170.0));
}

/// Open/save dialog for models and scenes.
pub fn file_dialog(ctx: &mut Ctx, title: &str, purpose: Command, ok_caption: &str) {
    let size = Vec2::new(350.0, 400.0);
    let (_, content) = ctx.window(title, Vec2::new(50.0, 50.0), size, WindowKind::FileOpen, Some(Action::FileOk), Some(Action::CloseWindow));

    let browser = FileBrowser { dir: filebrowser::start_dir(), purpose };
    let mut path_box = TextBox { read_only: true, ..default() };
    path_box.set_text(filebrowser::path_caption(browser.dir.as_deref()));
    spawn_text_box(ctx.commands, content, Some(Field("path")), path_box, Vec2::new(300.0, 18.0), Vec2::new(25.0, 32.0));

    let mut list = ListBox::new(17);
    list.activate = Some(Action::FileOk);
    filebrowser::populate(&browser, &mut list);
    let list_entity = spawn_list_box(ctx.commands, content, list, Vec2::new(300.0, 280.0), Vec2::new(25.0, 55.0));
    ctx.commands.entity(list_entity).insert(browser);

    let name = TextBox::default();
    let name_box = spawn_text_box(ctx.commands, content, Some(Field("filename")), name, Vec2::new(300.0, 22.0), Vec2::new(25.0, 342.0));
    ctx.ui.text_focus = Some(name_box);
    add_button(ctx.commands, content, ok_caption, Action::FileOk, Vec2::new(120.0, 24.0), Vec2::new(115.0, 368.0));
}

/// What the properties dialog shows about an object.
pub struct ObjectValues {
    pub name: String,
    pub comments: String,
    pub scale: Vec3,
    pub translation: Vec3,
}

/// The transformations dialog: translate, scale and rotate-by fields.
pub fn object_properties(ctx: &mut Ctx, values: &ObjectValues) {
    let size = Vec2::new(550.0, 350.0);
    let (_, content) = ctx.window("Transformations", Vec2::new(50.0, 50.0), size, WindowKind::ObjectProp, Some(Action::ApplyTransform), Some(Action::CloseWindow));
    let label = |ctx: &mut Ctx, text: &str, x: f32, y: f32, w: f32| {
        add_read_only(ctx.commands, content, text, Vec2::new(w, 18.0), Vec2::new(x, y));
    };
    label(ctx, "Object Information", 200.0, 30.0, 200.0);
    label(ctx, "Name:", 15.0, 55.0, 100.0);
    label(ctx, "Caption:", 15.0, 75.0, 100.0);
    for (y, text) in [(55.0, values.name.as_str()), (75.0, values.comments.as_str())] {
        let mut tb = TextBox { read_only: true, ..default() };
        tb.set_text(text);
        spawn_text_box(ctx.commands, content, None, tb, Vec2::new(400.0, 20.0), Vec2::new(150.0, y));
    }

    label(ctx, "Transformations", 200.0, 150.0, 200.0);
    let columns = [
        ("Translate", 15.0, 40.0, ["xTrans", "yTrans", "zTrans"], values.translation),
        ("Scale", 200.0, 225.0, ["xScale", "yScale", "zScale"], values.scale),
        ("Rotate By", 385.0, 410.0, ["xRotate", "yRotate", "zRotate"], Vec3::ZERO),
    ];
    let mut order = 0;
    for (title, label_x, box_x, fields, initial) in columns {
        label(ctx, title, label_x, 175.0, 100.0);
        for (i, (axis, field)) in ["X:", "Y:", "Z:"].into_iter().zip(fields).enumerate() {
            let y = 200.0 + 25.0 * i as f32;
            label(ctx, axis, label_x, y, 15.0);
            let mut tb = TextBox { digits_only: true, order, ..default() };
            order += 1;
            tb.set_text(format!("{}", initial[i]));
            spawn_text_box(ctx.commands, content, Some(Field(field)), tb, Vec2::new(103.0, 18.0), Vec2::new(box_x, y));
        }
    }
    add_button(ctx.commands, content, "Apply", Action::ApplyTransform, Vec2::new(100.0, 22.0), Vec2::new(225.0, 285.0));
    add_button(ctx.commands, content, "Done", Action::CloseWindow, Vec2::new(100.0, 22.0), Vec2::new(225.0, 315.0));
}

/// Server address entry.
pub fn net_connect(ctx: &mut Ctx, ip: &str, port: &str) {
    let size = Vec2::new(550.0, 250.0);
    let (_, content) = ctx.window("Network", Vec2::new(75.0, 75.0), size, WindowKind::NetConnect, Some(Action::NetConnect), Some(Action::CloseWindow));
    add_read_only(ctx.commands, content, "Connect To Server:", Vec2::new(250.0, 18.0), Vec2::new(15.0, 30.0));
    add_read_only(ctx.commands, content, "IP:", Vec2::new(50.0, 18.0), Vec2::new(15.0, 70.0));
    add_read_only(ctx.commands, content, "Port:", Vec2::new(100.0, 18.0), Vec2::new(300.0, 70.0));
    add_read_only(ctx.commands, content, "Username:", Vec2::new(100.0, 18.0), Vec2::new(15.0, 115.0));
    add_read_only(ctx.commands, content, "Password:", Vec2::new(100.0, 18.0), Vec2::new(15.0, 150.0));

    let field = |ctx: &mut Ctx, name, text: &str, order, password, size: Vec2, pos: Vec2| {
        let mut tb = TextBox { order, password, ..default() };
        tb.set_text(text);
        spawn_text_box(ctx.commands, content, Some(Field(name)), tb, size, pos)
    };
    let ip_box = field(ctx, "ip", ip, 0, false, Vec2::new(175.0, 18.0), Vec2::new(100.0, 70.0));
    field(ctx, "port", port, 1, false, Vec2::new(100.0, 18.0), Vec2::new(400.0, 70.0));
    field(ctx, "user", "", 2, false, Vec2::new(200.0, 18.0), Vec2::new(150.0, 115.0));
    field(ctx, "pass", "", 3, true, Vec2::new(200.0, 18.0), Vec2::new(150.0, 150.0));
    ctx.ui.text_focus = Some(ip_box);
    add_button(ctx.commands, content, "Connect", Action::NetConnect, Vec2::new(100.0, 24.0), Vec2::new(410.0, 200.0));
}

/// The two-list dialog for choosing which forces act on an object.
pub fn attach_physics(ctx: &mut Ctx, available: Vec<String>, applied: Vec<String>) {
    let size = Vec2::new(600.0, 550.0);
    let (_, content) = ctx.window("Attach Physics", Vec2::new(75.0, 75.0), size, WindowKind::AttachPhysics, None, Some(Action::CloseWindow));
    add_read_only(ctx.commands, content, "Available forces", Vec2::new(200.0, 18.0), Vec2::new(30.0, 45.0));
    add_read_only(ctx.commands, content, "Applied forces", Vec2::new(200.0, 18.0), Vec2::new(375.0, 45.0));

    let mut left = ListBox::new(21);
    left.items = available;
    left.activate = Some(Action::AddForce);
    let left = spawn_list_box(ctx.commands, content, left, Vec2::new(200.0, 350.0), Vec2::new(30.0, 75.0));
    ctx.commands.entity(left).insert(Field("forces"));

    let mut right = ListBox::new(21);
    right.items = applied;
    right.click_lock = true;
    let right = spawn_list_box(ctx.commands, content, right, Vec2::new(200.0, 350.0), Vec2::new(375.0, 75.0));
    ctx.commands.entity(right).insert(Field("applied"));

    add_button(ctx.commands, content, " >>  ", Action::AddForce, Vec2::new(85.0, 24.0), Vec2::new(258.0, 215.0));
    add_button(ctx.commands, content, " <<  ", Action::RemoveForce, Vec2::new(85.0, 24.0), Vec2::new(258.0, 275.0));
    add_button(ctx.commands, content, "Close", Action::CloseWindow, Vec2::new(85.0, 24.0), Vec2::new(258.0, 480.0));
}

/// The "add a named force to the world" dialog.
pub fn world_force(ctx: &mut Ctx) {
    let size = Vec2::new(600.0, 320.0);
    let (_, content) = ctx.window("World Forces", Vec2::new(75.0, 75.0), size, WindowKind::WorldForce, Some(Action::AddWorldForce), Some(Action::CloseWindow));
    add_read_only(ctx.commands, content, "Force Direction", Vec2::new(200.0, 20.0), Vec2::new(30.0, 40.0));
    let mut first = None;
    for (i, (label, field, x)) in [("X:", "vX", 50.0), ("Y:", "vY", 150.0), ("Z:", "vZ", 250.0)].into_iter().enumerate() {
        add_read_only(ctx.commands, content, label, Vec2::new(20.0, 20.0), Vec2::new(x - 35.0, 80.0));
        let tb = TextBox { digits_only: true, order: i, ..default() };
        let id = spawn_text_box(ctx.commands, content, Some(Field(field)), tb, Vec2::new(60.0, 20.0), Vec2::new(x, 80.0));
        first.get_or_insert(id);
    }
    add_read_only(ctx.commands, content, "Duration (seconds, 0 = constant):", Vec2::new(300.0, 20.0), Vec2::new(30.0, 125.0));
    spawn_text_box(ctx.commands, content, Some(Field("duration")), TextBox { digits_only: true, order: 3, ..default() }, Vec2::new(80.0, 20.0), Vec2::new(30.0, 150.0));
    add_read_only(ctx.commands, content, "Name:", Vec2::new(150.0, 20.0), Vec2::new(30.0, 190.0));
    spawn_text_box(ctx.commands, content, Some(Field("name")), TextBox { order: 4, ..default() }, Vec2::new(200.0, 20.0), Vec2::new(30.0, 215.0));
    ctx.ui.text_focus = first;
    add_button(ctx.commands, content, "Add", Action::AddWorldForce, Vec2::new(100.0, 24.0), Vec2::new(250.0, 270.0));
}

/// A window showing a camera's picture.
pub fn video_window(ctx: &mut Ctx, title: &str, image: Handle<Image>) -> Entity {
    let size = Vec2::new(260.0, 290.0);
    let (window, content) = ctx.window(title, Vec2::new(30.0, 30.0), size, WindowKind::Video, None, Some(Action::CloseWindow));
    let picture = ctx
        .commands
        .spawn((
            ImageNode::new(image),
            Node { position_type: PositionType::Absolute, left: Val::Px(30.0), top: Val::Px(58.0), width: Val::Px(200.0), height: Val::Px(200.0), ..default() },
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
