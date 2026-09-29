//! A material for things that must always be visible, such as manipulator
//! handles: unlit, vertex-coloured, and drawn regardless of depth.

use bevy::pbr::{MaterialPipeline, MaterialPipelineKey};
use bevy::prelude::*;
use bevy::render::mesh::MeshVertexBufferLayoutRef;
use bevy::render::render_resource::{
    AsBindGroup, CompareFunction, RenderPipelineDescriptor, ShaderRef, SpecializedMeshPipelineError,
};

#[derive(Asset, TypePath, AsBindGroup, Clone, Debug)]
pub struct OverlayMaterial {
    /// Multiplied with the vertex colours.
    #[uniform(0)]
    pub tint: LinearRgba,
}

impl OverlayMaterial {
    pub fn new(color: Color) -> Self {
        Self { tint: color.to_linear() }
    }
}

impl Material for OverlayMaterial {
    fn fragment_shader() -> ShaderRef {
        "shaders/overlay.wgsl".into()
    }

    // Blend mode places these after all opaque geometry, sorted back to
    // front, so with the depth test off nearer handles cover farther ones.
    fn alpha_mode(&self) -> AlphaMode {
        AlphaMode::Blend
    }

    fn specialize(
        _pipeline: &MaterialPipeline<Self>,
        descriptor: &mut RenderPipelineDescriptor,
        _layout: &MeshVertexBufferLayoutRef,
        _key: MaterialPipelineKey<Self>,
    ) -> Result<(), SpecializedMeshPipelineError> {
        descriptor.primitive.cull_mode = None;
        if let Some(depth) = descriptor.depth_stencil.as_mut() {
            depth.depth_compare = CompareFunction::Always;
            depth.depth_write_enabled = false;
        }
        Ok(())
    }
}
