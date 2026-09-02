#ifndef USER_API_H
#define USER_API_H

#include "entity.h"
#include "arena.h"
#include "strings.h"

typedef struct User_API User_API;
struct User_API {
  Entity *(*entity)(String8 name);
};

typedef void (*User_Render_Func)(User_API, Arena *);

#endif
