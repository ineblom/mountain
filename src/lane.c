#if (CPU_ && TYP_)
typedef struct LaneCtx LaneCtx;
struct LaneCtx {
  Arena *arena;
  Arena *scratch_arenas[2];
  L1 lane_idx;
  L1 lane_count;
  OS_Barrier *barrier;
};

typedef struct Range Range;
struct Range {
  L1 min, max;
};

Internal void lane(Arena *);

Inline Temp_Arena scratch_begin(Arena **conflicts, L1 count);
Inline void scratch_end(Temp_Arena temp);
#endif

#if (CPU_ && ROM_)

Global L1 sync_L1_val = 0;
Global ThreadLocal LaneCtx *lane_ctx = 0;

#define lane_idx() (lane_ctx->lane_idx)
#define lane_count() (lane_ctx->lane_count)
#define lane_from_task_idx(idx) ((idx) % lane_ctx->lane_count)
#define lane_sync() os_barrier_wait(lane_ctx->barrier)
#define lane_range(count) lane_range_for_section(lane_idx(), lane_count(), count)

Inline void lane_sync_L1(L1 *ptr, L1 src_lane_idx) {
  if (lane_idx() == src_lane_idx) {
    sync_L1_val = ptr[0];
  }
  lane_sync();
  ptr[0] = sync_L1_val;
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

  for EachIndex(i, ArrayCount(lane_ctx->scratch_arenas)) {
    I1 has_conflict = 0;
    for EachIndex(j, count) {
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
  lane_ctx = (LaneCtx *)arg;
  Arena *arena = lane_ctx->arena;
  lane(arena);

  return 0;
}

SI1 main(void) {
  I1 thread_count = os_num_cores();

  OS_Barrier barrier = os_barrier_alloc(thread_count);

  Arena *threads_arena = arena_create(MiB(1));

  LaneCtx *lane_contexts = push_array(threads_arena, LaneCtx, thread_count);

  for EachIndex(i, thread_count) {
    LaneCtx ctx = {
      .arena = arena_create(MiB(512)),
      .lane_idx = i,
      .lane_count = thread_count,
      .barrier = &barrier,
    };
    for EachIndex(j, ArrayCount(ctx.scratch_arenas)) {
      ctx.scratch_arenas[j] = arena_create(MiB(64));
    }
    lane_contexts[i] = ctx;
  }

  OS_Thread *threads = push_array(threads_arena, OS_Thread, thread_count);
  for EachIndex(i, thread_count) {
    LaneCtx *lane_ctx = lane_contexts + i;
    threads[i] = os_thread_launch(&lane_thread_entrypoint, lane_ctx);
  }

  for EachIndex(i, thread_count) {
    os_thread_join(threads[i]);
  }

  for EachIndex(i, thread_count) {
    arena_release(lane_contexts[i].arena);
  }

  os_barrier_release(&barrier);
  arena_release(threads_arena);

  return 0;
}

#endif
