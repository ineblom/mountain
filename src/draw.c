#if (CPU_ && TYP_)

typedef struct DR_Bucket DR_Bucket;
struct DR_Bucket {
	DR_Bucket *next;
	GFX_Batch_List batches;
};

typedef struct DR_State DR_State;
struct DR_State {
	Arena *arena;
	L1 arena_frame_start_pos;
	DR_Bucket *top_bucket;
};

#endif

#if (CPU_ && ROM_)

Global DR_State *dr_state;

Internal DR_Bucket *dr_bucket_make(void) {
	DR_Bucket *bucket = push_array(dr_state->arena, DR_Bucket, 1);
	return bucket;
}

Internal void dr_push_bucket(DR_Bucket *bucket) {
	SLLStackPush(dr_state->top_bucket, bucket);
}

Internal void dr_pop_bucket(void) {
	SLLStackPop(dr_state->top_bucket);
}

Internal void dr_begin_frame(void) {
	if (dr_state == 0) {
		Arena *arena = arena_create(MiB(64));
		dr_state = push_array(arena, DR_State, 1);
		dr_state->arena = arena;
		dr_state->arena_frame_start_pos = arena_pos(arena);
	}
	arena_pop_to(dr_state->arena, dr_state->arena_frame_start_pos);
}

Internal GFX_Rect_Instance *dr_rect(F4 dst, F4 color, F1 corner_radius, F1 edge_softness) {
	GFX_Rect_Instance *result = 0;

	DR_Bucket *bucket = dr_state->top_bucket;
	if (bucket != 0) {
		GFX_Batch *batch = bucket->batches.last;
		if (batch == 0 || batch->instance_count >= batch->instance_cap) {
			batch = push_array(dr_state->arena, GFX_Batch, 1);
			batch->instance_cap = 512;
			batch->instances = push_array(dr_state->arena, GFX_Rect_Instance, batch->instance_cap);
			SLLQueuePush(bucket->batches.first, bucket->batches.last, batch);
		}

		result = &batch->instances[batch->instance_count];
		batch->instance_count += 1;

		*result = (GFX_Rect_Instance){
			.dst_rect = dst,
			.colors = { color, color, color, color },
			.corner_radii = (F4){corner_radius, corner_radius, corner_radius, corner_radius},
			// .border_width = border_width,
			.softness = edge_softness,
			.omit_texture = 1.0f,
		};
	}

	return result;
}

Internal void dr_submit_bucket(OS_Window *window, GFX_Window *gfx_window, DR_Bucket *bucket) {
	gfx_window_submit(window, gfx_window, bucket->batches);
}

// Internal void dr_run(FC_Run run, F2 pos) {
// 	for EachIndex(i, run.pieces.count) {
// 		SW4 subrect = run.pieces.v[i].subrect;
// 		SW2 offset = run.pieces.v[i].offset;
// 		instances[instance_count]	= (GFX_Rect_Instance){
// 			.dst_rect = {
// 				pos.x+offset.x, pos.y+offset.y,
// 				subrect.z, subrect.w
// 			},
// 			.src_rect = {
// 				subrect.x, subrect.y,
// 				subrect.z, subrect.w
// 			},
// 			.colors = { white, white, white, white },
// 		};
// 		instance_count += 1;
// 		pos.x += run.pieces.v[i].advance;
// 	}
// }


#endif
