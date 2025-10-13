
#if (CPU_ && TYP_)
typedef struct Arena Arena;
struct Arena {
  L1 pos;
  L1 reserved;
};
#endif

#if (CPU_ && ROM_)
Internal L1 arena_create(L1 max_size) {
  Assert(max_size > sizeof(Arena));

  L1 base = os_reserve(max_size);
  Assert(base != 0);
  os_commit(base, max_size);

  TR(Arena) arena = TR_(Arena, base);
  arena->pos = sizeof(Arena);
  arena->reserved = max_size;
  return L1_(arena);
}

Internal void arena_release(L1 arena_ptr) {
  os_release(arena_ptr, TR_(Arena, arena_ptr)->reserved);
}

Inline L1 arena_pos(L1 arena_ptr) {
  L1 result = TR_(Arena, arena_ptr)->pos;
  return result;
}

Inline void arena_pop_to(L1 arena_ptr, L1 pos) {
  TR_(Arena, arena_ptr)->pos = pos;
}

Internal L1 arena_push(L1 arena_ptr, L1 size, L1 align) {
  TR(Arena) arena = TR_(Arena, arena_ptr);

  L1 aligned_pos = AlignPow2(arena->pos, align);

  Assert(arena->reserved - aligned_pos >= size);

  L1 result = L1_(arena) + aligned_pos;

  memset(result, 0, size);

  arena->pos = aligned_pos + size;

  return result;
}

#define push_array(arena, T, count) arena_push(arena, sizeof(T)*count, Max(8, AlignOf(T)))

#endif