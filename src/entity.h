#ifndef ENTITY_H
#define ENTITY_H

#include "math.h"
#include "rt.h"

////////////////////////////////
//~ kti: Entity

enum {
  ENTITY_FLAG__SHAPE  = 1 << 0,
  ENTITY_FLAG__CAMERA = 1 << 1,
  ENTITY_FLAG__SELECTED = 1 << 2,
};

typedef struct Entity Entity;
struct Entity {
  Entity *next;
  Entity *prev;
  L1 gen;

  L1 flags;
  B1 name[128];
  L1 name_len;
  F4 pos;
  F4 size;
  F4 direction;
  F1 sphere_diameter;
  Shape_Kind shape_kind;
  RT_Material material;
  F1 camera_vertical_fov;
  F1 camera_aperture_radius;
  F1 camera_focal_distance;
};

typedef struct Entity_Handle Entity_Handle;
struct Entity_Handle {
  Entity *ptr;
  L1 gen;
};

#endif
