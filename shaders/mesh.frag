#version 460

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec3 in_normal;
layout(location = 2) flat in uint in_feature_flags;

layout(location = 0) out vec4 out_color;

void main() {
  const uint MESH_FEATURE_UNLIT = 1u << 0;

  vec3 normal = normalize(in_normal);
  float light = max(dot(normal, normalize(vec3(0.4, 0.7, 0.6))), 0.0);
  float shade = 0.25 + 0.75 * light;
  if ((in_feature_flags & MESH_FEATURE_UNLIT) != 0) {
    out_color = in_color;
  } else {
    out_color = vec4(in_color.rgb * shade, in_color.a);
  }
}
