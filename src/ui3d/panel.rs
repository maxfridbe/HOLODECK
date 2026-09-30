//! The panel material used to draw the UI chrome like the original: gradient
//! quads with cut corners (bevy_ui has neither gradients nor chamfers).

use bevy::prelude::*;
use bevy::render::render_resource::{AsBindGroup, ShaderRef};

#[derive(Asset, TypePath, AsBindGroup, Clone, Debug)]
pub struct PanelMaterial {
    #[uniform(0)]
    pub top: LinearRgba,
    #[uniform(0)]
    pub bottom: LinearRgba,
    #[uniform(0)]
    pub border_color: LinearRgba,
    /// Corner sizes in pixels: top-left, top-right, bottom-right, bottom-left.
    #[uniform(0)]
    pub corners: Vec4,
    /// x: 1 for chamfered corners, 0 for rounded; y: border width in pixels.
    #[uniform(0)]
    pub params: Vec4,
}

impl UiMaterial for PanelMaterial {
    fn fragment_shader() -> ShaderRef {
        "shaders/ui_panel.wgsl".into()
    }
}

impl PanelMaterial {
    pub fn flat(color: Color) -> Self {
        Self::gradient(color, color)
    }

    /// Top-to-bottom gradient.
    pub fn gradient(top: Color, bottom: Color) -> Self {
        Self { top: top.to_linear(), bottom: bottom.to_linear(), border_color: LinearRgba::NONE, corners: Vec4::ZERO, params: Vec4::ZERO }
    }

    /// Rounded corners (top-left, top-right, bottom-right, bottom-left).
    pub fn rounded(mut self, corners: [f32; 4]) -> Self {
        self.corners = Vec4::from_array(corners);
        self.params.x = 0.0;
        self
    }

    /// Chamfered (cut) corners (top-left, top-right, bottom-right, bottom-left).
    pub fn chamfered(mut self, corners: [f32; 4]) -> Self {
        self.corners = Vec4::from_array(corners);
        self.params.x = 1.0;
        self
    }

    pub fn bordered(mut self, color: Color, width: f32) -> Self {
        self.border_color = color.to_linear();
        self.params.y = width;
        self
    }
}

/// Grey level helper: the original specified colours as 0-255 bytes.
pub fn grey(level: u8) -> Color {
    Color::srgb_u8(level, level, level)
}
