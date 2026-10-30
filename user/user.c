#include "base.h"
#include "user_api.h"
#include "math.c"

Image render(User_API api, Arena *frame_arena, L1 frame_index, F1 time)
{
  //- kti: Scene
  Entity *e = api.entity(str8("Sphere"));
  
  e->flags                |=  ENTITY_FLAG__SHAPE;
  e->material.base_color  =   (F4){1.0f, 0.0f, 0.0f, 1.0f};
  e->material.metallic    =   0.5f;
  e->material.roughness   =   0.2f;

  if (frame_index == 1)
  {
    e->pos.x = -0.45f;
  }

  e                       =   api.entity(str8("Box"));
  e->flags                |=  ENTITY_FLAG__SHAPE;
  e->shape_kind           =   SHAPE_KIND__BOX;
  e->material.base_color  =   (F4){0.0f, 1.0f, 0.5f, 1.0f};
  e->material.metallic    =   0.2f;
  e->material.roughness   =   0.8f;

  if (frame_index == 1)
  {
    e->pos.x = 0.45f;
    e->pos.z = 1.45f;
  }

  //- kti: Floor
  e                      =   api.entity(str8("Floor"));
  e->flags               |=  ENTITY_FLAG__SHAPE;
  e->shape_kind          =   SHAPE_KIND__PLANE;
  e->direction           =   v3(0.0f, 1.0f, 0.0f);
  e->material.metallic   =   0.5f;
  e->material.roughness  =   0.4f;
  e->pos.y              =   -0.5f;

  //- kti: Light
  e                     =   api.entity(str8("Light"));
  e->flags              |=  ENTITY_FLAG__SHAPE;
  e->shape_kind         =   SHAPE_KIND__BOX;
  e->pos.y             =   5.0f;
  e->size               =   v3(8.0f, 0.1f, 8.0f);
  e->material.emissive  =   (F4){2.0f, 2.0f, 2.0f, 1.0f};

  //- kti: Camera
  e             =   api.entity(str8("Camera"));
  e->flags      |=  ENTITY_FLAG__CAMERA;
  e->pos.z     =   -2.0f;
  e->direction  =   v3(0.0f, 0.0f, 1.0f);

  Image image = api.image_alloc(frame_arena, 320, 240);

  for (L1 y = 0; y < image.height; y += 1)
  {
    F4 *row = image.pixels + y*image.row_stride;

    for (L1 x = 0; x < image.width; x += 1)
    {
      F1 a = (F1)x/20.0f+time;
      F1 b = (F1)x/20.0f;
      F1 r = Square(sin_F1(PI*a));
      F1 g = sin_F1(PI*b);

      g  =  g*g*g*g;

      row[x] = (F4){r, g, 0.0f, 1.0f};
    }
  }

  return image;
}
