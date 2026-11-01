Global ThreadLocal Worker_Ctx *worker_ctx = 0;

#define worker_idx() (worker_ctx->worker_idx)
#define worker_count() (worker_ctx->group->worker_count)
#define worker_from_task_idx(idx) ((idx) % worker_ctx->group->worker_count)
#define worker_sync() os_barrier_wait(&worker_ctx->group->barrier)
#define worker_range(count) worker_range_for_section(worker_idx(), worker_count(), count)
#define worker_arena() (worker_ctx->arena)

Inline void worker_broadcast_L1(L1 *ptr, L1 src_worker_idx)
{
  Worker_Group *group = worker_ctx->group;

  if (worker_idx() == src_worker_idx)
  {
    group->broadcast_L1_val = ptr[0];
  }

  worker_sync();

  ptr[0] = group->broadcast_L1_val;

  worker_sync();
}

Internal Range worker_range_for_section(L1 section_idx, L1 section_count, L1 count)
{
  L1  main_quotient  =  count / section_count;
  L1  leftover       =  count - main_quotient * section_count;

  L1  leftovers_consumed_before_section  =  Min(section_idx, leftover);
  L1  min                                =  section_idx * main_quotient + leftovers_consumed_before_section;
  L1  min__clamped                       =  Min(min, count);

  L1  max           =  min__clamped + main_quotient + (section_idx < leftover ? 1 : 0);
  L1  max__clamped  =  Min(max, count);

  return (Range){min__clamped, max__clamped};
}

Internal Arena *worker_get_scratch_arena(Arena **conflicts, L1 count)
{
  Arena *result = 0;

  for (L1 i = 0; i < ArrayCount(worker_ctx->scratch_arenas); i += 1)
  {
    I1 has_conflict = 0;

    for (L1 j = 0; j < count; j += 1)
    {
      if (conflicts[j] == worker_ctx->scratch_arenas[i])
      {
        has_conflict = 1;
        break;
      }
    }

    if (!has_conflict)
    {
      result = worker_ctx->scratch_arenas[i];
      break;
    }
  }

  return result;
}

Inline Temp_Arena scratch_begin(Arena **conflicts, L1 count)
{
  Temp_Arena result = temp_arena_begin(worker_get_scratch_arena(conflicts, count));
  return result;
}

Inline void scratch_end(Temp_Arena temp)
{
  temp_arena_end(temp);
}

Internal void *worker_thread_entrypoint(void *arg)
{
  Worker_Ctx *ctx = (Worker_Ctx *)arg;

  worker_ctx = ctx;

  Worker_Group  *group        =  ctx->group;
  L1            worker_count  =  group->worker_count;

  group->proc(group->user_data);

  worker_ctx = 0;

  L1 completed_before = atomic_add_L1(&group->workers_completed, 1);

  if (completed_before + 1 == worker_count)
  {
    for (L1 i = 0; i < group->worker_count; i += 1)
    {
      Worker_Ctx *ctx = &group->contexts[i];

      for (L1 j = 0; j < ArrayCount(ctx->scratch_arenas); j += 1)
      {
        arena_release(ctx->scratch_arenas[j]);
      }

      arena_release(ctx->arena);
    }

    I1 *completed = group->completed;

    os_barrier_release(&group->barrier);
    arena_release(group->arena);

    if (completed)
    {
      atomic_swap_I1(completed, 1);
    }
  }

  return 0;
}

Internal void worker_group_launch(Worker_Group_Params params)
{
  Arena         *arena            =  arena_alloc(MiB(1));
  Worker_Group  *group            =  push_array(arena, Worker_Group, 1);
  I1            caller_completed  =  0;

  group->arena         =  arena;
  group->worker_count  =  params.count;

  if (params.completed == 0 && params.worker_zero_on_caller)
  {
    params.completed = &caller_completed;
  }

  if (params.completed)
  {
    atomic_swap_I1(params.completed, 0);
  }

  group->completed  =  params.completed;
  group->proc       =  params.proc;
  group->user_data  =  params.user_data;
  group->barrier    =  os_barrier_alloc(params.count);
  group->contexts   =  push_array(arena, Worker_Ctx, params.count);

  for (L1 i = 0; i < params.count; i += 1)
  {
    Worker_Ctx *ctx = &group->contexts[i];

    ctx->group       =  group;
    ctx->worker_idx  =  i;
    ctx->arena       =  arena_alloc(params.arena_size);

    for (L1 j = 0; j < ArrayCount(ctx->scratch_arenas); j += 1)
    {
      ctx->scratch_arenas[j] = arena_alloc(params.scratch_size);
    }
  }

  for (L1 i = params.worker_zero_on_caller; i < params.count; i += 1)
  {
    Worker_Ctx   *ctx    =  &group->contexts[i];
    OS_Thread    thread  =  os_thread_launch(&worker_thread_entrypoint, ctx);

    os_thread_detach(thread);
  }

  if (params.count > 0 && params.worker_zero_on_caller)
  {
    // Worker zero owns the main thread. This is required by AppKit and is also
    // useful for other platform APIs that attach state to the process thread.
    worker_thread_entrypoint(&group->contexts[0]);

    while (!atomic_load_I1(params.completed))
    {
      Pause();
    }
  }
}
