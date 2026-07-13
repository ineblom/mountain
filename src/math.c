#if (HEADER)

#define PI 3.141592653598979f
#define GOLDEN_RATIO 1.61803398875f

typedef struct Random_State Random_State;
struct Random_State {
  L1 state;
  L1 inc;
};

typedef enum Side {
  SIDE__INVALID = -1,
  SIDE__MIN,
  SIDE__MAX,
  SIDE_COUNT,
} Side;
#define side_flip(s) ((Side)(!(s)))

typedef I1 Axis;
enum {
  AXIS__INVALID = -1,
  AXIS__X,
  AXIS__Y,
  AXIS__Z,

  AXIS2_COUNT = 2,
  AXIS3_COUNT = 3,
};
#define axis2_flip(a) ((Axis)(!(a)))

typedef I1 Dir;
enum {
  DIR__RIGHT = 0,
  DIR__UP,
  DIR__LEFT,
  DIR__DOWN,

  DIR_COUNT,
};

#endif

#if (SOURCE)

////////////////////////////////
//~ General
#define Square(x) ((x)*(x))

////////////////////////////////
//~ F1

#define abs_F1(x) fabsf(x)
#define abs_SI1(x) abs(x)
#define abs_SL1(x) llabs(x)
#define mod_F1(x, y) fmodf(x, y)
#define floor_F1(x) floorf(x)
#define round_F1(x) roundf(x)
#define ceil_F1(x) ceilf(x)
#define sin_F1(x) sinf(x)
#define cos_F1(x) cosf(x)
#define tan_F1(x) tanf(x)

Inline F1 clamp01_F1(F1 x) { return Min(Max(x, 0.0f), 1.0f); }
Inline F1 saturate_F1(F1 x) { return clamp01_F1(x); }

Inline F1 sign_F1(F1 a) {
  F1 result = -1.0f;
  if (a > 0.0f) result = 1.0f;
  return result;
}

Inline F1 lerp_F1(F1 a, F1 t, F1 b) {
  F1 result = a * (1-t) + b*t;
  return result;
}

Inline F1 lerp_snap_F1(F1 a, F1 t, F1 b, F1 min_dist) {
  F1 result = lerp_F1(a, t, b);
  if (abs_F1(a-b) < min_dist) {
    result = b;
  }
  return result;
}

////////////////////////////////
//~ F2

Inline F1 dot_F2(F2 v) {
  F1 result = v[0]*v[0] + v[1]*v[1];
  return result;
}

Inline F1 length_sq_F2(F2 v) {
  F1 result = dot_F2(v);
  return result;
}

Inline F1 length_F2(F2 v) {
  F1 result = sqrtf(length_sq_F2(v));
  return result;
}

////////////////////////////////
//~ F4

Inline F1 dot_F4(F4 a, F4 b) {
  F1 result = a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
  return result;
}

Inline F1 length_sq_F4(F4 v) {
  F1 result = dot_F4(v, v);
  return result;
}

Inline F1 length_F4(F4 v) {
  F1 result = sqrtf(length_sq_F4(v));
  return result;
}

Inline F4 normalize_F4(F4 v) {
  F4 result = v * (1.0f / length_F4(v));
  return result;
}

Inline F4 cross_F4(F4 a, F4 b) {
  F4 result = {0};

  result[0] = a[1]*b[2] - a[2]*b[1];
  result[1] = a[2]*b[0] - a[0]*b[2];
  result[2] = a[0]*b[1] - a[1]*b[0];

  return result;
}

Inline F4 lerp_F4(F4 a, F1 t, F4 b) {
  F4 result = a * (1-t) + b*t;
  return result;
}

Inline F4 lerp_snap_F4(F4 a, F1 t, F4 b, F1 min_dist) {
  F4 result = lerp_F4(a, t, b);
  if (length_sq_F4(a-b) < Square(min_dist)) {
    result = b;
  }
  return result;
}

Inline F4 abs_F4(F4 v) {
  F4 result = { fabsf(v[0]), fabsf(v[1]), fabsf(v[2]), fabsf(v[3]) };
  return result;
}

Inline F4 clamp01_F4(F4 v) {
  F4 result= {
    clamp01_F1(v[0]),
    clamp01_F1(v[1]),
    clamp01_F1(v[2]),
    clamp01_F1(v[3])
  };
  return result;
}

Inline F1 luminance_F4(F4 v) {
  F1 result = 0.2126f*v[0] + 0.7152f*v[1] + 0.0722f*v[2];
  return result;
}

Inline F4 pow_F4(F4 v, F1 exp) {
  F4 result = {
    powf(v[0], exp),
    powf(v[1], exp),
    powf(v[2], exp),
    powf(v[3], exp)
  };
  return result;
}

Inline F4 reflect_F4(F4 v, F4 normal) {
  F4 result = v - 2*dot_F4(v, normal) * normal;
  return result;
}

////////////////////////////////
//~ M4F

Inline M4F identity_M4F(void) {
  M4F result = {0};
  result.m[0][0] = 1.0f;
  result.m[1][1] = 1.0f;
  result.m[2][2] = 1.0f;
  result.m[3][3] = 1.0f;
  return result;
}

Inline M4F mul_M4F(M4F a, M4F b) {
  M4F result = {0};
  for (I1 row = 0; row < 4; row += 1) {
    for (I1 col = 0; col < 4; col += 1) {
      F1 sum = 0.0f;
      for (I1 i = 0; i < 4; i += 1) {
        sum += a.m[row][i] * b.m[i][col];
      }
      result.m[row][col] = sum;
    }
  }
  return result;
}

Inline M4F translate_M4F(F4 p) {
  M4F result = identity_M4F();
  result.m[3][0] = p[0];
  result.m[3][1] = p[1];
  result.m[3][2] = p[2];
  return result;
}

Inline M4F scale_M4F(F4 s) {
  M4F result = identity_M4F();
  result.m[0][0] = s[0];
  result.m[1][1] = s[1];
  result.m[2][2] = s[2];
  return result;
}

Inline M4F rotate_x_M4F(F1 angle_rad) {
  F1 c = cos_F1(angle_rad);
  F1 s = sin_F1(angle_rad);
  M4F result = identity_M4F();
  result.m[1][1] = c;
  result.m[1][2] = s;
  result.m[2][1] = -s;
  result.m[2][2] = c;
  return result;
}

Inline M4F rotate_y_M4F(F1 angle_rad) {
  F1 c = cos_F1(angle_rad);
  F1 s = sin_F1(angle_rad);
  M4F result = identity_M4F();
  result.m[0][0] = c;
  result.m[0][2] = -s;
  result.m[2][0] = s;
  result.m[2][2] = c;
  return result;
}

Inline M4F rotate_z_M4F(F1 angle_rad) {
  F1 c = cos_F1(angle_rad);
  F1 s = sin_F1(angle_rad);
  M4F result = identity_M4F();
  result.m[0][0] = c;
  result.m[0][1] = s;
  result.m[1][0] = -s;
  result.m[1][1] = c;
  return result;
}

Inline M4F perspective_fov_M4F(F1 fov_angle_y, F1 aspect_ratio, F1 near_distance, F1 far_distance) {
  Assert(fov_angle_y > 2.0E-8f);
  Assert(aspect_ratio > 1.0E-8f);
  Assert(near_distance > 0.0f && far_distance > 0.0f);
  Assert((far_distance - near_distance) > 1.0E-8f);

  F1 sin_fov = sin_F1(fov_angle_y * 0.5f);
  F1 cos_fov = cos_F1(fov_angle_y * 0.5f);
  F1 scaled_view_height = cos_fov / sin_fov;
  F1 scaled_view_width = scaled_view_height / aspect_ratio;
  F1 scaled_far_distance = far_distance / (far_distance - near_distance);

  M4F result = {0};
  result.m[0][0] = scaled_view_width;
  result.m[1][1] = scaled_view_height;
  result.m[2][2] = scaled_far_distance;
  result.m[2][3] = 1.0f;
  result.m[3][2] = -scaled_far_distance * near_distance;
  return result;
}

Inline F4 mul_M4F_F4(M4F m, F4 v) {
  F4 result = {0};
  for (I1 col = 0; col < 4; col += 1) {
    result[col] =
      v[0] * m.m[0][col] +
      v[1] * m.m[1][col] +
      v[2] * m.m[2][col] +
      v[3] * m.m[3][col];
  }
  return result;
}

////////////////////////////////
//~ Ray

Internal F1 ray_sphere_intersect(F4 ray_origin, F4 ray_direction, F4 sphere_origin, F1 sphere_radius) {
  F1 result = 0.0f;

  F4 sphere_relative_ray_origin = ray_origin - sphere_origin;
  F1 b = 2.0f * dot_F4(ray_direction, sphere_relative_ray_origin);
  F1 c = dot_F4(sphere_relative_ray_origin, sphere_relative_ray_origin) - sphere_radius*sphere_radius;

  F1 discriminant = b*b - 4.0f*c;
  if (discriminant > 0.0f) {
    F1 root_term = sqrtf(discriminant);
    if (root_term > 0.0001f) {
      F1 tp = (-b + root_term) / 2.0f;
      F1 tn = (-b - root_term) / 2.0f;

      result = tp;
      if (tn > 0.001f && tn < tp) result = tn;
    }
  }
  return result;
}

Internal F1 ray_aabb_intersect(F4 ray_origin, F4 ray_direction, F4 aabb_min, F4 aabb_max) {
  F1 t_min = (aabb_min[0] - ray_origin[0]) / ray_direction[0];
  F1 t_max = (aabb_max[0] - ray_origin[0]) / ray_direction[0];
  if (t_min > t_max) Swap(t_min, t_max);

  F1 ty_min = (aabb_min[1] - ray_origin[1]) / ray_direction[1];
  F1 ty_max = (aabb_max[1] - ray_origin[1]) / ray_direction[1];
  if (ty_min > ty_max) Swap(ty_min, ty_max);

  t_min = Max(t_min, ty_min);
  t_max = Min(t_max, ty_max);

  if (t_min > t_max) return 0.0f;

  F1 tz_min = (aabb_min[2] - ray_origin[2]) / ray_direction[2];
  F1 tz_max = (aabb_max[2] - ray_origin[2]) / ray_direction[2];
  if (tz_min > tz_max) Swap(tz_min, tz_max);

  t_min = Max(t_min, tz_min);
  t_max = Min(t_max, tz_max);

  if (t_min > t_max) return 0.0f;

  return t_min;
}

////////////////////////////////
//~ Random

Inline I1 rand_pcg(Random_State *rng) {
  L1 oldstate = rng->state;
  rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
  I1 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
  I1 rot = oldstate >> 59u;
  I1 result = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
  return result;
}

Inline F1 random_unilateral(Random_State *rng) {
  F1 result = (F1)rand_pcg(rng) / (F1)I1_MAX;
  return result;
}

Inline F1 random_bilateral(Random_State *rng) {
  F1 result = -1 + 2 * random_unilateral(rng);
  return result;
}

////////////////////////////////
//~ Rectangle

Inline F4 rect_overlap(F4 a, F4 b) {
  F4 result = {0};

  result[0] = Max(a[0], b[0]);
  result[1] = Max(a[1], b[1]);
  result[2] = Min(a[0]+a[2], b[0]+b[2]) - result[0];
  result[3] = Min(a[1]+a[3], b[1]+b[3]) - result[1];

  return result;
}

Inline I1 rect_contains(F4 rect, F2 point) {
  I1 result = 0;

  if (point[0] >= rect[0] &&
      point[1] >= rect[1] &&
      point[0] <= rect[0]+rect[2] &&
      point[1] <= rect[1]+rect[3]) {
    result = 1;
  }

  return result;
}

Inline F4 rect_pad(F4 rect, F1 amt) {
  F4 result = {
    rect[0] - amt,
    rect[1] - amt,
    rect[2] + amt*2,
    rect[3] + amt*2,
  };
  return result;
}

Inline F2 rect_center(F4 rect) {
  F2 result = {
    rect[0] + rect[2]*0.5,
    rect[1] + rect[3]*0.5,
  };
  return result;
}

////////////////////////////////
//~ kti: Colors

F4 oklch(F1 l, F1 c, F1 h, F1 alpha) {
  F1 h_rad = h * (PI / 180.0f);
  F4 result = { l, c, h_rad, alpha };
  return result;
}

F4 oklch_from_linear_rgb(F4 rgba) {
  F1 r = rgba[0];
  F1 g = rgba[1];
  F1 b = rgba[2];
  F1 a = rgba[3];

  F1 l = 0.4122214708f*r + 0.5363325363f*g + 0.0514459929f*b;
  F1 m = 0.2119034982f*r + 0.6806995451f*g + 0.1073969566f*b;
  F1 s = 0.0883024619f*r + 0.2817188376f*g + 0.6299787005f*b;

  F1 l_ = cbrtf(l);
  F1 m_ = cbrtf(m);
  F1 s_ = cbrtf(s);

  F1 ok_l = 0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_;
  F1 ok_a = 1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_;
  F1 ok_b = 0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_;

  F1 chroma = sqrtf(ok_a*ok_a + ok_b*ok_b);
  F1 hue = atan2f(ok_b, ok_a);
  if (hue < 0.0f) hue += 2.0f*PI;

  F4 result = { ok_l, chroma, hue, a };
  return result;
}


#endif
