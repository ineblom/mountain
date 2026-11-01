#include "math.h"

#if defined(__AVX__)
#include <immintrin.h>
#elif defined(__SSE__)
#include <xmmintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

//- kti: Constructor.
Inline F2 f2(F1 x, F1 y) { return (F2){.x = x, .y = y}; }
Inline F3 f3(F1 x, F1 y, F1 z) { return (F3){.x = x, .y = y, .z = z}; }
Inline F4 f4(F1 x, F1 y, F1 z, F1 w) { return (F4){.x = x, .y = y, .z = z, .w = w}; }
Inline W2 w2(W1 x, W1 y) { return (W2){.x = x, .y = y}; }
Inline W4 w4(W1 x, W1 y, W1 z, W1 w) { return (W4){.x = x, .y = y, .z = z, .w = w}; }
Inline SW2 sw2(SW1 x, SW1 y) { return (SW2){.x = x, .y = y}; }
Inline SW4 sw4(SW1 x, SW1 y, SW1 z, SW1 w) { return (SW4){.x = x, .y = y, .z = z, .w = w}; }

//- kti: Add.
Inline F2 add_F2(F2 a, F2 b) { return f2(a.x + b.x, a.y + b.y); }
Inline F3 add_F3(F3 a, F3 b) { return f3(a.x+b.x, a.y+b.y, a.z+b.z); }
Inline F4 add_F4(F4 a, F4 b) { return f4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w); }
Inline W2 add_W2(W2 a, W2 b) { return w2(a.x + b.x, a.y + b.y); }
Inline W4 add_W4(W4 a, W4 b) { return w4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
Inline SW2 add_SW2(SW2 a, SW2 b) { return sw2(a.x + b.x, a.y + b.y); }
Inline SW4 add_SW4(SW4 a, SW4 b) { return sw4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }

//- kti: Subtract.
Inline F2 sub_F2(F2 a, F2 b) { return f2(a.x - b.x, a.y - b.y); }
Inline F3 sub_F3(F3 a, F3 b) { return f3(a.x-b.x, a.y-b.y, a.z-b.z); }
Inline F4 sub_F4(F4 a, F4 b) { return f4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w); }
Inline W2 sub_W2(W2 a, W2 b) { return w2(a.x - b.x, a.y - b.y); }
Inline W4 sub_W4(W4 a, W4 b) { return w4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
Inline SW2 sub_SW2(SW2 a, SW2 b) { return sw2(a.x - b.x, a.y - b.y); }
Inline SW4 sub_SW4(SW4 a, SW4 b) { return sw4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }

//- kti: Multiply.
Inline F2 mul_F2(F2 a, F2 b) { return f2(a.x * b.x, a.y * b.y); }
Inline F3 mul_F3(F3 a, F3 b) { return f3(a.x*b.x, a.y*b.y, a.z*b.z); }
Inline F4 mul_F4(F4 a, F4 b) { return f4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }
Inline W2 mul_W2(W2 a, W2 b) { return w2(a.x * b.x, a.y * b.y); }
Inline W4 mul_W4(W4 a, W4 b) { return w4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
Inline SW2 mul_SW2(SW2 a, SW2 b) { return sw2(a.x * b.x, a.y * b.y); }
Inline SW4 mul_SW4(SW4 a, SW4 b) { return sw4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }

//- kti: Divide.
Inline F2 div_F2(F2 a, F2 b) { return f2(a.x / b.x, a.y / b.y); }
Inline F3 div_F3(F3 a, F3 b) { return f3(a.x/b.x, a.y/b.y, a.z/b.z); }
Inline F4 div_F4(F4 a, F4 b) { return f4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w); }
Inline W2 div_W2(W2 a, W2 b) { return w2(a.x / b.x, a.y / b.y); }
Inline W4 div_W4(W4 a, W4 b) { return w4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
Inline SW2 div_SW2(SW2 a, SW2 b) { return sw2(a.x / b.x, a.y / b.y); }
Inline SW4 div_SW4(SW4 a, SW4 b) { return sw4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
 
//- kti: Scale.
Inline F2 scale_F2(F2 v, F1 s) { return f2(v.x * s, v.y * s); }
Inline F3 scale_F3(F3 v, F1 s) { return f3(v.x*s, v.y*s, v.z*s); }
Inline F4 scale_F4(F4 v, F1 s) { return f4(v.x*s, v.y*s, v.z*s, v.w*s); }
Inline W2 scale_W2(W2 v, W1 s) { return w2(v.x * s, v.y * s); }
Inline W4 scale_W4(W4 v, W1 s) { return w4(v.x * s, v.y * s, v.z * s, v.w * s); }
Inline SW2 scale_SW2(SW2 v, SW1 s) { return sw2(v.x * s, v.y * s); }
Inline SW4 scale_SW4(SW4 v, SW1 s) { return sw4(v.x * s, v.y * s, v.z * s, v.w * s); }

//- kti: Add scaled.
Inline F2 add_scaled_F2(F2 a, F1 s, F2 b) { return add_F2(a, scale_F2(b, s)); }
Inline F3 add_scaled_F3(F3 a, F1 s, F3 b) { return add_F3(a, scale_F3(b, s)); }
Inline F4 add_scaled_F4(F4 a, F1 s, F4 b) { return add_F4(a, scale_F4(b, s)); }

//- kti: Negate.
Inline F2 neg_F2(F2 v) { return scale_F2(v, -1.0f); }
Inline F3 neg_F3(F3 v) { return scale_F3(v, -1.0f); }
Inline F4 neg_F4(F4 v) { return scale_F4(v, -1.0f); }
Inline SW2 neg_SW2(SW2 v) { return sw2(-v.x, -v.y); }
Inline SW4 neg_SW4(SW4 v) { return sw4(-v.x, -v.y, -v.z, -v.w); }

//- kti: Absolute value.
Inline F3 abs_F3(F3 v) { return f3(abs_F1(v.x), abs_F1(v.y), abs_F1(v.z)); }
Inline F4 abs_F4(F4 v) { return f4(abs_F1(v.x), abs_F1(v.y), abs_F1(v.z), abs_F1(v.w)); }
Inline SW2 abs_SW2(SW2 v) { return sw2(abs(v.x), abs(v.y)); }
Inline SW4 abs_SW4(SW4 v) { return sw4(abs(v.x), abs(v.y), abs(v.z), abs(v.w)); }

//- kti: Splat
Inline F2 splat_F2(F1 x) { return f2(x, x); }
Inline F3 splat_F3(F1 x) { return f3(x, x, x); }
Inline F4 splat_F4(F1 x) { return f4(x, x, x, x); }
Inline W2 splat_W2(W1 x) { return w2(x, x); }
Inline W4 splat_W4(W1 x) { return w4(x, x, x, x); }
Inline SW2 splat_SW2(SW1 x) { return sw2(x, x); }
Inline SW4 splat_SW4(SW1 x) { return sw4(x, x, x, x); }

//- kti: Dot

Inline F1 dot_F2(F2 a, F2 b) {
  return a.x * b.x + a.y * b.y;
}

Inline F1 dot_F3(F3 a, F3 b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

Inline F1 dot_F4(F4 a, F4 b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

//- kti: Square Root
Inline F4 sqrt_F4(F4 v)
{
  return f4(sqrt_F1(v.x), sqrt_F1(v.y), sqrt_F1(v.z), sqrt_F1(v.w));
}

//- kti: Min.
Inline F4 min_F4(F4 a, F4 b) {
  return f4(Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z), Min(a.w, b.w));
}

//- kti: Max.
Inline F4 max_F4(F4 a, F4 b) {
  return f4(Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z), Max(a.w, b.w));
}

//- kti: Clamp
Inline F1 clamp01_F1(F1 x) { return Min(Max(x, 0.0f), 1.0f); }
Inline F4 clamp01_F4(F4 v) { return min_F4(max_F4(v, splat_F4(0)), splat_F4(1)); }

//- kti: Length

Inline F1 length_sq_F2(F2 v) { return dot_F2(v, v); }
Inline F1 length_F2(F2 v) { return sqrt_F1(length_sq_F2(v)); }

Inline F1 length_sq_F3(F3 v) { return dot_F3(v, v); }
Inline F1 length_F3(F3 v) { return sqrt_F1(length_sq_F3(v)); }
Inline F3 normalize_F3(F3 v) { return scale_F3(v, 1.0f/length_F3(v)); }

//- kti: Other

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

Inline F2 lerp_F2(F2 a, F1 t, F2 b)
{
  return add_F2(scale_F2(a, 1.0f-t), scale_F2(b, t));
}

Internal F1 distance_to_segment_F2(F2 p, F2 a, F2 b)
{
  F2  ab        = sub_F2(b, a);
  F2  pa        = sub_F2(p, a);
  F1  ab_len_sq = dot_F2(ab, ab);

  if (ab_len_sq <= 0.00001f)
  {
    return length_F2(pa);
  }

  F1 t = dot_F2(pa, ab) / ab_len_sq;

  t = Clamp(0.0f, t, 1.0f);

  F2 closest = add_F2(a, scale_F2(ab, t));
  return length_F2(sub_F2(p, closest));
}

////////////////////////////////
//~ kti: F3

Inline F3 F3_from_F4(F4 v)
{
  return f3(v.x, v.y, v.z);
}

Inline F3 cross_F3(F3 a, F3 b)
{
  return f3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

Inline F3 reflect_F3(F3 v, F3 normal)
{
  return sub_F3(v, scale_F3(normal, 2.0f*dot_F3(v, normal)));
}

Inline F3 lerp_F3(F3 a, F1 t, F3 b)
{
  return add_F3(scale_F3(a, 1.0f-t), scale_F3(b, t));
}

Inline F3 lerp_snap_F3(F3 a, F1 t, F3 b, F1 min_dist)
{
  F3 result = lerp_F3(a, t, b);
  if (length_sq_F3(sub_F3(a, b)) < Square(min_dist)) result = b;
  return result;
}

Inline F3 safe_inv_dir_F3(F3 v)
{
  return f3(1.0f/(v.x == 0.0f ? 1.0f : v.x),
            1.0f/(v.y == 0.0f ? 1.0f : v.y),
            1.0f/(v.z == 0.0f ? 1.0f : v.z));
}

////////////////////////////////
//~ kti: F4

Inline F4 F4_from_F3(F3 v, F1 w)
{
  F4 result = {{v.x, v.y, v.z, w}};
  return result;
}

Inline F4 lerp_F4(F4 a, F1 t, F4 b)
{
  return add_F4(scale_F4(a, 1.0f-t), scale_F4(b, t));
}

Inline F1 luminance_F4(F4 v)
{
  F1 result = 0.2126f * v.x + 0.7152f * v.y + 0.0722f * v.z;
  return result;
}

Inline F4 pow_F4(F4 v, F1 exp)
{
  F4 result = {{powf(v.x, exp), powf(v.y, exp), powf(v.z, exp), powf(v.w, exp)}};
  return result;
}

////////////////////////////////
//~ Lanes

#define Lane_Repeat4(v) (v), (v), (v), (v)
#if LANE_COUNT == 4
#define Lane_Values(v) Lane_Repeat4(v)
#elif LANE_COUNT == 8
#define Lane_Values(v) Lane_Repeat4(v), Lane_Repeat4(v)
#else
#define Lane_Values(v) Lane_Repeat4(v), Lane_Repeat4(v), Lane_Repeat4(v), Lane_Repeat4(v)
#endif

Inline Lane_F1 splat_Lane_F1(F1 value)
{
  return (Lane_F1){Lane_Values(value)};
}

Inline Lane_SI1 splat_Lane_SI1(SI1 value)
{
  return (Lane_SI1){Lane_Values(value)};
}

#undef Lane_Values
#undef Lane_Repeat4

Inline Lane_SI1 select_Lane_SI1(Lane_Mask mask, Lane_SI1 yes, Lane_SI1 no)
{
  return (mask & yes) | (~mask & no);
}

Inline Lane_F1 select_Lane_F1(Lane_Mask mask, Lane_F1 yes, Lane_F1 no)
{
  union { Lane_F1 f; Lane_SI1 i; } a = {.f = yes}, b = {.f = no}, result;
  result.i = select_Lane_SI1(mask, a.i, b.i);
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

Inline Lane_F1 min_Lane_F1(Lane_F1 a, Lane_F1 b)
{
#if LANE_COUNT == 4 && defined(__SSE__)
  return (Lane_F1)_mm_min_ps((__m128)a, (__m128)b);
#elif LANE_COUNT == 4 && defined(__ARM_NEON)
  return (Lane_F1)vminq_f32((float32x4_t)a, (float32x4_t)b);
#elif LANE_COUNT == 8 && defined(__AVX__)
  return (Lane_F1)_mm256_min_ps((__m256)a, (__m256)b);
#elif LANE_COUNT == 16 && defined(__AVX512F__)
  return (Lane_F1)_mm512_min_ps((__m512)a, (__m512)b);
#else
  return select_Lane_F1(a < b, a, b);
#endif
}

Inline Lane_F1 max_Lane_F1(Lane_F1 a, Lane_F1 b)
{
#if LANE_COUNT == 4 && defined(__SSE__)
  return (Lane_F1)_mm_max_ps((__m128)a, (__m128)b);
#elif LANE_COUNT == 4 && defined(__ARM_NEON)
  return (Lane_F1)vmaxq_f32((float32x4_t)a, (float32x4_t)b);
#elif LANE_COUNT == 8 && defined(__AVX__)
  return (Lane_F1)_mm256_max_ps((__m256)a, (__m256)b);
#elif LANE_COUNT == 16 && defined(__AVX512F__)
  return (Lane_F1)_mm512_max_ps((__m512)a, (__m512)b);
#else
  return select_Lane_F1(a > b, a, b);
#endif
}

Inline Lane_F1 abs_Lane_F1(Lane_F1 v)
{
  union { Lane_F1 f; Lane_SI1 i; } bits = {.f = v};
  bits.i &= splat_Lane_SI1(0x7fffffff);
  return bits.f;
}

Inline Lane_F1 sqrt_Lane_F1(Lane_F1 v)
{
#if LANE_COUNT == 4 && defined(__SSE__)
  return (Lane_F1)_mm_sqrt_ps((__m128)v);
#elif LANE_COUNT == 4 && defined(__ARM_NEON)
  return (Lane_F1)vsqrtq_f32((float32x4_t)v);
#elif LANE_COUNT == 16 && defined(__AVX512F__)
  return (Lane_F1)_mm512_sqrt_ps((__m512)v);
#elif LANE_COUNT == 8 && defined(__AVX__)
  return (Lane_F1)_mm256_sqrt_ps((__m256)v);
#else
  Lane_F1 result = {0};
  for (L1 i = 0; i < LANE_COUNT; i += 1)
  {
    result[i] = sqrt_F1(v[i]);
  }
  return result;
#endif
}

Inline Lane_F3 broadcast_Lane_F3(F3 v)
{
  return (Lane_F3){splat_Lane_F1(v.x), splat_Lane_F1(v.y), splat_Lane_F1(v.z)};
}

Inline F3 extract_lane_F3(Lane_F3 v, L1 lane)
{
  return f3(v.x[lane], v.y[lane], v.z[lane]);
}

Inline void set_lane_F3(Lane_F3 *v, L1 lane, F3 value)
{
  v->x[lane] = value.x;
  v->y[lane] = value.y;
  v->z[lane] = value.z;
}

Inline Lane_F3 pack_Lane_F3(const F3 *values)
{
  Lane_F3 result = {0};
  for (L1 i = 0; i < LANE_COUNT; i += 1) set_lane_F3(&result, i, values[i]);
  return result;
}

Inline Lane_F3 add_Lane_F3(Lane_F3 a, Lane_F3 b) { return (Lane_F3){a.x+b.x, a.y+b.y, a.z+b.z}; }
Inline Lane_F3 sub_Lane_F3(Lane_F3 a, Lane_F3 b) { return (Lane_F3){a.x-b.x, a.y-b.y, a.z-b.z}; }
Inline Lane_F3 scale_Lane_F3(Lane_F3 v, Lane_F1 s) { return (Lane_F3){v.x*s, v.y*s, v.z*s}; }
Inline Lane_F1 dot_Lane_F3(Lane_F3 a, Lane_F3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

Inline Lane_F3 cross_Lane_F3(Lane_F3 a, Lane_F3 b)
{
  return (Lane_F3){a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}

Inline Lane_F3 normalize_Lane_F3(Lane_F3 v)
{
  return scale_Lane_F3(v, 1.0f/sqrt_Lane_F1(dot_Lane_F3(v, v)));
}

////////////////////////////////
//~ kti: Lane_F4

Inline Lane_F4 broadcast_Lane_F4(F4 v)
{
  Lane_F4 result = {splat_Lane_F1(v.x), splat_Lane_F1(v.y), splat_Lane_F1(v.z), splat_Lane_F1(v.w)};
  return result;
}

Inline F4 extract_lane_F4(Lane_F4 v, L1 lane)
{
  return f4(v.x[lane], v.y[lane], v.z[lane], v.w[lane]);
}

Inline void set_lane_F4(Lane_F4 *v, L1 lane, F4 value)
{
  v->x[lane] = value.x;
  v->y[lane] = value.y;
  v->z[lane] = value.z;
  v->w[lane] = value.w;
}

Inline Lane_F4 pack_Lane_F4(const F4 *values)
{
  Lane_F4 result = {0};
  for (L1 i = 0; i < LANE_COUNT; i += 1) set_lane_F4(&result, i, values[i]);
  return result;
}

Inline Lane_F4 add_Lane_F4(Lane_F4 a, Lane_F4 b) { return (Lane_F4){a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w}; }
Inline Lane_F4 sub_Lane_F4(Lane_F4 a, Lane_F4 b) { return (Lane_F4){a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w}; }
Inline Lane_F4 scale_Lane_F4(Lane_F4 v, Lane_F1 s) { return (Lane_F4){v.x*s, v.y*s, v.z*s, v.w*s}; }

Inline Lane_F4 lerp_Lane_F4(Lane_F4 a, Lane_F1 t, Lane_F4 b)
{
  Lane_F4  left_contribution   =  scale_Lane_F4(a, 1.0f - t);
  Lane_F4  right_contribution  =  scale_Lane_F4(b, t);
  Lane_F4  result              =  add_Lane_F4(left_contribution, right_contribution);

  return result;
}

Inline Lane_F1 luminance_Lane_F4(Lane_F4 c)
{
  return 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
}

Inline Lane_F4 pad_Lane_F4(Lane_F4 v, L1 count)
{
  for (L1 i = count; i < LANE_COUNT; ++i)
  {
    v.x[i] = v.x[count - 1];
    v.y[i] = v.y[count - 1];
    v.z[i] = v.z[count - 1];
    v.w[i] = v.w[count - 1];
  }
  return v;
}

Inline Lane_F4 gather_Lane_F4(const Lane_F4 *blocks, const L1 *indices)
{
  Lane_F4 result = {0};

  for (L1 i = 0; i < LANE_COUNT; ++i)
  {
    const Lane_F4  *block  =  &blocks[indices[i] / LANE_COUNT];
    L1             lane    =  indices[i] % LANE_COUNT;

    result.x[i] = block->x[lane];
    result.y[i] = block->y[lane];
    result.z[i] = block->z[lane];
    result.w[i] = block->w[lane];
  }

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

Inline F4x4 translate_F4x4(F3 p)
{
  F4x4 result = identity_F4x4();

  result.m[3][0]  =  p.x;
  result.m[3][1]  =  p.y;
  result.m[3][2]  =  p.z;

  return result;
}

Inline F4x4 scale_F4x4(F3 s)
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
    result.v[col] = v.x * m.m[0][col] + v.y * m.m[1][col] + v.z * m.m[2][col] + v.w * m.m[3][col];
  }

  return result;
}

Inline F3 transform_direction_F4x4(F4x4 m, F3 direction)
{
  return F3_from_F4(mul_F4x4_F4(m, F4_from_F3(direction, 0.0f)));
}

Inline F4 project_point_F4x4(F4x4 m, F3 point)
{
  return mul_F4x4_F4(m, F4_from_F3(point, 1.0f));
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
  F3  plane_normal  =  plane.normal;
  F1  denom         =  dot_F3(plane_normal, ray.dir);

  if (abs_F1(denom) > 0.0001f)
  {
    F1 t = (-plane.d - dot_F3(plane_normal, ray.pos)) / denom;

    if (t > min_t)
    {
      result = t;
    }
  }

  return result;
}

Internal Lane_F1 ray_lane_plane_intersect(Ray ray, Lane_Plane planes)
{
  F1 min_t = 0.001f;

  Lane_F3 ray_pos = broadcast_Lane_F3(ray.pos);
  Lane_F3 ray_dir = broadcast_Lane_F3(ray.dir);

  Lane_F1 denom = dot_Lane_F3(planes.normal, ray_dir);

  Lane_F1 t = (-planes.d - dot_Lane_F3(planes.normal, ray_pos)) / denom;

  Lane_Mask valid  = (abs_Lane_F1(denom) > 0.0001f) & (t > min_t);
  Lane_F1  result = select_Lane_F1(valid, t, splat_Lane_F1(F1_MAX));

  return result;
}

Internal F1 ray_sphere_intersect(Ray ray, Sphere sphere)
{
  F1  min_t                    =  0.001f;
  F1  result                   =  F1_MAX;
  F3  sphere_pos               =  sphere.pos;
  F3  sphere_relative_ray_pos  =  sub_F3(ray.pos, sphere_pos);
  F1  half_b                   =  dot_F3(ray.dir, sphere_relative_ray_pos);
  F1  c                        =  dot_F3(sphere_relative_ray_pos, sphere_relative_ray_pos) - Square(sphere.radius);
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

Internal Lane_F1 ray_lane_sphere_intersect(Ray ray, Lane_Sphere spheres)
{
  F1 min_t = 0.001f;

  Lane_F3 ray_pos = broadcast_Lane_F3(ray.pos);
  Lane_F3 ray_dir = broadcast_Lane_F3(ray.dir);
  Lane_F3 rel     = sub_Lane_F3(ray_pos, spheres.pos);

  Lane_F1 half_b       = dot_Lane_F3(ray_dir, rel);
  Lane_F1 c            = dot_Lane_F3(rel, rel) - Square(spheres.radius);
  Lane_F1 discriminant = Square(half_b) - c;

  Lane_F1 root_term = sqrt_Lane_F1(max_Lane_F1(discriminant, splat_Lane_F1(0.0f)));

  Lane_F1 far_t  = -half_b + root_term;
  Lane_F1 near_t = -half_b - root_term;
  Lane_F1 t      = select_Lane_F1(near_t > min_t, near_t, far_t);

  Lane_Mask valid_discriminant = discriminant > 0;
  Lane_Mask valid_root         = root_term > 0.0001f;
  Lane_Mask is_valid           = valid_discriminant & valid_root & (t > min_t);

  Lane_F1 result = select_Lane_F1(is_valid, t, splat_Lane_F1(F1_MAX));

  return result;
}

Internal F1 ray_box_intersect(Ray ray, Box box)
{
  F1  min_t     =  0.001f;
  F3  aabb_min  =  box.min;
  F3  aabb_max  =  box.max;
  F1  t_min     =  -F1_MAX;
  F1  t_max     =  F1_MAX;

  for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1)
  {
    F1 origin = ray.pos.v[axis];
    F1 direction = ray.dir.v[axis];
    F1 low = aabb_min.v[axis];
    F1 high = aabb_max.v[axis];

    if (direction == 0.0f)
    {
      if (origin < low || origin > high)
        return F1_MAX;

      continue;
    }

    F1 inv_dir = ray.inv_dir.v[axis];
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

Inline Lane_Mask ray_lane_slab_intersect(Lane_F1 low, Lane_F1 high, F1 origin_scalar, F1 dir_scalar, F1 inv_dir, Lane_F1 *near_t, Lane_F1 *far_t)
{
  Lane_F1 origin = splat_Lane_F1(origin_scalar);
  Lane_F1 dir = splat_Lane_F1(dir_scalar);

  Lane_Mask parallel = dir == 0.0f;
  Lane_Mask inside = (low <= origin) & (origin <= high);

  Lane_F1 t0 = (low - origin) * inv_dir;
  Lane_F1 t1 = (high - origin) * inv_dir;

  Lane_F1 min = min_Lane_F1(t0, t1);
  Lane_F1 max = max_Lane_F1(t0, t1);

  near_t[0] = select_Lane_F1(parallel, splat_Lane_F1(-INFINITY), min);
  far_t[0]  = select_Lane_F1(parallel, splat_Lane_F1(INFINITY), max);

  Lane_Mask can_intersect = ~parallel | inside;

  return can_intersect;
}

Internal Lane_F1 ray_lane_box_intersect(Ray ray, Lane_Box boxes)
{
  F1 min_t = 0.001f;

  Lane_F1 near_x, far_x, near_y, far_y, near_z, far_z;

  Lane_Mask valid_x = ray_lane_slab_intersect(boxes.min.x, boxes.max.x, ray.pos.x, ray.dir.x, ray.inv_dir.x, &near_x, &far_x);
  Lane_Mask valid_y = ray_lane_slab_intersect(boxes.min.y, boxes.max.y, ray.pos.y, ray.dir.y, ray.inv_dir.y, &near_y, &far_y);
  Lane_Mask valid_z = ray_lane_slab_intersect(boxes.min.z, boxes.max.z, ray.pos.z, ray.dir.z, ray.inv_dir.z, &near_z, &far_z);

  Lane_F1 t_min = max_Lane_F1(max_Lane_F1(near_x, near_y), near_z);
  Lane_F1 t_max = min_Lane_F1(min_Lane_F1(far_x, far_y), far_z);

  Lane_F1 t = select_Lane_F1(t_min > min_t, t_min, t_max);

  Lane_Mask valid = valid_x & valid_y & valid_z & (t_min <= t_max) & (t > min_t);

  Lane_F1 result = select_Lane_F1(valid, t, splat_Lane_F1(F1_MAX));

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

  result.x  =  Max(a.x, b.x);
  result.y  =  Max(a.y, b.y);
  result.z  =  Min(a.x + a.z, b.x + b.z) - result.x;
  result.w  =  Min(a.y + a.w, b.y + b.w) - result.y;

  return result;
}

Inline I1 rect_contains(F4 rect, F2 point)
{
  I1 result = 0;

  if (   point.x >= rect.x
      && point.y >= rect.y
      && point.x <= rect.x + rect.z
      && point.y <= rect.y + rect.w)
  {
    result = 1;
  }

  return result;
}

Inline F4 rect_pad(F4 rect, F1 amt)
{
  F4 result = {
    {rect.x - amt,
    rect.y - amt,
    rect.z + amt * 2,
    rect.w + amt * 2,
  }};
  return result;
}

Inline F2 rect_center(F4 rect)
{
  F2 result = {
    {rect.x + rect.z * 0.5,
    rect.y + rect.w * 0.5,
  }};
  return result;
}

////////////////////////////////
//~ kti: Colors

F4 oklch_from_linear_rgba(F4 rgba)
{
  F1  l  =  0.4122214708f * rgba.x + 0.5363325363f * rgba.y + 0.0514459929f * rgba.z;
  F1  m  =  0.2119034982f * rgba.x + 0.6806995451f * rgba.y + 0.1073969566f * rgba.z;
  F1  s  =  0.0883024619f * rgba.x + 0.2817188376f * rgba.y + 0.6299787005f * rgba.z;

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

  F4 result = {{ok_l, chroma, hue, rgba.w}};

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
    {+4.0767416621f * lms_l - 3.3077115913f * lms_m + 0.2309699292f * lms_s,
    -1.2684380046f * lms_l + 2.6097574011f * lms_m - 0.3413193965f * lms_s,
    -0.0041960863f * lms_l - 0.7034186147f * lms_m + 1.7076147010f * lms_s,
    alpha,
  }};

  return result;
}
