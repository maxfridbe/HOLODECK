// Flat or vertically graded UI panel with per-corner rounding or chamfers
// and an optional border: the look of the original immediate-mode GL UI
// (gradient quads with cut corners), which bevy_ui cannot draw by itself.
#import bevy_ui::ui_vertex_output::UiVertexOutput

// Colours arrive as sRGB (0-1): the original's fixed-function GL blended
// vertex colours in that space, so gradients are graded there too and only
// converted to linear for output.
struct Panel {
    top: vec4<f32>,
    bottom: vec4<f32>,
    border_color: vec4<f32>,
    // Corner sizes in pixels: top-left, top-right, bottom-right, bottom-left.
    corners: vec4<f32>,
    // x: 1 = chamfered corners, 0 = rounded; y: border width in pixels.
    params: vec4<f32>,
};

@group(1) @binding(0) var<uniform> panel: Panel;

// Signed distance to the panel edge in pixels (negative inside).
fn panel_distance(p: vec2<f32>, size: vec2<f32>) -> f32 {
    let right = p.x > size.x * 0.5;
    let lower = p.y > size.y * 0.5;
    var r = panel.corners.x;
    if right && !lower { r = panel.corners.y; }
    if right && lower { r = panel.corners.z; }
    if !right && lower { r = panel.corners.w; }
    r = min(r, min(size.x, size.y) * 0.5);

    // Distance from the nearest corner, mirrored into the top-left quadrant.
    let q = min(p, size - p);
    let edge = -min(q.x, q.y);
    if r <= 0.0 || q.x >= r || q.y >= r {
        return edge;
    }
    let local = vec2<f32>(r, r) - q;
    if panel.params.x > 0.5 {
        // Chamfer: cut the corner with a 45 degree line.
        return max(edge, (local.x + local.y - r) * 0.70710678);
    }
    return length(local) - r;
}

fn srgb_to_linear(c: vec3<f32>) -> vec3<f32> {
    let low = c / 12.92;
    let high = pow((c + 0.055) / 1.055, vec3<f32>(2.4));
    return select(high, low, c <= vec3<f32>(0.04045));
}

@fragment
fn fragment(in: UiVertexOutput) -> @location(0) vec4<f32> {
    let p = in.uv * in.size;
    let d = panel_distance(p, in.size);
    let coverage = clamp(0.5 - d, 0.0, 1.0);
    var color = mix(panel.top, panel.bottom, in.uv.y);
    let border = panel.params.y;
    if border > 0.0 {
        let inner = clamp(0.5 - (d + border), 0.0, 1.0);
        color = mix(panel.border_color, color, inner);
    }
    return vec4<f32>(srgb_to_linear(color.rgb), color.a * coverage);
}
