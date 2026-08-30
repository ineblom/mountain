#ifndef USER_API_H
#define USER_API_H

#include "entity.h"

typedef struct User_API User_API;
struct User_API {
  Entity *(*entity)(const char *name);
};

typedef void (*User_Render_Func)(User_API);

#endif
