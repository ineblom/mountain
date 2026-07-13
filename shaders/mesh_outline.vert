#version 460

layout(push_constant) uniform PushConstants {
  mat4 view_projection;
  vec2 viewport_size;
  float outline_width;
} push;

layout(location = 0) in vec4 in_pos;
layout(location = 4) in mat4 in_transform;
layout(location = 8) in vec4 in_instance_color;

layout(location = 0) flat out vec4 out_color;

void main() {
  vec4 world_pos = in_transform * vec4(in_pos.xyz, 1.0);
  vec4 world_center = in_transform * vec4(0.0, 0.0, 0.0, 1.0);
  vec4 clip_pos = push.view_projection * world_pos;
  vec4 clip_center = push.view_projection * world_center;

  vec2 vertex_ndc = clip_pos.xy / clip_pos.w;
  vec2 center_ndc = clip_center.xy / clip_center.w;
  vec2 outline_dir = vertex_ndc - center_ndc;
  float outline_dir_len = length(outline_dir);
  if (outline_dir_len > 0.00001) {
    outline_dir /= outline_dir_len;
  }

  clip_pos.xy += outline_dir * push.outline_width * 2.0 / push.viewport_size * clip_pos.w;
  gl_Position = clip_pos;
  out_color = in_instance_color;
}
