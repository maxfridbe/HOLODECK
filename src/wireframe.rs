//! Wireframe mode: swaps each object's filled surfaces for its triangle
//! edges. Edge meshes are built on first use and shared between copies.

use std::collections::HashMap;
use std::sync::Arc;

use bevy::prelude::*;

use crate::objects::model::{ModelGeometry, ObjectSurface, visible_to_all};
use crate::objects::visuals;
use crate::settings::Settings;

/// Marks the child entity that draws an object's edges.
#[derive(Component)]
struct EdgeLines;

/// Edge meshes by model, so duplicated objects share one.
#[derive(Resource, Default)]
struct EdgeMeshes(HashMap<usize, Handle<Mesh>>);

pub struct WireframePlugin;

impl Plugin for WireframePlugin {
    fn build(&self, app: &mut App) {
        app.init_resource::<EdgeMeshes>().add_systems(Update, apply);
    }
}

fn apply(
    settings: Res<Settings>,
    mut commands: Commands,
    mut cache: ResMut<EdgeMeshes>,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    mut edge_material: Local<Option<Handle<StandardMaterial>>>,
    objects: Query<(Entity, &ModelGeometry, Option<&Children>)>,
    mut surfaces: Query<&mut Visibility, (With<ObjectSurface>, Without<EdgeLines>)>,
    mut edges: Query<&mut Visibility, (With<EdgeLines>, Without<ObjectSurface>)>,
) {
    let wireframe = settings.wireframe_mode;
    let edge_material = edge_material
        .get_or_insert_with(|| {
            materials.add(StandardMaterial { base_color: Color::WHITE, unlit: true, ..default() })
        })
        .clone();

    for (entity, geometry, children) in &objects {
        let children: &[Entity] = children.map_or(&[], |c| c);
        let has_edges = children.iter().any(|&c| edges.contains(c));

        if wireframe && !has_edges {
            // The address only serves as an identity key for this model's data.
            let key = Arc::as_ptr(&geometry.data) as usize;
            let mesh = cache.0.entry(key).or_insert_with(|| meshes.add(visuals::build_wireframe(&geometry.data))).clone();
            let bounds = geometry.data.bounding_box;
            let edges = commands
                .spawn((
                    EdgeLines,
                    Mesh3d(mesh),
                    MeshMaterial3d(edge_material.clone()),
                    bevy::render::primitives::Aabb::from_min_max(bounds.min, bounds.max),
                    visible_to_all(),
                ))
                .id();
            commands.entity(entity).add_child(edges);
        }

        for &child in children {
            let set = |visible: bool, vis: &mut Visibility| {
                let want = if visible { Visibility::Inherited } else { Visibility::Hidden };
                if *vis != want {
                    *vis = want;
                }
            };
            if let Ok(mut v) = surfaces.get_mut(child) {
                set(!wireframe, &mut v);
            }
            if let Ok(mut v) = edges.get_mut(child) {
                set(wireframe, &mut v);
            }
        }
    }
}
