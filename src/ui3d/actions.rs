//! What the buttons and menu items do.

use bevy::app::AppExit;
use bevy::core_pipeline::tonemapping::Tonemapping;
use bevy::prelude::*;
use bevy::render::camera::RenderTarget;
use bevy::render::render_resource::{Extent3d, TextureDimension, TextureFormat, TextureUsages};
use bevy::render::view::RenderLayers;
use bevy::asset::RenderAssetUsages;
use bevy::math::DVec3;

use super::dialogs::{self, Ctx, ObjectValues};
use super::menu::{self, ContextTarget, MenuBar};
use super::widgets::{Field, ListBox, TextBox};
use super::filebrowser::FileBrowser;
use super::{Command, UiState};
use crate::camera::{CameraManager, CameraObject};
use crate::net::{self, ConnectRequest};
use crate::objects::manip::{ManipKind, Manipulator};
use crate::objects::model::{self, ModelGeometry, PhysicsBody, SceneObject};
use crate::objects::pick::Selection;
use crate::physics::{ForceManager, PNode};
use crate::settings::{GridColorScheme, GridState, Settings, SystemState};
use crate::view::{MainCamera, ViewPort, ViewType};

/// Mass of an object once forces are attached to it.
const DEFAULT_MASS: f64 = 50.0;
/// Resolution of picture-in-picture views.
const QUICK_VIEW_SIZE: u32 = 400;

#[derive(Clone, Debug, PartialEq)]
pub enum Action {
    Nothing,
    CloseWindow,
    // Menus
    LoadScene,
    SaveScene,
    NewScene,
    LoadModel,
    UnloadModel,
    Connect,
    Quit,
    ObjectProperties,
    SetManip(ManipKind),
    Duplicate,
    Deselect,
    ToggleInnerGrid,
    ToggleHoloGrid,
    ToggleWireframe,
    ColorSettings,
    SetView(ViewType),
    LookFromCamera,
    ControlCamera,
    ModifyForces,
    AddWorldForcesMenu,
    // Buttons
    ConfirmOk,
    ApplyTransform,
    AddForce,
    RemoveForce,
    AddWorldForce,
    NetConnect,
    FileOk,
    GridScheme(GridColorScheme),
}

/// An action together with the window it came from, if any.
#[derive(Event, Clone, Debug)]
pub struct UiAction {
    pub action: Action,
    pub window: Option<Entity>,
}

/// Shows a message box.
#[derive(Event)]
pub struct ShowMessage {
    pub title: String,
    pub text: String,
}

impl ShowMessage {
    pub fn new(title: impl Into<String>, text: impl Into<String>) -> Self {
        Self { title: title.into(), text: text.into() }
    }
}

/// Marks a camera that renders a picture-in-picture view.
#[derive(Component)]
pub struct QuickCam {
    pub index: usize,
}

/// Marks the window showing camera `index`'s picture.
#[derive(Component)]
pub struct VideoWindow(pub usize);

pub fn run_actions(world: &mut World) {
    let actions: Vec<UiAction> = world.resource_mut::<Events<UiAction>>().drain().collect();
    let messages: Vec<ShowMessage> = world.resource_mut::<Events<ShowMessage>>().drain().collect();
    for action in actions {
        perform(world, action);
    }
    for message in messages {
        message_box(world, &message.title, &message.text);
    }
}

fn main_camera(world: &mut World) -> Option<Entity> {
    world.query_filtered::<Entity, With<MainCamera>>().iter(world).next()
}

/// Runs `f` with a UI spawning context, then applies the spawned commands.
fn with_ctx<R>(world: &mut World, f: impl FnOnce(&mut Ctx) -> R) -> Option<R> {
    let camera = main_camera(world)?;
    let result = world.resource_scope(|world, mut ui: Mut<UiState>| {
        let mut commands = world.commands();
        f(&mut Ctx { commands: &mut commands, camera, ui: &mut ui })
    });
    world.flush();
    Some(result)
}

fn message_box(world: &mut World, title: &str, text: &str) {
    with_ctx(world, |ctx| dialogs::message_box(ctx, title, text));
}

fn command(world: &World) -> Command {
    world.resource::<UiState>().command
}

fn set_command(world: &mut World, command: Command) {
    world.resource_mut::<UiState>().command = command;
}

/// The selected scene object, if the selection is one (not a camera).
fn selected_object(world: &World) -> Option<(Entity, usize)> {
    let entity = world.resource::<Selection>().entity?;
    Some((entity, world.get::<SceneObject>(entity)?.id))
}

fn selected_camera(world: &World) -> Option<usize> {
    let entity = world.resource::<Selection>().entity?;
    Some(world.get::<CameraObject>(entity)?.index)
}

fn deselect(world: &mut World) {
    world.resource_mut::<Selection>().clear();
    world.resource_mut::<Manipulator>().hide();
}

fn main_menu_bar<R>(world: &mut World, f: impl FnOnce(&mut MenuBar) -> R) -> Option<R> {
    let mut query = world.query::<&mut MenuBar>();
    let mut bar = query.iter_mut(world).find(|b| !b.is_context)?;
    Some(f(&mut bar))
}

/// The entity of the field called `name` inside `window`.
fn field_entity(world: &mut World, window: Entity, name: &str) -> Option<Entity> {
    let candidates: Vec<Entity> = world.query::<(Entity, &Field)>().iter(world).filter(|(_, f)| f.0 == name).map(|(e, _)| e).collect();
    candidates.into_iter().find(|&e| in_window(world, e, window))
}

fn in_window(world: &World, mut entity: Entity, window: Entity) -> bool {
    loop {
        if entity == window {
            return true;
        }
        match world.get::<Parent>(entity) {
            Some(parent) => entity = parent.get(),
            None => return false,
        }
    }
}

fn field_text(world: &mut World, window: Entity, name: &str) -> String {
    field_entity(world, window, name).and_then(|e| world.get::<TextBox>(e)).map(|t| t.text.clone()).unwrap_or_default()
}

fn field_number(world: &mut World, window: Entity, name: &str) -> f32 {
    field_entity(world, window, name).and_then(|e| world.get::<TextBox>(e)).map_or(0.0, TextBox::parse_f32)
}

fn open_dialog_allowed(world: &World) -> bool {
    command(world) == Command::None
}

fn perform(world: &mut World, UiAction { action, window }: UiAction) {
    match action {
        Action::Nothing => {}
        Action::CloseWindow => {
            if let Some(window) = window {
                close_window(world, window);
            }
        }
        Action::Quit => {
            if open_dialog_allowed(world) {
                set_command(world, Command::RequestExit);
                with_ctx(world, |ctx| dialogs::confirm(ctx, "Exit", "Do you wish to exit?", "Ok", "Cancel"));
            }
        }
        Action::NewScene => {
            if open_dialog_allowed(world) {
                set_command(world, Command::RequestUnloadScene);
                with_ctx(world, |ctx| dialogs::confirm(ctx, "Unload", "Are you sure you want to remove\nthe scene?", "Create", "Cancel"));
            }
        }
        Action::UnloadModel => {
            if open_dialog_allowed(world) && selected_object(world).is_some() {
                set_command(world, Command::RequestUnloadModel);
                with_ctx(world, |ctx| dialogs::confirm(ctx, "Unload", "Are you sure you want to remove\nthe object from the scene?", "Remove", "Cancel"));
            }
        }
        Action::ConfirmOk => {
            let pending = command(world);
            if let Some(window) = window {
                close_window(world, window);
            }
            match pending {
                Command::RequestExit => {
                    world.send_event(AppExit::Success);
                }
                Command::RequestUnloadScene => {
                    deselect(world);
                    model::clear(world);
                }
                Command::RequestUnloadModel => {
                    if let Some((_, id)) = selected_object(world) {
                        deselect(world);
                        if let Err(e) = model::close(world, id) {
                            message_box(world, "Error", &e.to_string());
                        }
                    }
                }
                _ => {}
            }
        }
        Action::Connect => {
            if open_dialog_allowed(world) {
                set_command(world, Command::RequestNew);
                let port = net::DEFAULT_PORT.to_string();
                with_ctx(world, |ctx| dialogs::net_connect(ctx, "127.0.0.1", &port));
            }
        }
        Action::NetConnect => {
            let Some(window) = window else { return };
            let host = field_text(world, window, "ip");
            let port = field_text(world, window, "port").trim().parse().unwrap_or(net::DEFAULT_PORT);
            close_window(world, window);
            world.send_event(ConnectRequest { host: host.trim().to_owned(), port });
        }
        Action::LoadScene => open_file_dialog(world, "Open Scene", Command::RequestLoadScene, "Open Scene"),
        Action::SaveScene => open_file_dialog(world, "Save Scene", Command::RequestSaveScene, "Save Scene"),
        Action::LoadModel => open_file_dialog(world, "Load Model", Command::RequestLoadModel, "Load Model"),
        Action::FileOk => {
            if let Some(window) = window {
                accept_file(world, window);
            }
        }
        Action::ObjectProperties => {
            if !open_dialog_allowed(world) {
                return;
            }
            let Some((entity, _)) = selected_object(world) else {
                message_box(world, "Properties", "Select an object first.");
                return;
            };
            let (Some(transform), Some(geometry)) = (world.get::<Transform>(entity).copied(), world.get::<ModelGeometry>(entity).map(|g| g.data.clone())) else { return };
            set_command(world, Command::RequestEditObject);
            let values = ObjectValues { name: geometry.name.clone(), comments: geometry.comments.clone(), scale: transform.scale, translation: transform.translation };
            with_ctx(world, |ctx| dialogs::object_properties(ctx, &values));
        }
        Action::ApplyTransform => {
            if let (Some(window), Some((entity, _))) = (window, selected_object(world)) {
                apply_transform(world, window, entity);
            }
        }
        Action::SetManip(kind) => {
            world.resource_mut::<Manipulator>().kind = Some(kind);
        }
        Action::Duplicate => {
            if let Some((_, id)) = selected_object(world) {
                if let Err(e) = model::clone_object(world, id) {
                    message_box(world, "Error", &e.to_string());
                }
            }
        }
        Action::Deselect => deselect(world),
        Action::ToggleInnerGrid => {
            let checked = main_menu_bar(world, |bar| {
                let now = !bar.is_checked("Inner Grid");
                bar.set_checked("Inner Grid", now);
                now
            });
            world.resource_mut::<Settings>().inner_grid = checked.unwrap_or(false);
        }
        Action::ToggleHoloGrid => {
            let enabled = main_menu_bar(world, |bar| {
                let now = !bar.is_checked("HoloGrid");
                bar.set_checked("HoloGrid", now);
                now
            })
            .unwrap_or(true);
            let mut settings = world.resource_mut::<Settings>();
            match (enabled, settings.grid.state) {
                (true, GridState::Off) => settings.grid.state = GridState::Enable,
                (false, GridState::On) => settings.grid.state = GridState::Disable,
                _ => {}
            }
        }
        Action::ToggleWireframe => {
            let wireframe = {
                let mut settings = world.resource_mut::<Settings>();
                settings.wireframe_mode = !settings.wireframe_mode;
                settings.wireframe_mode
            };
            main_menu_bar(world, |bar| menu::sync_wireframe_caption(bar, wireframe));
        }
        Action::ColorSettings => {
            if open_dialog_allowed(world) {
                set_command(world, Command::RequestGridSettings);
                with_ctx(world, dialogs::grid_settings);
            }
        }
        Action::GridScheme(scheme) => {
            if let Some(window) = window {
                close_window(world, window);
            }
            world.resource_mut::<Settings>().grid.apply_scheme(scheme);
        }
        Action::SetView(view) => set_view(world, view),
        Action::LookFromCamera => {
            if let Some(index) = selected_camera(world) {
                open_video_window(world, index);
            }
        }
        Action::ControlCamera => {
            if let Some(index) = selected_camera(world) {
                crate::camera::enter_control(world, index);
            }
        }
        Action::ModifyForces => {
            if !open_dialog_allowed(world) {
                return;
            }
            let Some((entity, _)) = selected_object(world) else {
                message_box(world, "Forces", "Select an object first.");
                return;
            };
            set_command(world, Command::RequestWorldForces);
            let applied: Vec<String> = world.get::<PhysicsBody>(entity).map(|b| b.0.forces().iter().map(|f| f.name.clone()).collect()).unwrap_or_default();
            let available: Vec<String> = world.resource::<ForceManager>().forces().iter().map(|f| f.name.clone()).filter(|n| !applied.contains(n)).collect();
            with_ctx(world, |ctx| dialogs::attach_physics(ctx, available, applied));
        }
        Action::AddForce => {
            if let Some(window) = window {
                move_force(world, window, true);
            }
        }
        Action::RemoveForce => {
            if let Some(window) = window {
                move_force(world, window, false);
            }
        }
        Action::AddWorldForcesMenu => {
            with_ctx(world, dialogs::world_force);
        }
        Action::AddWorldForce => {
            if let Some(window) = window {
                add_world_force(world, window);
            }
        }
    }
}

fn close_window(world: &mut World, window: Entity) {
    set_command(world, Command::None);
    if let Some(&VideoWindow(index)) = world.get::<VideoWindow>(window) {
        if let Some(rig) = world.resource_mut::<CameraManager>().cameras.get_mut(index) {
            rig.viewport_on = false;
            rig.video_window = None;
        }
        let cameras: Vec<Entity> = world.query::<(Entity, &QuickCam)>().iter(world).filter(|(_, q)| q.index == index).map(|(e, _)| e).collect();
        for camera in cameras {
            world.entity_mut(camera).despawn_recursive();
        }
    }
    let mut ui = world.resource_mut::<UiState>();
    if ui.focused_window == Some(window) {
        ui.focused_window = None;
    }
    let focus = ui.text_focus;
    if focus.is_some_and(|f| in_window(world, f, window)) {
        world.resource_mut::<UiState>().text_focus = None;
    }
    if let Ok(entity) = world.get_entity_mut(window) {
        entity.despawn_recursive();
    }
}

fn open_file_dialog(world: &mut World, title: &str, purpose: Command, ok: &str) {
    if open_dialog_allowed(world) {
        set_command(world, purpose);
        with_ctx(world, |ctx| dialogs::file_dialog(ctx, title, purpose, ok));
    }
}

/// OK in a file dialog: open or save the chosen file.
fn accept_file(world: &mut World, window: Entity) {
    let name = field_text(world, window, "filename");
    let lists: Vec<Entity> = world.query_filtered::<Entity, With<FileBrowser>>().iter(world).collect();
    let Some(list) = lists.into_iter().find(|&l| in_window(world, l, window)) else { return };
    let (dir, purpose) = {
        let browser = world.get::<FileBrowser>(list).expect("queried above");
        (browser.dir.clone(), browser.purpose)
    };
    let Some(path) = dialogs::chosen_file(dir, &name) else { return };
    close_window(world, window);

    let stem = path.file_stem().and_then(|s| s.to_str()).unwrap_or_default().to_owned();
    let result = match purpose {
        Command::RequestLoadModel => {
            let opened = if path.is_file() { model::open_path(world, &path) } else { model::open(world, None, &stem) };
            opened.map(|_| Vec::new())
        }
        Command::RequestLoadScene => {
            deselect(world);
            if path.is_file() { model::load_scene(world, &path) } else { model::load_bundled_scene(world, &stem) }
        }
        Command::RequestSaveScene => model::save_scene(world, &path).map(|()| Vec::new()),
        _ => return,
    };
    match result {
        Ok(problems) if problems.is_empty() => {}
        Ok(problems) => {
            let text = problems.iter().map(ToString::to_string).collect::<Vec<_>>().join("\n");
            message_box(world, "Some models could not be loaded", &text);
        }
        Err(e) => message_box(world, "Error", &e.to_string()),
    }
}

fn apply_transform(world: &mut World, window: Entity, entity: Entity) {
    let value = |world: &mut World, name| field_number(world, window, name);
    let rotate = [value(world, "xRotate"), value(world, "yRotate"), value(world, "zRotate")];
    let mut scale = Vec3::new(value(world, "xScale"), value(world, "yScale"), value(world, "zScale"));
    let translate = Vec3::new(value(world, "xTrans"), value(world, "yTrans"), value(world, "zTrans"));
    for s in scale.as_mut() {
        if *s == 0.0 {
            *s = 0.001;
        }
    }

    if let Some(mut transform) = world.get_mut::<Transform>(entity) {
        // Rotations are applied about the world axes, X then Y then Z.
        let [x, y, z] = rotate.map(f32::to_radians);
        transform.rotation = (Quat::from_rotation_z(z) * Quat::from_rotation_y(y) * Quat::from_rotation_x(x) * transform.rotation).normalize();
        transform.scale = scale;
        transform.translation = translate;
    }
    // "Rotate By" is relative, so clear it to avoid re-applying on the next Apply.
    for name in ["xRotate", "yRotate", "zRotate"] {
        if let Some(mut text_box) = field_entity(world, window, name).and_then(|e| world.get_mut::<TextBox>(e)) {
            text_box.set_text("0");
        }
    }
}

fn set_view(world: &mut World, view_type: ViewType) {
    let camera = world.resource::<ViewPort>().camera;
    world.resource_scope(|world, mut view: Mut<ViewPort>| {
        world.resource_scope(|world, mut settings: Mut<Settings>| {
            let mut cameras = world.resource_mut::<CameraManager>();
            if let Some(rig) = cameras.cameras.get_mut(camera) {
                view.set_view(view_type, rig, &mut settings);
            }
        });
    });
    main_menu_bar(world, |bar| menu::check_view(bar, view_type));
}

fn list_entity(world: &mut World, window: Entity, field: &str) -> Option<Entity> {
    field_entity(world, window, field).filter(|&e| world.get::<ListBox>(e).is_some())
}

/// `>>` attaches the selected available force to the object; `<<` removes it.
fn move_force(world: &mut World, window: Entity, attach: bool) {
    let (from, to) = if attach { ("forces", "applied") } else { ("applied", "forces") };
    let (Some(from), Some(to)) = (list_entity(world, window, from), list_entity(world, window, to)) else { return };
    let Some(name) = world.get::<ListBox>(from).and_then(|l| l.selected_item().map(str::to_owned)) else { return };
    let Some((entity, _)) = selected_object(world) else { return };

    if attach {
        let Some(force) = world.resource::<ForceManager>().by_name(&name).cloned() else { return };
        if world.get::<PhysicsBody>(entity).is_none() {
            let mut node = PNode::new(DEFAULT_MASS);
            node.set_position(world.get::<Transform>(entity).map_or(DVec3::ZERO, |t| t.translation.as_dvec3()));
            world.entity_mut(entity).insert(PhysicsBody(node));
        }
        if let Some(mut body) = world.get_mut::<PhysicsBody>(entity) {
            body.0.add_force(force);
        }
    } else if let Some(mut body) = world.get_mut::<PhysicsBody>(entity) {
        body.0.remove_force(&name);
    }

    if let Some(mut list) = world.get_mut::<ListBox>(from) {
        list.items.retain(|i| *i != name);
        list.selected = None;
    }
    if let Some(mut list) = world.get_mut::<ListBox>(to) {
        list.items.push(name);
    }
}

fn add_world_force(world: &mut World, window: Entity) {
    let name = field_text(world, window, "name").trim().to_owned();
    if name.is_empty() {
        message_box(world, "World Forces", "Give the force a name.");
        return;
    }
    let value = |world: &mut World, field| f64::from(field_number(world, window, field));
    let vector = DVec3::new(value(world, "vX"), value(world, "vY"), value(world, "vZ"));
    let duration = value(world, "duration").max(0.0);
    let added = world.resource_mut::<ForceManager>().add(crate::physics::Force::new(name.clone(), vector, duration));
    if added {
        close_window(world, window);
    } else {
        message_box(world, "World Forces", &format!("There is already a force called '{name}'."));
    }
}

/// Shows a picture-in-picture view from camera `index`.
fn open_video_window(world: &mut World, index: usize) {
    let already = world.resource::<CameraManager>().cameras.get(index).is_none_or(|r| r.video_window.is_some());
    if let Some(rig) = world.resource_mut::<CameraManager>().cameras.get_mut(index) {
        rig.viewport_on = true;
    }
    if already {
        return;
    }

    let mut image = Image::new_fill(
        Extent3d { width: QUICK_VIEW_SIZE, height: QUICK_VIEW_SIZE, depth_or_array_layers: 1 },
        TextureDimension::D2,
        &[0, 0, 0, 255],
        TextureFormat::Bgra8UnormSrgb,
        RenderAssetUsages::default(),
    );
    image.texture_descriptor.usage = TextureUsages::TEXTURE_BINDING | TextureUsages::COPY_DST | TextureUsages::RENDER_ATTACHMENT;
    let handle = world.resource_mut::<Assets<Image>>().add(image);

    let background = world.resource::<Settings>().grid.back_color();
    let fov = world.resource::<ViewPort>().fov.to_radians();
    world.spawn((
        QuickCam { index },
        Camera3d::default(),
        Camera { target: RenderTarget::Image(handle.clone()), order: -1, clear_color: ClearColorConfig::Custom(background), ..default() },
        Projection::Perspective(PerspectiveProjection { fov, near: 1.0, far: 8000.0, aspect_ratio: 1.0 }),
        Tonemapping::None,
        RenderLayers::layer(model::quick_camera_layer(index.saturating_sub(1))),
    ));

    let name = world.resource::<CameraManager>().cameras[index].name.clone();
    let window = with_ctx(world, |ctx| dialogs::video_window(ctx, &name, handle));
    if let Some(window) = window {
        world.entity_mut(window).insert(VideoWindow(index));
        world.resource_mut::<CameraManager>().cameras[index].video_window = Some(window);
    }
}

/// Keeps quick cameras looking through their camera objects.
pub fn update_quick_cameras(cameras: Res<CameraManager>, view: Res<ViewPort>, mut quick: Query<(&QuickCam, &mut Transform, &mut Projection)>) {
    for (quick, mut transform, mut projection) in &mut quick {
        let Some(rig) = cameras.cameras.get(quick.index) else { continue };
        *transform = Transform::from_translation(rig.pos).looking_to(rig.direction(), Vec3::Y);
        if let Projection::Perspective(p) = &mut *projection {
            p.fov = view.fov.to_radians();
        }
    }
}

/// Right-click: open a context menu that matches the selection.
#[allow(clippy::too_many_arguments)]
pub fn context_menu_on_right_click(
    mouse: Res<ButtonInput<MouseButton>>,
    ui: Res<UiState>,
    selection: Res<Selection>,
    cameras: Res<CameraManager>,
    windows: Query<&Window, With<bevy::window::PrimaryWindow>>,
    camera: Query<Entity, With<MainCamera>>,
    existing: Query<(Entity, &MenuBar)>,
    mut commands: Commands,
) {
    if !mouse.just_pressed(MouseButton::Right) || ui.active {
        return;
    }
    let (Some(pointer), Ok(camera)) = (menu::cursor_position(&windows), camera.get_single()) else { return };
    if menu::in_menu_tab_area(pointer) {
        return;
    }
    for (entity, bar) in &existing {
        if bar.is_context {
            commands.entity(entity).despawn_recursive();
        }
    }

    let target = match selection.entity {
        _ if selection.on_handle => ContextTarget::Manipulator,
        Some(e) if cameras.is_camera_model(e).is_some() => ContextTarget::Camera,
        Some(_) => ContextTarget::Object,
        None => ContextTarget::Nothing,
    };
    let headers = menu::context_menu(target);
    let at = windows.get_single().map_or(pointer, |w| menu::keep_on_screen(pointer, &headers, Vec2::new(w.width(), w.height())));
    menu::spawn_context_menu(&mut commands, camera, at, headers);
}

/// Hides the whole UI while a camera is being controlled.
pub fn hide_ui_in_camera_control(settings: Res<Settings>, mut roots: Query<&mut Node, With<super::UiRoot>>) {
    let display = if settings.system == SystemState::CameraControl { Display::None } else { Display::Flex };
    for mut node in &mut roots {
        if node.display != display {
            node.display = display;
        }
    }
}
