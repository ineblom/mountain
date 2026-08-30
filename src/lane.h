#ifndef LANE_H
#define LANE_H

typedef void Lane_Group_Proc(void *user_data);

typedef struct Lane_Group_Params Lane_Group_Params;
struct Lane_Group_Params {
  L1 count;

  I1 *completed;

  Lane_Group_Proc *proc;
  void *user_data;

  L1 arena_size; 
  L1 scratch_size; 

  I1 lane_zero_on_caller;
};

typedef struct Lane_Ctx Lane_Ctx;
typedef struct Lane_Group Lane_Group;
struct Lane_Group {
  Arena *arena;

  I1 *completed;

  L1 lane_count;
  L1 lanes_completed;

  Lane_Ctx *contexts;
  OS_Barrier barrier;

  L1 sync_L1_val;

  Lane_Group_Proc *proc;
  void *user_data;
};

struct Lane_Ctx {
  Lane_Group *group;

  Arena *arena;
  Arena *scratch_arenas[2];

  L1 lane_idx;
};

typedef struct Range Range;
struct Range {
  L1 min, max;
};

Inline Temp_Arena scratch_begin(Arena **conflicts, L1 count);
Inline void scratch_end(Temp_Arena temp);

#endif
