//! Reader for the `.3dbin` model format (see `cpp/packer/3dbinFormatSpec.txt`).
//!
//! All values are little-endian and tightly packed:
//!
//! ```text
//! i32 magic = 4, i32 magic = 2
//! char name[32], char comments[128]
//! i32 lodCount, i32 objectType
//! per lod:   i32 meshCount
//!   per mesh: i32 triangleCount
//!     per vertex (triangleCount * 3):
//!       f32 position[3], f32 color[4], i32 textureBinding, f32 uv[2], f32 normal[3]
//! f32 magic = 3.14                       (optional texture block)
//!   i32 textureCount
//!   per texture: i32 id, i32 width, i32 height, u8 rgba[width*height*4]
//! ```

use std::fmt;

use bevy::prelude::*;

use super::BoundingBox;

const MAGIC: [i32; 2] = [4, 2];
const TEXTURE_MAGIC: f32 = 3.14;
const NAME_LEN: usize = 32;
const COMMENT_LEN: usize = 128;
/// The format has room for this many levels of detail.
const MAX_LODS: usize = 3;
const VERTEX_BYTES: usize = 52;
/// Sanity limit on a single texture edge so a corrupt header cannot make us
/// allocate gigabytes.
const MAX_TEXTURE_EDGE: i32 = 16_384;

#[derive(Debug)]
pub enum FormatError {
    /// The file does not start with the 3dbin magic numbers.
    NotA3dbin,
    /// The file ended before a field could be read.
    Truncated { wanted: &'static str },
    /// A count or size in the header is out of range.
    Invalid { what: &'static str, value: i64 },
}

impl fmt::Display for FormatError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::NotA3dbin => write!(f, "not a 3dbin file (bad magic numbers)"),
            Self::Truncated { wanted } => write!(f, "file ended while reading {wanted}"),
            Self::Invalid { what, value } => write!(f, "invalid {what}: {value}"),
        }
    }
}

impl std::error::Error for FormatError {}

/// What a model is used for. Part of the 3dbin specification; nothing acts
/// on it yet.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ObjectType {
    Static,
    Usable,
    Programmable,
}

impl ObjectType {
    fn from_raw(raw: i32) -> Self {
        match raw {
            1 => Self::Usable,
            2 => Self::Programmable,
            _ => Self::Static,
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Vertex {
    pub position: Vec3,
    pub color: [f32; 4],
    /// 1-based index into [`ModelData::textures`] order, or 0 for untextured.
    pub texture_binding: i32,
    pub uv: Vec2,
    pub normal: Vec3,
}

/// A triangle list: every three vertices form one triangle.
#[derive(Clone, Debug, Default)]
pub struct MeshData {
    pub vertices: Vec<Vertex>,
}

#[derive(Clone, Debug)]
pub struct TextureData {
    /// The id used by [`Vertex::texture_binding`] (1-based).
    pub id: i32,
    pub width: u32,
    pub height: u32,
    pub rgba: Vec<u8>,
}

#[derive(Clone, Debug)]
pub struct ModelData {
    pub name: String,
    pub comments: String,
    pub object_type: ObjectType,
    /// `lods[level]` is the list of meshes at that level of detail. Only
    /// level 0 is ever drawn.
    pub lods: Vec<Vec<MeshData>>,
    pub textures: Vec<TextureData>,
    /// Bounds of the drawn (level 0) geometry in model space.
    pub bounding_box: BoundingBox,
}

impl ModelData {
    /// The meshes that get drawn.
    pub fn meshes(&self) -> &[MeshData] {
        self.lods.first().map_or(&[], Vec::as_slice)
    }

    pub fn texture(&self, binding: i32) -> Option<&TextureData> {
        self.textures.iter().find(|t| t.id == binding)
    }
}

/// Little-endian cursor over a byte slice.
struct Reader<'a> {
    bytes: &'a [u8],
}

impl<'a> Reader<'a> {
    fn take(&mut self, n: usize, wanted: &'static str) -> Result<&'a [u8], FormatError> {
        if self.bytes.len() < n {
            return Err(FormatError::Truncated { wanted });
        }
        let (head, tail) = self.bytes.split_at(n);
        self.bytes = tail;
        Ok(head)
    }

    fn i32(&mut self, wanted: &'static str) -> Result<i32, FormatError> {
        Ok(i32::from_le_bytes(self.take(4, wanted)?.try_into().unwrap()))
    }

    fn f32(&mut self, wanted: &'static str) -> Result<f32, FormatError> {
        Ok(f32::from_le_bytes(self.take(4, wanted)?.try_into().unwrap()))
    }

    fn count(&mut self, what: &'static str) -> Result<usize, FormatError> {
        let n = self.i32(what)?;
        usize::try_from(n).map_err(|_| FormatError::Invalid { what, value: n.into() })
    }

    fn string(&mut self, len: usize, wanted: &'static str) -> Result<String, FormatError> {
        let raw = self.take(len, wanted)?;
        let end = raw.iter().position(|&b| b == 0).unwrap_or(len);
        Ok(String::from_utf8_lossy(&raw[..end]).into_owned())
    }
}

/// Parses a complete `.3dbin` file.
pub fn parse(bytes: &[u8]) -> Result<ModelData, FormatError> {
    let mut r = Reader { bytes };

    if r.i32("magic")? != MAGIC[0] || r.i32("magic")? != MAGIC[1] {
        return Err(FormatError::NotA3dbin);
    }
    let name = r.string(NAME_LEN, "name")?;
    let comments = r.string(COMMENT_LEN, "comments")?;
    let lod_count = r.count("lod count")?;
    if lod_count > MAX_LODS {
        return Err(FormatError::Invalid { what: "lod count", value: lod_count as i64 });
    }
    let object_type = ObjectType::from_raw(r.i32("object type")?);

    let mut lods = Vec::with_capacity(lod_count);
    for _ in 0..lod_count {
        let mesh_count = r.count("mesh count")?;
        let mut meshes = Vec::new();
        for _ in 0..mesh_count {
            let triangles = r.count("triangle count")?;
            let vertex_count = triangles * 3;
            if r.bytes.len() / VERTEX_BYTES < vertex_count {
                return Err(FormatError::Truncated { wanted: "mesh vertices" });
            }
            let mut vertices = Vec::with_capacity(vertex_count);
            for _ in 0..vertex_count {
                vertices.push(read_vertex(&mut r)?);
            }
            meshes.push(MeshData { vertices });
        }
        lods.push(meshes);
    }

    let textures = read_textures(&mut r)?;

    let bounding_box = lods
        .first()
        .and_then(|meshes| {
            BoundingBox::from_points(meshes.iter().flat_map(|m| m.vertices.iter().map(|v| v.position)))
        })
        .unwrap_or_default();

    Ok(ModelData { name, comments, object_type, lods, textures, bounding_box })
}

fn read_vertex(r: &mut Reader) -> Result<Vertex, FormatError> {
    let mut f = || r.f32("vertex");
    let position = Vec3::new(f()?, f()?, f()?);
    let color = [f()?, f()?, f()?, f()?];
    let texture_binding = r.i32("vertex")?;
    let mut f = || r.f32("vertex");
    let uv = Vec2::new(f()?, f()?);
    let normal = Vec3::new(f()?, f()?, f()?);
    Ok(Vertex { position, color, texture_binding, uv, normal })
}

/// The texture block is optional: a model without it simply ends (or has a
/// different marker) after the geometry.
fn read_textures(r: &mut Reader) -> Result<Vec<TextureData>, FormatError> {
    match r.f32("texture marker") {
        Ok(marker) if marker == TEXTURE_MAGIC => {}
        _ => return Ok(Vec::new()),
    }

    let count = r.count("texture count")?;
    let mut textures = Vec::new();
    for _ in 0..count {
        let id = r.i32("texture id")?;
        let width = r.i32("texture width")?;
        let height = r.i32("texture height")?;
        for (what, value) in [("texture width", width), ("texture height", height)] {
            if !(1..=MAX_TEXTURE_EDGE).contains(&value) {
                return Err(FormatError::Invalid { what, value: value.into() });
            }
        }
        let rgba = r.take(width as usize * height as usize * 4, "texture pixels")?.to_vec();
        textures.push(TextureData { id, width: width as u32, height: height as u32, rgba });
    }
    Ok(textures)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn header(lods: i32) -> Vec<u8> {
        let mut b = Vec::new();
        b.extend(4i32.to_le_bytes());
        b.extend(2i32.to_le_bytes());
        let mut name = [0u8; NAME_LEN];
        name[..4].copy_from_slice(b"quad");
        b.extend(name);
        b.extend([0u8; COMMENT_LEN]);
        b.extend(lods.to_le_bytes());
        b.extend(0i32.to_le_bytes());
        b
    }

    fn vertex(x: f32, y: f32, z: f32, binding: i32) -> Vec<u8> {
        let mut b = Vec::new();
        for v in [x, y, z, 1.0, 0.5, 0.25, 1.0] {
            b.extend(v.to_le_bytes());
        }
        b.extend(binding.to_le_bytes());
        for v in [0.0f32, 1.0, 0.0, 0.0, 1.0] {
            b.extend(v.to_le_bytes());
        }
        b
    }

    fn one_triangle(positions: [[f32; 3]; 3]) -> Vec<u8> {
        let mut b = header(1);
        b.extend(1i32.to_le_bytes()); // meshes
        b.extend(1i32.to_le_bytes()); // triangles
        for p in positions {
            b.extend(vertex(p[0], p[1], p[2], 0));
        }
        b
    }

    #[test]
    fn parses_a_minimal_untextured_model() {
        let bytes = one_triangle([[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 2.0, 0.0]]);
        let model = parse(&bytes).unwrap();
        assert_eq!(model.name, "quad");
        assert_eq!(model.meshes().len(), 1);
        assert_eq!(model.meshes()[0].vertices.len(), 3);
        assert!(model.textures.is_empty());
        assert_eq!(model.bounding_box.max, Vec3::new(1.0, 2.0, 0.0));
        let v = model.meshes()[0].vertices[0];
        assert_eq!(v.color, [1.0, 0.5, 0.25, 1.0]);
        assert_eq!(v.uv, Vec2::new(0.0, 1.0));
        assert_eq!(v.normal, Vec3::new(0.0, 0.0, 1.0));
    }

    #[test]
    fn model_entirely_below_the_origin_keeps_negative_bounds() {
        let bytes = one_triangle([[-5.0, -5.0, -5.0], [-4.0, -5.0, -5.0], [-5.0, -3.0, -5.0]]);
        let model = parse(&bytes).unwrap();
        assert_eq!(model.bounding_box.max, Vec3::new(-4.0, -3.0, -5.0));
    }

    #[test]
    fn parses_the_optional_texture_block() {
        let mut bytes = one_triangle([[0.0; 3]; 3]);
        bytes.extend(3.14f32.to_le_bytes());
        bytes.extend(1i32.to_le_bytes()); // textures
        bytes.extend(1i32.to_le_bytes()); // id
        bytes.extend(2i32.to_le_bytes()); // w
        bytes.extend(1i32.to_le_bytes()); // h
        bytes.extend([1, 2, 3, 4, 5, 6, 7, 8]);
        let model = parse(&bytes).unwrap();
        assert_eq!(model.textures.len(), 1);
        assert_eq!(model.texture(1).unwrap().rgba, [1, 2, 3, 4, 5, 6, 7, 8]);
        assert!(model.texture(2).is_none());
    }

    #[test]
    fn wrong_magic_is_rejected() {
        let mut bytes = one_triangle([[0.0; 3]; 3]);
        bytes[0] = 9;
        assert!(matches!(parse(&bytes), Err(FormatError::NotA3dbin)));
    }

    #[test]
    fn truncated_geometry_is_an_error_not_a_panic() {
        let mut bytes = one_triangle([[0.0; 3]; 3]);
        bytes.truncate(bytes.len() - 10);
        assert!(matches!(parse(&bytes), Err(FormatError::Truncated { .. })));
    }

    #[test]
    fn absurd_triangle_count_is_rejected_without_allocating() {
        let mut bytes = header(1);
        bytes.extend(1i32.to_le_bytes());
        bytes.extend(i32::MAX.to_le_bytes());
        assert!(parse(&bytes).is_err());
    }

    #[test]
    fn every_bundled_model_parses() {
        let dir = concat!(env!("CARGO_MANIFEST_DIR"), "/assets/data");
        let mut seen = 0;
        for entry in std::fs::read_dir(dir).unwrap() {
            let path = entry.unwrap().path();
            if path.extension().is_some_and(|e| e == "3dbin") {
                let bytes = std::fs::read(&path).unwrap();
                let model = parse(&bytes).unwrap_or_else(|e| panic!("{}: {e}", path.display()));
                assert!(!model.meshes().is_empty(), "{} has no geometry", path.display());
                seen += 1;
            }
        }
        assert_eq!(seen, 6);
    }
}
