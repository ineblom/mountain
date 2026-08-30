#ifndef ARENA_H
#define ARENA_H
typedef struct Arena Arena;
struct Arena {
  L1 pos;
  L1 reserved;
};

typedef struct Temp_Arena Temp_Arena;
struct Temp_Arena {
  L1 pos;
  Arena *arena;
};

#endif
