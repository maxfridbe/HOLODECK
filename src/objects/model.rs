//! The set of models in the world: opening, cloning, closing, and
//! loading/saving scenes.
//!
//! Every model in the scene is an entity with a [`SceneObject`]. The
//! [`Model`] resource maps object ids (stable slot numbers, like the C++
//! `Model::objects` vector) to entities; closed models leave an empty slot so
//! ids in a saved scene keep their meaning.

use std::fmt;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use bevy::prelude::*;
use bevy::render::primitives::Aabb;
use bevy::render::view::RenderLayers;

use super::format::{self, FormatError, ModelData};
use super::scene_file::{self, SceneEntry, SceneError, SceneFile};
use super::visuals;
use crate::data;
use crate::physics::PNode;

/// Number of picture-in-picture camera views that can exist at once. Each
/// gets its own render layer so a camera can leave its own model out of its
/// picture.
pub const MAX_QUICK_CAMERAS: usize = 4;

/// Render layer used by the main view.
pub const MAIN_LAYER: usize = 0;

/// The render layer a picture-in-picture camera renders.
pub const fn quick_camera_layer(index: usize) -> usize {
    1 + index
}

/// Layers that see an object: the main view and every quick camera.
pub fn visible_to_all() -> RenderLayers {
    RenderLayers::from_layers(&(0..=MAX_QUICK_CAMERAS).collect::<Vec<_>>())
}

/// Layers that see an object everywhere except in one quick camera's view.
pub fn hidden_from_quick_camera(index: usize) -> RenderLayers {
    let layers: Vec<usize> = (0..=MAX_QUICK_CAMERAS).filter(|&l| l != quick_camera_layer(index)).collect();
    RenderLayers::from_layers(&layers)
}

#[derive(Debug)]
pub enum ModelError {
    NotFound(String),
    Io(PathBuf, std::io::Error),
    Format(String, FormatError),
    Scene(SceneError),
    NoSuchObject(usize),
}

impl fmt::Display for ModelError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::NotFound(name) => write!(f, "Could not find the model '{name}'."),
            Self::Io(path, e) => write!(f, "Could not access {}: {e}", path.display()),
            Self::Format(name, e) => write!(f, "'{name}' is not a valid model: {e}."),
            Self::Scene(e) => write!(f, "Invalid scene: {e}."),
            Self::NoSuchObject(id) => write!(f, "There is no object {id}."),
        }
    }
}

impl std::error::Error for ModelError {}

/// A scene object (as opposed to a camera model, which is not saved).
#[derive(Component, Clone, Debug)]
pub struct SceneObject {
    pub id: usize,
    /// For a copy, the id of the object it was cloned from.
    pub parent_id: Option<usize>,
    pub is_copy: bool,
    /// Model name without extension; how the scene file refers to it.
    pub file_name: String,
}

/// The CPU-side geometry of a model, used for picking and bounds.
#[derive(Component, Clone)]
pub struct ModelGeometry {
    pub data: Arc<ModelData>,
    surfaces: Vec<Surface>,
}

impl ModelGeometry {
    /// Local-space bounds transformed into the world.
    pub fn world_bounds(&self, transform: &Transform) -> super::BoundingBox {
        self.data.bounding_box.transformed(transform)
    }
}

#[derive(Clone)]
struct Surface {
    mesh: Handle<Mesh>,
    texture: Option<Handle<Image>>,
}

/// Marks the child entities that draw a model. `textured` surfaces are the
/// ones tinted while the object is selected.
#[derive(Component)]
pub struct ObjectSurface {
    pub textured: bool,
}

/// A physics node driving an object's position.
#[derive(Component)]
pub struct PhysicsBody(pub PNode);

/// Slot table of scene objects.
#[derive(Resource, Default)]
pub struct Model {
    slots: Vec<Option<Entity>>,
}

impl Model {
    pub fn entity(&self, id: usize) -> Option<Entity> {
        self.slots.get(id).copied().flatten()
    }

    /// Number of slots, including freed ones (the next id to be assigned).
    pub fn slot_count(&self) -> usize {
        self.slots.len()
    }

    pub fn entities(&self) -> impl Iterator<Item = Entity> + '_ {
        self.slots.iter().flatten().copied()
    }
}

/// Where the model named `name` lives: next to the scene, in the working
/// directory, in `data/`, and finally the copy bundled with the program.
fn read_model_bytes(dir: Option<&Path>, name: &str) -> Result<Vec<u8>, ModelError> {
    #[cfg(not(target_arch = "wasm32"))]
    {
        let file = format!("{name}.3dbin");
        let candidates = dir
            .map(|d| d.join(&file))
            .into_iter()
            .chain([PathBuf::from(&file), Path::new("data").join(&file), Path::new("assets/data").join(&file)]);
        for path in candidates {
            match std::fs::read(&path) {
                Ok(bytes) => return Ok(bytes),
                Err(e) if e.kind() == std::io::ErrorKind::NotFound => {}
                Err(e) => return Err(ModelError::Io(path, e)),
            }
        }
    }
    #[cfg(target_arch = "wasm32")]
    let _ = dir;
    data::model(name).map(<[u8]>::to_vec).ok_or_else(|| ModelError::NotFound(name.to_owned()))
}

/// Creates meshes and textures for a model.
fn load_geometry(world: &mut World, name: &str, bytes: &[u8]) -> Result<ModelGeometry, ModelError> {
    let model = format::parse(bytes).map_err(|e| ModelError::Format(name.to_owned(), e))?;

    let mut meshes = world.resource_mut::<Assets<Mesh>>();
    let built = visuals::build_meshes(&model);
    let surfaces_meshes: Vec<(Option<i32>, Handle<Mesh>)> =
        built.into_iter().map(|s| (s.binding, meshes.add(s.mesh))).collect();

    let mut images = world.resource_mut::<Assets<Image>>();
    let mut texture_handles: Vec<(i32, Handle<Image>)> = Vec::new();
    let surfaces = surfaces_meshes
        .into_iter()
        .map(|(binding, mesh)| {
            let texture = binding.and_then(|b| {
                if let Some((_, h)) = texture_handles.iter().find(|(id, _)| *id == b) {
                    return Some(h.clone());
                }
                let handle = images.add(visuals::build_texture(model.texture(b)?));
                texture_handles.push((b, handle.clone()));
                Some(handle)
            });
            Surface { mesh, texture }
        })
        .collect();

    Ok(ModelGeometry { data: Arc::new(model), surfaces })
}

/// Spawns the entity that draws `geometry`, with its own materials.
pub fn spawn_model_entity(world: &mut World, geometry: ModelGeometry, transform: Transform, layers: RenderLayers) -> Entity {
    let bounds = geometry.data.bounding_box;
    let aabb = Aabb::from_min_max(bounds.min, bounds.max);
    let mut children = Vec::new();

    for surface in &geometry.surfaces {
        let material = world.resource_mut::<Assets<StandardMaterial>>().add(StandardMaterial {
            base_color: Color::WHITE,
            base_color_texture: surface.texture.clone(),
            unlit: true,
            double_sided: true,
            cull_mode: None,
            ..default()
        });
        children.push(
            world
                .spawn((
                    Mesh3d(surface.mesh.clone()),
                    MeshMaterial3d(material),
                    aabb,
                    layers.clone(),
                    ObjectSurface { textured: surface.texture.is_some() },
                ))
                .id(),
        );
    }

    world.spawn((transform, Visibility::default(), geometry)).add_children(&children).id()
}

/// Loads a model from bytes and spawns it (not registered as a scene object).
pub fn spawn_from_bytes(world: &mut World, name: &str, bytes: &[u8], transform: Transform, layers: RenderLayers) -> Result<Entity, ModelError> {
    let geometry = load_geometry(world, name, bytes)?;
    Ok(spawn_model_entity(world, geometry, transform, layers))
}

fn register(world: &mut World, entity: Entity, parent_id: Option<usize>, is_copy: bool, file_name: &str) -> usize {
    let mut model = world.resource_mut::<Model>();
    let id = model.slots.len();
    model.slots.push(Some(entity));
    world.entity_mut(entity).insert(SceneObject { id, parent_id, is_copy, file_name: file_name.to_owned() });
    id
}

/// Opens the model `name` (no extension) at the origin and adds it to the scene.
pub fn open(world: &mut World, dir: Option<&Path>, name: &str) -> Result<usize, ModelError> {
    let bytes = read_model_bytes(dir, name)?;
    let entity = spawn_from_bytes(world, name, &bytes, Transform::IDENTITY, visible_to_all())?;
    Ok(register(world, entity, None, false, name))
}

/// Opens a model from an explicit path such as one chosen in the file dialog.
pub fn open_path(world: &mut World, path: &Path) -> Result<usize, ModelError> {
    let name = path.file_stem().and_then(|s| s.to_str()).unwrap_or_default().to_owned();
    let bytes = std::fs::read(path).map_err(|e| ModelError::Io(path.to_owned(), e))?;
    let entity = spawn_from_bytes(world, &name, &bytes, Transform::IDENTITY, visible_to_all())?;
    Ok(register(world, entity, None, false, &name))
}

/// Duplicates an object: same geometry and textures, same placement.
pub fn clone_object(world: &mut World, id: usize) -> Result<usize, ModelError> {
    let source = world.resource::<Model>().entity(id).ok_or(ModelError::NoSuchObject(id))?;
    let geometry = world.get::<ModelGeometry>(source).cloned().ok_or(ModelError::NoSuchObject(id))?;
    let transform = *world.get::<Transform>(source).ok_or(ModelError::NoSuchObject(id))?;
    let original = world.get::<SceneObject>(source).cloned().ok_or(ModelError::NoSuchObject(id))?;

    // A copy of a copy points at the original.
    let parent = if original.is_copy { original.parent_id } else { Some(original.id) };
    let entity = spawn_model_entity(world, geometry, transform, visible_to_all());
    Ok(register(world, entity, parent, true, &original.file_name))
}

/// Removes an object. If it was the original of some copies, the first copy
/// takes over as the original so the scene stays loadable.
pub fn close(world: &mut World, id: usize) -> Result<(), ModelError> {
    let entity = world.resource_mut::<Model>().slots.get_mut(id).and_then(Option::take).ok_or(ModelError::NoSuchObject(id))?;

    let mut children: Vec<(usize, Entity)> = world
        .query::<(Entity, &SceneObject)>()
        .iter(world)
        .filter(|(_, o)| o.parent_id == Some(id))
        .map(|(e, o)| (o.id, e))
        .collect();
    children.sort_by_key(|(id, _)| *id);

    if let Some(&(new_parent, promoted)) = children.first() {
        if let Some(mut o) = world.get_mut::<SceneObject>(promoted) {
            o.parent_id = None;
            o.is_copy = false;
        }
        for &(_, e) in &children[1..] {
            if let Some(mut o) = world.get_mut::<SceneObject>(e) {
                o.parent_id = Some(new_parent);
            }
        }
    }

    world.entity_mut(entity).despawn_recursive();
    Ok(())
}

/// Removes every object in the scene.
pub fn clear(world: &mut World) {
    let entities: Vec<Entity> = world.resource::<Model>().entities().collect();
    for entity in entities {
        world.entity_mut(entity).despawn_recursive();
    }
    world.resource_mut::<Model>().slots.clear();
}

/// Describes the current scene as a scene file.
pub fn snapshot(world: &mut World) -> SceneFile {
    let slots: Vec<Option<Entity>> = world.resource::<Model>().slots.clone();
    let slots = slots
        .into_iter()
        .map(|slot| {
            let entity = slot?;
            let object = world.get::<SceneObject>(entity)?;
            let transform = *world.get::<Transform>(entity)?;
            Some(SceneEntry {
                id: object.id,
                parent_id: object.parent_id,
                is_copy: object.is_copy,
                file_name: object.file_name.clone(),
                transform,
            })
        })
        .collect();
    SceneFile { slots, ..default() }
}

/// Replaces the scene with the contents of `scene`, finding models next to
/// `dir`. Objects that fail to load leave an empty slot and are reported in
/// the returned list.
pub fn apply(world: &mut World, scene: &SceneFile, dir: Option<&Path>) -> Vec<ModelError> {
    clear(world);
    let mut errors = Vec::new();

    for slot in &scene.slots {
        let Some(entry) = slot else {
            world.resource_mut::<Model>().slots.push(None);
            continue;
        };
        let result = if entry.is_copy {
            entry.parent_id.map_or(Err(ModelError::NoSuchObject(entry.id)), |p| clone_object(world, p))
        } else {
            open(world, dir, &entry.file_name)
        };
        match result {
            Ok(id) => {
                debug_assert_eq!(id, entry.id);
                let entity = world.resource::<Model>().entity(id).expect("just registered");
                if let Some(mut t) = world.get_mut::<Transform>(entity) {
                    *t = entry.transform;
                }
            }
            Err(e) => {
                world.resource_mut::<Model>().slots.push(None);
                errors.push(e);
            }
        }
    }
    errors
}

/// Loads a scene file from disk.
pub fn load_scene(world: &mut World, path: &Path) -> Result<Vec<ModelError>, ModelError> {
    let text = std::fs::read_to_string(path).map_err(|e| ModelError::Io(path.to_owned(), e))?;
    let scene = scene_file::parse(&text).map_err(ModelError::Scene)?;
    Ok(apply(world, &scene, path.parent()))
}

/// Loads a bundled scene by name.
pub fn load_bundled_scene(world: &mut World, name: &str) -> Result<Vec<ModelError>, ModelError> {
    let bytes = data::scene(name).ok_or_else(|| ModelError::NotFound(name.to_owned()))?;
    let scene = scene_file::parse(&String::from_utf8_lossy(bytes)).map_err(ModelError::Scene)?;
    Ok(apply(world, &scene, None))
}

pub fn save_scene(world: &mut World, path: &Path) -> Result<(), ModelError> {
    let text = scene_file::serialize(&snapshot(world));
    std::fs::write(path, text).map_err(|e| ModelError::Io(path.to_owned(), e))
}

/// Advances objects that have a physics node.
pub fn step_physics(time: Res<Time>, mut bodies: Query<(&mut PhysicsBody, &mut Transform)>) {
    let dt = f64::from(time.delta_secs());
    for (mut body, mut transform) in &mut bodies {
        let node = &mut body.0;
        node.update_forces(dt);
        node.update_position(dt);
        node.update_velocity(dt);
        transform.translation = node.position().as_vec3();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bevy::asset::AssetPlugin;

    fn world() -> World {
        let mut app = App::new();
        app.add_plugins(AssetPlugin::default())
            .init_asset::<Mesh>()
            .init_asset::<Image>()
            .init_asset::<StandardMaterial>()
            .init_resource::<Model>();
        std::mem::take(app.world_mut())
    }

    #[test]
    fn opening_assigns_sequential_ids_and_clones_share_geometry() {
        let mut w = world();
        let a = open(&mut w, None, "cone").unwrap();
        let b = clone_object(&mut w, a).unwrap();
        let c = clone_object(&mut w, b).unwrap();
        assert_eq!((a, b, c), (0, 1, 2));

        let objects: Vec<SceneObject> = (0..3).map(|i| w.get::<SceneObject>(w.resource::<Model>().entity(i).unwrap()).unwrap().clone()).collect();
        assert_eq!(objects[1].parent_id, Some(0));
        // A copy of a copy still points at the original.
        assert_eq!(objects[2].parent_id, Some(0));
        assert!(objects[2].is_copy && !objects[0].is_copy);

        let geo = |i| w.get::<ModelGeometry>(w.resource::<Model>().entity(i).unwrap()).unwrap().data.clone();
        assert!(Arc::ptr_eq(&geo(0), &geo(2)));
    }

    #[test]
    fn closing_an_original_promotes_its_first_copy() {
        let mut w = world();
        let a = open(&mut w, None, "cone").unwrap();
        clone_object(&mut w, a).unwrap();
        clone_object(&mut w, a).unwrap();
        close(&mut w, a).unwrap();

        let get = |w: &World, i| w.get::<SceneObject>(w.resource::<Model>().entity(i).unwrap()).unwrap().clone();
        assert!(w.resource::<Model>().entity(0).is_none());
        let first = get(&w, 1);
        assert!(!first.is_copy && first.parent_id.is_none());
        let second = get(&w, 2);
        assert!(second.is_copy && second.parent_id == Some(1));
        // The slot is kept so ids stay stable.
        assert_eq!(w.resource::<Model>().slot_count(), 3);
        assert!(matches!(close(&mut w, 0), Err(ModelError::NoSuchObject(0))));
    }

    #[test]
    fn saved_scene_reloads_with_the_same_layout() {
        let mut w = world();
        let a = open(&mut w, None, "cone").unwrap();
        let b = clone_object(&mut w, a).unwrap();
        open(&mut w, None, "camera").unwrap();
        let entity = w.resource::<Model>().entity(b).unwrap();
        *w.get_mut::<Transform>(entity).unwrap() = Transform::from_xyz(1.0, -2.0, 3.0).with_scale(Vec3::splat(2.0));
        close(&mut w, 2).unwrap();

        let scene = snapshot(&mut w);
        let text = scene_file::serialize(&scene);
        let errors = apply(&mut w, &scene_file::parse(&text).unwrap(), None);
        assert!(errors.is_empty());
        assert_eq!(w.resource::<Model>().slot_count(), 3);
        assert!(w.resource::<Model>().entity(2).is_none());
        let t = w.get::<Transform>(w.resource::<Model>().entity(1).unwrap()).unwrap();
        assert_eq!(t.translation, Vec3::new(1.0, -2.0, 3.0));
        assert_eq!(t.scale, Vec3::splat(2.0));
    }

    #[test]
    fn every_bundled_scene_loads() {
        for name in ["demo", "fixed", "fixed2", "scenex", "test", "testing", "world"] {
            let mut w = world();
            let errors = load_bundled_scene(&mut w, name).unwrap();
            assert!(errors.is_empty(), "{name}: {errors:?}");
            assert!(w.resource::<Model>().entities().count() > 0, "{name}");
        }
    }

    #[test]
    fn unknown_model_is_an_error() {
        let mut w = world();
        assert!(matches!(open(&mut w, None, "nope"), Err(ModelError::NotFound(_))));
    }
}
