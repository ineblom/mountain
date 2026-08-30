#ifndef MATH_H
#define MATH_H

#define PI 3.141592653598979f
#define GOLDEN_RATIO 1.61803398875f

typedef struct Random_State Random_State;
struct Random_State {
  L1 state;
  L1 inc;
};

// NOTE(kti): Used for compact storage.
typedef struct V3 V3;
struct V3 {
  F1 x, y, z;
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

typedef enum Shape_Kind {
  SHAPE_KIND__SPHERE,
  SHAPE_KIND__BOX,
  SHAPE_KIND__PLANE,

  SHAPE_KIND_COUNT,
} Shape_Kind;

typedef struct Sphere Sphere;
struct Sphere {
  V3 pos;
  F1 radius;
};

typedef struct Box Box;
struct Box {
  V3 min;
  V3 max;
};

typedef struct Plane Plane;
struct Plane {
  V3 normal;
  F1 d;
};

typedef struct Shape Shape;
struct Shape {
  Shape_Kind kind;
  union {
    Sphere sphere;
    Box box;
    Plane plane;
  };
};

typedef struct Ray Ray;
struct Ray {
  F4 pos;
  F4 dir;
  F4 inv_dir;
};

#endif
