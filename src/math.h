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

// NOTE(kti): Used for compact storage.
typedef struct V3 V3;
struct V3
{
  F1 x, y, z;
};

// Four independent three-component values, one value per SIMD lane.
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

typedef struct Box Box;
struct Box
{
  V3 min;
  V3 max;
};

typedef struct Plane Plane;
struct Plane
{
  V3 normal;
  F1 d;
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
  F4 pos;
  F4 dir;
  F4 inv_dir;
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
//~ SIMD helpers

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
