#version 460

layout(push_constant) uniform PushConstants {
  mat4 view_projection;
  vec2 viewport_size;
} push;

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_normal;
layout(location = 4) in mat4 in_transform;
layout(location = 8) in vec4 in_instance_color;
layout(location = 9) in uint in_flags;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_normal;

void main() {
  vec4 world_pos = in_transform * vec4(in_pos.xyz, 1.0);
  gl_Position = push.view_projection * world_pos;

  out_color = in_instance_color;
  out_normal = mat3(in_transform) * in_normal.xyz;
}
