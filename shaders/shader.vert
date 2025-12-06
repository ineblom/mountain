#version 460

layout(push_constant) uniform PushConstants {
	vec2 viewport_size;
} push;

layout(location = 0) in vec4 in_rect;
layout(location = 1) in vec4 in_color_tl;
layout(location = 2) in vec4 in_color_tr;
layout(location = 3) in vec4 in_color_bl;
layout(location = 4) in vec4 in_color_br;
layout(location = 5) in vec4 in_corner_radii;
layout(location = 6) in vec4 in_border_color;
layout(location = 7) in float in_border_width;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_rect_pos;
layout(location = 2) out vec2 out_rect_size;
layout(location = 3) out vec4 out_corner_radii;
layout(location = 4) out vec4 out_border_color;
layout(location = 5) out float out_border_width;

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

	vec2 pixel_pos = in_rect.xy + vertex_pos * in_rect.zw;
	vec2 ndc_pos = (pixel_pos / push.viewport_size) * 2.0 - 1.0;

	gl_Position = vec4(ndc_pos, 0, 1);

	// Select corner color based on vertex position
	if (vertex_pos.x < 0.5) {
		out_color = (vertex_pos.y < 0.5) ? in_color_tl : in_color_bl;
	} else {
		out_color = (vertex_pos.y < 0.5) ? in_color_tr : in_color_br;
	}

	out_rect_pos = vertex_pos;
	out_rect_size = in_rect.zw;
	out_corner_radii = in_corner_radii;
	out_border_color = in_border_color;
	out_border_width = in_border_width;
}