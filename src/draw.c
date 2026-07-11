#if (HEADER)

typedef struct DR_FStr_Params DR_FStr_Params;
struct DR_FStr_Params {
  FC_Tag font;
  F4 color;
  F1 size;
  F1 underline_thickness;
  F1 strikethrough_thickness;
};

typedef struct DR_FStr DR_FStr;
struct DR_FStr {
  String8 string;
  DR_FStr_Params params;
};

typedef struct DR_FStr_Node DR_FStr_Node;
struct DR_FStr_Node {
  DR_FStr_Node *next;
  DR_FStr value;
};

typedef struct DR_FStr_List DR_FStr_List;
struct DR_FStr_List {
  DR_FStr_Node *first;
  DR_FStr_Node *last;
  L1 node_count;
  L1 total_len;
};

typedef struct DR_FRun DR_FRun;
struct DR_FRun {
  FC_Run run;
  F4 color;
  F1 underline_thickness;
  F1 strikethrough_thickness;
};

typedef struct DR_FRun_Node DR_FRun_Node;
struct DR_FRun_Node {
  DR_FRun_Node *next;
  DR_FRun value;
};

typedef struct DR_FRun_List DR_FRun_List;
struct DR_FRun_List {
  DR_FRun_Node *first;
  DR_FRun_Node *last;
  L1 node_count;
  F2 dim;
};

typedef struct DR_Clip_Node DR_Clip_Node;
struct DR_Clip_Node {
  DR_Clip_Node *next;
  F4 rect;
};

typedef struct DR_Bucket DR_Bucket;
struct DR_Bucket {
  DR_Bucket *next;
  GFX_Pass_List passes;
  DR_Clip_Node *top_clip;
  L1 stack_gen;
  L1 last_cmd_stack_gen;
};

typedef struct DR_State DR_State;
struct DR_State {
  Arena *arena;
  L1 arena_frame_start_pos;
  DR_Bucket *top_bucket;
};

#endif

#if (SOURCE)

Global DR_Clip_Node dr_nil_clip_node = {0, (F4){0}};
Global DR_State *dr_state = 0;

Internal void dr_push_clip(F4 rect) {
  DR_Bucket *bucket = dr_state->top_bucket;
  if (bucket != 0) {
    DR_Clip_Node *node = push_array(dr_state->arena, DR_Clip_Node, 1);
    node->rect = rect;
    SLLStackPush(bucket->top_clip, node);
    bucket->stack_gen += 1;
  }
}

Internal void dr_pop_clip(void) {
  DR_Bucket *bucket = dr_state->top_bucket;
  if (bucket != 0 && bucket->top_clip != &dr_nil_clip_node) {
    SLLStackPop(bucket->top_clip);
    bucket->stack_gen += 1;
  }
}

Internal F4 dr_top_clip(void) {
  F4 result = (F4){0};
  DR_Bucket *bucket = dr_state->top_bucket;
  if (bucket != 0) {
    result = bucket->top_clip->rect;
  }
  return result;
}

Internal DR_Bucket *dr_bucket_make(void) {
  DR_Bucket *bucket = push_array(dr_state->arena, DR_Bucket, 1);
  bucket->passes = (GFX_Pass_List){0};
  bucket->top_clip = &dr_nil_clip_node;
  return bucket;
}

Internal void dr_push_bucket(DR_Bucket *bucket) {
  SLLStackPush(dr_state->top_bucket, bucket);
}

Internal void dr_pop_bucket(void) {
  SLLStackPop(dr_state->top_bucket);
}

Internal void dr_begin_frame(void) {
  ProfFuncBegin();

  if (dr_state == 0) {
    Arena *arena = arena_alloc(MiB(64));
    dr_state = push_array(arena, DR_State, 1);
    dr_state->arena = arena;
    dr_state->arena_frame_start_pos = arena_pos(arena);
  }
  arena_pop_to(dr_state->arena, dr_state->arena_frame_start_pos);

  ProfEnd();
}

Internal GFX_Pass *dr_pass_from_kind(DR_Bucket *bucket, GFX_Pass_Kind kind) {
  GFX_Pass *pass = bucket->passes.last;
  if (pass == 0 || pass->kind != kind) {
    pass = push_array(dr_state->arena, GFX_Pass, 1);
    pass->kind = kind;
    pass->next = 0;
    MemoryZeroStruct(&pass->rect);
    if (kind == GFX_PASS_KIND__MESH) {
      pass->mesh.view_projection = identity_M4F();
    }
    SLLQueuePush(bucket->passes.first, bucket->passes.last, pass);
  }

  return pass;
}

Internal GFX_Rect_Batch *dr_rect_batch_push(DR_Bucket *bucket, GFX_Rect_Pass *pass, GFX_Texture *texture) {
  GFX_Rect_Batch *batch = push_array(dr_state->arena, GFX_Rect_Batch, 1);
  batch->texture = texture;
  batch->clip_rect = bucket->top_clip->rect;
  batch->instance_cap = 256;
  batch->instances = push_array(dr_state->arena, GFX_Rect_Instance, batch->instance_cap);
  SLLQueuePush(pass->first_batch, pass->last_batch, batch);
  return batch;
}

Internal GFX_Rect_Instance *dr_rect(F4 dst, F4 color, F1 corner_radius, F1 edge_softness) {
  ProfFuncBegin();

  GFX_Rect_Instance *result = 0;

  DR_Bucket *bucket = dr_state->top_bucket;
  if (bucket != 0) {
    GFX_Pass *pass_n = dr_pass_from_kind(bucket, GFX_PASS_KIND__RECT);
    GFX_Rect_Pass *pass = &pass_n->rect;
    GFX_Rect_Batch *batch = pass->last_batch;

    I1 out_of_space = 0;
    I1 texture_requires_new_batch = 0;
    I1 stack_requires_new_batch = 0;
    if (batch) {
      out_of_space = batch->instance_count >= batch->instance_cap;
      texture_requires_new_batch = batch->texture != 0;
      stack_requires_new_batch = bucket->stack_gen != bucket->last_cmd_stack_gen;
    }

    if (batch == 0 || out_of_space || texture_requires_new_batch || stack_requires_new_batch) {
      batch = dr_rect_batch_push(bucket, pass, 0);
    }

    result = &batch->instances[batch->instance_count];
    batch->instance_count += 1;
    bucket->last_cmd_stack_gen = bucket->stack_gen;

    result[0] = (GFX_Rect_Instance){
      .dst_rect = dst,
      .colors = { color, color, color, color },
      .border_color = color,
      .corner_radii = (F4){corner_radius, corner_radius, corner_radius, corner_radius},
      .softness = edge_softness,
      .omit_texture = 1.0f,
    };
  }

  ProfEnd();
  return result;
}

Internal GFX_Rect_Instance *dr_img(F4 dst, F4 src, GFX_Texture *texture, F4 color, F1 corner_radius, F1 edge_softness) {
  ProfFuncBegin();

  GFX_Rect_Instance *result = 0;

  DR_Bucket *bucket = dr_state->top_bucket;
  if (bucket != 0) {
    GFX_Pass *pass_n = dr_pass_from_kind(bucket, GFX_PASS_KIND__RECT);
    GFX_Rect_Pass *pass = &pass_n->rect;
    GFX_Rect_Batch *batch = pass->last_batch;

    I1 out_of_space = 0;
    I1 texture_requires_new_batch = 0;
    I1 stack_requires_new_batch = 0;
    if (batch) {
      out_of_space = batch->instance_count >= batch->instance_cap;
      texture_requires_new_batch = batch->texture != texture;
      stack_requires_new_batch = bucket->stack_gen != bucket->last_cmd_stack_gen;
    }

    if (batch == 0 || out_of_space || texture_requires_new_batch || stack_requires_new_batch) {
      batch = dr_rect_batch_push(bucket, pass, texture);
    }

    batch->texture = texture;

    result = &batch->instances[batch->instance_count];
    batch->instance_count += 1;
    bucket->last_cmd_stack_gen = bucket->stack_gen;

    result[0] = (GFX_Rect_Instance){
      .dst_rect = dst,
      .src_rect = src,
      .colors = { color, color, color, color },
      .corner_radii = (F4){corner_radius, corner_radius, corner_radius, corner_radius},
      .softness = edge_softness,
    };
  }

  ProfEnd();
  return result;
}

Internal void dr_mesh_view_projection(M4F view_projection) {
  DR_Bucket *bucket = dr_state->top_bucket;
  if (bucket != 0) {
    GFX_Pass *pass_n = dr_pass_from_kind(bucket, GFX_PASS_KIND__MESH);
    pass_n->mesh.view_projection = view_projection;
  }
}

Internal GFX_Mesh_Instance *dr_mesh(GFX_Buffer *vertex_buffer, L1 vertex_offset, L1 vertex_count, GFX_Buffer *index_buffer, L1 index_offset, L1 index_count, M4F transform, F4 color) {
  ProfFuncBegin();

  GFX_Mesh_Instance *result = 0;

  DR_Bucket *bucket = dr_state->top_bucket;
  if (bucket != 0) {
    GFX_Pass *pass_n = dr_pass_from_kind(bucket, GFX_PASS_KIND__MESH);
    GFX_Mesh_Pass *pass = &pass_n->mesh;
    GFX_Mesh_Batch *batch = pass->last_batch;

    I1 out_of_space = 0;
    I1 mesh_requires_new_batch = 0;
    if (batch) {
      out_of_space = batch->instance_count >= batch->instance_cap;
      mesh_requires_new_batch = batch->vertex_buffer != vertex_buffer ||
                                batch->vertex_offset != vertex_offset ||
                                batch->vertex_count != vertex_count ||
                                batch->index_buffer != index_buffer ||
                                batch->index_offset != index_offset ||
                                batch->index_count != index_count;
    }

    if (batch == 0 || out_of_space || mesh_requires_new_batch) {
      batch = push_array(dr_state->arena, GFX_Mesh_Batch, 1);
      batch->vertex_buffer = vertex_buffer;
      batch->vertex_offset = vertex_offset;
      batch->vertex_count = vertex_count;
      batch->index_buffer = index_buffer;
      batch->index_offset = index_offset;
      batch->index_count = index_count;
      batch->instance_cap = 256;
      batch->instances = push_array(dr_state->arena, GFX_Mesh_Instance, batch->instance_cap);
      SLLQueuePush(pass->first_batch, pass->last_batch, batch);
    }

    result = &batch->instances[batch->instance_count];
    batch->instance_count += 1;

    result[0] = (GFX_Mesh_Instance){
      .transform = transform,
      .color = color,
    };
  }

  ProfEnd();
  return result;
}

Internal void dr_text_run(FC_Run run, F2 pos, F4 color) {
  for (L1 i = 0; i < run.pieces.count; i += 1) {
    FC_Piece *piece = &run.pieces.v[i];
    SW4 subrect = piece->subrect;
    SW2 offset = piece->offset;

    F1 dst_w = (F1)subrect[2] * piece->scale;
    F1 dst_h = (F1)subrect[3] * piece->scale;
    dr_img(
        (F4){pos[0]+offset[0], pos[1]+offset[1],
        dst_w, dst_h},
        (F4){subrect[0], subrect[1],
        subrect[2], subrect[3]},
        piece->texture, color, 0.0f, 0.0f);

    pos[0] += piece->advance;
  }
}

Internal void dr_submit_bucket(OS_Window *window, GFX_Window *gfx_window, DR_Bucket *bucket) {
  ProfFuncBegin();
  gfx_window_submit(window, gfx_window, bucket->passes);
  ProfEnd();
}

Internal void dr_fstrs_push(Arena *arena, DR_FStr_List *list, DR_FStr *fstr) {
  DR_FStr_Node *node = push_array_no_zero(arena, DR_FStr_Node, 1);
  memmove(&node->value, fstr, sizeof(DR_FStr));
  SLLQueuePush(list->first, list->last, node);
  list->node_count += 1;
  list->total_len += fstr->string.len;
}

Internal DR_FStr_List dr_fstrs_copy(Arena *arena, DR_FStr_List *src) {
  DR_FStr_List dst = {0};
  for (DR_FStr_Node *src_n = src->first; src_n != 0; src_n = src_n->next) {
    DR_FStr fstr = src_n->value;
    fstr.string = push_str8_copy(arena, fstr.string);
    dr_fstrs_push(arena, &dst, &fstr);
  }
  return dst;
}

Internal DR_FRun_List dr_fruns_from_fstrs(Arena *arena, F1 tab_size, DR_FStr_List *src) {
  DR_FRun_List dst = {0};

  F1 base_align_px = 0;
  for (DR_FStr_Node *n = src->first; n != 0; n = n->next) {
    DR_FRun_Node *dst_n = push_array(arena, DR_FRun_Node, 1);
    dst_n->value.run = fc_run_from_string(n->value.params.font, n->value.params.size, base_align_px, tab_size, n->value.string);
    dst_n->value.color = n->value.params.color;
    dst_n->value.underline_thickness = n->value.params.underline_thickness;
    dst_n->value.strikethrough_thickness = n->value.params.strikethrough_thickness;
    SLLQueuePush(dst.first, dst.last, dst_n);
    dst.node_count += 1;
    dst.dim[0] += dst_n->value.run.dim[0];
    dst.dim[1] = Max(dst.dim[1], dst_n->value.run.dim[1]);
    base_align_px += dst_n->value.run.dim[0];
  }

  return dst;
}

#endif
