#ifndef MATH_H
#define MATH_H

#define PI 3.141592653598979f
#define GOLDEN_RATIO 1.61803398875f

#include <math.h>

typedef struct Random_State Random_State;
struct Random_State
{
  L1 state;
  L1 inc;
};

// One xyz value. Keep this a plain struct so loops can auto-vectorize.
typedef struct V3 V3;
struct V3
{
  F1 x, y, z;
};

typedef struct Lane_V3 Lane_V3;
struct Lane_V3
{
  Lane_F32 x, y, z;
};

// Fixed four-pixel RGB layout used by the image filtering backend.
typedef struct F4x3 F4x3;
struct F4x3
{
  F4 x, y, z;
};

typedef union F4x4 F4x4;
union F4x4
{
  F4 r[4];
  F1 m[4][4];
  F1 v[16];
};

typedef enum Side
{
  SIDE__INVALID = -1,
  SIDE__MIN,
  SIDE__MAX,
  SIDE_COUNT,
} Side;
#define side_flip(s) ((Side)(!(s)))

typedef I1 Axis;
enum
{
  AXIS__INVALID = -1,
  AXIS__X,
  AXIS__Y,
  AXIS__Z,

  AXIS2_COUNT  =  2,
  AXIS3_COUNT  =  3,
};
#define axis2_flip(a) ((Axis)(!(a)))

typedef I1 Dir;
enum
{
  DIR__RIGHT = 0,
  DIR__UP,
  DIR__LEFT,
  DIR__DOWN,

  DIR_COUNT,
};

typedef enum Shape_Kind
{
  SHAPE_KIND__SPHERE,
  SHAPE_KIND__BOX,
  SHAPE_KIND__PLANE,

  SHAPE_KIND_COUNT,
} Shape_Kind;

typedef struct Sphere Sphere;
struct Sphere
{
  V3 pos;
  F1 radius;
};

typedef struct Lane_Sphere Lane_Sphere;
struct Lane_Sphere
{
  Lane_V3 pos;
  Lane_F32 radius;
};

typedef struct Box Box;
struct Box
{
  V3 min;
  V3 max;
};

typedef struct Lane_Box Lane_Box;
struct Lane_Box
{
  Lane_V3 min;
  Lane_V3 max;
};

typedef struct Plane Plane;
struct Plane
{
  V3 normal;
  F1 d;
};

typedef struct Lane_Plane Lane_Plane;
struct Lane_Plane
{
  Lane_V3 normal;
  Lane_F32 d;
};

typedef struct Shape Shape;
struct Shape
{
  Shape_Kind kind;
  union
  {
    Sphere sphere;
    Box    box;
    Plane  plane;
  };
};

typedef struct Ray Ray;
struct Ray
{
  V3 pos;
  V3 dir;
  V3 inv_dir;
};

#define Square(x) ((x) * (x))

#define sqrt_F1(x) sqrtf(x)
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

////////////////////////////////
//~ Scalar geometry

Inline V3 v3(F1 x, F1 y, F1 z);
// Axis access without treating a struct as an array. Axis must be X, Y, or Z.
Inline F1 component_V3(V3 v, Axis axis);
Inline F1 *component_ptr_V3(V3 *v, Axis axis);
Inline V3 V3_from_F4(F4 v);
Inline F4 F4_from_V3(V3 v);       // xyz with w = 0 (direction).
Inline F4 point_F4_from_V3(V3 v); // xyz with w = 1 (point).
Inline V3 add_V3(V3 a, V3 b);
Inline V3 sub_V3(V3 a, V3 b);
Inline V3 mul_V3(V3 a, V3 b);
Inline V3 div_V3(V3 a, V3 b);
Inline V3 scale_V3(V3 v, F1 scale);
Inline V3 add_scaled_V3(V3 a, F1 scale, V3 b); // a + scale*b
Inline V3 neg_V3(V3 v);
Inline V3 abs_V3(V3 v);
Inline F1 dot_V3(V3 a, V3 b);
Inline F1 length_sq_V3(V3 v);
Inline F1 length_V3(V3 v);
Inline V3 normalize_V3(V3 v);
Inline V3 cross_V3(V3 a, V3 b);
Inline V3 reflect_V3(V3 v, V3 normal);
Inline V3 lerp_V3(V3 a, F1 t, V3 b);
Inline V3 lerp_snap_V3(V3 a, F1 t, V3 b, F1 min_dist);
Inline V3 safe_inv_dir_V3(V3 v);

////////////////////////////////
//~ Geometry lanes

Inline Lane_F32 splat_Lane_F32(F1 value);
Inline Lane_S32 splat_Lane_S32(SI1 value);
Inline Lane_U32 splat_Lane_U32(I1 value);
Inline Lane_F32 select_Lane_F32(Lane_Mask mask, Lane_F32 yes, Lane_F32 no);
Inline Lane_S32 select_Lane_S32(Lane_Mask mask, Lane_S32 yes, Lane_S32 no);
Inline Lane_U32 select_Lane_U32(Lane_Mask mask, Lane_U32 yes, Lane_U32 no);
Inline I1 any_Lane_Mask(Lane_Mask mask);
Inline I1 all_Lane_Mask(Lane_Mask mask);
Inline Lane_F32 min_Lane_F32(Lane_F32 a, Lane_F32 b);
Inline Lane_F32 max_Lane_F32(Lane_F32 a, Lane_F32 b);
Inline Lane_F32 abs_Lane_F32(Lane_F32 v);
Inline Lane_F32 sqrt_Lane_F32(Lane_F32 v);
Inline Lane_V3 broadcast_Lane_V3(V3 v);
// Extraction and insertion require lane < LANE_COUNT; packing reads LANE_COUNT V3s.
Inline V3 extract_lane_V3(Lane_V3 v, L1 lane);
Inline void set_lane_V3(Lane_V3 *v, L1 lane, V3 value);
Inline Lane_V3 pack_Lane_V3(const V3 *values);
Inline Lane_V3 add_Lane_V3(Lane_V3 a, Lane_V3 b);
Inline Lane_V3 sub_Lane_V3(Lane_V3 a, Lane_V3 b);
Inline Lane_V3 scale_Lane_V3(Lane_V3 v, Lane_F32 scale);
Inline Lane_F32 dot_Lane_V3(Lane_V3 a, Lane_V3 b);
Inline Lane_V3 cross_Lane_V3(Lane_V3 a, Lane_V3 b);
Inline Lane_V3 normalize_Lane_V3(Lane_V3 v);

////////////////////////////////
//~ Fixed four-component SIMD helpers

enum
{
  F4_A0, F4_A1, F4_A2, F4_A3,
  F4_B0, F4_B1, F4_B2, F4_B3,
};

#define shuffle_F4(a, b, lane0, lane1, lane2, lane3) __builtin_shufflevector((a), (b), (lane0), (lane1), (lane2), (lane3))

Inline F4 splat_F4(F1 x);
Inline F4 select_F4(SI4 mask, F4 yes, F4 no);
Inline F4 max_F4(F4 a, F4 b);
Inline F4 min_F4(F4 a, F4 b);
Inline F4 clamp01_F4(F4 v);

Inline F4 luminance_F4x3(F4x3 c);
Inline F4x3 scale_F4x3(F4x3 c, F4 scale);
Inline F4x3 add_F4x3(F4x3 a, F4x3 b);
Inline F4x3 lerp_F4x3(F4x3 a, F4 t, F4x3 b);
Inline F4x3 pad_F4x3(F4x3 c, L1 count);
Inline F4x3 F4x3_from_F4(F4 a, F4 b, F4 c, F4 d);
Inline F4x3 gather_F4x3(const F4x3 *row, const L1 indices[4]);

#endif
