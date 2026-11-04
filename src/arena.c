
#define ARENA_COMMIT_SIZE KiB(64)

Internal Arena *arena_alloc(L1 max_size)
{
  L1     reserve_size = AlignPow2(Max(max_size, sizeof(Arena)), ARENA_COMMIT_SIZE);
  void  *base         = os_reserve(reserve_size);
  Arena *arena        = (Arena *)base;
  I1     committed    = 0;

  Assert(base != 0);

  committed = os_commit(base, ARENA_COMMIT_SIZE);
  Assert(committed);

  arena->pos       = sizeof(Arena);
  arena->committed = ARENA_COMMIT_SIZE;
  arena->reserved  = reserve_size;

  return arena;
}

Internal void arena_release(Arena *arena)
{
  if (arena)
  {
    os_memory_release(arena, arena->reserved);
  }
}

Inline L1 arena_pos(Arena *arena)
{
  L1 result = 0;

  if (arena)
  {
    result = arena->pos;
  }

  return result;
}

Inline void arena_pop_to(Arena *arena, L1 pos)
{
  if (arena)
  {
    arena->pos = Clamp(sizeof(Arena), pos, arena->pos);
  }
}

Inline void arena_pop(Arena *arena, L1 amount)
{
  if (amount <= arena->pos)
  {
    L1 new_pos = arena->pos - amount;

    arena_pop_to(arena, new_pos);
  }
}

Inline void arena_clear(Arena *arena)
{
  arena_pop_to(arena, 0);
}

Internal void *arena_push(Arena *arena, L1 size, L1 align, I1 zero)
{
  void *result = 0;

  Assert(align != 0 && (align & (align - 1)) == 0);

  if (arena)
  {
    L1 aligned_pos = AlignPow2(arena->pos, align);
    L1 next_pos    = aligned_pos + size;

    if (aligned_pos <= arena->reserved && size <= arena->reserved - aligned_pos)
    {
      I1 committed = 1;

      if (next_pos > arena->committed)
      {
        L1 new_committed = Min(AlignPow2(next_pos, ARENA_COMMIT_SIZE), arena->reserved);

        committed = os_commit((B1 *)arena + arena->committed, new_committed - arena->committed);

        if (committed)
        {
          arena->committed = new_committed;
        }
      }

      if (committed)
      {
        result     = (void *)((B1 *)arena + aligned_pos);
        arena->pos = next_pos;

        if (zero != 0)
        {
          MemoryZero(result, size);
        }
      }
    }
  }

  return result;
}

Internal Temp_Arena temp_arena_begin(Arena *arena)
{
  Temp_Arena temp  =  {
    .pos    =  arena->pos,
    .arena  =  arena,
  };

  return temp;
}

Internal void temp_arena_end(Temp_Arena temp)
{
  arena_pop_to(temp.arena, temp.pos);
}

#define push_array_no_zero_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T) * (c), (align), (0))
#define push_array_aligned(a, T, c, align) (T *)arena_push((a), sizeof(T) * (c), (align), (1))
#define push_array_no_zero(a, T, c) push_array_no_zero_aligned(a, T, c, Max(8, AlignOf(T)))
#define push_array(a, T, c) push_array_aligned(a, T, c, Max(8, AlignOf(T)))
