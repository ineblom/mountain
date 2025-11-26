
#if (CPU_ && TYP_)
typedef struct LaneCtx LaneCtx;
struct LaneCtx {
  L1 arena;
  L1 lane_idx;
  L1 lane_count;
  L1 barrier;
};

typedef struct Range Range;
struct Range {
  L1 min, max;
};

Internal void lane(L1);
#endif

#if (CPU_ && RAM_)

L1 sync_L1_val;

#endif

#if (CPU_ && ROM_)

#define lane_idx() (TR_(LaneCtx, lane_ctx)->lane_idx)
#define lane_count() (TR_(LaneCtx, lane_ctx)->lane_count)
#define lane_from_task_idx(idx) ((idx) % TR_(LaneCtx, lane_ctx)->lane_count)
#define lane_sync() os_barrier_wait(TR_(LaneCtx, lane_ctx)->barrier)
#define lane_range(count) range_for_section(lane_idx(), lane_count(), count)

Inline void lane_sync_L1(L1R ptr, L1 src_lane_idx) {
  if (lane_idx() == src_lane_idx) {
    ramR->sync_L1_val = ptr[0];
  }
  lane_sync();
  ptr[0] = ramR->sync_L1_val;
}

Internal Range range_for_section(L1 section_idx, L1 section_count, L1 count) {
  L1 main_quotient = count/section_count;
  L1 leftover = count - main_quotient*section_count;

  L1 leftovers_consumed_before_section = Min(section_idx, leftover);
  L1 min = section_idx*main_quotient + leftovers_consumed_before_section;
  L1 min__clamped = Min(min, count);

  L1 max = min__clamped + main_quotient + (section_idx < leftover ? 1 : 0);
  L1 max__clamped = Min(max, count);

  return (Range){min__clamped, max__clamped};
}

Internal L1 thread_entrypoint(L1 arg) {
  lane_ctx = arg;
  L1 arena = TR_(LaneCtx, lane_ctx)->arena;
  lane(arena);

  return 0;
}

SI1 main(void) {
  I1 thread_count = os_num_cores();

  OS_Barrier barrier = os_barrier_alloc(thread_count);

  L1 threads_arena = arena_create(MiB(1));

  L1 lane_contexts = push_array(threads_arena, LaneCtx, thread_count);
  for EachIndex(i, thread_count) {
    TR_(LaneCtx, lane_contexts)[i] = (LaneCtx){
      .arena = arena_create(MiB(512)),
      .lane_idx = i,
      .lane_count = thread_count,
      .barrier = L1_(&barrier),
    };
  }

  L1 threads = push_array(threads_arena, OS_Thread, thread_count);
  for EachIndex(i, thread_count) {
    L1 lane_ctx = lane_contexts + sizeof(LaneCtx) * i;
    TR_(OS_Thread, threads)[i] = os_thread_launch(&thread_entrypoint, lane_ctx);
  }

  for EachIndex(i, thread_count) {
    os_thread_join(TR_(OS_Thread, threads)[i]);
  }

  for EachIndex(i, thread_count) {
    arena_release(TR_(LaneCtx, lane_contexts)[i].arena);
  }

  os_barrier_release(L1_(&barrier));
  arena_release(threads_arena);

  return 0;
}

#endif