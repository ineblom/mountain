
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
  L1 clamped_pos = Max(sizeof(Arena), pos);
  arena->pos = clamped_pos;
}

Inline void arena_pop(Arena *arena, L1 amount) {
  L1 old_pos = arena->pos;
  L1 new_pos = old_pos;
  if (amount < old_pos) {
    new_pos = old_pos - amount;
  }
  arena_pop_to(arena, new_pos);
}

Internal void *arena_push(Arena *arena, L1 size, L1 align, I1 zero) {
  L1 aligned_pos = AlignPow2(arena->pos, align);

  Assert(arena->reserved - aligned_pos >= size);

  void *result = (void *)((B1 *)arena + aligned_pos);
  arena->pos = aligned_pos + size;

  if (zero != 0) {
    memset(result, 0, size);
  }

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

#define push_array_no_zero_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align), (0))
#define push_array_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align), (1))
#define push_array_no_zero(a, T, c) push_array_no_zero_aligned(a, T, c, Max(8, AlignOf(T)))
#define push_array(a, T, c) push_array_aligned(a, T, c, Max(8, AlignOf(T)))

#endif
