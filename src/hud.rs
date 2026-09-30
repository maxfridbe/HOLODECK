//! On-screen text: frame rate, controls hint and camera-control readout.

use bevy::prelude::*;

use crate::camera::CameraManager;
use crate::settings::{Settings, SystemState};
use crate::view::{MainCamera, ViewPort};

#[derive(Component)]
struct FpsText;
#[derive(Component)]
struct ControlText;
#[derive(Component)]
struct HintText;

/// Frames per second, refreshed once a second.
#[derive(Resource, Default)]
struct FrameStats {
    fps: u32,
    frames: u32,
    elapsed: f32,
}

impl FrameStats {
    fn tick(&mut self, dt: f32) {
        self.frames += 1;
        self.elapsed += dt;
        if self.elapsed >= 1.0 {
            self.fps = self.frames;
            self.frames = 0;
            self.elapsed = 0.0;
        }
    }
}

pub struct HudPlugin;

impl Plugin for HudPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<FrameStats>().add_systems(Startup, spawn.after(crate::spawn_main_camera)).add_systems(Update, update);
    }
}

fn spawn(mut commands: Commands, camera: Query<Entity, With<MainCamera>>) {
    let Ok(camera) = camera.get_single() else { return };
    let font = TextFont { font_size: 16.0, ..default() };
    commands.spawn((
        FpsText,
        Text::new("FPS: 0"),
        font.clone(),
        TextColor(Color::srgb(1.0, 0.0, 0.0)),
        Node { position_type: PositionType::Absolute, right: Val::Px(16.0), top: Val::Px(6.0), ..default() },
        TargetCamera(camera),
        PickingBehavior::IGNORE,
    ));
    commands.spawn((
        ControlText,
        Text::new(""),
        font.clone(),
        TextColor(Color::srgb(1.0, 0.0, 0.0)),
        Node { position_type: PositionType::Absolute, left: Val::Px(30.0), top: Val::Px(44.0), ..default() },
        TargetCamera(camera),
        PickingBehavior::IGNORE,
    ));
    commands.spawn((
        HintText,
        Text::new(DESKTOP_HINT),
        TextFont { font_size: 13.0, ..default() },
        TextColor(Color::srgba(0.75, 0.75, 0.75, 0.8)),
        Node { position_type: PositionType::Absolute, left: Val::Px(12.0), bottom: Val::Px(8.0), ..default() },
        TargetCamera(camera),
        PickingBehavior::IGNORE,
    ));
}

const DESKTOP_HINT: &str = "WASD fly   drag / Ctrl+mouse look   wheel zoom   Space menu   right-click context menu   click select   G grid   Q quit";
const TOUCH_HINT: &str = "tap select   drag look   long-press menu";

fn update(
    time: Res<Time>,
    ui: Res<crate::ui3d::UiState>,
    mut stats: ResMut<FrameStats>,
    settings: Res<Settings>,
    cameras: Res<CameraManager>,
    view: Res<ViewPort>,
    mut fps: Query<&mut Text, (With<FpsText>, Without<ControlText>)>,
    mut control: Query<&mut Text, (With<ControlText>, Without<FpsText>)>,
    mut hint: Query<(&mut Visibility, &mut Text, &mut Node), (With<HintText>, Without<FpsText>, Without<ControlText>)>,
) {
    stats.tick(time.delta_secs());
    if let Ok(mut text) = fps.get_single_mut() {
        text.0 = format!("FPS: {}", stats.fps);
    }
    let controlling = settings.system == SystemState::CameraControl;
    if let Ok(mut text) = control.get_single_mut() {
        text.0 = match cameras.cameras.get(view.camera) {
            Some(rig) if controlling => format!(
                "Camera Control Mode, Press ESC when camera is positioned.\nPhi: {:.1}\nTheta: {:.1}",
                rig.phi.to_degrees(),
                rig.theta.to_degrees()
            ),
            _ => String::new(),
        };
    }
    if let Ok((mut visibility, mut text, mut node)) = hint.get_single_mut() {
        // On narrow touch screens there is no room between the stick and buttons.
        let room = !ui.touch_mode || ui.screen.x >= 640.0;
        *visibility = if controlling || !room { Visibility::Hidden } else { Visibility::Inherited };
        // On touch screens the bottom corners belong to the stick and
        // buttons, so the (short) hint sits between them.
        let (wanted, left) = if ui.touch_mode { (TOUCH_HINT, Val::Px(190.0)) } else { (DESKTOP_HINT, Val::Px(12.0)) };
        if text.0 != wanted {
            text.0 = wanted.to_owned();
        }
        if node.left != left {
            node.left = left;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fps_updates_once_a_second() {
        let mut stats = FrameStats::default();
        for _ in 0..59 {
            stats.tick(1.0 / 60.0);
        }
        assert_eq!(stats.fps, 0);
        stats.tick(1.0 / 60.0 + 0.001);
        assert_eq!(stats.fps, 60);
    }
}
