#ifndef WORKER_H
#define WORKER_H

typedef void Worker_Proc(void *user_data);

typedef struct Worker_Group_Params Worker_Group_Params;
struct Worker_Group_Params
{
  L1 count;

  I1 *completed;

  Worker_Proc *proc;
  void        *user_data;

  L1 arena_size;
  L1 scratch_size;

  I1 worker_zero_on_caller;
};

typedef struct Worker_Ctx   Worker_Ctx;
typedef struct Worker_Group Worker_Group;
struct Worker_Group
{
  Arena *arena;

  I1 *completed;

  L1 worker_count;
  L1 workers_completed;

  Worker_Ctx  *contexts;
  OS_Barrier barrier;

  L1 broadcast_L1_val;

  Worker_Proc *proc;
  void        *user_data;
};

struct Worker_Ctx
{
  Worker_Group *group;

  Arena *arena;
  Arena *scratch_arenas[2];

  L1 worker_idx;
};

typedef struct Range Range;
struct Range
{
  L1 min, max;
};

Inline Temp_Arena scratch_begin(Arena **conflicts, L1 count);
Inline void       scratch_end(Temp_Arena temp);

#endif
