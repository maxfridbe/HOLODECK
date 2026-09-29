// Unlit, vertex-coloured material that ignores depth so manipulator handles
// always draw on top of the scene.
#import bevy_pbr::forward_io::VertexOutput

@group(2) @binding(0) var<uniform> tint: vec4<f32>;

@fragment
fn fragment(in: VertexOutput) -> @location(0) vec4<f32> {
#ifdef VERTEX_COLORS
    return in.color * tint;
#else
    return tint;
#endif
}
