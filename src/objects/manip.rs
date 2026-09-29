//! The model manipulator: translate, scale and rotate handles around the
//! selected object that can be dragged with the mouse.
//!
//! * Translate: arrows along the world axes.
//! * Scale: cubes along the object's own axes.
//! * Rotate: rings around the world axes.
//!
//! Handles keep a constant on-screen size (they scale with the camera's
//! distance) and are positioned relative to the object's *world-space*
//! bounds, so they hug the object wherever it is and however it is rotated.

use bevy::asset::RenderAssetUsages;
use bevy::prelude::*;
use bevy::render::mesh::PrimitiveTopology;

use super::BoundingBox;
use super::model::{MAIN_LAYER, ModelGeometry};
use crate::labels::{LabelSet, WorldLabels};
use crate::overlay::OverlayMaterial;
use crate::view::MainCamera;

/// Length of an arrow/cube handle, in unscaled handle units.
pub const BASE: f32 = 2.5;
/// Half-width of an arrow/cube handle and thickness of a ring.
pub const RAD: f32 = 0.25;
/// Handles are `distance / SCALE_DIVISOR` times their nominal size.
pub const SCALE_DIVISOR: f32 = 30.0;
const RING_SEGMENTS: usize = 40;
/// The rotate rings hug small objects but stop growing at this many handle
/// units, so a huge object never gets a screen-filling ring.
const RING_MAX_UNITS: f32 = 6.0;
/// Mouse grabbing is more forgiving than the drawn shapes.
const GRAB_MARGIN: f32 = 2.5;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ManipKind {
    Translate,
    Scale,
    Rotate,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Axis {
    X,
    Y,
    Z,
}

impl Axis {
    pub const ALL: [Axis; 3] = [Axis::X, Axis::Y, Axis::Z];

    pub fn index(self) -> usize {
        self as usize
    }

    pub fn unit(self) -> Vec3 {
        match self {
            Axis::X => Vec3::X,
            Axis::Y => Vec3::Y,
            Axis::Z => Vec3::Z,
        }
    }

    fn tint(self) -> Color {
        match self {
            Axis::X => Color::srgb_u8(255, 30, 30),
            Axis::Y => Color::srgb_u8(30, 255, 30),
            Axis::Z => Color::srgb_u8(30, 30, 255),
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Handle {
    pub kind: ManipKind,
    pub axis: Axis,
}

/// Where the handles for an object go this frame.
#[derive(Clone, Debug, PartialEq)]
pub struct Layout {
    /// Centre of the object's bounds in the world.
    pub center: Vec3,
    /// Size of one handle unit in world units (constant on screen).
    pub scale: f32,
    /// World direction of each handle.
    pub axes: [Vec3; 3],
    /// Distance from the centre to the tip of each arrow/cube.
    pub tip: [f32; 3],
    pub ring_radius: f32,
}

impl Layout {
    pub fn compute(kind: ManipKind, local_bounds: &BoundingBox, transform: &Transform, camera: Vec3) -> Self {
        let center = transform.transform_point(local_bounds.center());
        let scale = (camera.distance(center) / SCALE_DIVISOR).max(0.01);
        let margin = BASE * scale;

        let (axes, half) = match kind {
            ManipKind::Translate | ManipKind::Rotate => {
                ([Vec3::X, Vec3::Y, Vec3::Z], local_bounds.transformed(transform).half_extents())
            }
            ManipKind::Scale => (
                [transform.rotation * Vec3::X, transform.rotation * Vec3::Y, transform.rotation * Vec3::Z],
                (local_bounds.half_extents() * transform.scale).abs(),
            ),
        };

        Self {
            center,
            scale,
            axes,
            tip: [half.x + margin, half.y + margin, half.z + margin],
            ring_radius: half.max_element().min(RING_MAX_UNITS * scale) + margin * 0.5,
        }
    }
}

/// Distance between a ray and a segment, with the ray parameter of the
/// closest approach.
fn ray_segment(origin: Vec3, dir: Vec3, a: Vec3, b: Vec3) -> (f32, f32) {
    let seg = b - a;
    let len = seg.length();
    if len < 1e-6 {
        let s = (a - origin).dot(dir).max(0.0);
        return (origin.distance(a).min((origin + dir * s).distance(a)), s);
    }
    let seg_dir = seg / len;
    let t = ray_line_param(origin, dir, a, seg_dir).unwrap_or(0.0).clamp(0.0, len);
    let on_segment = a + seg_dir * t;
    let s = (on_segment - origin).dot(dir).max(0.0);
    ((origin + dir * s).distance(on_segment), s)
}

/// Where along the line `line_origin + t * line_dir` a ray passes closest.
/// `None` if the ray is parallel to the line. Directions must be normalised.
pub fn ray_line_param(origin: Vec3, dir: Vec3, line_origin: Vec3, line_dir: Vec3) -> Option<f32> {
    let w = origin - line_origin;
    let b = dir.dot(line_dir);
    let denom = 1.0 - b * b;
    if denom < 1e-6 {
        return None;
    }
    Some((line_dir.dot(w) - b * dir.dot(w)) / denom)
}

/// Where a ray crosses a plane (in front of the ray origin only).
pub fn ray_plane(origin: Vec3, dir: Vec3, point: Vec3, normal: Vec3) -> Option<Vec3> {
    let denom = normal.dot(dir);
    if denom.abs() < 1e-6 {
        return None;
    }
    let t = normal.dot(point - origin) / denom;
    (t > 0.0).then(|| origin + dir * t)
}

/// The nearest handle of `kind` under the ray, if any.
pub fn pick_handle(kind: ManipKind, layout: &Layout, origin: Vec3, dir: Vec3) -> Option<(Handle, f32)> {
    let grab = RAD * layout.scale * GRAB_MARGIN;
    Axis::ALL
        .into_iter()
        .filter_map(|axis| {
            let i = axis.index();
            let along = layout.axes[i];
            let (hit, distance) = match kind {
                ManipKind::Translate | ManipKind::Scale => {
                    let (dist, s) = ray_segment(origin, dir, layout.center, layout.center + along * layout.tip[i]);
                    (dist <= grab, s)
                }
                ManipKind::Rotate => {
                    let p = ray_plane(origin, dir, layout.center, along)?;
                    (((p - layout.center).length() - layout.ring_radius).abs() <= grab, origin.distance(p))
                }
            };
            hit.then_some((Handle { kind, axis }, distance))
        })
        .min_by(|a, b| a.1.total_cmp(&b.1))
}

/// Progress of a mouse drag on a handle.
#[derive(Clone, Copy, Debug)]
struct Drag {
    handle: Handle,
    /// Line (translate/scale) or plane (rotate) the drag is constrained to.
    anchor: Vec3,
    axis: Vec3,
    /// Last position along the line, or last direction in the plane.
    last: DragPoint,
}

#[derive(Clone, Copy, Debug)]
enum DragPoint {
    Along(f32),
    InPlane(Vec3),
}

#[derive(Resource, Default)]
pub struct Manipulator {
    /// Which handles to show; `None` hides them.
    pub kind: Option<ManipKind>,
    drag: Option<Drag>,
}

impl Manipulator {
    pub fn is_dragging(&self) -> bool {
        self.drag.is_some()
    }

    pub fn hide(&mut self) {
        self.kind = None;
        self.drag = None;
    }

    /// Starts dragging `handle` if the ray touches the constraint geometry.
    pub fn begin_drag(&mut self, handle: Handle, layout: &Layout, origin: Vec3, dir: Vec3) {
        let axis = layout.axes[handle.axis.index()];
        let last = match handle.kind {
            ManipKind::Translate | ManipKind::Scale => ray_line_param(origin, dir, layout.center, axis).map(DragPoint::Along),
            ManipKind::Rotate => ray_plane(origin, dir, layout.center, axis).map(|p| DragPoint::InPlane((p - layout.center).normalize_or_zero())),
        };
        if let Some(last) = last {
            self.drag = Some(Drag { handle, anchor: layout.center, axis, last });
        }
    }

    pub fn end_drag(&mut self) {
        self.drag = None;
    }

    /// Applies the mouse ray to the dragged object.
    pub fn drag_to(&mut self, transform: &mut Transform, local_bounds: &BoundingBox, origin: Vec3, dir: Vec3) {
        let Some(drag) = &mut self.drag else { return };
        match (drag.handle.kind, drag.last) {
            (ManipKind::Translate, DragPoint::Along(last)) => {
                if let Some(t) = ray_line_param(origin, dir, drag.anchor, drag.axis) {
                    transform.translation += drag.axis * (t - last);
                    drag.last = DragPoint::Along(t);
                }
            }
            (ManipKind::Scale, DragPoint::Along(last)) => {
                if let Some(t) = ray_line_param(origin, dir, drag.anchor, drag.axis) {
                    let i = drag.handle.axis.index();
                    let half = local_bounds.half_extents()[i];
                    let step = (t - last) / if half.abs() > 1e-6 { half } else { 1.0 };
                    let mut scale = transform.scale;
                    scale[i] += step;
                    if scale[i] == 0.0 {
                        scale[i] = 0.001;
                    }
                    transform.scale = scale;
                    drag.last = DragPoint::Along(t);
                }
            }
            (ManipKind::Rotate, DragPoint::InPlane(last)) => {
                if let Some(p) = ray_plane(origin, dir, drag.anchor, drag.axis) {
                    let now = (p - drag.anchor).normalize_or_zero();
                    if now != Vec3::ZERO && last != Vec3::ZERO {
                        let angle = drag.axis.dot(last.cross(now)).atan2(last.dot(now));
                        // Applied about the world axis, like RotateXBy & co.
                        transform.rotation = (Quat::from_axis_angle(drag.axis, angle) * transform.rotation).normalize();
                    }
                    drag.last = DragPoint::InPlane(now);
                }
            }
            _ => {}
        }
    }
}

/// Entity drawing one handle.
#[derive(Component)]
pub struct HandleVisual(pub Handle);

/// Gizmo group for the lines joining the object to its handles.
#[derive(Default, Reflect, GizmoConfigGroup)]
pub struct HandleGizmos;

pub struct ManipulatorPlugin;

impl Plugin for ManipulatorPlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<Manipulator>()
            .init_gizmo_group::<HandleGizmos>()
            .add_plugins(MaterialPlugin::<OverlayMaterial>::default())
            .add_systems(Startup, (spawn_handles, configure_gizmos))
            .add_systems(Update, update_handles.in_set(LabelSet::Produce));
    }
}

fn configure_gizmos(mut store: ResMut<GizmoConfigStore>) {
    let (config, _) = store.config_mut::<HandleGizmos>();
    config.depth_bias = -1.0;
    config.line_width = 2.0;
}

fn build_mesh(triangles: Vec<[[f32; 3]; 3]>, colors: Vec<f32>) -> Mesh {
    let positions: Vec<[f32; 3]> = triangles.iter().flatten().copied().collect();
    let colors: Vec<[f32; 4]> = colors.iter().flat_map(|&c| [[c, c, c, 1.0]; 3]).collect();
    Mesh::new(PrimitiveTopology::TriangleList, RenderAssetUsages::default())
        .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, positions)
        .with_inserted_attribute(Mesh::ATTRIBUTE_COLOR, colors)
}

const BRIGHT: f32 = 1.0;
const DARK: f32 = 125.0 / 255.0;

/// Arrow pointing along +X with its tip at the origin.
pub fn arrow_mesh() -> Mesh {
    let corner = |y: f32, z: f32| [-BASE, y * RAD, z * RAD];
    let tip = [0.0, 0.0, 0.0];
    let ring = [corner(1., 1.), corner(1., -1.), corner(-1., -1.), corner(-1., 1.)];
    let mut tris = Vec::new();
    let mut colors = Vec::new();
    for i in 0..4 {
        tris.push([tip, ring[i], ring[(i + 1) % 4]]);
        colors.push(BRIGHT);
    }
    tris.extend([[ring[0], ring[1], ring[2]], [ring[0], ring[2], ring[3]]]);
    colors.extend([DARK, DARK]);
    // Colours are per triangle; bright ones blend toward the dark base via
    // the vertex colour on the tip vertex, so give the sides a gradient.
    let mut mesh = build_mesh(tris, colors);
    let vertex_colors: Vec<[f32; 4]> = (0..4)
        .flat_map(|_| [[BRIGHT; 4], [DARK; 4], [DARK; 4]])
        .chain(std::iter::repeat_n([DARK; 4], 6))
        .map(|c| [c[0], c[1], c[2], 1.0])
        .collect();
    mesh.insert_attribute(Mesh::ATTRIBUTE_COLOR, vertex_colors);
    mesh
}

/// Cube extending back from the origin along -X.
pub fn cube_mesh() -> Mesh {
    let (front, back) = (0.0, -BASE);
    let p = |x: f32, y: f32, z: f32| [x, y * RAD, z * RAD];
    let mut tris = Vec::new();
    let mut quad = |a: [f32; 3], b: [f32; 3], c: [f32; 3], d: [f32; 3]| {
        tris.push([a, b, c]);
        tris.push([a, c, d]);
    };
    quad(p(front, 1., 1.), p(front, 1., -1.), p(front, -1., -1.), p(front, -1., 1.));
    quad(p(back, 1., 1.), p(back, 1., -1.), p(back, -1., -1.), p(back, -1., 1.));
    for (a, b) in [((1., 1.), (1., -1.)), ((1., -1.), (-1., -1.)), ((-1., -1.), (-1., 1.)), ((-1., 1.), (1., 1.))] {
        quad(p(front, a.0, a.1), p(front, b.0, b.1), p(back, b.0, b.1), p(back, a.0, a.1));
    }
    let colors: Vec<[f32; 4]> = tris
        .iter()
        .flatten()
        .map(|v| {
            let c = if v[0] == front { BRIGHT } else { DARK };
            [c, c, c, 1.0]
        })
        .collect();
    let positions: Vec<[f32; 3]> = tris.iter().flatten().copied().collect();
    Mesh::new(PrimitiveTopology::TriangleList, RenderAssetUsages::default())
        .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, positions)
        .with_inserted_attribute(Mesh::ATTRIBUTE_COLOR, colors)
}

/// A unit-radius band in the YZ plane, `RAD` thick along +X.
pub fn ring_mesh() -> Mesh {
    let mut tris = Vec::new();
    let point = |i: usize| {
        let a = i as f32 * std::f32::consts::TAU / RING_SEGMENTS as f32;
        (a.cos(), a.sin())
    };
    for i in 0..RING_SEGMENTS {
        let ((y0, z0), (y1, z1)) = (point(i), point((i + 1) % RING_SEGMENTS));
        // Thickness is applied by the entity's X scale, so it is 1 here.
        let (a, b, c, d) = ([0., y0, z0], [1., y0, z0], [1., y1, z1], [0., y1, z1]);
        tris.push([a, b, c]);
        tris.push([a, c, d]);
    }
    let count = tris.len();
    build_mesh(tris, vec![1.0; count])
}

fn spawn_handles(mut commands: Commands, mut meshes: ResMut<Assets<Mesh>>, mut materials: ResMut<Assets<OverlayMaterial>>) {
    let arrow = meshes.add(arrow_mesh());
    let cube = meshes.add(cube_mesh());
    let ring = meshes.add(ring_mesh());
    for kind in [ManipKind::Translate, ManipKind::Scale, ManipKind::Rotate] {
        for axis in Axis::ALL {
            let mesh = match kind {
                ManipKind::Translate => arrow.clone(),
                ManipKind::Scale => cube.clone(),
                ManipKind::Rotate => ring.clone(),
            };
            commands.spawn((
                HandleVisual(Handle { kind, axis }),
                Mesh3d(mesh),
                MeshMaterial3d(materials.add(OverlayMaterial::new(axis.tint()))),
                Transform::default(),
                Visibility::Hidden,
                bevy::render::view::RenderLayers::layer(MAIN_LAYER),
                bevy::render::primitives::Aabb::from_min_max(Vec3::splat(-1e6), Vec3::splat(1e6)),
            ));
        }
    }
}

/// Positions and shows the handles for the selected object.
#[allow(clippy::too_many_arguments)]
fn update_handles(
    manip: Res<Manipulator>,
    selection: Res<super::pick::Selection>,
    targets: Query<(&Transform, &ModelGeometry), Without<HandleVisual>>,
    camera: Query<&Transform, (With<MainCamera>, Without<HandleVisual>)>,
    mut handles: Query<(&HandleVisual, &mut Transform, &mut Visibility)>,
    mut gizmos: Gizmos<HandleGizmos>,
    mut labels: ResMut<WorldLabels>,
) {
    let layout = manip.kind.and_then(|kind| {
        let (transform, geometry) = targets.get(selection.entity?).ok()?;
        let camera = camera.get_single().ok()?;
        Some((kind, Layout::compute(kind, &geometry.data.bounding_box, transform, camera.translation), *transform))
    });

    for (visual, mut transform, mut visibility) in &mut handles {
        let Some((kind, layout, object)) = layout.as_ref().filter(|(kind, ..)| *kind == visual.0.kind) else {
            *visibility = Visibility::Hidden;
            continue;
        };
        *visibility = Visibility::Inherited;
        let i = visual.0.axis.index();
        let axis = layout.axes[i];
        // Handle meshes point along +X.
        let facing = Quat::from_rotation_arc(Vec3::X, axis);
        match kind {
            ManipKind::Translate | ManipKind::Scale => {
                *transform = Transform { translation: layout.center + axis * layout.tip[i], rotation: facing, scale: Vec3::splat(layout.scale) };
            }
            ManipKind::Rotate => {
                // Ring mesh is unit radius and unit thickness.
                *transform = Transform {
                    translation: layout.center - axis * (RAD * layout.scale * 0.5),
                    rotation: facing,
                    scale: Vec3::new(RAD * layout.scale, layout.ring_radius, layout.ring_radius),
                };
            }
        }
        if *kind != ManipKind::Rotate {
            let tip = layout.center + axis * layout.tip[i];
            gizmos.line(layout.center, tip, Color::WHITE);
            let value = match kind {
                ManipKind::Translate => object.translation[i],
                _ => object.scale[i],
            };
            labels.add(tip, format!("{value:.2}"), Color::WHITE);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn cube_bounds() -> BoundingBox {
        BoundingBox::new(Vec3::splat(-1.0), Vec3::splat(1.0))
    }

    #[test]
    fn handles_keep_a_constant_size_on_screen() {
        let t = Transform::default();
        let near = Layout::compute(ManipKind::Translate, &cube_bounds(), &t, Vec3::new(0.0, 0.0, 30.0));
        let far = Layout::compute(ManipKind::Translate, &cube_bounds(), &t, Vec3::new(0.0, 0.0, 300.0));
        assert!((far.scale / near.scale - 10.0).abs() < 1e-4);
    }

    #[test]
    fn distance_is_measured_to_the_objects_actual_position() {
        // The C++ measured to the *local* box centre, so an object far from
        // the origin got huge handles when the camera was next to it.
        let t = Transform::from_xyz(500.0, 0.0, 0.0);
        let layout = Layout::compute(ManipKind::Translate, &cube_bounds(), &t, Vec3::new(500.0, 0.0, 30.0));
        assert!((layout.scale - 1.0).abs() < 1e-4, "{}", layout.scale);
        assert_eq!(layout.center, Vec3::new(500.0, 0.0, 0.0));
    }

    #[test]
    fn object_extent_is_not_multiplied_by_the_view_distance() {
        // Handle tip = object edge + BASE * scale. Moving the camera away
        // must only grow the BASE part.
        let t = Transform::from_scale(Vec3::splat(4.0));
        let near = Layout::compute(ManipKind::Translate, &cube_bounds(), &t, Vec3::new(0.0, 0.0, 30.0));
        let far = Layout::compute(ManipKind::Translate, &cube_bounds(), &t, Vec3::new(0.0, 0.0, 60.0));
        assert!((near.tip[0] - (4.0 + BASE * near.scale)).abs() < 1e-4);
        assert!((far.tip[0] - (4.0 + BASE * far.scale)).abs() < 1e-4);
    }

    #[test]
    fn rotate_ring_hugs_small_objects_and_is_capped_for_huge_ones() {
        let camera = Vec3::new(0.0, 0.0, 30.0);
        let small = Layout::compute(ManipKind::Rotate, &cube_bounds(), &Transform::default(), camera);
        assert!(small.ring_radius > 1.0 && small.ring_radius < 1.0 + 2.0 * BASE * small.scale);

        // A 1000x larger object must not get a 1000x larger ring.
        let huge = Layout::compute(ManipKind::Rotate, &cube_bounds(), &Transform::from_scale(Vec3::splat(1000.0)), camera);
        assert!(huge.ring_radius <= (RING_MAX_UNITS + BASE) * huge.scale + 1e-3, "{}", huge.ring_radius);
    }

    #[test]
    fn manipulator_is_centred_on_a_box_in_negative_space() {
        let bounds = BoundingBox::new(Vec3::new(-9.0, -9.0, -9.0), Vec3::new(-5.0, -5.0, -5.0));
        let layout = Layout::compute(ManipKind::Translate, &bounds, &Transform::default(), Vec3::ZERO);
        assert_eq!(layout.center, Vec3::splat(-7.0));
    }

    #[test]
    fn scale_handles_follow_the_objects_rotation() {
        let t = Transform::from_rotation(Quat::from_rotation_z(std::f32::consts::FRAC_PI_2));
        let layout = Layout::compute(ManipKind::Scale, &cube_bounds(), &t, Vec3::new(0.0, 0.0, 30.0));
        assert!((layout.axes[0] - Vec3::Y).length() < 1e-5);
        let translate = Layout::compute(ManipKind::Translate, &cube_bounds(), &t, Vec3::new(0.0, 0.0, 30.0));
        assert_eq!(translate.axes[0], Vec3::X);
    }

    #[test]
    fn ray_line_param_finds_the_closest_point() {
        // Ray straight down -Z at x=3 crosses the X axis line at t=3.
        let t = ray_line_param(Vec3::new(3.0, 5.0, 10.0), Vec3::NEG_Z, Vec3::ZERO, Vec3::X).unwrap();
        assert!((t - 3.0).abs() < 1e-5);
        assert!(ray_line_param(Vec3::new(0.0, 1.0, 0.0), Vec3::X, Vec3::ZERO, Vec3::X).is_none());
    }

    #[test]
    fn picks_the_arrow_under_the_ray_and_ignores_a_miss() {
        let layout = Layout::compute(ManipKind::Translate, &cube_bounds(), &Transform::default(), Vec3::new(0.0, 0.0, 30.0));
        // Aim at the X arrow midway along its length.
        let x = layout.tip[0] * 0.75;
        let hit = pick_handle(ManipKind::Translate, &layout, Vec3::new(x, 0.0, 30.0), Vec3::NEG_Z).unwrap();
        assert_eq!(hit.0.axis, Axis::X);
        assert!(pick_handle(ManipKind::Translate, &layout, Vec3::new(x, 50.0, 30.0), Vec3::NEG_Z).is_none());
    }

    #[test]
    fn picks_a_ring_where_the_ray_crosses_its_circle() {
        let layout = Layout::compute(ManipKind::Rotate, &cube_bounds(), &Transform::default(), Vec3::new(0.0, 0.0, 30.0));
        // Looking down -Z, the Z ring is seen face-on; hit it on its circle.
        let hit = pick_handle(ManipKind::Rotate, &layout, Vec3::new(layout.ring_radius, 0.0, 30.0), Vec3::NEG_Z).unwrap();
        assert_eq!(hit.0.axis, Axis::Z);
        // The middle of the ring is empty (of the Z ring; other rings are edge-on).
        let mid = pick_handle(ManipKind::Rotate, &layout, Vec3::new(0.3, 0.3, 30.0), Vec3::NEG_Z);
        assert!(mid.is_none_or(|(h, _)| h.axis != Axis::Z));
    }

    fn drag_setup(kind: ManipKind, axis: Axis) -> (Manipulator, Layout, Transform) {
        let transform = Transform::default();
        let layout = Layout::compute(kind, &cube_bounds(), &transform, Vec3::new(0.0, 0.0, 30.0));
        let mut manip = Manipulator::default();
        // Grab in the middle of the handle.
        let (origin, dir) = (Vec3::new(3.0, 0.0, 30.0), Vec3::NEG_Z);
        manip.begin_drag(Handle { kind, axis }, &layout, origin, dir);
        (manip, layout, transform)
    }

    #[test]
    fn translate_drag_moves_the_object_exactly_as_far_as_the_cursor() {
        let (mut manip, _, mut t) = drag_setup(ManipKind::Translate, Axis::X);
        assert!(manip.is_dragging());
        manip.drag_to(&mut t, &cube_bounds(), Vec3::new(8.5, 0.0, 30.0), Vec3::NEG_Z);
        assert!((t.translation - Vec3::new(5.5, 0.0, 0.0)).length() < 1e-4, "{:?}", t.translation);
        // Movement is relative to where the handle was grabbed, not absolute.
        manip.drag_to(&mut t, &cube_bounds(), Vec3::new(8.5, 7.0, 30.0), Vec3::NEG_Z);
        assert!((t.translation - Vec3::new(5.5, 0.0, 0.0)).length() < 1e-4);
    }

    #[test]
    fn scale_drag_grows_along_the_axis_only() {
        let (mut manip, _, mut t) = drag_setup(ManipKind::Scale, Axis::Y);
        // Y axis line: grab point projects to y=0; cursor moves up by 2.
        manip.drag_to(&mut t, &cube_bounds(), Vec3::new(0.0, 2.0, 30.0), Vec3::NEG_Z);
        assert!(t.scale.y > 1.0);
        assert_eq!((t.scale.x, t.scale.z), (1.0, 1.0));
    }

    #[test]
    fn rotate_drag_turns_by_the_angle_swept_around_the_ring() {
        let layout = Layout::compute(ManipKind::Rotate, &cube_bounds(), &Transform::default(), Vec3::new(0.0, 0.0, 30.0));
        let mut manip = Manipulator::default();
        let r = layout.ring_radius;
        manip.begin_drag(Handle { kind: ManipKind::Rotate, axis: Axis::Z }, &layout, Vec3::new(r, 0.0, 30.0), Vec3::NEG_Z);
        let mut t = Transform::default();
        // Sweep a quarter turn counter-clockwise as seen from +Z.
        manip.drag_to(&mut t, &cube_bounds(), Vec3::new(0.0, r, 30.0), Vec3::NEG_Z);
        let turned = t.rotation * Vec3::X;
        assert!((turned - Vec3::Y).length() < 1e-4, "{turned:?}");
    }

    #[test]
    fn ending_a_drag_stops_changes() {
        let (mut manip, _, mut t) = drag_setup(ManipKind::Translate, Axis::X);
        manip.end_drag();
        manip.drag_to(&mut t, &cube_bounds(), Vec3::new(9.0, 0.0, 30.0), Vec3::NEG_Z);
        assert_eq!(t.translation, Vec3::ZERO);
    }

    #[test]
    fn handle_meshes_are_well_formed() {
        for mesh in [arrow_mesh(), cube_mesh(), ring_mesh()] {
            let n = mesh.count_vertices();
            assert_eq!(n % 3, 0);
            assert_eq!(mesh.attribute(Mesh::ATTRIBUTE_COLOR).unwrap().len(), n);
        }
    }
}
