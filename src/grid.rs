//! The holographic grid: a box of grid lines around the origin that grows
//! and shrinks when toggled, plus the always-on axis names and the inner
//! grid lines shown in orthographic views.

use bevy::prelude::*;
use bevy::render::mesh::PrimitiveTopology;
use bevy::render::primitives::Aabb;
use bevy::asset::RenderAssetUsages;

use crate::labels::{LabelSet, WorldLabels};
use crate::objects::model::visible_to_all;
use crate::settings::{GridSettings, GridState, Settings};
use crate::view::ViewPort;


#[derive(Component)]
pub struct GridBody;

#[derive(Resource, Default)]
struct BuiltFor(Option<([u32; 5], [u8; 3], [u8; 3])>);

pub struct GridPlugin;

impl Plugin for GridPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<BuiltFor>()
            .add_systems(Startup, spawn_body)
            .add_systems(Update, (animate_grid, rebuild_body, draw_inner_lines, queue_axis_labels.in_set(LabelSet::Produce)).chain());
    }
}

fn quad(out: &mut Vec<[f32; 3]>, corners: [[f32; 3]; 4]) {
    // Two triangles per quad.
    out.extend([corners[0], corners[1], corners[2], corners[0], corners[2], corners[3]]);
}

/// Builds the room: box faces and grid strips, centred horizontally with the
/// floor at y = 0 (the entity is placed at the floor height).
pub fn build_body_mesh(grid: &GridSettings) -> Mesh {
    let (w, h, d) = (grid.width, grid.height, grid.depth);
    let (spacing, thick) = (grid.spacing.max(0.1), grid.thickness);
    // Faces sit just outside the strips so the strips are never hidden.
    let gap = (thick * 2.0).max(0.05);
    let back = grid.back_color().to_linear().to_f32_array();
    let line = grid.line_color().to_linear().to_f32_array();

    let mut positions: Vec<[f32; 3]> = Vec::new();
    let mut colors: Vec<[f32; 4]> = Vec::new();
    let mut push = |quads: Vec<[[f32; 3]; 4]>, color: [f32; 4], positions: &mut Vec<[f32; 3]>| {
        for q in quads {
            let before = positions.len();
            quad(positions, q);
            colors.extend(std::iter::repeat_n(color, positions.len() - before));
        }
    };

    let (lo, wx, hy, dz) = (-gap, w + gap, h + gap, d + gap);
    let faces = vec![
        [[lo, lo, lo], [wx, lo, lo], [wx, lo, dz], [lo, lo, dz]],
        [[lo, lo, lo], [wx, lo, lo], [wx, hy, lo], [lo, hy, lo]],
        [[lo, lo, dz], [wx, lo, dz], [wx, hy, dz], [lo, hy, dz]],
        [[lo, lo, lo], [lo, hy, lo], [lo, hy, dz], [lo, lo, dz]],
        [[wx, lo, lo], [wx, hy, lo], [wx, hy, dz], [wx, lo, dz]],
        [[wx, hy, lo], [wx, hy, dz], [lo, hy, dz], [lo, hy, lo]],
    ];
    push(faces, back, &mut positions);

    let steps = |extent: f32| {
        let count = (extent / spacing).floor() as usize;
        (0..=count).map(|i| i as f32 * spacing).collect::<Vec<_>>()
    };
    let mut strips = Vec::new();
    // Each face of the room gets strips along both of its axes.
    for &y in &[0.0, h] {
        for x in steps(w) {
            strips.push([[x, y, 0.], [x, y, d], [x + thick, y, d], [x + thick, y, 0.]]);
        }
        for z in steps(d) {
            strips.push([[0., y, z], [w, y, z], [w, y, z + thick], [0., y, z + thick]]);
        }
    }
    for &z in &[0.0, d] {
        for x in steps(w) {
            strips.push([[x, 0., z], [x, h, z], [x + thick, h, z], [x + thick, 0., z]]);
        }
        for y in steps(h) {
            strips.push([[0., y, z], [w, y, z], [w, y + thick, z], [0., y + thick, z]]);
        }
    }
    for &x in &[0.0, w] {
        for z in steps(d) {
            strips.push([[x, 0., z], [x, h, z], [x, h, z + thick], [x, 0., z + thick]]);
        }
        for y in steps(h) {
            strips.push([[x, y, 0.], [x, y, d], [x, y + thick, d], [x, y + thick, 0.]]);
        }
    }
    push(strips, line, &mut positions);

    let offset = Vec3::new(w / 2.0, 0.0, d / 2.0);
    let positions: Vec<[f32; 3]> = positions.into_iter().map(|p| (Vec3::from(p) - offset).to_array()).collect();

    Mesh::new(PrimitiveTopology::TriangleList, RenderAssetUsages::default())
        .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, positions)
        .with_inserted_attribute(Mesh::ATTRIBUTE_COLOR, colors)
}

fn spawn_body(mut commands: Commands, mut meshes: ResMut<Assets<Mesh>>, mut materials: ResMut<Assets<StandardMaterial>>, settings: Res<Settings>) {
    commands.spawn((
        GridBody,
        Mesh3d(meshes.add(build_body_mesh(&settings.grid))),
        MeshMaterial3d(materials.add(StandardMaterial {
            base_color: Color::WHITE,
            unlit: true,
            double_sided: true,
            cull_mode: None,
            ..default()
        })),
        Transform::from_xyz(0.0, settings.grid.floor, 0.0).with_scale(Vec3::new(1.0, 0.0001, 1.0)),
        Visibility::Hidden,
        visible_to_all(),
        // The body is huge; never cull it.
        Aabb::from_min_max(Vec3::splat(-1e6), Vec3::splat(1e6)),
    ));
}

/// Grows/shrinks the box and decides when it is drawn.
fn animate_grid(
    time: Res<Time>,
    mut settings: ResMut<Settings>,
    view: Res<ViewPort>,
    mut body: Query<(&mut Transform, &mut Visibility), With<GridBody>>,
    mut clear: ResMut<ClearColor>,
) {
    let grid = &mut settings.grid;
    let dt = time.delta_secs();
    match grid.state {
        GridState::Enable => {
            grid.dynamic_scale += dt;
            if grid.dynamic_scale >= grid.scale {
                grid.state = GridState::On;
                grid.dynamic_scale = grid.scale;
            }
        }
        GridState::Disable => {
            grid.dynamic_scale -= dt;
            if grid.dynamic_scale <= 0.0 {
                grid.state = GridState::Off;
                grid.dynamic_scale = 0.0;
            }
        }
        GridState::On | GridState::Off => {}
    }

    clear.0 = grid.back_color();

    let drawn = match grid.state {
        GridState::On => view.is_perspective(),
        GridState::Enable | GridState::Disable => true,
        GridState::Off => false,
    };
    if let Ok((mut transform, mut visibility)) = body.get_single_mut() {
        // Grows up from the floor.
        transform.translation.y = grid.floor;
        transform.scale = Vec3::new(grid.scale, grid.dynamic_scale.max(0.0001), grid.scale);
        *visibility = if drawn { Visibility::Inherited } else { Visibility::Hidden };
    }
}

/// Regenerates the mesh when colours, spacing or size change.
fn rebuild_body(
    settings: Res<Settings>,
    mut built: ResMut<BuiltFor>,
    body: Query<&Mesh3d, With<GridBody>>,
    mut meshes: ResMut<Assets<Mesh>>,
) {
    let g = &settings.grid;
    let signature = ([g.width, g.height, g.depth, g.spacing, g.thickness].map(f32::to_bits), g.color, g.back_color);
    if built.0.is_none() {
        built.0 = Some(signature);
        return;
    }
    if built.0 == Some(signature) {
        return;
    }
    if let Ok(mesh) = body.get_single() {
        if let Some(target) = meshes.get_mut(&mesh.0) {
            *target = build_body_mesh(g);
            built.0 = Some(signature);
        }
    }
}

/// Grid lines through the origin, drawn in orthographic views and when the
/// inner grid is switched on. They span the room.
fn draw_inner_lines(mut gizmos: Gizmos, settings: Res<Settings>, view: Res<ViewPort>) {
    if !(settings.inner_grid || !view.is_perspective()) {
        return;
    }
    let g = &settings.grid;
    let (hx, hz) = (g.width / 2.0, g.depth / 2.0);
    let (bottom, top) = (g.floor, g.ceiling());
    let steps = |lo: f32, hi: f32| {
        let spacing = g.spacing.max(0.1);
        let first = (lo / spacing).ceil() as i64;
        let last = (hi / spacing).floor() as i64;
        (first..=last).map(move |k| k as f32 * spacing).filter(|v| *v != 0.0)
    };

    let green = Color::srgb_u8(0, 255, 0);
    let red = Color::srgb_u8(255, 0, 0);
    let blue = Color::srgb_u8(0, 0, 255);
    for x in steps(-hx, hx) {
        gizmos.line(Vec3::new(x, bottom, 0.0), Vec3::new(x, top, 0.0), green); // XY plane
        gizmos.line(Vec3::new(x, 0.0, -hz), Vec3::new(x, 0.0, hz), blue); // XZ plane
    }
    for y in steps(bottom, top) {
        gizmos.line(Vec3::new(-hx, y, 0.0), Vec3::new(hx, y, 0.0), red); // XY plane
        gizmos.line(Vec3::new(0.0, y, -hz), Vec3::new(0.0, y, hz), blue); // YZ plane
    }
    for z in steps(-hz, hz) {
        gizmos.line(Vec3::new(0.0, bottom, z), Vec3::new(0.0, top, z), green); // YZ plane
        gizmos.line(Vec3::new(-hx, 0.0, z), Vec3::new(hx, 0.0, z), red); // XZ plane
    }
    gizmos.line(Vec3::new(-hx, 0.0, 0.0), Vec3::new(hx, 0.0, 0.0), Color::WHITE);
    gizmos.line(Vec3::new(0.0, bottom, 0.0), Vec3::new(0.0, top, 0.0), Color::WHITE);
    gizmos.line(Vec3::new(0.0, 0.0, -hz), Vec3::new(0.0, 0.0, hz), Color::WHITE);
}

/// Axis names where each axis meets the walls, floor and ceiling.
fn queue_axis_labels(mut labels: ResMut<WorldLabels>, settings: Res<Settings>) {
    let g = &settings.grid;
    let (hx, hz) = (g.width / 2.0, g.depth / 2.0);
    for (position, name) in [
        (Vec3::new(hx, 0.0, 0.0), "X"),
        (Vec3::new(-hx, 0.0, 0.0), "X"),
        (Vec3::new(0.0, g.ceiling(), 0.0), "Y"),
        (Vec3::new(0.0, g.floor, 0.0), "Y"),
        (Vec3::new(0.0, 0.0, hz), "Z"),
        (Vec3::new(0.0, 0.0, -hz), "Z"),
    ] {
        labels.add(position, name, Color::WHITE);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn body_mesh_is_centred_and_coloured_per_vertex() {
        let grid = GridSettings::default();
        let mesh = build_body_mesh(&grid);
        let n = mesh.count_vertices();
        assert!(n > 36, "faces plus strips");
        assert_eq!(n % 3, 0);
        let bevy::render::mesh::VertexAttributeValues::Float32x3(p) = mesh.attribute(Mesh::ATTRIBUTE_POSITION).unwrap() else { panic!() };
        let min = p.iter().fold(Vec3::MAX, |m, v| m.min(Vec3::from(*v)));
        let max = p.iter().fold(Vec3::MIN, |m, v| m.max(Vec3::from(*v)));
        // Centred horizontally, standing on y = 0 (the entity sits at the
        // floor), faces a hair outside the room.
        let gap = grid.thickness * 2.0;
        assert_eq!(min, Vec3::new(-grid.width / 2.0 - gap, -gap, -grid.depth / 2.0 - gap));
        assert!((max.y - (grid.height + gap)).abs() < 1e-4);
        assert!(max.x >= grid.width / 2.0 && max.x < grid.width / 2.0 + 1.0);
        assert_eq!(mesh.attribute(Mesh::ATTRIBUTE_COLOR).unwrap().len(), n);
    }

    #[test]
    fn denser_grids_have_more_strips() {
        let mut grid = GridSettings::default();
        let sparse = build_body_mesh(&grid).count_vertices();
        grid.spacing = 5.0;
        assert!(build_body_mesh(&grid).count_vertices() > sparse);
    }
}
