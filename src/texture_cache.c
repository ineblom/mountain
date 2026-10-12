Global TC_State  *tc_state     =  0;
Global TC_Node    tc_nil_node  =  {0};

Internal void tc_init(void)
{
  Arena *arena = arena_alloc(MiB(4));

  tc_state         =  push_array(arena, TC_State, 1);
  tc_state->arena  =  arena;
}

Internal TC_Key tc_key_from_string(String8 string)
{
  Hash128 hash = hash128(string.str, string.len);
  return hash;
}

Internal I1 tc_key_match(TC_Key a, TC_Key b) {
  I1 result = a.l1[0] == b.l1[0] && a.l1[1] == b.l1[1];
  return result;
}

Internal I1 tc_node_is_nil(TC_Node *node) {
  I1 result = node == 0 || node == &tc_nil_node;
  return result;
}

Internal TC_Node *tc_node(TC_Key key)
{
  TC_Node  *result   =  &tc_nil_node;
  L1       slot_idx  =  key.l1[1] % ArrayCount(tc_state->slots);
  TC_Node  *first    =  tc_state->slots[slot_idx];

  for (TC_Node *n = first; n; n = n->next)
  {
    if (tc_key_match(n->key, key))
    {
      result = n;
      break;
    }
  }

  return result;
}

Internal GFX_Texture *tc_texture(TC_Key key)
{
  TC_Node *n = tc_node(key);
  return n->texture;
}

Internal void tc_set(TC_Key key, Image_RGBA8 image)
{
  if (image.width == 0 || image.height == 0 || image.pixels == 0)
  {
    tc_release(key);
    return;
  }

  Temp_Arena scratch = scratch_begin(0, 0);

  // The renderer accepts packed rows; callers may provide strided images.
  if (image.row_stride != image.width)
  {
    Image_RGBA8 packed = image_rgba8_alloc(scratch.arena, image.width, image.height);
    for (L1 y = 0; y < image.height; y += 1)
    {
      memcpy(image_rgba8_row(packed, y), image_rgba8_row(image, y), image.width * sizeof(RGBA8));
    }
    image = packed;
  }

  TC_Node *n = tc_node(key);

  //- kti: If not found, grab from free list or alloc.

  if (tc_node_is_nil(n))
  {
    n = tc_state->free_nodes;

    if (n)
    {
      SLLStackPop(tc_state->free_nodes);
    }
    else
    {
      n = push_array(tc_state->arena, TC_Node, 1);
    }

    MemoryZeroStruct(n);

    n->key = key;

    L1 slot_idx = key.l1[1] % ArrayCount(tc_state->slots);

    SLLStackPush(tc_state->slots[slot_idx], n);
  }

  //- kti: Replace texture if dimensions changed.

  GFX_Texture_Filter filter = n->texture ? n->texture->filter : GFX_TEXTURE_FILTER__LINEAR;

  if (n->texture && (n->texture->width != image.width || n->texture->height != image.height))
  {
    gfx_tex2d_free(n->texture);
    n->texture = 0;
  }

  //- kti: Create or fill texture.

  if (n->texture == 0)
  {
    n->texture = gfx_tex2d_from_image(image, GFX_TEXTURE_USAGE__DYNAMIC);
    n->texture->filter = filter;
  }
  else
  {
    gfx_fill_tex2d_region(n->texture, (SI4){0, 0, image.width, image.height}, image.pixels);
  }

  scratch_end(scratch);
}

Internal void tc_release(TC_Key key)
{
  L1       slot_idx  =  key.l1[1] % ArrayCount(tc_state->slots);
  TC_Node  **link    =  &tc_state->slots[slot_idx];

  for (; link[0]; link = &(link[0])->next)
  {
    TC_Node *n = link[0];

    if (tc_key_match(n->key, key))
    {
      link[0] = n->next;

      gfx_tex2d_free(n->texture);
      SLLStackPush(tc_state->free_nodes, n);

      break;
    }
  }
}
