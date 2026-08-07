#version 460

layout(binding = 0, set = 0) uniform sampler2D u_texture;

layout(location = 0) flat in vec4 in_color_tl;
layout(location = 1) flat in vec4 in_color_tr;
layout(location = 2) flat in vec4 in_color_bl;
layout(location = 3) flat in vec4 in_color_br;
layout(location = 4) in vec2 in_rect_pos;
layout(location = 5) flat in vec2 in_rect_size;
layout(location = 6) flat in vec4 in_corner_radii;
layout(location = 7) flat in vec4 in_border_color;
layout(location = 8) flat in float in_border_width;
layout(location = 9) flat in float in_softness;
layout(location = 10) in vec2 in_texture_uv;
layout(location = 11) flat in float in_omit_texture;

layout(location = 0) out vec4 out_color;

float rounded_box_sdf(vec2 sample_pos, vec2 rect_half_size, float r) {
    vec2 q = abs(sample_pos) - rect_half_size + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

float sdf_coverage(float distance, float softness) {
    // fwidth keeps the transition approximately one pixel wide. Softness is
    // retained as an optional minimum width for deliberately softer edges.
    float width = max(fwidth(distance), softness);
    return 1.0 - smoothstep(-0.5 * width, 0.5 * width, distance);
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

vec4 interpolate_linear_rgba(vec4 tl, vec4 tr, vec4 bl, vec4 br, vec2 pos) {
  vec4 top = mix(tl, tr, pos.x);
  vec4 bottom = mix(bl, br, pos.x);
  return mix(top, bottom, pos.y);
}

void main() {
    vec2 rect_half_size = in_rect_size * 0.5;
    vec2 sdf_sample_pos = (2.0 * in_rect_pos - 1.0) * rect_half_size;

    bool is_solid = all(equal(in_color_tl, in_color_tr)) &&
                    all(equal(in_color_tl, in_color_bl)) &&
                    all(equal(in_color_tl, in_color_br));
    vec4 fill_color = is_solid ? in_color_tl : interpolate_linear_rgba(in_color_tl, in_color_tr, in_color_bl, in_color_br, in_rect_pos);

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

    float outer_sdf = rounded_box_sdf(
        sdf_sample_pos,
        rect_half_size,
        corner_radius
    );
    float outer_coverage = sdf_coverage(outer_sdf, in_softness);

    float inner_coverage = 1.0;
    if (in_border_width > 0.0) {
        float inner_sdf = rounded_box_sdf(
            sdf_sample_pos,
            max(rect_half_size - vec2(in_border_width), vec2(0.0)),
            max(corner_radius - in_border_width, 0.0)
        );
        inner_coverage = sdf_coverage(inner_sdf, in_softness);
    }

    vec2 pixel_pos = in_rect_pos * in_rect_size;
    float dither_value = bayer4x4(pixel_pos);

    vec4 base_color = mix(in_border_color, fill_color, inner_coverage);
    base_color.rgb += vec3(dither_value);
    base_color.a *= outer_coverage;

    out_color = base_color;
}
