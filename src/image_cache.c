Global IC_State    *ic_state       =  0;
Global IC_Version  ic_nil_version  =  {0};
Global IC_Node     ic_nil_node     =  {
  .version = &ic_nil_version,
};

Internal void ic_init(void)
{
  Arena *arena = arena_alloc(MiB(4));

  ic_state         =  push_array(arena, IC_State, 1);
  ic_state->arena  =  arena;
}

Internal IC_Key ic_key_from_string(String8 string)
{
  Hash128 hash =  hash128(string.str, string.len);
  return hash;
}

Internal I1 ic_key_match(IC_Key a, IC_Key b) {
  I1 result = a.l1[0] == b.l1[0] && a.l1[1] == b.l1[1];
  return result;
}

Internal I1 ic_node_is_nil(IC_Node *node) {
  I1 result = node == 0 || node == &ic_nil_node;
  return result;
}

Internal I1 ic_version_is_nil(IC_Version *version) {
  I1 result = version == 0 || version == &ic_nil_version;
  return result;
}

Internal IC_Node *ic_node(IC_Key key)
{
  IC_Node  *result   =  &ic_nil_node;
  L1       slot_idx  =  key.l1[1] % ArrayCount(ic_state->slots);
  IC_Node  *first    =  ic_state->slots[slot_idx];

  for (IC_Node *n = first; n; n = n->next)
  {
    if (ic_key_match(n->key, key))
    {
      result = n;
      break;
    }
  }

  return result;
}

Internal void ic_version_drop(IC_Version *version)
{
  if (!ic_version_is_nil(version))
  {
    version->ref_count -= 1;

    if (version->ref_count == 0)
    {
      arena_release(version->arena);
      SLLStackPush(ic_state->free_versions, version);
    }
  }
}

Internal Image ic_image(IC_Key key)
{
  IC_Node *n = ic_node(key);
  return n->version->image;
}

Internal void ic_set(IC_Key key, Arena **arena, Image image)
{
  IC_Node *n = ic_node(key);

  //- kti: If not found, grab from free list or alloc.

  if (ic_node_is_nil(n))
  {
    n = ic_state->free_nodes;

    if (n)
    {
      SLLStackPop(ic_state->free_nodes);
    }
    else
    {
      n = push_array(ic_state->arena, IC_Node, 1);
    }

    MemoryZeroStruct(n);

    n->key = key;

    L1 slot_idx = key.l1[1] % ArrayCount(ic_state->slots);

    SLLStackPush(ic_state->slots[slot_idx], n);
  }

  //- kti: Grab version from free list, or alloc.

  IC_Version *version = ic_state->free_versions;

  if (version)
  {
    SLLStackPop(ic_state->free_versions);
  }
  else
  {
    version = push_array(ic_state->arena, IC_Version, 1);
  }

  //- kti: Init version.

  version[0] = (IC_Version){
    .arena      =  arena[0],
    .image      =  image,
    .ref_count  =  1
  };
  arena[0] = 0;
  
  //- kti: Drop old version if any.

  ic_version_drop(n->version);

  //- kti: Set version on node.

  n->version     =  version;
  n->generation  =  ++ic_state->generation;
}

Internal void ic_changed(IC_Key key)
{
  IC_Node *n = ic_node(key);
  if (!ic_node_is_nil(n))
  {
    n->generation = ++ic_state->generation;
  }
}

Internal L1 ic_generation(IC_Key key)
{
  IC_Node *n = ic_node(key);
  return n->generation;
}

Internal IC_Ref ic_acquire(IC_Key key)
{
  IC_Ref   result  =  {0};
  IC_Node  *n      =  ic_node(key);

  if (!ic_node_is_nil(n))
  {
    n->version->ref_count     +=  1;
    ic_state->acquired_count  +=  1;
    result.version            =   n->version;
    result.image              =   n->version->image;
  }

  return result;
}

Internal void ic_drop(IC_Ref *ref)
{
  if (!ic_version_is_nil(ref->version))
  {
    ic_state->acquired_count -= 1;

    ic_version_drop(ref->version);
  }

  MemoryZeroStruct(ref);
}

Internal void ic_release(IC_Key key)
{
  L1       slot_idx  =  key.l1[1] % ArrayCount(ic_state->slots);
  IC_Node  **link    =  &ic_state->slots[slot_idx];

  for (; link[0]; link = &(link[0])->next)
  {
    IC_Node *n = link[0];

    if (ic_key_match(n->key, key))
    {
      link[0] = n->next;

      ic_version_drop(n->version);
      SLLStackPush(ic_state->free_nodes, n);

      break;
    }
  }
}
