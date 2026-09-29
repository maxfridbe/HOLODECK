//! Holodeck: a 3D scene editor and simulator.
//!
//! Fly around a holographic grid, load `.3dbin` models and `.world` scenes,
//! select objects and move, scale and rotate them with on-screen handles.
//!
//! This is a Rust/Bevy port of the original C++/OpenGL project; the sources
//! of the original live in `cpp/` for reference.

use bevy::core_pipeline::tonemapping::Tonemapping;
use bevy::prelude::*;
use bevy::render::view::RenderLayers;
use bevy_embedded_assets::{EmbeddedAssetPlugin, PluginMode};

pub mod axis;
pub mod camera;
pub mod data;
pub mod devtools;
pub mod effects;
pub mod grid;
pub mod hud;
pub mod input;
pub mod labels;
pub mod math;
pub mod net;
pub mod objects;
pub mod overlay;
pub mod physics;
pub mod settings;
pub mod ui3d;
pub mod view;
pub mod wireframe;

use camera::CameraManager;
use objects::manip::ManipulatorPlugin;
use objects::model::{self, Model};
use objects::pick::{self, Selection};
use physics::ForceManager;
use settings::Settings;
use view::{MainCamera, ViewPort};

/// Android entry point. cargo-apk builds the cdylib and NativeActivity
/// calls into this via the #[bevy_main] generated android_main.
#[bevy_main]
fn main() {
    run_game();
}

pub fn run_game() {
    let mut app = App::new();
    app.insert_resource(ClearColor(Color::BLACK))
        // EmbeddedAssetPlugin must be added BEFORE DefaultPlugins so the
        // embedded asset source is registered before the AssetServer starts.
        // ReplaceDefault makes plain load("...") paths resolve to the
        // embedded copies; the AutoLoad default only serves embedded:// URLs,
        // so on the web assets would be fetched over HTTP and 404.
        .add_plugins(EmbeddedAssetPlugin { mode: PluginMode::ReplaceDefault })
        .add_plugins(default_plugins())
        .init_resource::<Settings>()
        .init_resource::<ViewPort>()
        .init_resource::<CameraManager>()
        .init_resource::<Model>()
        .init_resource::<Selection>()
        .init_resource::<ForceManager>()
        .add_plugins((
            grid::GridPlugin,
            ManipulatorPlugin,
            ui3d::Ui3dPlugin,
            net::NetPlugin,
            input::InputPlugin,
            hud::HudPlugin,
            effects::EffectsPlugin,
            axis::AxisPlugin,
            wireframe::WireframePlugin,
            devtools::DevtoolsPlugin,
        ))
        .add_systems(Startup, (spawn_main_camera, setup_world.after(spawn_main_camera)))
        .add_systems(
            Update,
            (
                (pick::select_on_click, pick::drag_handle).chain(),
                camera::follow_models,
                view::sync_main_camera,
                model::step_physics,
                ui3d::actions::update_quick_cameras,
                ui3d::actions::context_menu_on_right_click,
                ui3d::actions::hide_ui_in_camera_control,
                ui3d::filebrowser::activate_entries,
                ui3d::filebrowser::show_selected_name,
            ),
        );
    labels::configure(&mut app);
    app.run();
}

fn default_plugins() -> impl PluginGroup {
    DefaultPlugins.set(WindowPlugin {
        primary_window: Some(Window {
            title: "Holodeck".into(),
            // Browser build: render into the canvas provided by
            // web/index.html and track its CSS size. Ignored on native.
            canvas: Some("#game-canvas".into()),
            fit_canvas_to_parent: true,
            prevent_default_event_handling: true,
            ..default()
        }),
        ..default()
    })
}

pub fn spawn_main_camera(mut commands: Commands) {
    commands.spawn((
        MainCamera,
        Camera3d::default(),
        // File colours were written straight to the framebuffer, so keep
        // the output untouched by tone mapping.
        Tonemapping::None,
        RenderLayers::layer(model::MAIN_LAYER),
        IsDefaultUiCamera,
    ));
}

/// Places the camera object the simulator starts with.
fn setup_world(world: &mut World) {
    if let Err(e) = camera::spawn_placed_camera(world, "Camera 1", Vec3::new(10.0, 10.0, 10.0)) {
        error!("{e}");
    }
}
