#include "base.h"
#include "user_api.h"
#include "math.c"

Image render(User_API api, Arena *frame_arena, L1 frame_index, F1 time) {
  //- kti: Scene
  Entity *e = api.entity(str8("Sphere"));
  e->flags |= ENTITY_FLAG__SHAPE;
  e->material.base_color = (F4){1.0f, 0.0f, 0.0f, 1.0f};

  e = api.entity(str8("Box"));
  e->flags |= ENTITY_FLAG__SHAPE;
  e->shape_kind = SHAPE_KIND__BOX;
  e->material.base_color = (F4){0.0f, 1.0f, 0.5f, 1.0f};

  //- kti: Floor
  Entity *floor = api.entity(str8("Floor"));
  floor->flags |= ENTITY_FLAG__SHAPE;
  floor->shape_kind = SHAPE_KIND__PLANE;
  floor->direction = (F4){0.0f, 1.0f, 0.0f, 0.0f};
  floor->material.metallic = 0.5f;
  floor->material.roughness = 0.4f;
  floor->pos[1] = -0.5f;

  //- kti: Light
  Entity *light = api.entity(str8("Light"));
  light->flags |= ENTITY_FLAG__SHAPE;
  light->shape_kind = SHAPE_KIND__BOX;
  light->pos[1] = 5.0f;
  light->size = (F4){8.0f, 0.1f, 8.0f, 1.0f};
  light->material.emissive = (F4){2.0f, 2.0f, 2.0f, 1.0f};

  //- kti: Camera
  Entity *c = api.entity(str8("Camera"));
  c->flags |= ENTITY_FLAG__CAMERA;
  c->pos[2] = -2.0f;
  c->direction = (F4){0.0f, 0.0f, 1.0f, 0.0f};

  Image image = api.image_alloc(frame_arena, 160, 80, IMAGE_FORMAT__RGBA32F_LINEAR);

  for (L1 y = 0; y < image.height; y += 1) {
    F4 *row = (F4 *)(image.pixels + y*image.row_pitch);
    for (L1 x = 0; x < image.width; x += 1) {
      F1 r = sin_F1(x);
      row[x] = (F4){r, (F1)(x/10)/(F1)image.width, 0.0f, 1.0f};
    }
  }

  return image;
}
