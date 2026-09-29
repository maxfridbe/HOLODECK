//! Turns parsed model data into GPU-ready meshes and textures.
//!
//! The original drew models with a compiled display list of unlit, untextured
//! or textured triangles. Here every distinct texture binding becomes one
//! mesh (plus one for all untextured triangles), rendered with an unlit,
//! double-sided material.

use bevy::asset::RenderAssetUsages;
use bevy::image::{ImageAddressMode, ImageFilterMode, ImageSampler, ImageSamplerDescriptor};
use bevy::prelude::*;
use bevy::render::mesh::PrimitiveTopology;
use bevy::render::render_resource::{Extent3d, TextureDimension, TextureFormat};

use super::format::{ModelData, TextureData, Vertex};

/// One mesh of a model: all the triangles that share a texture (or, with
/// `binding == None`, all the untextured ones).
pub struct SurfaceMesh {
    pub binding: Option<i32>,
    pub mesh: Mesh,
}

/// Splits level-of-detail 0 of a model into one mesh per texture binding.
///
/// Textured triangles get white vertex colours so the texture shows
/// unmodified; untextured ones use their vertex colours. A binding that
/// names a texture the file does not contain is drawn untextured.
pub fn build_meshes(data: &ModelData) -> Vec<SurfaceMesh> {
    let mut groups: Vec<(Option<i32>, Vec<Vertex>)> = Vec::new();

    for mesh in data.meshes() {
        for triangle in mesh.vertices.chunks_exact(3) {
            let binding = Some(triangle[0].texture_binding).filter(|&b| b > 0 && data.texture(b).is_some());
            let group = match groups.iter_mut().find(|(b, _)| *b == binding) {
                Some(group) => group,
                None => {
                    groups.push((binding, Vec::new()));
                    groups.last_mut().expect("just pushed")
                }
            };
            group.1.extend_from_slice(triangle);
        }
    }

    groups
        .into_iter()
        .map(|(binding, vertices)| SurfaceMesh { binding, mesh: triangle_mesh(&vertices, binding.is_some()) })
        .collect()
}

fn triangle_mesh(vertices: &[Vertex], textured: bool) -> Mesh {
    let colors: Vec<[f32; 4]> = vertices
        .iter()
        .map(|v| {
            if textured {
                [1.0; 4]
            } else {
                // GL wrote file colours straight to the framebuffer; treat
                // them as sRGB so they look the same after Bevy's conversion.
                let [r, g, b, a] = v.color;
                Color::srgba(r, g, b, a).to_linear().to_f32_array()
            }
        })
        .collect();

    Mesh::new(PrimitiveTopology::TriangleList, RenderAssetUsages::default())
        .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, vertices.iter().map(|v| v.position.to_array()).collect::<Vec<_>>())
        .with_inserted_attribute(Mesh::ATTRIBUTE_NORMAL, vertices.iter().map(|v| v.normal.to_array()).collect::<Vec<_>>())
        .with_inserted_attribute(Mesh::ATTRIBUTE_UV_0, vertices.iter().map(|v| v.uv.to_array()).collect::<Vec<_>>())
        .with_inserted_attribute(Mesh::ATTRIBUTE_COLOR, colors)
}

/// The triangle edges of level-of-detail 0 as a line mesh, used for
/// wireframe mode (which therefore needs no special GPU support).
pub fn build_wireframe(data: &ModelData) -> Mesh {
    let mut positions: Vec<[f32; 3]> = Vec::new();
    let mut colors: Vec<[f32; 4]> = Vec::new();
    for mesh in data.meshes() {
        for triangle in mesh.vertices.chunks_exact(3) {
            let textured = triangle[0].texture_binding > 0 && data.texture(triangle[0].texture_binding).is_some();
            for (a, b) in [(0, 1), (1, 2), (2, 0)] {
                for vertex in [&triangle[a], &triangle[b]] {
                    positions.push(vertex.position.to_array());
                    colors.push(if textured {
                        [1.0; 4]
                    } else {
                        let [r, g, b, a] = vertex.color;
                        Color::srgba(r, g, b, a).to_linear().to_f32_array()
                    });
                }
            }
        }
    }
    Mesh::new(PrimitiveTopology::LineList, RenderAssetUsages::default())
        .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, positions)
        .with_inserted_attribute(Mesh::ATTRIBUTE_COLOR, colors)
}

/// Builds a texture with a full mip chain, sampled like the original
/// (`GL_NEAREST_MIPMAP_LINEAR`, repeating).
pub fn build_texture(texture: &TextureData) -> Image {
    let (mut width, mut height) = (texture.width, texture.height);
    let mut level = texture.rgba.clone();
    let mut all_levels = level.clone();
    let mut mip_levels = 1;

    while width > 1 || height > 1 {
        let (next_w, next_h) = ((width / 2).max(1), (height / 2).max(1));
        level = downsample(&level, width, height, next_w, next_h);
        all_levels.extend_from_slice(&level);
        (width, height) = (next_w, next_h);
        mip_levels += 1;
    }

    let mut image = Image::new(
        Extent3d { width: texture.width, height: texture.height, depth_or_array_layers: 1 },
        TextureDimension::D2,
        texture.rgba.clone(),
        TextureFormat::Rgba8UnormSrgb,
        RenderAssetUsages::RENDER_WORLD,
    );
    image.data = all_levels;
    image.texture_descriptor.mip_level_count = mip_levels;
    image.sampler = ImageSampler::Descriptor(ImageSamplerDescriptor {
        address_mode_u: ImageAddressMode::Repeat,
        address_mode_v: ImageAddressMode::Repeat,
        mag_filter: ImageFilterMode::Nearest,
        min_filter: ImageFilterMode::Nearest,
        mipmap_filter: ImageFilterMode::Linear,
        ..default()
    });
    image
}

/// 2x2 box filter (clamping at odd edges).
fn downsample(src: &[u8], src_w: u32, src_h: u32, dst_w: u32, dst_h: u32) -> Vec<u8> {
    let mut out = Vec::with_capacity((dst_w * dst_h * 4) as usize);
    for y in 0..dst_h {
        for x in 0..dst_w {
            let xs = [(x * 2).min(src_w - 1), (x * 2 + 1).min(src_w - 1)];
            let ys = [(y * 2).min(src_h - 1), (y * 2 + 1).min(src_h - 1)];
            for channel in 0..4 {
                let sum: u32 = ys
                    .iter()
                    .flat_map(|&sy| xs.iter().map(move |&sx| (sx, sy)))
                    .map(|(sx, sy)| u32::from(src[((sy * src_w + sx) * 4 + channel) as usize]))
                    .sum();
                out.push((sum / 4) as u8);
            }
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::objects::format;

    fn bundled(name: &str) -> ModelData {
        format::parse(crate::data::model(name).unwrap()).unwrap()
    }

    fn vertex_count(mesh: &Mesh) -> usize {
        mesh.count_vertices()
    }

    #[test]
    fn every_triangle_lands_in_exactly_one_mesh() {
        for name in ["backpack", "blueEye", "camera", "cone", "SkyDome", "vrupl"] {
            let data = bundled(name);
            let expected: usize = data.meshes().iter().map(|m| m.vertices.len() / 3 * 3).sum();
            let built: usize = build_meshes(&data).iter().map(|s| vertex_count(&s.mesh)).sum();
            assert_eq!(built, expected, "{name}");
        }
    }

    #[test]
    fn textured_models_get_one_mesh_per_texture() {
        let data = bundled("blueEye");
        let surfaces = build_meshes(&data);
        let textured = surfaces.iter().filter(|s| s.binding.is_some()).count();
        assert!(textured <= data.textures.len());
        for surface in surfaces.iter().filter_map(|s| s.binding) {
            assert!(data.texture(surface).is_some());
        }
    }

    #[test]
    fn untextured_colours_are_converted_from_srgb() {
        let vertices = [Vertex {
            position: Vec3::ZERO,
            color: [0.5, 0.5, 0.5, 1.0],
            texture_binding: 0,
            uv: Vec2::ZERO,
            normal: Vec3::Y,
        }; 3];
        let mesh = triangle_mesh(&vertices, false);
        let Some(bevy::render::mesh::VertexAttributeValues::Float32x4(colors)) = mesh.attribute(Mesh::ATTRIBUTE_COLOR) else {
            panic!("colours missing");
        };
        // sRGB 0.5 is roughly 0.214 in linear space.
        assert!((colors[0][0] - 0.214).abs() < 0.01, "{}", colors[0][0]);
        assert_eq!(colors[0][3], 1.0);
    }

    #[test]
    fn wireframe_has_three_edges_per_triangle() {
        let data = bundled("cone");
        let triangles: usize = data.meshes().iter().map(|m| m.vertices.len() / 3).sum();
        let mesh = build_wireframe(&data);
        assert_eq!(mesh.count_vertices(), triangles * 6);
        assert_eq!(mesh.primitive_topology(), PrimitiveTopology::LineList);
    }

    #[test]
    fn mip_chain_covers_every_level_down_to_one_pixel() {
        let texture = TextureData { id: 1, width: 4, height: 2, rgba: vec![200; 4 * 2 * 4] };
        let image = build_texture(&texture);
        // 4x2 + 2x1 + 1x1 pixels, 4 bytes each.
        assert_eq!(image.texture_descriptor.mip_level_count, 3);
        assert_eq!(image.data.len(), (8 + 2 + 1) * 4);
        assert!(image.data.iter().all(|&b| b == 200));
    }

    #[test]
    fn downsample_averages_each_block() {
        // 2x2 -> 1x1: (0 + 100 + 100 + 200) / 4
        let src = [0, 0, 0, 255, 100, 100, 100, 255, 100, 100, 100, 255, 200, 200, 200, 255];
        assert_eq!(downsample(&src, 2, 2, 1, 1), vec![100, 100, 100, 255]);
    }
}
