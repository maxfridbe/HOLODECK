//! On-screen text as the original printed it: the frame rate and the
//! camera-control readout.

use bevy::prelude::*;

use crate::camera::CameraManager;
use crate::settings::{Settings, SystemState};
use crate::ui3d::UiState;
use crate::ui3d::widgets::ui_font;
use crate::view::{MainCamera, ViewPort};

#[derive(Component)]
struct FpsText;
/// One of the three camera-control lines.
#[derive(Component)]
struct ControlText(usize);

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

/// The original printed text with its baseline at the given y; the font's
/// ascent is about 14 pixels.
const BASELINE: f32 = 14.0;
const RED: Color = Color::srgb(1.0, 0.0, 0.0);

fn spawn(mut commands: Commands, camera: Query<Entity, With<MainCamera>>) {
    let Ok(camera) = camera.get_single() else { return };
    // "FPS: n" in red, 128 pixels from the right edge, baseline 24.
    commands.spawn((
        FpsText,
        Text::new("FPS: 0"),
        ui_font(),
        TextColor(RED),
        Node { position_type: PositionType::Absolute, top: Val::Px(24.0 - BASELINE), ..default() },
        TargetCamera(camera),
        PickingBehavior::IGNORE,
    ));
    // Camera control mode: three red lines at x 30, baselines 44, 66 and 86.
    for (i, baseline) in [44.0, 66.0, 86.0].into_iter().enumerate() {
        commands.spawn((
            ControlText(i),
            Text::new(""),
            ui_font(),
            TextColor(RED),
            Node { position_type: PositionType::Absolute, left: Val::Px(30.0), top: Val::Px(baseline - BASELINE), ..default() },
            TargetCamera(camera),
            PickingBehavior::IGNORE,
        ));
    }
}

fn update(
    time: Res<Time>,
    ui: Res<UiState>,
    mut stats: ResMut<FrameStats>,
    settings: Res<Settings>,
    cameras: Res<CameraManager>,
    view: Res<ViewPort>,
    mut fps: Query<(&mut Text, &mut Node), (With<FpsText>, Without<ControlText>)>,
    mut control: Query<(&ControlText, &mut Text), Without<FpsText>>,
) {
    stats.tick(time.delta_secs());
    if let Ok((mut text, mut node)) = fps.get_single_mut() {
        text.0 = format!("FPS: {}", stats.fps);
        node.left = Val::Px(ui.screen.x - 128.0);
    }
    let controlling = settings.system == SystemState::CameraControl;
    let rig = cameras.cameras.get(view.camera);
    for (line, mut text) in &mut control {
        let wanted = match (controlling, rig) {
            (true, Some(rig)) => match line.0 {
                0 => "Camera Control Mode, Press ESC when camera is positioned.".to_owned(),
                1 => format!("Phi: {}", rig.phi.to_degrees() as f32),
                _ => format!("Theta: {}", rig.theta.to_degrees() as f32),
            },
            _ => String::new(),
        };
        if text.0 != wanted {
            text.0 = wanted;
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
