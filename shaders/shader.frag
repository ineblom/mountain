#version 460

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec2 in_rect_pos;        // 0-1 position within rect
layout(location = 2) in vec2 in_rect_size;       // width, height in pixels
layout(location = 3) in vec4 in_corner_radii;    // TL, TR, BR, BL
layout(location = 4) in vec4 in_border_color;
layout(location = 5) in float in_border_width;

layout(location = 0) out vec4 out_color;

float rounded_box_sdf(vec2 pos, vec2 size, vec4 radii) {
    vec2 pixel_pos = pos * size;
    vec2 half_size = size * 0.5;
    vec2 p = pixel_pos - half_size;

    float r;
    if (p.x > 0.0) {
        r = (p.y > 0.0) ? radii.z : radii.y;  // BR : TR
    } else {
        r = (p.y > 0.0) ? radii.w : radii.x;  // BL : TL
    }

    r = min(r, min(half_size.x, half_size.y));

    vec2 q = abs(p) - half_size + r;
    float dist = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;

    return dist;
}

void main() {
    float distance = rounded_box_sdf(in_rect_pos, in_rect_size, in_corner_radii);

    if (distance > 0.0) {
        discard;
    }

    float outer_alpha = 1.0 - smoothstep(-1.0, 0.0, distance);

    float border_distance = distance + in_border_width;

    if (in_border_width > 0.0 && border_distance > 0.0) {
        float border_alpha = smoothstep(-1.0, 0.0, border_distance);
        out_color = vec4(in_border_color.rgb, in_border_color.a * outer_alpha * border_alpha);
    } else {
        out_color = vec4(in_color.rgb, in_color.a * outer_alpha);
    }
}