#version 460

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_rect_pos;        // 0-1 position within rect
layout(location = 2) in vec2 in_rect_size;       // width, height in pixels
layout(location = 3) in vec4 in_corner_radii;    // TL, TR, BL, BR
layout(location = 4) in vec4 in_border_color;
layout(location = 5) in float in_border_width;
layout(location = 6) in float in_softness;

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

vec4 oklab_to_linear_rgb(vec4 color) {
  float l = color.r + 0.3963377774f * color.g + 0.2158037573f * color.b;
  float m = color.r - 0.1055613458f * color.g - 0.0638541728f * color.b;
  float s = color.r - 0.0894841775f * color.g - 1.2914855480f * color.b;

  float l3 = l * l * l;
  float m3 = m * m * m;
  float s3 = s * s * s;

  return vec4(
    +4.0767416621f * l3 - 3.3077115913f * m3 + 0.2309699292f * s3,
    -1.2684380046f * l3 + 2.6097574011f * m3 - 0.3413193965f * s3,
    -0.0041960863f * l3 - 0.7034186147f * m3 + 1.7076147010f * s3,
    color.a
  );
}

void main() {
    vec2 rect_half_size = in_rect_size * 0.5;
    vec2 sdf_sample_pos = (2.0 * in_rect_pos - 1.0) * rect_half_size;

    vec4 fill_color = in_color;

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

    vec4 base_color = mix(fill_color, in_border_color, border_mix);

    vec2 pixel_pos = in_rect_pos * in_rect_size;
    float dither_value = bayer4x4(pixel_pos);

    vec4 linear_color = oklab_to_linear_rgb(base_color);
    linear_color.rgb += vec3(dither_value);
    linear_color.a *= corner_sdf_t;

    out_color = linear_color;
}