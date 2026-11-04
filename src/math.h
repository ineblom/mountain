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

typedef union F2 F2;
union F2
{
  struct { F1 x, y; };
  struct { F1 r, g; };
  F1 v[2];
};

typedef union F3 F3;
union F3
{
  struct { F1 x, y, z; };
  struct { F1 r, g, b; };
  F1 v[3];
};

typedef union F4 F4;
union F4
{
  struct { F1 x, y, z, w; };
  struct { F1 r, g, b, a; };
  F1 v[4];
};

typedef union SI2 SI2;
union SI2
{
  struct { SI1 x, y; };
  SI1 v[2];
};

typedef union SI4 SI4;
union SI4
{
  struct { SI1 x, y, z, w; };
  SI1 v[4];
};

typedef union I4 I4;
union I4
{
  struct { I1 x, y, z, w; };
  I1 v[4];
};

typedef union W2 W2;
union W2
{
  struct { W1 x, y; };
  W1 v[2];
};

typedef union W4 W4;
union W4
{
  struct { W1 x, y, z, w; };
  W1 v[4];
};

typedef union SW2 SW2;
union SW2
{
  struct { SW1 x, y; };
  SW1 v[2];
};

typedef union SW4 SW4;
union SW4
{
  struct { SW1 x, y, z, w; };
  SW1 v[4];
};

typedef struct Simd_F3 Simd_F3;
struct Simd_F3
{
  Simd_F1 x, y, z;
};

typedef struct Simd_F4 Simd_F4;
struct Simd_F4
{
  Simd_F1 x, y, z, w;
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
  AXIS__X = 0,
  AXIS__Y,
  AXIS__Z,
  AXIS__INVALID = 0xFFFFFFFF,

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
  F3 pos;
  F1 radius;
};

typedef struct Simd_Sphere Simd_Sphere;
struct Simd_Sphere
{
  Simd_F3 pos;
  Simd_F1 radius;
};

typedef struct Box Box;
struct Box
{
  F3 min;
  F3 max;
};

typedef struct Simd_Box Simd_Box;
struct Simd_Box
{
  Simd_F3 min;
  Simd_F3 max;
};

typedef struct Plane Plane;
struct Plane
{
  F3 normal;
  F1 d;
};

typedef struct Simd_Plane Simd_Plane;
struct Simd_Plane
{
  Simd_F3 normal;
  Simd_F1 d;
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
  F3 pos;
  F3 dir;
  F3 inv_dir;
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

#endif
