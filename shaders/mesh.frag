#version 460

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec4 out_color;

void main() {
  vec3 normal = normalize(in_normal);
  float light = max(dot(normal, normalize(vec3(0.4, 0.7, 0.6))), 0.0);
  float shade = 0.25 + 0.75 * light;
  out_color = vec4(in_color.rgb * shade, in_color.a);
}
