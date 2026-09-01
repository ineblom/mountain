#include "base.h"
#include "user_api.h"

void render(User_API api) {
  //- kti: Scene
  Entity *e = api.entity("Sphere");
  e->flags |= ENTITY_FLAG__SHAPE;
  e->material.base_color = (F4){1.0f, 0.0f, 0.0f, 1.0f};

  //- kti: Floor
  Entity *floor = api.entity("Floor");
  floor->flags |= ENTITY_FLAG__SHAPE;
  floor->shape_kind = SHAPE_KIND__PLANE;
  floor->direction = (F4){0.0f, 1.0f, 0.0f, 0.0f};
  floor->material.metallic = 0.3f;
  floor->material.roughness = 0.6f;
  floor->pos[1] = -0.5f;

  //- kti: Light
  Entity *light = api.entity("Light");
  light->flags |= ENTITY_FLAG__SHAPE;
  light->shape_kind = SHAPE_KIND__BOX;
  light->pos[1] = 3.0f;
  light->size = (F4){5.0f, 0.2f, 5.0f, 1.0f};
  light->material.emissive = (F4){3.0f, 3.0f, 3.0f, 1.0f};

  //- kti: Camera
  Entity *c = api.entity("Camera");
  c->flags |= ENTITY_FLAG__CAMERA;
  c->pos[2] = -2.0f;
  c->direction = (F4){0.0f, 0.0f, 1.0f, 0.0f};
}
