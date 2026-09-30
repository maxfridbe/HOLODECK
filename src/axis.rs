//! The small axis indicator in the corner of the screen: three coloured
//! lines that turn with the view, labelled X, Y and Z.

use bevy::core_pipeline::tonemapping::Tonemapping;
use bevy::prelude::*;
use bevy::render::camera::{ScalingMode, Viewport};
use bevy::render::view::RenderLayers;
use bevy::window::PrimaryWindow;

use crate::view::MainCamera;

const LAYER: usize = 15;
/// Size and margin of the indicator, in logical pixels.
const SIZE: f32 = 50.0;
const MARGIN: f32 = 10.0;
/// Half-width of the indicator's view, and the length of each axis.
const REACH: f32 = 10.0;

#[derive(Component)]
struct AxisCamera;

#[derive(Component)]
struct AxisLabel(usize);

#[derive(Default, Reflect, GizmoConfigGroup)]
struct AxisGizmos;

fn axes() -> [(Vec3, Color, &'static str); 3] {
    [
        (Vec3::X, Color::srgb_u8(255, 0, 0), "X"),
        (Vec3::Y, Color::srgb_u8(0, 255, 0), "Y"),
        (Vec3::Z, Color::srgb_u8(0, 128, 255), "Z"),
    ]
}

pub struct AxisPlugin;

impl Plugin for AxisPlugin {
    fn build(&self, app: &mut App) {
        app.init_gizmo_group::<AxisGizmos>()
            .add_systems(Startup, spawn.after(crate::spawn_main_camera))
            .add_systems(Update, (draw, place_labels))
            // After everything that can resize the window this frame, and after
            // cameras have picked up the new target size.
            .add_systems(PostUpdate, place_camera.after(bevy::render::camera::CameraUpdateSystem));
    }
}

fn spawn(mut commands: Commands, mut store: ResMut<GizmoConfigStore>, main: Query<Entity, With<MainCamera>>) {
    let (config, _) = store.config_mut::<AxisGizmos>();
    config.render_layers = RenderLayers::layer(LAYER);
    config.line_width = 3.0;

    commands.spawn((
        AxisCamera,
        Camera3d::default(),
        Camera { order: 1, clear_color: ClearColorConfig::None, ..default() },
        Projection::Orthographic(OrthographicProjection {
            near: 0.1,
            far: 200.0,
            scaling_mode: ScalingMode::Fixed { width: 2.0 * REACH, height: 2.0 * REACH },
            ..OrthographicProjection::default_3d()
        }),
        Tonemapping::None,
        RenderLayers::layer(LAYER),
    ));

    let Ok(main) = main.get_single() else { return };
    for (i, (_, color, name)) in axes().into_iter().enumerate() {
        commands.spawn((
            AxisLabel(i),
            Text::new(name),
            TextFont { font_size: 13.0, ..default() },
            TextColor(color),
            Node { position_type: PositionType::Absolute, ..default() },
            GlobalZIndex(-10),
            TargetCamera(main),
            PickingBehavior::IGNORE,
        ));
    }
}

/// Keeps the indicator in the bottom-left corner, looking the same way as the main camera.
fn place_camera(
    windows: Query<&Window, With<PrimaryWindow>>,
    ui_scale: Res<UiScale>,
    main: Query<&Transform, (With<MainCamera>, Without<AxisCamera>)>,
    mut axis: Query<(&mut Camera, &mut Transform), With<AxisCamera>>,
) {
    let (Ok(window), Ok(main), Ok((mut camera, mut transform))) = (windows.get_single(), main.get_single(), axis.get_single_mut()) else { return };
    // The viewport must lie inside the render target, which is the window
    // as it will be drawn this frame. Take the smaller of the window and the
    // camera's own idea of its target, so a resize (window, browser, phone
    // rotation) can never leave the viewport hanging off the edge.
    let target = UVec2::new(window.physical_width(), window.physical_height());
    let target = camera.physical_target_size().map_or(target, |t| t.min(target));
    match inset_viewport(target, window.scale_factor() * ui_scale.0) {
        Some(viewport) => {
            camera.is_active = true;
            camera.viewport = Some(viewport);
        }
        None => {
            camera.is_active = false;
            camera.viewport = None;
        }
    }
    *transform = Transform::from_translation(main.rotation * Vec3::new(0.0, 0.0, 50.0)).with_rotation(main.rotation);
}

/// The bottom-left square for the indicator, or `None` if it does not fit.
fn inset_viewport(target: UVec2, scale: f32) -> Option<Viewport> {
    let size = (SIZE * scale).round() as u32;
    let margin = (MARGIN * scale).round() as u32;
    if size == 0 || target.x < size + margin || target.y < size + margin {
        return None;
    }
    Some(Viewport { physical_position: UVec2::new(margin, target.y - size - margin), physical_size: UVec2::splat(size), ..default() })
}

fn draw(mut gizmos: Gizmos<AxisGizmos>) {
    for (axis, color, _) in axes() {
        gizmos.line(Vec3::ZERO, axis * REACH, color);
    }
}

fn place_labels(
    windows: Query<&Window, With<PrimaryWindow>>,
    ui_scale: Res<UiScale>,
    axis: Query<(&Camera, &Transform), With<AxisCamera>>,
    mut labels: Query<(&AxisLabel, &mut Node, &mut Visibility)>,
) {
    let (Ok(window), Ok((camera, transform))) = (windows.get_single(), axis.get_single()) else { return };
    // In UI units, like the label nodes.
    let origin = Vec2::new(MARGIN, window.height() / ui_scale.0 - SIZE - MARGIN);
    let global = GlobalTransform::from(*transform);
    for (label, mut node, mut visibility) in &mut labels {
        let (direction, ..) = axes()[label.0];
        match camera.world_to_viewport(&global, direction * REACH * 1.05) {
            Ok(p) if camera.is_active => {
                let p = p / ui_scale.0;
                node.left = Val::Px(origin.x + p.x - 4.0);
                node.top = Val::Px(origin.y + p.y - 8.0);
                *visibility = Visibility::Inherited;
            }
            _ => *visibility = Visibility::Hidden,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn the_inset_always_lies_inside_the_target() {
        for (w, h, scale) in [(1280, 720, 1.0), (915, 412, 1.0), (2400, 1080, 2.625), (60, 60, 1.0), (5000, 3000, 4.0)] {
            if let Some(v) = inset_viewport(UVec2::new(w, h), scale) {
                let end = v.physical_position + v.physical_size;
                assert!(end.x <= w && end.y <= h, "{w}x{h}@{scale}: {v:?}");
            }
        }
        assert!(inset_viewport(UVec2::new(40, 40), 1.0).is_none());
    }
}
