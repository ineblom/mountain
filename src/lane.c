#if (SOURCE)

Global ThreadLocal Lane_Ctx *lane_ctx = 0;

#define lane_idx() (lane_ctx->lane_idx)
#define lane_count() (lane_ctx->group->lane_count)
#define lane_from_task_idx(idx) ((idx) % lane_ctx->group->lane_count)
#define lane_sync() os_barrier_wait(&lane_ctx->group->barrier)
#define lane_range(count) lane_range_for_section(lane_idx(), lane_count(), count)
#define lane_arena() (lane_ctx->arena)

Inline void lane_sync_L1(L1 *ptr, L1 src_lane_idx) {
  Lane_Group *group = lane_ctx->group;

  if (lane_idx() == src_lane_idx) {
    group->sync_L1_val = ptr[0];
  }
  lane_sync();
  ptr[0] = group->sync_L1_val;
  lane_sync();
}

Internal Range lane_range_for_section(L1 section_idx, L1 section_count, L1 count) {
  L1 main_quotient = count/section_count;
  L1 leftover = count - main_quotient*section_count;

  L1 leftovers_consumed_before_section = Min(section_idx, leftover);
  L1 min = section_idx*main_quotient + leftovers_consumed_before_section;
  L1 min__clamped = Min(min, count);

  L1 max = min__clamped + main_quotient + (section_idx < leftover ? 1 : 0);
  L1 max__clamped = Min(max, count);

  return (Range){min__clamped, max__clamped};
}

Internal Arena *lane_get_scratch_arena(Arena **conflicts, L1 count) {
  Arena *result = 0;

  for (L1 i = 0; i < ArrayCount(lane_ctx->scratch_arenas); i += 1) {
    I1 has_conflict = 0;
    for (L1 j = 0; j < count; j += 1) {
      if (conflicts[j] == lane_ctx->scratch_arenas[i]) {
        has_conflict = 1;
        break;
      }
    }

    if (!has_conflict) {
      result = lane_ctx->scratch_arenas[i];
      break;
    }
  }

  return result;
}

Inline Temp_Arena scratch_begin(Arena **conflicts, L1 count) {
  Temp_Arena result = temp_arena_begin(lane_get_scratch_arena(conflicts, count));
  return result;
} 

Inline void scratch_end(Temp_Arena temp) {
  temp_arena_end(temp);
}

Internal void *lane_thread_entrypoint(void *arg) {
  Lane_Ctx *ctx = (Lane_Ctx *)arg;
  lane_ctx = ctx;

  Lane_Group *group = ctx->group;
  L1 lane_count = group->lane_count;
  group->proc(group->user_data);
  lane_ctx = 0;

  L1 completed_before = atomic_add_L1(&group->lanes_completed, 1);

  if (completed_before + 1 == lane_count) {
    for (L1 i = 0; i < group->lane_count; i += 1) {
      Lane_Ctx *ctx = &group->contexts[i];
      for (L1 j = 0; j < ArrayCount(ctx->scratch_arenas); j += 1) {
        arena_release(ctx->scratch_arenas[j]);
      }
      arena_release(ctx->arena);
    }

    os_barrier_release(&group->barrier);

    I1 *completed = group->completed;

    arena_release(group->arena);

    if (completed) {
      atomic_swap_I1(completed, 1);
    }
  }

  return 0;
}

Internal void lane_group_launch(Lane_Group_Params params) {
  Arena *arena = arena_alloc(MiB(1));
  Lane_Group *group = push_array(arena, Lane_Group, 1);
  group->arena = arena;

  group->lane_count = params.count;

  // NOTE(kti): If lane zero is on the caller we need to have a completed var we can block on.
  I1 caller_completed = 0;
  if (params.completed == 0 && params.lane_zero_on_caller) {
    params.completed = &caller_completed;  
  }

  if (params.completed) {
    atomic_swap_I1(params.completed, 0);
  } 
  group->completed = params.completed;

  group->proc = params.proc;
  group->user_data = params.user_data;

  group->barrier = os_barrier_alloc(params.count);

  group->contexts = push_array(arena, Lane_Ctx, params.count);

  for (L1 i = 0; i < params.count; i += 1) {
    Lane_Ctx *ctx = &group->contexts[i];

    ctx->group = group;
    ctx->lane_idx = i;
    ctx->arena = arena_alloc(params.arena_size);

    for (L1 j = 0; j < ArrayCount(ctx->scratch_arenas); j += 1) {
      ctx->scratch_arenas[j] = arena_alloc(params.scratch_size);
    }
  }

  for (L1 i = params.lane_zero_on_caller; i < params.count; i += 1) {
    Lane_Ctx *ctx = &group->contexts[i];
    OS_Thread thread = os_thread_launch(&lane_thread_entrypoint, ctx);
    os_thread_detach(thread);
  }

  if (params.count > 0 && params.lane_zero_on_caller) {
    // Lane zero owns the main thread. This is required by AppKit and is also
    // useful for other platform APIs that attach state to the process thread.
    lane_thread_entrypoint(&group->contexts[0]);

    while (!atomic_load_I1(params.completed)) {
      Pause();
    }
  }
}

#endif
