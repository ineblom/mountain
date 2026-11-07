#version 460

// Final pass: reads the linear float scene target, encodes it to sRGB once,
// and dithers in encoded space right before the 8-bit UNORM swapchain
// quantizes it. This is the only place in the pipeline that knows about sRGB.

layout(binding = 0, set = 0) uniform sampler2D u_scene;

layout(location = 0) out vec4 out_color;

vec3 srgb_from_linear(vec3 c) {
  return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(0.0031308, c));
}

// Centered ordered dither, +-0.5 of an 8-bit step.
float bayer4x4(ivec2 p) {
  const float bayer[16] = float[](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
  );
  return ((bayer[(p.y & 3) * 4 + (p.x & 3)] + 0.5) / 16.0 - 0.5) / 255.0;
}

void main() {
  ivec2 pixel   = ivec2(gl_FragCoord.xy);
  vec3  linear  = clamp(texelFetch(u_scene, pixel, 0).rgb, 0.0, 1.0);
  vec3  encoded = srgb_from_linear(linear);

  out_color = vec4(encoded + vec3(bayer4x4(pixel)), 1.0);
}
