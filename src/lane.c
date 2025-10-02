
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

#if (CPU_ && ROM_)

#define lane_idx() (TR_(LaneCtx, lane_ctx)->lane_idx)
#define lane_count() (TR_(LaneCtx, lane_ctx)->lane_count)
#define lane_from_task_idx(idx) ((idx) % TR_(LaneCtx, lane_ctx)->lane_count)
#define lane_sync() os_barrier_wait(TR_(LaneCtx, lane_ctx)->barrier)
#define lane_range(count) lane_idx_range_from_count(count)

Internal Range lane_idx_range_from_count(L1 count) {
  L1 main_indexes_per_lane = count/lane_count();
  L1 leftover_indexes_count = count - main_indexes_per_lane*lane_count();
  L1 leftover_indexes_before_this_lane_count = Min(lane_idx(), leftover_indexes_count);
  L1 lane_base_idx = lane_idx()*main_indexes_per_lane + leftover_indexes_before_this_lane_count;
  L1 lane_base_idx__clamped = Min(lane_base_idx, count);
  L1 lane_opl_index = lane_base_idx__clamped + main_indexes_per_lane + (lane_idx() < leftover_indexes_count ? 1 : 0);
  L1 lane_opl_index__clamped = Min(lane_opl_index, count);
  return (Range){lane_base_idx__clamped, lane_opl_index__clamped};
}

Internal L1 thread_entrypoint(L1 arg) {
  lane_ctx = arg;
  L1 arena = TR_(LaneCtx, lane_ctx)->arena;
  lane(arena);

  return 0;
}

SI1 main(void) {
  I1 thread_count = os_num_cores();

  Barrier barrier = os_barrier_alloc(thread_count);

  L1 threads_arena = arena_create(MiB(1));

  L1 lane_contexts = arena_push(threads_arena, sizeof(LaneCtx) * thread_count);
  for EachIndex(i, thread_count) {
    TR_(LaneCtx, lane_contexts)[i] = (LaneCtx){
      .arena = arena_create(MiB(32)),
      .lane_idx = i,
      .lane_count = thread_count,
      .barrier = L1_(&barrier),
    };
  }

  L1 threads = arena_push(threads_arena, sizeof(Thread) * thread_count);
  for EachIndex(i, thread_count) {
    L1 lane_ctx = lane_contexts + sizeof(LaneCtx) * i;
    TR_(Thread, threads)[i] = os_thread_launch(&thread_entrypoint, lane_ctx);
  }

  for EachIndex(i, thread_count) {
    os_thread_join(TR_(Thread, threads)[i]);
  }

  for EachIndex(i, thread_count) {
    arena_destroy(TR_(LaneCtx, lane_contexts)[i].arena);
  }

  os_barrier_release(L1_(&barrier));
  arena_destroy(threads_arena);

  return 0;
}

#endif