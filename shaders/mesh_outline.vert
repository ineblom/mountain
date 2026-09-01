#version 460

layout(push_constant) uniform PushConstants {
  mat4 view_projection;
  vec2 viewport_size;
  vec2 outline_offset;
} push;

layout(location = 0) in vec4 in_pos;
layout(location = 4) in mat4 in_transform;
layout(location = 8) in vec4 in_instance_color;

layout(location = 0) flat out vec4 out_color;

void main() {
  vec4 world_pos = in_transform * vec4(in_pos.xyz, 1.0);
  vec4 clip_pos = push.view_projection * world_pos;
  clip_pos.xy += push.outline_offset * 2.0 / push.viewport_size * clip_pos.w;
  gl_Position = clip_pos;
  out_color = in_instance_color;
}
