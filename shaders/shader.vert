#version 460

layout(push_constant) uniform PushConstants {
	vec2 viewport_size;
	vec2 texture_size;
} push;

layout(location = 0) in vec4 in_dst_rect;
layout(location = 1) in vec4 in_src_rect;
layout(location = 2) in vec4 in_color_tl;
layout(location = 3) in vec4 in_color_tr;
layout(location = 4) in vec4 in_color_bl;
layout(location = 5) in vec4 in_color_br;
layout(location = 6) in vec4 in_corner_radii;
layout(location = 7) in vec4 in_border_color;
layout(location = 8) in float in_border_width;
layout(location = 9) in float in_softness;
layout(location = 10) in float in_omit_texture;

layout(location = 0) flat out vec4 out_color_tl;
layout(location = 1) flat out vec4 out_color_tr;
layout(location = 2) flat out vec4 out_color_bl;
layout(location = 3) flat out vec4 out_color_br;
layout(location = 4) out vec2 out_rect_pos;
layout(location = 5) flat out vec2 out_rect_size;
layout(location = 6) flat out vec4 out_corner_radii;
layout(location = 7) flat out vec4 out_border_color;
layout(location = 8) flat out float out_border_width;
layout(location = 9) flat out float out_softness;
layout(location = 10) out vec2 out_texture_uv;
layout(location = 11) flat out float out_omit_texture;

void main() {
	vec2 positions[6] = vec2[](
	   vec2(0, 0),
	   vec2(1, 0),
	   vec2(0, 1),
	   vec2(1, 0),
	   vec2(1, 1),
	   vec2(0, 1)
	);

	vec2 vertex_pos = positions[gl_VertexIndex];

	vec2 pixel_pos = in_dst_rect.xy + vertex_pos * in_dst_rect.zw;
	vec2 ndc_pos = (pixel_pos / push.viewport_size) * 2.0 - 1.0;

	gl_Position = vec4(ndc_pos, 0, 1);

	out_color_tl = in_color_tl;
	out_color_tr = in_color_tr;
	out_color_bl = in_color_bl;
	out_color_br = in_color_br;

	out_rect_pos = vertex_pos;
	out_rect_size = in_dst_rect.zw;
	out_corner_radii = in_corner_radii;
	out_border_color = in_border_color;
	out_border_width = in_border_width;
	out_softness = in_softness;

	// Calculate texture UV from src_rect
	vec2 src_pos = in_src_rect.xy;
	vec2 src_size = in_src_rect.zw;
	vec2 pixel_uv = src_pos + vertex_pos * src_size;
	out_texture_uv = pixel_uv / push.texture_size;
	out_omit_texture = in_omit_texture;
}