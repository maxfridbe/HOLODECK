//! A small windowing toolkit drawn with `bevy_ui`: a menu bar, context
//! menus, draggable windows, buttons, text boxes and list boxes, plus the
//! dialogs the simulator uses.
//!
//! Widgets carry a [`UiAction`] describing what they do. Clicking one sends
//! the action to [`actions::run_actions`], which performs it with full access
//! to the world (this replaces the original's function-pointer event
//! handlers and message queue).

use bevy::prelude::*;

pub mod actions;
pub mod dialogs;
pub mod filebrowser;
pub mod menu;
pub mod theme;
pub mod widgets;

pub use actions::UiAction;

use crate::view::MainCamera;

/// The one popup a menu command may currently have open, so commands cannot
/// stack windows on top of each other.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub enum Command {
    #[default]
    None,
    RequestExit,
    RequestWorldForces,
    RequestNew,
    RequestUnloadModel,
    RequestUnloadScene,
    RequestLoadModel,
    RequestLoadScene,
    RequestSaveScene,
    RequestEditObject,
    RequestGridSettings,
}

/// Shared UI state read by input handling.
#[derive(Resource, Default, Debug)]
pub struct UiState {
    /// A menu is open or a modal window is up or being dragged; the world
    /// should ignore the mouse and keyboard.
    pub active: bool,
    /// The pointer is over a window or menu.
    pub pointer_over_ui: bool,
    pub command: Command,
    /// The text box receiving keystrokes.
    pub text_focus: Option<Entity>,
    /// The window on top.
    pub focused_window: Option<Entity>,
    pub(crate) next_z: i32,
    pub(crate) drag: Option<widgets::WindowDrag>,
}

/// Marks every top-level UI node so the whole UI can be hidden (camera
/// control mode).
#[derive(Component)]
pub struct UiRoot;

pub struct Ui3dPlugin;

impl Plugin for Ui3dPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<UiState>()
            .add_event::<UiAction>()
            .add_event::<actions::ShowMessage>()
            .add_plugins((widgets::WidgetsPlugin, menu::MenuPlugin))
            .add_systems(Startup, menu::spawn_main_menu.after(crate::spawn_main_camera))
            .add_systems(Update, actions::run_actions)
            .add_systems(PreUpdate, scale_ui)
            .add_systems(PostUpdate, update_ui_state);
    }
}

/// The UI is laid out for a 1280x720 window and scaled to fit the actual
/// one, so it stays the same proportion of the screen at any resolution.
pub const REFERENCE_SIZE: Vec2 = Vec2::new(1280.0, 720.0);

pub fn ui_scale_for(window: Vec2) -> f32 {
    (window / REFERENCE_SIZE).min_element().clamp(0.5, 4.0)
}

fn scale_ui(windows: Query<&Window, With<bevy::window::PrimaryWindow>>, mut scale: ResMut<UiScale>) {
    let Ok(window) = windows.get_single() else { return };
    let wanted = ui_scale_for(Vec2::new(window.width(), window.height()));
    if (scale.0 - wanted).abs() > 1e-3 {
        scale.0 = wanted;
    }
}

/// Decides whether the UI currently wants the mouse and keyboard.
fn update_ui_state(
    mut state: ResMut<UiState>,
    windows: Query<(&widgets::Window3d, &Interaction)>,
    menus: Query<&menu::MenuBar>,
    interactions: Query<&Interaction, Or<(With<widgets::Window3d>, With<menu::MenuNode>)>>,
) {
    let menu_active = menus.iter().any(menu::MenuBar::is_active);
    let blocking_window = windows.iter().any(|(w, _)| w.kind.blocks_world());
    state.active = menu_active || blocking_window || state.drag.is_some();
    state.pointer_over_ui = interactions.iter().any(|i| *i != Interaction::None);
}

/// The camera entity UI nodes attach to.
pub fn ui_camera(query: &Query<Entity, With<MainCamera>>) -> Option<Entity> {
    query.get_single().ok()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ui_scales_with_the_window() {
        assert_eq!(ui_scale_for(REFERENCE_SIZE), 1.0);
        assert_eq!(ui_scale_for(Vec2::new(3840.0, 2160.0)), 3.0);
        // Limited by the tighter dimension, so everything still fits.
        assert_eq!(ui_scale_for(Vec2::new(2560.0, 720.0)), 1.0);
        assert_eq!(ui_scale_for(Vec2::new(200.0, 100.0)), 0.5);
    }
}
