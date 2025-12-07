#version 460

layout(location = 0) in vec4 in_color_tl;
layout(location = 1) in vec4 in_color_tr;
layout(location = 2) in vec4 in_color_bl;
layout(location = 3) in vec4 in_color_br;
layout(location = 4) in vec2 in_rect_pos;        // 0-1 position within rect
layout(location = 5) in vec2 in_rect_size;       // width, height in pixels
layout(location = 6) in vec4 in_corner_radii;    // TL, TR, BL, BR
layout(location = 7) in vec4 in_border_color;
layout(location = 8) in float in_border_width;

layout(location = 0) out vec4 out_color;

float rounded_box_sdf(vec2 pos, vec2 size, vec4 radii) {
    vec2 pixel_pos = pos * size;
    vec2 half_size = size * 0.5;
    vec2 p = pixel_pos - half_size;

    float r;
    if (p.x > 0.0) {
        r = (p.y > 0.0) ? radii.w : radii.y;  // BR : TR
    } else {
        r = (p.y > 0.0) ? radii.z : radii.x;  // BL : TL
    }

    r = min(r, min(half_size.x, half_size.y));

    vec2 q = abs(p) - half_size + r;
    float dist = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;

    return dist;
}

// Bayer matrix dithering for better quality
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

// Bilinear gradient interpolation (colors already in linear space)
vec4 bilinear_gradient(vec4 tl, vec4 tr, vec4 bl, vec4 br, vec2 pos) {
    // Bilinear interpolation
    vec4 top = mix(tl, tr, pos.x);
    vec4 bottom = mix(bl, br, pos.x);
    return mix(top, bottom, pos.y);
}

void main() {
    float distance = rounded_box_sdf(in_rect_pos, in_rect_size, in_corner_radii);

    if (distance > 0.0) {
        discard;
    }

    // Derivative-based anti-aliasing for smoother edges
    float aa_range = fwidth(distance);
    float outer_alpha = 1.0 - smoothstep(-aa_range, 0.0, distance);

    float border_distance = distance + in_border_width;

    vec2 pixel_pos = in_rect_pos * in_rect_size;
    float dither_value = bayer4x4(pixel_pos);

    if (in_border_width > 0.0 && border_distance > 0.0) {
        float border_alpha = smoothstep(-aa_range, 0.0, border_distance);
        vec3 color = in_border_color.rgb + vec3(dither_value);
        out_color = vec4(color, in_border_color.a * outer_alpha * border_alpha);
    } else {
        // Per-fragment gamma-correct color interpolation
        vec4 interpolated = bilinear_gradient(in_color_tl, in_color_tr, in_color_bl, in_color_br, in_rect_pos);
        vec3 color = interpolated.rgb + vec3(dither_value);
        out_color = vec4(color, interpolated.a * outer_alpha);
    }
}