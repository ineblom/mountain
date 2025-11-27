
#if (CPU_ && TYP_)
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

#if (CPU_ && ROM_)
Internal Arena *arena_create(L1 max_size) {
  Assert(max_size > sizeof(Arena));

  void *base = os_reserve(max_size);
  Assert(base != 0);
  os_commit(base, max_size);

  Arena *arena = (Arena *)base;
  arena->pos = sizeof(Arena);
  arena->reserved = max_size;
  return arena;
}

Internal void arena_release(Arena *arena) {
  os_release(arena, arena->reserved);
}

Inline L1 arena_pos(Arena *arena) {
  L1 result = arena->pos;
  return result;
}

Inline void arena_pop_to(Arena *arena, L1 pos) {
  arena->pos = pos;
}

Internal void *arena_push(Arena *arena, L1 size, L1 align) {
  L1 aligned_pos = AlignPow2(arena->pos, align);

  Assert(arena->reserved - aligned_pos >= size);

  void *result = (void *)((B1 *)arena + aligned_pos);

  memset(result, 0, size);

  arena->pos = aligned_pos + size;

  return result;
}

Internal Temp_Arena temp_arena_begin(Arena *arena) {
  L1 pos = arena->pos;
  Temp_Arena temp = {
    .pos = pos,
    .arena = arena,
  };
  return temp;
}

Internal void temp_arena_end(Temp_Arena temp) {
  arena_pop_to(temp.arena, temp.pos);
}

#define push_array(arena, T, count) (T *)arena_push(arena, sizeof(T)*count, Max(8, AlignOf(T)))

#endif