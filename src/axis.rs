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
            .add_systems(Update, (place_camera, draw, place_labels).chain());
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
    main: Query<&Transform, (With<MainCamera>, Without<AxisCamera>)>,
    mut axis: Query<(&mut Camera, &mut Transform), With<AxisCamera>>,
) {
    let (Ok(window), Ok(main), Ok((mut camera, mut transform))) = (windows.get_single(), main.get_single(), axis.get_single_mut()) else { return };
    let scale = window.scale_factor();
    let size = (SIZE * scale) as u32;
    let position = UVec2::new((MARGIN * scale) as u32, window.physical_height().saturating_sub(size + (MARGIN * scale) as u32));
    if window.physical_width() < size || window.physical_height() < size {
        camera.viewport = None;
        camera.is_active = false;
        return;
    }
    camera.is_active = true;
    camera.viewport = Some(Viewport { physical_position: position, physical_size: UVec2::splat(size), ..default() });
    *transform = Transform::from_translation(main.rotation * Vec3::new(0.0, 0.0, 50.0)).with_rotation(main.rotation);
}

fn draw(mut gizmos: Gizmos<AxisGizmos>) {
    for (axis, color, _) in axes() {
        gizmos.line(Vec3::ZERO, axis * REACH, color);
    }
}

fn place_labels(
    windows: Query<&Window, With<PrimaryWindow>>,
    axis: Query<(&Camera, &Transform), With<AxisCamera>>,
    mut labels: Query<(&AxisLabel, &mut Node, &mut Visibility)>,
) {
    let (Ok(window), Ok((camera, transform))) = (windows.get_single(), axis.get_single()) else { return };
    let origin = Vec2::new(MARGIN, window.height() - SIZE - MARGIN);
    let global = GlobalTransform::from(*transform);
    for (label, mut node, mut visibility) in &mut labels {
        let (direction, ..) = axes()[label.0];
        match camera.world_to_viewport(&global, direction * REACH * 1.05) {
            Ok(p) if camera.is_active => {
                node.left = Val::Px(origin.x + p.x - 4.0);
                node.top = Val::Px(origin.y + p.y - 8.0);
                *visibility = Visibility::Inherited;
            }
            _ => *visibility = Visibility::Hidden,
        }
    }
}
