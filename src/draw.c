#if (CPU_ && TYP_)

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
	GFX_Batch_List batches;
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

#if (CPU_ && ROM_)

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
	if (bucket != 0) {
		SLLStackPop(bucket->top_clip);
		bucket->stack_gen += 1;
	}
}

Internal DR_Bucket *dr_bucket_make(void) {
	DR_Bucket *bucket = push_array(dr_state->arena, DR_Bucket, 1);
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
		Arena *arena = arena_create(MiB(64));
		dr_state = push_array(arena, DR_State, 1);
		dr_state->arena = arena;
		dr_state->arena_frame_start_pos = arena_pos(arena);
	}
	arena_pop_to(dr_state->arena, dr_state->arena_frame_start_pos);

	ProfEnd();
}

Internal GFX_Rect_Instance *dr_rect(F4 dst, F4 color, F1 corner_radius, F1 edge_softness) {
	ProfFuncBegin();

	GFX_Rect_Instance *result = 0;

	DR_Bucket *bucket = dr_state->top_bucket;
	if (bucket != 0) {
		GFX_Batch *batch = bucket->batches.last;

		I1 out_of_space = 0;
		I1 stack_requires_new_batch = 0;
		if (batch) {
			out_of_space = batch->instance_count >= batch->instance_cap;
			stack_requires_new_batch = bucket->stack_gen != bucket->last_cmd_stack_gen;
		}

		if (batch == 0 || out_of_space || stack_requires_new_batch) {
			batch = push_array(dr_state->arena, GFX_Batch, 1);
			batch->instance_cap = 256;
			batch->instances = push_array(dr_state->arena, GFX_Rect_Instance, batch->instance_cap);
			batch->clip_rect = bucket->top_clip->rect;
			SLLQueuePush(bucket->batches.first, bucket->batches.last, batch);
		}

		result = &batch->instances[batch->instance_count];
		batch->instance_count += 1;
		bucket->last_cmd_stack_gen = bucket->stack_gen;

		*result = (GFX_Rect_Instance){
			.dst_rect = dst,
			.colors = { color, color, color, color },
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
		GFX_Batch *batch = bucket->batches.last;

		I1 out_of_space = 0;
		I1 texture_requires_new_batch = 0;
		I1 stack_requires_new_batch = 0;
		if (batch) {
			out_of_space = batch->instance_count >= batch->instance_cap;
			texture_requires_new_batch = batch->texture != 0 && batch->texture != texture;
			stack_requires_new_batch = bucket->stack_gen != bucket->last_cmd_stack_gen;
		}

		if (batch == 0 || out_of_space || texture_requires_new_batch || stack_requires_new_batch) {
			batch = push_array(dr_state->arena, GFX_Batch, 1);
			batch->instance_cap = 256;
			batch->instances = push_array(dr_state->arena, GFX_Rect_Instance, batch->instance_cap);
			batch->clip_rect = bucket->top_clip->rect;
			SLLQueuePush(bucket->batches.first, bucket->batches.last, batch);
		}

		batch->texture = texture;

		result = &batch->instances[batch->instance_count];
		batch->instance_count += 1;
		bucket->last_cmd_stack_gen = bucket->stack_gen;

		*result = (GFX_Rect_Instance){
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

Internal void dr_text_run(FC_Run run, F2 pos, F4 color) {
	for EachIndex(i, run.pieces.count) {
		FC_Piece *piece = &run.pieces.v[i];
		SW4 subrect = piece->subrect;
		SW2 offset = piece->offset;

		dr_img(
				(F4){pos[0]+offset[0], pos[1]+offset[1],
				subrect[2], subrect[3]},
				(F4){subrect[0], subrect[1],
				subrect[2], subrect[3]},
				piece->texture, color, 0.0f, 0.0f);

		pos[0] += piece->advance;
	}
}

Internal void dr_submit_bucket(OS_Window *window, GFX_Window *gfx_window, DR_Bucket *bucket) {
	ProfFuncBegin();
	gfx_window_submit(window, gfx_window, bucket->batches);
	ProfEnd();
}

#endif
