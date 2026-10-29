#include "math.h"

#if defined(__SSE__)
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

Inline V3 V3_from_F4(F4 v)
{
  V3 result = {v[0], v[1], v[2]};
  return result;
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

Inline F4 F4_with_w(F4 v, F1 w)
{
  F4 result = v;

  result[3] = w;

  return result;
}

Inline F1 dot_F4(F4 a, F4 b)
{
  F1 result = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
  return result;
}

Inline F1 length_sq_F4(F4 v)
{
  F1 result = dot_F4(v, v);
  return result;
}

Inline F1 length_F4(F4 v)
{
  F1 result = sqrt_F1(length_sq_F4(v));
  return result;
}

Inline F4 normalize_F4(F4 v)
{
  F4 result = v * (1.0f / length_F4(v));
  return result;
}

Inline F4 cross_F4(F4 a, F4 b)
{
  F4 result = {0};

  result[0]  =  a[1] * b[2] - a[2] * b[1];
  result[1]  =  a[2] * b[0] - a[0] * b[2];
  result[2]  =  a[0] * b[1] - a[1] * b[0];

  return result;
}

Inline F4 lerp_F4(F4 a, F1 t, F4 b)
{
  F4 result = a * (1 - t) + b * t;
  return result;
}

Inline F4 lerp_snap_F4(F4 a, F1 t, F4 b, F1 min_dist)
{
  F4 result = lerp_F4(a, t, b);

  if (length_sq_F4(a - b) < Square(min_dist))
  {
    result = b;
  }

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

Inline F4 reflect_F4(F4 v, F4 normal)
{
  F4 result = v - 2 * dot_F4(v, normal) * normal;
  return result;
}

Internal F4 safe_inv_dir_F4(F4 dir)
{
  F4  one      = splat_F4(1.0f);
  SI4 is_zero  = dir == 0.0f;
  F4  safe_dir = select_F4(is_zero, one, dir);
  F4  inv_dir  = 1.0f / safe_dir;
  return inv_dir;
}

////////////////////////////////
//~ F4x3

// TODO: This is very confusing. It takes a V3 (as F4) and replicates it across every lane.
// This should be more clear.
Inline F4x3 splat_F4x3(F4 v)
{
  F4x3 result = {
    .x = splat_F4(v[0]),
    .y = splat_F4(v[1]),
    .z = splat_F4(v[2]),
  };
  return result;
}

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

// Transpose four vectors into component lanes, discarding the fourth component.
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

Inline F4x4 translate_F4x4(F4 p)
{
  F4x4 result = identity_F4x4();

  result.m[3][0]  =  p[0];
  result.m[3][1]  =  p[1];
  result.m[3][2]  =  p[2];

  return result;
}

Inline F4x4 scale_F4x4(F4 s)
{
  F4x4 result = identity_F4x4();

  result.m[0][0]  =  s[0];
  result.m[1][1]  =  s[1];
  result.m[2][2]  =  s[2];

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
  F4  plane_normal  =  F4_from_V3(plane.normal);
  F1  denom         =  dot_F4(plane_normal, ray.dir);

  if (abs_F1(denom) > 0.0001f)
  {
    F1 t = (-plane.d - dot_F4(plane_normal, ray.pos)) / denom;

    if (t > min_t)
    {
      result = t;
    }
  }

  return result;
}

Internal F4 ray_plane4_intersect(Ray ray, Plane4 planes)
{
  F1 min_t = 0.001f;

  F4x3 ray_pos = splat_F4x3(ray.pos);
  F4x3 ray_dir = splat_F4x3(ray.dir);

  F4 denom = dot_F4x3(planes.normal, ray_dir);

  F4 t = (-planes.d - dot_F4x3(planes.normal, ray_pos)) / denom;

  SI4 valid  = (abs_F4(denom) > 0.0001f) & (t > min_t);
  F4  result = select_F4(valid, t, splat_F4(F1_MAX));

  return result;
}

Internal F1 ray_sphere_intersect(Ray ray, Sphere sphere)
{
  F1  min_t                    =  0.001f;
  F1  result                   =  F1_MAX;
  F4  sphere_pos               =  F4_from_V3(sphere.pos);
  F4  sphere_relative_ray_pos  =  ray.pos - sphere_pos;
  F1  half_b                   =  dot_F4(ray.dir, sphere_relative_ray_pos);
  F1  c                        =  dot_F4(sphere_relative_ray_pos, sphere_relative_ray_pos) - Square(sphere.radius);
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

Internal F4 ray_sphere4_intersect(Ray ray, Sphere4 spheres)
{
  F1 min_t = 0.001f;

  F4x3 ray_pos = splat_F4x3(ray.pos);
  F4x3 ray_dir = splat_F4x3(ray.dir);
  F4x3 rel     = sub_F4x3(ray_pos, spheres.pos);

  F4 half_b       = dot_F4x3(ray_dir, rel);
  F4 c            = dot_F4x3(rel, rel) - Square(spheres.radius);
  F4 discriminant = Square(half_b) - c;

  F4 root_term = sqrt_F4(max_F4(discriminant, splat_F4(0.0f)));

  F4 far_t  = -half_b + root_term;
  F4 near_t = -half_b - root_term;
  F4 t      = select_F4(near_t > min_t, near_t, far_t);

  SI4 valid_discriminant = discriminant > 0;
  SI4 valid_root         = root_term > 0.0001f;
  SI4 is_valid           = valid_discriminant & valid_root & (t > min_t);

  F4 result = select_F4(is_valid, t, splat_F4(F1_MAX));

  return result;
}

Internal F1 ray_box_intersect(Ray ray, Box box)
{
  F1  min_t     =  0.001f;
  F4  aabb_min  =  F4_from_V3(box.min);
  F4  aabb_max  =  F4_from_V3(box.max);
  F1  t_min     =  -F1_MAX;
  F1  t_max     =  F1_MAX;

  for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1)
  {
    if (ray.dir[axis] == 0.0f)
    {
      if (ray.pos[axis] < aabb_min[axis] || ray.pos[axis] > aabb_max[axis])
        return F1_MAX;

      continue;
    }

    F1 near_t = (aabb_min[axis] - ray.pos[axis]) * ray.inv_dir[axis];
    F1 far_t  = (aabb_max[axis] - ray.pos[axis]) * ray.inv_dir[axis];

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

Inline SI4 ray_slab4_intersect(F4 low, F4 high, F1 origin_scalar, F1 dir_scalar, F1 inv_dir, F4 *near_t, F4 *far_t)
{
  F4 origin = splat_F4(origin_scalar);
  F4 dir = splat_F4(dir_scalar);

  SI4 parallel = dir == 0.0f;
  SI4 inside = (low <= origin) & (origin <= high);

  F4 t0 = (low - origin) * inv_dir;
  F4 t1 = (high - origin) * inv_dir;

  F4 min = min_F4(t0, t1);
  F4 max = max_F4(t0, t1);

  near_t[0] = select_F4(parallel, splat_F4(-INFINITY), min);
  far_t[0] = select_F4(parallel, splat_F4(INFINITY), max);

  SI4 can_intersect = ~parallel | inside;

  return can_intersect;
}

Internal F4 ray_box4_intersect(Ray ray, Box4 boxes)
{
  F1 min_t = 0.001f;

  F4 near_x, far_x, near_y, far_y, near_z, far_z;

  SI4 valid_x = ray_slab4_intersect(boxes.min.x, boxes.max.x, ray.pos[0], ray.dir[0], ray.inv_dir[0], &near_x, &far_x);
  SI4 valid_y = ray_slab4_intersect(boxes.min.y, boxes.max.y, ray.pos[1], ray.dir[1], ray.inv_dir[1], &near_y, &far_y);
  SI4 valid_z = ray_slab4_intersect(boxes.min.z, boxes.max.z, ray.pos[2], ray.dir[2], ray.inv_dir[2], &near_z, &far_z);

  F4 t_min = max_F4(max_F4(near_x, near_y), near_z);
  F4 t_max = min_F4(min_F4(far_x, far_y), far_z);

  F4 t = select_F4(t_min > min_t, t_min, t_max);

  SI4 valid = valid_x & valid_y & valid_z & (t_min <= t_max) & (t > min_t);

  F4 result = select_F4(valid, t, splat_F4(F1_MAX));

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
