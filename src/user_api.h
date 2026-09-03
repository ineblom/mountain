#ifndef USER_API_H
#define USER_API_H

#include "entity.h"
#include "arena.h"
#include "strings.h"
#include "image.h"

typedef struct User_API User_API;
struct User_API {
  Entity *(*entity)(String8 name);
  Image (*image_alloc)(Arena *arena, I1 width, I1 height, Image_Format format);
};

typedef Image (*User_Render_Func)(User_API, Arena *, L1, F1);

#endif
