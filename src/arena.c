
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
  os_commit(base, max_size);
  Assert(base != 0);

  TR(Arena) arena = TR_(Arena, base);
  arena->pos = sizeof(Arena);
  arena->reserved = max_size;
  return L1_(arena);
}

Internal void arena_release(L1 arena_ptr) {
  os_release(arena_ptr, TR_(Arena, arena_ptr)->reserved);
}

Internal L1 arena_push(L1 arena_ptr, L1 size) {
  TR(Arena) arena = TR_(Arena, arena_ptr);

  Assert(arena->reserved - arena->pos >= size);

  L1 result = L1_(arena) + arena->pos;
  memset(result, 0, size);
  arena->pos += size;
  return result;
}

#endif