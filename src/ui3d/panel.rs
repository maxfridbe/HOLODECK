//! The panel material used to draw the UI chrome like the original: gradient
//! quads with cut corners (bevy_ui has neither gradients nor chamfers).

use bevy::prelude::*;
use bevy::render::render_resource::{AsBindGroup, ShaderRef};

#[derive(Asset, TypePath, AsBindGroup, Clone, Debug)]
pub struct PanelMaterial {
    /// Colours are stored as sRGB and graded in sRGB, like the original GL.
    #[uniform(0)]
    pub top: Vec4,
    #[uniform(0)]
    pub bottom: Vec4,
    #[uniform(0)]
    pub border_color: Vec4,
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
        Self { top: srgb(top), bottom: srgb(bottom), border_color: Vec4::ZERO, corners: Vec4::ZERO, params: Vec4::ZERO }
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
        self.border_color = srgb(color);
        self.params.y = width;
        self
    }
}

fn srgb(color: Color) -> Vec4 {
    Vec4::from_array(color.to_srgba().to_f32_array())
}

/// Grey level helper: the original specified colours as 0-255 bytes.
pub fn grey(level: u8) -> Color {
    Color::srgb_u8(level, level, level)
}
