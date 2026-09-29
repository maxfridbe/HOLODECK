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

/// How far grid lines and axes extend from the origin.
const REACH: f32 = 1000.0;

#[derive(Component)]
pub struct GridBody;

#[derive(Resource, Default)]
struct BuiltFor(Option<(i32, i32, i32, i32, i32, [u8; 3], [u8; 3])>);

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

/// Builds the box faces and the grid strips, centred on the origin.
pub fn build_body_mesh(grid: &GridSettings) -> Mesh {
    let (w, h, d) = (grid.width as f32, grid.height as f32, grid.depth as f32);
    let (spacing, thick) = (grid.spacing.max(1) as usize, grid.thickness as f32);
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

    // Faces sit one unit outside the strips so the strips are never hidden.
    let faces = vec![
        [[-1., -1., -1.], [w + 1., -1., -1.], [w + 1., -1., d + 1.], [-1., -1., d + 1.]],
        [[-1., -1., -1.], [w + 1., -1., -1.], [w + 1., h + 1., -1.], [-1., h + 1., -1.]],
        [[-1., -1., d + 1.], [w + 1., -1., d + 1.], [w + 1., h + 1., d + 1.], [-1., h + 1., d + 1.]],
        [[-1., -1., -1.], [-1., h + 1., -1.], [-1., h + 1., d + 1.], [-1., -1., d + 1.]],
        [[w + 1., -1., -1.], [w + 1., h + 1., -1.], [w + 1., h + 1., d + 1.], [w + 1., -1., d + 1.]],
        [[w + 1., h + 1., -1.], [w + 1., h + 1., d + 1.], [-1., h + 1., d + 1.], [-1., h + 1., -1.]],
    ];
    push(faces, back, &mut positions);

    let steps = |extent: f32| (0..=extent as usize).step_by(spacing).map(|i| i as f32).collect::<Vec<_>>();
    let mut strips = Vec::new();
    // Each plane gets strips along both of its axes.
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

    let centre = Vec3::new(w, h, d) / 2.0;
    let positions: Vec<[f32; 3]> = positions.into_iter().map(|p| (Vec3::from(p) - centre).to_array()).collect();

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
        Transform::from_scale(Vec3::new(1.0, 0.0001, 1.0)),
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
    let signature = (g.width, g.height, g.depth, g.spacing, g.thickness, g.color, g.back_color);
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
/// inner grid is switched on.
fn draw_inner_lines(mut gizmos: Gizmos, settings: Res<Settings>, view: Res<ViewPort>) {
    if !(settings.inner_grid || !view.is_perspective()) {
        return;
    }
    let spacing = settings.grid.spacing.max(1) as f32;
    let offsets: Vec<f32> = (1..).map(|k| k as f32 * spacing).take_while(|&o| o < REACH).flat_map(|o| [o, -o]).collect();

    let green = Color::srgb_u8(0, 255, 0);
    let red = Color::srgb_u8(255, 0, 0);
    let blue = Color::srgb_u8(0, 0, 255);
    for &o in &offsets {
        // XY plane
        gizmos.line(Vec3::new(o, REACH, 0.0), Vec3::new(o, -REACH, 0.0), green);
        gizmos.line(Vec3::new(REACH, o, 0.0), Vec3::new(-REACH, o, 0.0), red);
        // YZ plane
        gizmos.line(Vec3::new(0.0, REACH, o), Vec3::new(0.0, -REACH, o), green);
        gizmos.line(Vec3::new(0.0, o, REACH), Vec3::new(0.0, o, -REACH), blue);
        // XZ plane
        gizmos.line(Vec3::new(o, 0.0, REACH), Vec3::new(o, 0.0, -REACH), blue);
        gizmos.line(Vec3::new(REACH, 0.0, o), Vec3::new(-REACH, 0.0, o), red);
    }
    for axis in [Vec3::X, Vec3::Y, Vec3::Z] {
        gizmos.line(axis * REACH, -axis * REACH, Color::WHITE);
    }
}

/// Axis names at both ends of each axis, drawn even when the box is off.
fn queue_axis_labels(mut labels: ResMut<WorldLabels>) {
    for (axis, name) in [(Vec3::X, "X"), (Vec3::Y, "Y"), (Vec3::Z, "Z")] {
        labels.add(axis * REACH, name, Color::WHITE);
        labels.add(-axis * REACH, name, Color::WHITE);
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
        // Faces extend one unit past the 1000-unit box; the last strip on
        // each axis is `thickness` (3) wide, so it pokes out to 503.
        assert_eq!(min, Vec3::splat(-501.0));
        assert_eq!(max, Vec3::splat(503.0));
        assert_eq!(mesh.attribute(Mesh::ATTRIBUTE_COLOR).unwrap().len(), n);
    }

    #[test]
    fn denser_grids_have_more_strips() {
        let mut grid = GridSettings::default();
        let sparse = build_body_mesh(&grid).count_vertices();
        grid.spacing = 50;
        assert!(build_body_mesh(&grid).count_vertices() > sparse);
    }
}
