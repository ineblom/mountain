#include "math.h"

#if defined(__AVX__)
#include <immintrin.h>
#elif defined(__SSE__)
#include <xmmintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

////////////////////////////////
//~ General

////////////////////////////////
//~ F1

Inline F1 clamp01_F1(F1 x)
{
  return Min(Max(x, 0.0f), 1.0f);
}

Inline F1 saturate_F1(F1 x)
{
  return clamp01_F1(x);
}

Inline F1 sign_F1(F1 a)
{
  F1 result = -1.0f;

  if (a > 0.0f)
    result = 1.0f;
  return result;
}

Inline F1 lerp_F1(F1 a, F1 t, F1 b)
{
  F1 result = a * (1 - t) + b * t;
  return result;
}

Inline F1 lerp_snap_F1(F1 a, F1 t, F1 b, F1 min_dist)
{
  F1 result = lerp_F1(a, t, b);

  if (abs_F1(a - b) < min_dist)
  {
    result = b;
  }

  return result;
}

////////////////////////////////
//~ F2

Inline F1 dot_F2(F2 a, F2 b)
{
  F1 result = a[0] * b[0] + a[1] * b[1];
  return result;
}

Inline F1 length_sq_F2(F2 v)
{
  F1 result = dot_F2(v, v);
  return result;
}

Inline F1 length_F2(F2 v)
{
  F1 result = sqrt_F1(length_sq_F2(v));
  return result;
}

Internal F1 distance_to_segment_F2(F2 p, F2 a, F2 b)
{
  F2  ab         =  b - a;
  F1  ab_len_sq  =  dot_F2(ab, ab);

  if (ab_len_sq <= 0.00001f)
  {
    return length_F2(p - a);
  }

  F1 t = dot_F2(p - a, ab) / ab_len_sq;

  t = Clamp(0.0f, t, 1.0f);

  F2 closest = a + t * ab;
  return length_F2(p - closest);
}

Inline F2 F2_from_F4(F4 v)
{
  F2 result = {v[0], v[1]};
  return result;
}

////////////////////////////////
//~ kti: V3

Inline V3 v3(F1 x, F1 y, F1 z)
{
  return (V3){.x = x, .y = y, .z = z};
}

Inline F1 component_V3(V3 v, Axis axis)
{
  return axis == AXIS__X ? v.x : axis == AXIS__Y ? v.y : v.z;
}

Inline F1 *component_ptr_V3(V3 *v, Axis axis)
{
  return axis == AXIS__X ? &v->x : axis == AXIS__Y ? &v->y : &v->z;
}

Inline V3 V3_from_F4(F4 v)
{
  return v3(v[0], v[1], v[2]);
}

Inline V3 add_V3(V3 a, V3 b) { return v3(a.x+b.x, a.y+b.y, a.z+b.z); }
Inline V3 sub_V3(V3 a, V3 b) { return v3(a.x-b.x, a.y-b.y, a.z-b.z); }
Inline V3 mul_V3(V3 a, V3 b) { return v3(a.x*b.x, a.y*b.y, a.z*b.z); }
Inline V3 div_V3(V3 a, V3 b) { return v3(a.x/b.x, a.y/b.y, a.z/b.z); }
Inline V3 scale_V3(V3 v, F1 s) { return v3(v.x*s, v.y*s, v.z*s); }
Inline V3 add_scaled_V3(V3 a, F1 s, V3 b) { return add_V3(a, scale_V3(b, s)); }
Inline V3 neg_V3(V3 v) { return scale_V3(v, -1.0f); }
Inline V3 abs_V3(V3 v) { return v3(abs_F1(v.x), abs_F1(v.y), abs_F1(v.z)); }

Inline F1 dot_V3(V3 a, V3 b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

Inline F1 length_sq_V3(V3 v) { return dot_V3(v, v); }
Inline F1 length_V3(V3 v) { return sqrt_F1(length_sq_V3(v)); }
Inline V3 normalize_V3(V3 v) { return scale_V3(v, 1.0f/length_V3(v)); }

Inline V3 cross_V3(V3 a, V3 b)
{
  return v3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

Inline V3 reflect_V3(V3 v, V3 normal)
{
  return sub_V3(v, scale_V3(normal, 2.0f*dot_V3(v, normal)));
}

Inline V3 lerp_V3(V3 a, F1 t, V3 b)
{
  return add_V3(scale_V3(a, 1.0f-t), scale_V3(b, t));
}

Inline V3 lerp_snap_V3(V3 a, F1 t, V3 b, F1 min_dist)
{
  V3 result = lerp_V3(a, t, b);
  if (length_sq_V3(sub_V3(a, b)) < Square(min_dist)) result = b;
  return result;
}

Inline V3 safe_inv_dir_V3(V3 v)
{
  return v3(1.0f/(v.x == 0.0f ? 1.0f : v.x),
            1.0f/(v.y == 0.0f ? 1.0f : v.y),
            1.0f/(v.z == 0.0f ? 1.0f : v.z));
}

////////////////////////////////
//~ kti: F4

Inline F4 splat_F4(F1 x)
{
  return (F4){x, x, x, x};
}

Inline F4 select_F4(SI4 mask, F4 yes, F4 no)
{
  // Bitcasts preserve floating-point payloads (including signed zero).
  union
  {
    F4 f;
    SI4 i;
  } a = {.f = yes}, b = {.f = no}, result;
  result.i = (mask & a.i) | (~mask & b.i);
  return result.f;
}

Inline F4 sqrt_F4(F4 v)
{
#if defined(__SSE__)
  return (F4)_mm_sqrt_ps((__m128)v);
#elif defined(__aarch64__) && defined(__ARM_NEON)
  return (F4)vsqrtq_f32((float32x4_t)v);
#else
  return (F4){
    sqrt_F1(v[0]),
    sqrt_F1(v[1]),
    sqrt_F1(v[2]),
    sqrt_F1(v[3]),
  };
#endif
}

Inline F4 max_F4(F4 a, F4 b)
{
#if defined(__SSE__)
  return (F4)_mm_max_ps((__m128)a, (__m128)b);
#elif defined(__ARM_NEON)
  return (F4)vmaxq_f32((float32x4_t)a, (float32x4_t)b);
#else
  return select_F4(a > b, a, b);
#endif
}

Inline F4 min_F4(F4 a, F4 b)
{
#if defined(__SSE__)
  return (F4)_mm_min_ps((__m128)a, (__m128)b);
#elif defined(__ARM_NEON)
  return (F4)vminq_f32((float32x4_t)a, (float32x4_t)b);
#else
  return select_F4(a < b, a, b);
#endif
}

Inline F4 F4_from_V3(V3 v)
{
  F4 result = {v.x, v.y, v.z, 0.0f};
  return result;
}

Inline F4 point_F4_from_V3(V3 v)
{
  return (F4){v.x, v.y, v.z, 1.0f};
}

Inline F4 lerp_F4(F4 a, F1 t, F4 b)
{
  F4 result = a * (1 - t) + b * t;
  return result;
}

Inline F4 abs_F4(F4 v)
{
  F4 result = select_F4(v < 0, -v, v);
  return result;
}

Inline F4 clamp01_F4(F4 v)
{
  return min_F4(max_F4(v, splat_F4(0)), splat_F4(1));
}

Inline F1 luminance_F4(F4 v)
{
  F1 result = 0.2126f * v[0] + 0.7152f * v[1] + 0.0722f * v[2];
  return result;
}

Inline F4 pow_F4(F4 v, F1 exp)
{
  F4 result = {powf(v[0], exp), powf(v[1], exp), powf(v[2], exp), powf(v[3], exp)};
  return result;
}

////////////////////////////////
//~ Geometry lanes

#define Lane_Repeat4(v) (v), (v), (v), (v)
#if LANE_COUNT == 4
#define Lane_Values(v) Lane_Repeat4(v)
#elif LANE_COUNT == 8
#define Lane_Values(v) Lane_Repeat4(v), Lane_Repeat4(v)
#else
#define Lane_Values(v) Lane_Repeat4(v), Lane_Repeat4(v), Lane_Repeat4(v), Lane_Repeat4(v)
#endif

Inline Lane_F32 splat_Lane_F32(F1 value)
{
  return (Lane_F32){Lane_Values(value)};
}

Inline Lane_S32 splat_Lane_S32(SI1 value)
{
  return (Lane_S32){Lane_Values(value)};
}

Inline Lane_U32 splat_Lane_U32(I1 value)
{
  return (Lane_U32){Lane_Values(value)};
}

#undef Lane_Values
#undef Lane_Repeat4

Inline Lane_S32 select_Lane_S32(Lane_Mask mask, Lane_S32 yes, Lane_S32 no)
{
  return (mask & yes) | (~mask & no);
}

Inline Lane_U32 select_Lane_U32(Lane_Mask mask, Lane_U32 yes, Lane_U32 no)
{
  Lane_U32 bits = (Lane_U32)mask;
  return (bits & yes) | (~bits & no);
}

Inline Lane_F32 select_Lane_F32(Lane_Mask mask, Lane_F32 yes, Lane_F32 no)
{
  union { Lane_F32 f; Lane_S32 i; } a = {.f = yes}, b = {.f = no}, result;
  result.i = select_Lane_S32(mask, a.i, b.i);
  return result.f;
}

Inline I1 any_Lane_Mask(Lane_Mask mask)
{
  for (L1 i = 0; i < LANE_COUNT; i += 1) if (mask[i]) return 1;
  return 0;
}

Inline I1 all_Lane_Mask(Lane_Mask mask)
{
  for (L1 i = 0; i < LANE_COUNT; i += 1) if (!mask[i]) return 0;
  return 1;
}

Inline Lane_F32 min_Lane_F32(Lane_F32 a, Lane_F32 b)
{
#if LANE_COUNT == 4
  return min_F4(a, b);
#elif LANE_COUNT == 8 && defined(__AVX__)
  return (Lane_F32)_mm256_min_ps((__m256)a, (__m256)b);
#elif LANE_COUNT == 16 && defined(__AVX512F__)
  return (Lane_F32)_mm512_min_ps((__m512)a, (__m512)b);
#else
  return select_Lane_F32(a < b, a, b);
#endif
}

Inline Lane_F32 max_Lane_F32(Lane_F32 a, Lane_F32 b)
{
#if LANE_COUNT == 4
  return max_F4(a, b);
#elif LANE_COUNT == 8 && defined(__AVX__)
  return (Lane_F32)_mm256_max_ps((__m256)a, (__m256)b);
#elif LANE_COUNT == 16 && defined(__AVX512F__)
  return (Lane_F32)_mm512_max_ps((__m512)a, (__m512)b);
#else
  return select_Lane_F32(a > b, a, b);
#endif
}

Inline Lane_F32 abs_Lane_F32(Lane_F32 v)
{
  union { Lane_F32 f; Lane_U32 u; } bits = {.f = v};
  bits.u &= splat_Lane_U32(0x7fffffffu);
  return bits.f;
}

Inline Lane_F32 sqrt_Lane_F32(Lane_F32 v)
{
#if LANE_COUNT == 4
  return sqrt_F4(v);
#elif LANE_COUNT == 16 && defined(__AVX512F__)
  return (Lane_F32)_mm512_sqrt_ps((__m512)v);
#elif LANE_COUNT == 8 && defined(__AVX__)
  return (Lane_F32)_mm256_sqrt_ps((__m256)v);
#else
  // SSE/NEON backends also support wider logical batches in four-lane pieces.
  Lane_F32 result = {0};
  for (L1 i = 0; i < LANE_COUNT; i += 4)
  {
    F4 part = sqrt_F4((F4){v[i], v[i+1], v[i+2], v[i+3]});
    for (L1 j = 0; j < 4; j += 1) result[i+j] = part[j];
  }
  return result;
#endif
}

Inline Lane_V3 broadcast_Lane_V3(V3 v)
{
  return (Lane_V3){splat_Lane_F32(v.x), splat_Lane_F32(v.y), splat_Lane_F32(v.z)};
}

Inline V3 extract_lane_V3(Lane_V3 v, L1 lane)
{
  return v3(v.x[lane], v.y[lane], v.z[lane]);
}

Inline void set_lane_V3(Lane_V3 *v, L1 lane, V3 value)
{
  v->x[lane] = value.x;
  v->y[lane] = value.y;
  v->z[lane] = value.z;
}

Inline Lane_V3 pack_Lane_V3(const V3 *values)
{
  Lane_V3 result = {0};
  for (L1 i = 0; i < LANE_COUNT; i += 1) set_lane_V3(&result, i, values[i]);
  return result;
}

Inline Lane_V3 add_Lane_V3(Lane_V3 a, Lane_V3 b) { return (Lane_V3){a.x+b.x, a.y+b.y, a.z+b.z}; }
Inline Lane_V3 sub_Lane_V3(Lane_V3 a, Lane_V3 b) { return (Lane_V3){a.x-b.x, a.y-b.y, a.z-b.z}; }
Inline Lane_V3 scale_Lane_V3(Lane_V3 v, Lane_F32 s) { return (Lane_V3){v.x*s, v.y*s, v.z*s}; }
Inline Lane_F32 dot_Lane_V3(Lane_V3 a, Lane_V3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

Inline Lane_V3 cross_Lane_V3(Lane_V3 a, Lane_V3 b)
{
  return (Lane_V3){a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}

Inline Lane_V3 normalize_Lane_V3(Lane_V3 v)
{
  return scale_Lane_V3(v, 1.0f/sqrt_Lane_F32(dot_Lane_V3(v, v)));
}

////////////////////////////////
//~ F4x3

Inline F4x3 sub_F4x3(F4x3 a, F4x3 b)
{
  F4x3 result = {
    .x = a.x - b.x,
    .y = a.y - b.y,
    .z = a.z - b.z,
  };
  return result;
}

Inline F4 dot_F4x3(F4x3 a, F4x3 b)
{
  F4 result = a.x*b.x + a.y*b.y + a.z*b.z;
  return result;
}

Inline F4 luminance_F4x3(F4x3 c)
{
  return 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
}

Inline F4x3 scale_F4x3(F4x3 c, F4 scale)
{
  return (F4x3){c.x * scale, c.y * scale, c.z * scale};
}

Inline F4x3 add_F4x3(F4x3 a, F4x3 b)
{
  return (F4x3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Inline F4x3 lerp_F4x3(F4x3 a, F4 t, F4x3 b)
{
  F4x3  left_contribution   =  scale_F4x3(a, 1.0f - t);
  F4x3  right_contribution  =  scale_F4x3(b, t);
  F4x3  result              =  add_F4x3(left_contribution, right_contribution);

  return result;
}

// Repeat the last valid lane; count must be between one and four.
Inline F4x3 pad_F4x3(F4x3 c, L1 count)
{
  for (L1 i = count; i < 4; ++i)
  {
    c.x[i] = c.x[count - 1];
    c.y[i] = c.y[count - 1];
    c.z[i] = c.z[count - 1];
  }
  return c;
}

Inline F4x3 F4x3_from_F4(F4 a, F4 b, F4 c, F4 d)
{
  F4  first_xy   =  shuffle_F4(a, b, F4_A0, F4_A1, F4_B0, F4_B1); // {ax, ay, bx, by}
  F4  second_xy  =  shuffle_F4(c, d, F4_A0, F4_A1, F4_B0, F4_B1); // {cx, cy, dx, dy}
  F4  first_zw   =  shuffle_F4(a, b, F4_A2, F4_A3, F4_B2, F4_B3); // {az, aw, bz, bw}
  F4  second_zw  =  shuffle_F4(c, d, F4_A2, F4_A3, F4_B2, F4_B3); // {cz, cw, dz, dw}

  return (F4x3){
    shuffle_F4(first_xy, second_xy, F4_A0, F4_A2, F4_B0, F4_B2), // {ax, bx, cx, dx}
    shuffle_F4(first_xy, second_xy, F4_A1, F4_A3, F4_B1, F4_B3), // {ay, by, cy, dy}
    shuffle_F4(first_zw, second_zw, F4_A0, F4_A2, F4_B0, F4_B2)  // {az, bz, cz, dz}
  };
}

// Gather is explicit because SSSE3 and NEON have no arbitrary memory gather.
// The interpolation and filtering following these loads still process four pixels.
Inline F4x3 gather_F4x3(const F4x3 *row, const L1 indices[4])
{
  L1  first_block_index   =  indices[0] / 4;
  L1  second_block_index  =  indices[1] / 4;
  L1  third_block_index   =  indices[2] / 4;
  L1  fourth_block_index  =  indices[3] / 4;

  L1  first_lane   =  indices[0] % 4;
  L1  second_lane  =  indices[1] % 4;
  L1  third_lane   =  indices[2] % 4;
  L1  fourth_lane  =  indices[3] % 4;

  const F4x3  *first_block   =  &row[first_block_index];
  const F4x3  *second_block  =  &row[second_block_index];
  const F4x3  *third_block   =  &row[third_block_index];
  const F4x3  *fourth_block  =  &row[fourth_block_index];

  F4 x = {
    first_block->x[first_lane],
    second_block->x[second_lane],
    third_block->x[third_lane],
    fourth_block->x[fourth_lane]
  };
  F4 y = {
    first_block->y[first_lane],
    second_block->y[second_lane],
    third_block->y[third_lane],
    fourth_block->y[fourth_lane]
  };
  F4 z = {
    first_block->z[first_lane],
    second_block->z[second_lane],
    third_block->z[third_lane],
    fourth_block->z[fourth_lane]
  };

  F4x3 result = {x, y, z};
  return result;
}

////////////////////////////////
//~ F4x4

Inline F4x4 identity_F4x4(void)
{
  F4x4 result = {0};

  result.m[0][0]  =  1.0f;
  result.m[1][1]  =  1.0f;
  result.m[2][2]  =  1.0f;
  result.m[3][3]  =  1.0f;
  return result;
}

Inline F4x4 mul_F4x4(F4x4 a, F4x4 b)
{
  F4x4 result = {0};

  for (I1 row = 0; row < 4; row += 1)
  {
    for (I1 col = 0; col < 4; col += 1)
    {
      F1 sum = 0.0f;

      for (I1 i = 0; i < 4; i += 1)
      {
        sum += a.m[row][i] * b.m[i][col];
      }

      result.m[row][col] = sum;
    }
  }

  return result;
}

Inline F4x4 translate_F4x4(V3 p)
{
  F4x4 result = identity_F4x4();

  result.m[3][0]  =  p.x;
  result.m[3][1]  =  p.y;
  result.m[3][2]  =  p.z;

  return result;
}

Inline F4x4 scale_F4x4(V3 s)
{
  F4x4 result = identity_F4x4();

  result.m[0][0]  =  s.x;
  result.m[1][1]  =  s.y;
  result.m[2][2]  =  s.z;

  return result;
}

Inline F4x4 rotate_x_F4x4(F1 angle_rad)
{
  F1    c       =  cos_F1(angle_rad);
  F1    s       =  sin_F1(angle_rad);
  F4x4  result  =  identity_F4x4();

  result.m[1][1]  =  c;
  result.m[1][2]  =  s;
  result.m[2][1]  =  -s;
  result.m[2][2]  =  c;

  return result;
}

Inline F4x4 rotate_y_F4x4(F1 angle_rad)
{
  F1    c       =  cos_F1(angle_rad);
  F1    s       =  sin_F1(angle_rad);
  F4x4  result  =  identity_F4x4();

  result.m[0][0]  =  c;
  result.m[0][2]  =  -s;
  result.m[2][0]  =  s;
  result.m[2][2]  =  c;

  return result;
}

Inline F4x4 rotate_z_F4x4(F1 angle_rad)
{
  F1    c       =  cos_F1(angle_rad);
  F1    s       =  sin_F1(angle_rad);
  F4x4  result  =  identity_F4x4();

  result.m[0][0]  =  c;
  result.m[0][1]  =  s;
  result.m[1][0]  =  -s;
  result.m[1][1]  =  c;

  return result;
}

Inline F4x4 perspective_fov_F4x4(F1 fov_angle_y, F1 aspect_ratio, F1 near_distance, F1 far_distance)
{
  F1  sin_fov              =  sin_F1(fov_angle_y * 0.5f);
  F1  cos_fov              =  cos_F1(fov_angle_y * 0.5f);
  F1  scaled_view_height   =  cos_fov / sin_fov;
  F1  scaled_view_width    =  scaled_view_height / aspect_ratio;
  F1  scaled_far_distance  =  far_distance / (far_distance - near_distance);

  F4x4 result = {0};

  result.m[0][0]  =  scaled_view_width;
  result.m[1][1]  =  scaled_view_height;
  result.m[2][2]  =  scaled_far_distance;
  result.m[2][3]  =  1.0f;
  result.m[3][2]  =  -scaled_far_distance * near_distance;

  return result;
}

Inline F4 mul_F4x4_F4(F4x4 m, F4 v)
{
  F4 result = {0};

  for (I1 col = 0; col < 4; col += 1)
  {
    result[col] = v[0] * m.m[0][col] + v[1] * m.m[1][col] + v[2] * m.m[2][col] + v[3] * m.m[3][col];
  }

  return result;
}

Inline V3 transform_direction_F4x4(F4x4 m, V3 direction)
{
  return V3_from_F4(mul_F4x4_F4(m, F4_from_V3(direction)));
}

Inline F4 project_point_F4x4(F4x4 m, V3 point)
{
  return mul_F4x4_F4(m, point_F4_from_V3(point));
}

////////////////////////////////
//~ kti: Shape

Internal String8 shape_kind_name(Shape_Kind kind)
{
  String8 result = str8("Unknown");

  switch (kind)
  {
    case SHAPE_KIND__SPHERE: result  =  str8("Sphere"); break;
    case SHAPE_KIND__BOX:    result  =  str8("Box");    break;
    case SHAPE_KIND__PLANE:  result  =  str8("Plane");  break;
    default: break;
  }

  return result;
}

////////////////////////////////
//~ kti: Ray

Internal F1 ray_plane_intersect(Ray ray, Plane plane)
{
  F1  min_t         =  0.001f;
  F1  result        =  F1_MAX;
  V3  plane_normal  =  plane.normal;
  F1  denom         =  dot_V3(plane_normal, ray.dir);

  if (abs_F1(denom) > 0.0001f)
  {
    F1 t = (-plane.d - dot_V3(plane_normal, ray.pos)) / denom;

    if (t > min_t)
    {
      result = t;
    }
  }

  return result;
}

Internal Lane_F32 ray_lane_plane_intersect(Ray ray, Lane_Plane planes)
{
  F1 min_t = 0.001f;

  Lane_V3 ray_pos = broadcast_Lane_V3(ray.pos);
  Lane_V3 ray_dir = broadcast_Lane_V3(ray.dir);

  Lane_F32 denom = dot_Lane_V3(planes.normal, ray_dir);

  Lane_F32 t = (-planes.d - dot_Lane_V3(planes.normal, ray_pos)) / denom;

  Lane_Mask valid  = (abs_Lane_F32(denom) > 0.0001f) & (t > min_t);
  Lane_F32  result = select_Lane_F32(valid, t, splat_Lane_F32(F1_MAX));

  return result;
}

Internal F1 ray_sphere_intersect(Ray ray, Sphere sphere)
{
  F1  min_t                    =  0.001f;
  F1  result                   =  F1_MAX;
  V3  sphere_pos               =  sphere.pos;
  V3  sphere_relative_ray_pos  =  sub_V3(ray.pos, sphere_pos);
  F1  half_b                   =  dot_V3(ray.dir, sphere_relative_ray_pos);
  F1  c                        =  dot_V3(sphere_relative_ray_pos, sphere_relative_ray_pos) - Square(sphere.radius);
  F1  discriminant             =  Square(half_b) - c;

  if (discriminant > 0.0f)
  {
    F1 root_term = sqrt_F1(discriminant);

    if (root_term > 0.0001f)
    {
      F1 tp = -half_b + root_term;
      F1 tn = -half_b - root_term;
      F1 t  = tn > min_t ? tn : tp;

      if (t > min_t)
        result = t;
    }
  }

  return result;
}

Internal Lane_F32 ray_lane_sphere_intersect(Ray ray, Lane_Sphere spheres)
{
  F1 min_t = 0.001f;

  Lane_V3 ray_pos = broadcast_Lane_V3(ray.pos);
  Lane_V3 ray_dir = broadcast_Lane_V3(ray.dir);
  Lane_V3 rel     = sub_Lane_V3(ray_pos, spheres.pos);

  Lane_F32 half_b       = dot_Lane_V3(ray_dir, rel);
  Lane_F32 c            = dot_Lane_V3(rel, rel) - Square(spheres.radius);
  Lane_F32 discriminant = Square(half_b) - c;

  Lane_F32 root_term = sqrt_Lane_F32(max_Lane_F32(discriminant, splat_Lane_F32(0.0f)));

  Lane_F32 far_t  = -half_b + root_term;
  Lane_F32 near_t = -half_b - root_term;
  Lane_F32 t      = select_Lane_F32(near_t > min_t, near_t, far_t);

  Lane_Mask valid_discriminant = discriminant > 0;
  Lane_Mask valid_root         = root_term > 0.0001f;
  Lane_Mask is_valid           = valid_discriminant & valid_root & (t > min_t);

  Lane_F32 result = select_Lane_F32(is_valid, t, splat_Lane_F32(F1_MAX));

  return result;
}

Internal F1 ray_box_intersect(Ray ray, Box box)
{
  F1  min_t     =  0.001f;
  V3  aabb_min  =  box.min;
  V3  aabb_max  =  box.max;
  F1  t_min     =  -F1_MAX;
  F1  t_max     =  F1_MAX;

  for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1)
  {
    F1 origin = component_V3(ray.pos, axis);
    F1 direction = component_V3(ray.dir, axis);
    F1 low = component_V3(aabb_min, axis);
    F1 high = component_V3(aabb_max, axis);

    if (direction == 0.0f)
    {
      if (origin < low || origin > high)
        return F1_MAX;

      continue;
    }

    F1 inv_dir = component_V3(ray.inv_dir, axis);
    F1 near_t = (low - origin) * inv_dir;
    F1 far_t  = (high - origin) * inv_dir;

    if (near_t > far_t)
      Swap(near_t, far_t);

    t_min = Max(t_min, near_t);
    t_max = Min(t_max, far_t);

    if (t_min > t_max)
      return F1_MAX;
  }

  F1 t = t_min > min_t ? t_min : t_max;
  return t > min_t ? t : F1_MAX;
}

Inline Lane_Mask ray_lane_slab_intersect(Lane_F32 low, Lane_F32 high, F1 origin_scalar, F1 dir_scalar, F1 inv_dir, Lane_F32 *near_t, Lane_F32 *far_t)
{
  Lane_F32 origin = splat_Lane_F32(origin_scalar);
  Lane_F32 dir = splat_Lane_F32(dir_scalar);

  Lane_Mask parallel = dir == 0.0f;
  Lane_Mask inside = (low <= origin) & (origin <= high);

  Lane_F32 t0 = (low - origin) * inv_dir;
  Lane_F32 t1 = (high - origin) * inv_dir;

  Lane_F32 min = min_Lane_F32(t0, t1);
  Lane_F32 max = max_Lane_F32(t0, t1);

  near_t[0] = select_Lane_F32(parallel, splat_Lane_F32(-INFINITY), min);
  far_t[0] = select_Lane_F32(parallel, splat_Lane_F32(INFINITY), max);

  Lane_Mask can_intersect = ~parallel | inside;

  return can_intersect;
}

Internal Lane_F32 ray_lane_box_intersect(Ray ray, Lane_Box boxes)
{
  F1 min_t = 0.001f;

  Lane_F32 near_x, far_x, near_y, far_y, near_z, far_z;

  Lane_Mask valid_x = ray_lane_slab_intersect(boxes.min.x, boxes.max.x, ray.pos.x, ray.dir.x, ray.inv_dir.x, &near_x, &far_x);
  Lane_Mask valid_y = ray_lane_slab_intersect(boxes.min.y, boxes.max.y, ray.pos.y, ray.dir.y, ray.inv_dir.y, &near_y, &far_y);
  Lane_Mask valid_z = ray_lane_slab_intersect(boxes.min.z, boxes.max.z, ray.pos.z, ray.dir.z, ray.inv_dir.z, &near_z, &far_z);

  Lane_F32 t_min = max_Lane_F32(max_Lane_F32(near_x, near_y), near_z);
  Lane_F32 t_max = min_Lane_F32(min_Lane_F32(far_x, far_y), far_z);

  Lane_F32 t = select_Lane_F32(t_min > min_t, t_min, t_max);

  Lane_Mask valid = valid_x & valid_y & valid_z & (t_min <= t_max) & (t > min_t);

  Lane_F32 result = select_Lane_F32(valid, t, splat_Lane_F32(F1_MAX));

  return result;
}

Internal F1 ray_shape_intersect(Ray ray, Shape shape)
{
  F1 result = 0.0f;

  switch (shape.kind)
  {
    case SHAPE_KIND__SPHERE: result  =  ray_sphere_intersect(ray, shape.sphere); break;
    case SHAPE_KIND__BOX:    result  =  ray_box_intersect(ray, shape.box);       break;
    case SHAPE_KIND__PLANE:  result  =  ray_plane_intersect(ray, shape.plane);   break;
    default: break;
  }

  return result;
}

////////////////////////////////
//~ Random

Inline I1 rand_pcg(Random_State *rng)
{
  L1 oldstate = rng->state;

  rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);

  I1  xorshifted  =  ((oldstate >> 18u) ^ oldstate) >> 27u;
  I1  rot         =  oldstate >> 59u;
  I1  result      =  (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
  return result;
}

Inline F1 random_unilateral(Random_State *rng)
{
  F1 result = (F1)rand_pcg(rng) / (F1)I1_MAX;
  return result;
}

Inline F1 random_bilateral(Random_State *rng)
{
  F1 result = -1 + 2 * random_unilateral(rng);
  return result;
}

////////////////////////////////
//~ Rectangle

Inline F4 rect_overlap(F4 a, F4 b)
{
  F4 result = {0};

  result[0]  =  Max(a[0], b[0]);
  result[1]  =  Max(a[1], b[1]);
  result[2]  =  Min(a[0] + a[2], b[0] + b[2]) - result[0];
  result[3]  =  Min(a[1] + a[3], b[1] + b[3]) - result[1];

  return result;
}

Inline I1 rect_contains(F4 rect, F2 point)
{
  I1 result = 0;

  if (   point[0] >= rect[0]
      && point[1] >= rect[1]
      && point[0] <= rect[0] + rect[2]
      && point[1] <= rect[1] + rect[3])
  {
    result = 1;
  }

  return result;
}

Inline F4 rect_pad(F4 rect, F1 amt)
{
  F4 result = {
    rect[0] - amt,
    rect[1] - amt,
    rect[2] + amt * 2,
    rect[3] + amt * 2,
  };
  return result;
}

Inline F2 rect_center(F4 rect)
{
  F2 result = {
    rect[0] + rect[2] * 0.5,
    rect[1] + rect[3] * 0.5,
  };
  return result;
}

////////////////////////////////
//~ kti: Colors

F4 oklch_from_linear_rgba(F4 rgba)
{
  F1  l  =  0.4122214708f * rgba[0] + 0.5363325363f * rgba[1] + 0.0514459929f * rgba[2];
  F1  m  =  0.2119034982f * rgba[0] + 0.6806995451f * rgba[1] + 0.1073969566f * rgba[2];
  F1  s  =  0.0883024619f * rgba[0] + 0.2817188376f * rgba[1] + 0.6299787005f * rgba[2];

  F1  l_  =  cbrtf(l);
  F1  m_  =  cbrtf(m);
  F1  s_  =  cbrtf(s);

  F1  ok_l  =  0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_;
  F1  ok_a  =  1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_;
  F1  ok_b  =  0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_;

  F1  chroma  =  sqrt_F1(ok_a * ok_a + ok_b * ok_b);
  F1  hue     =  atan2f(ok_b, ok_a);

  if (hue < 0.0f)
  {
    hue += PI * 2;
  }

  F4 result = {ok_l, chroma, hue, rgba[3]};

  return result;
}

F4 linear_rgba_from_oklch(F1 l, F1 c, F1 h, F1 alpha)
{
  F1  h_rad  =  h;
  F1  a      =  c * cosf(h_rad);
  F1  b      =  c * sinf(h_rad);

  F1  l_  =  l + 0.3963377774f * a + 0.2158037573f * b;
  F1  m_  =  l - 0.1055613458f * a - 0.0638541728f * b;
  F1  s_  =  l - 0.0894841775f * a - 1.2914855480f * b;

  F1  lms_l  =  l_ * l_ * l_;
  F1  lms_m  =  m_ * m_ * m_;
  F1  lms_s  =  s_ * s_ * s_;

  F4 result = {
    +4.0767416621f * lms_l - 3.3077115913f * lms_m + 0.2309699292f * lms_s,
    -1.2684380046f * lms_l + 2.6097574011f * lms_m - 0.3413193965f * lms_s,
    -0.0041960863f * lms_l - 0.7034186147f * lms_m + 1.7076147010f * lms_s,
    alpha,
  };

  return result;
}
