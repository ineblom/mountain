#version 460

layout(binding = 0, set = 0) uniform sampler2D u_texture;

layout(location = 0) flat in vec4 in_color_tl;
layout(location = 1) flat in vec4 in_color_tr;
layout(location = 2) flat in vec4 in_color_bl;
layout(location = 3) flat in vec4 in_color_br;
layout(location = 4) in vec2 in_rect_pos;        // 0-1 position within rect
layout(location = 5) flat in vec2 in_rect_size;       // width, height in pixels
layout(location = 6) flat in vec4 in_corner_radii;    // TL, TR, BL, BR
layout(location = 7) flat in vec4 in_border_color;
layout(location = 8) flat in float in_border_width;
layout(location = 9) flat in float in_softness;
layout(location = 10) in vec2 in_texture_uv;
layout(location = 11) flat in float in_omit_texture;

layout(location = 0) out vec4 out_color;

float rounded_box_sdf(vec2 sample_pos, vec2 rect_half_size, float r) {
    return length(max(abs(sample_pos) - rect_half_size + r, 0.0)) - r;
}

float bayer4x4(vec2 pos) {
    int x = int(mod(pos.x, 4.0));
    int y = int(mod(pos.y, 4.0));
    const float bayer[16] = float[](
        0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
        12.0/16.0, 4.0/16.0, 14.0/16.0,  6.0/16.0,
        3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
        15.0/16.0, 7.0/16.0, 13.0/16.0,  5.0/16.0
    );
    return bayer[y * 4 + x] / 255.0 - 0.5/255.0;
}

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

float interpolate_hue(float h1, float h2, float t) {
  float diff = h2 - h1;
  diff = diff - TWO_PI * floor((diff + PI) / TWO_PI);
  return h1 + diff * t;
}

vec4 interpolate_oklch(vec4 tl, vec4 tr, vec4 bl, vec4 br, vec2 pos) {
  vec4 top = vec4(
    mix(tl.r, tr.r, pos.x),
    mix(tl.g, tr.g, pos.x),
    interpolate_hue(tl.b, tr.b, pos.x),
    mix(tl.a, tr.a, pos.x)
  );

  vec4 bottom = vec4(
    mix(bl.r, br.r, pos.x),
    mix(bl.g, br.g, pos.x),
    interpolate_hue(bl.b, br.b, pos.x),
    mix(bl.a, br.a, pos.x)
  );

  return vec4(
    mix(top.r, bottom.r, pos.y),
    mix(top.g, bottom.g, pos.y),
    interpolate_hue(top.b, bottom.b, pos.y),
    mix(top.a, bottom.a, pos.y)
  );
}

vec4 oklch_to_linear_rgb(vec4 oklch) {
  vec2 ab = oklch.g * vec2(cos(oklch.b), sin(oklch.b));

  vec3 lms = vec3(
    oklch.r + 0.3963377774 * ab.x + 0.2158037573 * ab.y,
    oklch.r - 0.1055613458 * ab.x - 0.0638541728 * ab.y,
    oklch.r - 0.0894841775 * ab.x - 1.2914855480 * ab.y
  );

  lms = lms * lms * lms;

  return vec4(
    dot(lms, vec3(+4.0767416621, -3.3077115913, +0.2309699292)),
    dot(lms, vec3(-1.2684380046, +2.6097574011, -0.3413193965)),
    dot(lms, vec3(-0.0041960863, -0.7034186147, +1.7076147010)),
    oklch.a
  );
}

void main() {
    vec2 rect_half_size = in_rect_size * 0.5;
    vec2 sdf_sample_pos = (2.0 * in_rect_pos - 1.0) * rect_half_size;

    bool is_solid = all(equal(in_color_tl, in_color_tr)) &&
                    all(equal(in_color_tl, in_color_bl)) &&
                    all(equal(in_color_tl, in_color_br));
    vec4 fill_color_oklch = is_solid ? in_color_tl : interpolate_oklch(in_color_tl, in_color_tr, in_color_bl, in_color_br, in_rect_pos);
    vec4 fill_color = oklch_to_linear_rgb(fill_color_oklch);

    vec4 texture_sample = vec4(1);
    if (in_omit_texture < 1.0) {
        texture_sample = texture(u_texture, in_texture_uv);
    }
    fill_color = fill_color * texture_sample;

    float corner_radius;
    if (in_rect_pos.x < 0.5) {
        corner_radius = (in_rect_pos.y < 0.5) ? in_corner_radii.x : in_corner_radii.z;  // TL : BL
    } else {
        corner_radius = (in_rect_pos.y < 0.5) ? in_corner_radii.y : in_corner_radii.w;  // TR : BR
    }
    corner_radius = min(corner_radius, min(rect_half_size.x, rect_half_size.y));

    float border_mix = 0.0;
    if (in_border_width > 0.0) {
        float border_sdf_s = rounded_box_sdf(
            sdf_sample_pos,
            rect_half_size - vec2(in_softness * 2.0) - in_border_width,
            max(corner_radius - in_border_width, 0.0)
        );
        border_mix = smoothstep(0.0, 2.0 * in_softness, border_sdf_s);
    }

    float corner_sdf_t = 1.0;
    if (corner_radius > 0.0 || in_softness > 0.75) {
        float corner_sdf_s = rounded_box_sdf(
            sdf_sample_pos,
            rect_half_size - vec2(in_softness * 2.0),
            corner_radius
        );
        corner_sdf_t = 1.0 - smoothstep(0.0, 2.0 * in_softness, corner_sdf_s);
    }

    vec2 pixel_pos = in_rect_pos * in_rect_size;
    float dither_value = bayer4x4(pixel_pos);

    vec4 border_linear = oklch_to_linear_rgb(in_border_color);
    vec4 base_color = mix(fill_color, border_linear, border_mix);
    base_color.rgb += vec3(dither_value);
    base_color.a *= corner_sdf_t;

    out_color = base_color;
}