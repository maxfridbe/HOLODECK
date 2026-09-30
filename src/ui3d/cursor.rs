//! The original's own mouse cursor (`Cursor3d`): a white corner made of
//! 2-pixel bars, 15 pixels each way, with three smaller corners nested
//! inside it and a black shadow one pixel down and right. The system cursor
//! is hidden. On touch screens there is no cursor.

use bevy::prelude::*;
use bevy::window::PrimaryWindow;

use super::UiState;
use crate::touch::Pointer;
use crate::view::MainCamera;

#[derive(Component)]
struct CursorRoot;

/// The cursor's bars as `(x, y, width, height)` from the hot spot.
pub fn cursor_bars() -> Vec<(f32, f32, f32, f32)> {
    let mut bars = vec![(0.0, 0.0, 2.0, 15.0), (2.0, 0.0, 13.0, 2.0)];
    for step in 1..=3 {
        let v = 4.0 * step as f32;
        bars.push((v, v, 2.0, 6.0));
        bars.push((v + 2.0, v, 4.0, 2.0));
    }
    bars
}

pub struct CursorPlugin;

impl Plugin for CursorPlugin {
    fn build(&self, app: &mut App) {
        app.add_systems(Startup, spawn.after(crate::spawn_main_camera)).add_systems(PostUpdate, follow);
    }
}

fn spawn(mut commands: Commands, camera: Query<Entity, With<MainCamera>>) {
    let Ok(camera) = camera.get_single() else { return };
    commands
        .spawn((
            CursorRoot,
            Node { position_type: PositionType::Absolute, width: Val::Px(17.0), height: Val::Px(17.0), ..default() },
            GlobalZIndex(i32::MAX),
            TargetCamera(camera),
            Visibility::Hidden,
            PickingBehavior::IGNORE,
        ))
        .with_children(|root| {
            for (offset, color) in [(1.0, Color::BLACK), (0.0, Color::WHITE)] {
                for (x, y, w, h) in cursor_bars() {
                    root.spawn((
                        Node { position_type: PositionType::Absolute, left: Val::Px(x + offset), top: Val::Px(y + offset), width: Val::Px(w), height: Val::Px(h), ..default() },
                        BackgroundColor(color),
                        PickingBehavior::IGNORE,
                    ));
                }
            }
        });
}

fn follow(
    pointer: Res<Pointer>,
    ui: Res<UiState>,
    scale: Res<UiScale>,
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
    mut cursor: Query<(&mut Node, &mut Visibility), With<CursorRoot>>,
) {
    let Ok(mut window) = windows.get_single_mut() else { return };
    let Ok((mut node, mut visibility)) = cursor.get_single_mut() else { return };
    // The drawn cursor replaces the system one (ShowCursor(false) in the
    // original), except while the pointer is captured for looking around.
    if window.cursor_options.visible {
        window.cursor_options.visible = false;
    }
    let position = window.cursor_position().filter(|_| !ui.touch_mode && !pointer.touch);
    match position {
        Some(p) => {
            // The cursor keeps its pixel size regardless of the UI scale.
            node.left = Val::Px(p.x / scale.0);
            node.top = Val::Px(p.y / scale.0);
            *visibility = Visibility::Inherited;
        }
        None => *visibility = Visibility::Hidden,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn cursor_matches_the_original_shape() {
        let bars = cursor_bars();
        // Outer corner: 15 tall and 15 wide.
        assert_eq!(bars[0], (0.0, 0.0, 2.0, 15.0));
        assert_eq!(bars[1].0 + bars[1].2, 15.0);
        // Three nested corners at 4, 8 and 12 pixels.
        assert_eq!(bars.len(), 8);
        assert_eq!(bars[6], (12.0, 12.0, 2.0, 6.0));
    }
}
