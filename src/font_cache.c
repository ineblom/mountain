#if (CPU_ && DEF_)

// TODO: Pull out XXH3_128bits to reduce complie time. 
# if !defined(XXH_IMPLEMENTATION)
#  define XXH_IMPLEMENTATION
#  define XXH_STATIC_LINKING_ONLY
#  include "xxhash.h"
# endif

#endif

#if (CPU_ && TYP_)

typedef struct FC_Tag FC_Tag;
struct FC_Tag {
	L1 l1[2];
};

typedef struct FC_Metrics FC_Metrics;
struct FC_Metrics {
  F1 ascent;
  F1 descent;
  F1 line_gap;
  F1 capital_height;
};

typedef struct FC_Font_HT_Node FC_Font_HT_Node;
struct FC_Font_HT_Node {
	FC_Font_HT_Node *hash_next;
	FC_Tag tag;
	FP_Handle handle;
	FP_Metrics metrics;
	String8 path;
};

typedef struct FC_Font_HT_Slot FC_Font_HT_Slot;
struct FC_Font_HT_Slot {
	FC_Font_HT_Node *first;
	FC_Font_HT_Node *last;
};

typedef struct FC_Raster_Info FC_Raster_Info;
struct FC_Raster_Info {
	SW4 subrect;
	SW2 raster_dim;
	W1 atlas_num;
	F1 advance;
};

typedef struct FC_Raster_Info_HT_Node FC_Raster_Info_HT_Node;
struct FC_Raster_Info_HT_Node {
	FC_Raster_Info_HT_Node *hash_next;
	FC_Raster_Info_HT_Node *hash_prev;
	L1 hash;
	FC_Raster_Info info;
};

typedef struct FC_Raster_Info_HT_Slot FC_Raster_Info_HT_Slot;
struct FC_Raster_Info_HT_Slot {
	FC_Raster_Info_HT_Node *first;
	FC_Raster_Info_HT_Node *last;
};

typedef struct FC_Style_Raster_HT_Node FC_Style_Raster_HT_Node;
struct FC_Style_Raster_HT_Node {
	FC_Style_Raster_HT_Node *hash_next;
	FC_Style_Raster_HT_Node *hash_prev;

	L1 style_hash;

	F1 ascent;
	F1 descent;
	F1 column_width;

	FC_Raster_Info *utf8_length1_direct_map;
	L1 raster_info_hash_table_size;
	FC_Raster_Info_HT_Slot *raster_info_hash_table;
};

typedef struct FC_Style_Raster_HT_Slot FC_Style_Raster_HT_Slot;
struct FC_Style_Raster_HT_Slot {
	FC_Style_Raster_HT_Node *first;
	FC_Style_Raster_HT_Node *last;
};

typedef struct FC_State FC_State;
struct FC_State {
	Arena *arena;
	Arena *raster_arena;

	L1 font_hash_table_size;
	FC_Font_HT_Slot *font_hash_table;

	L1 style_raster_hash_table_size;
	FC_Style_Raster_HT_Slot *style_raster_hash_table;
};

#endif

#if (CPU_ && ROM_)

Global FC_State *fc_state = 0;

Internal void fc_reset(void) {
	arena_clear(fc_state->raster_arena);
	fc_state->style_raster_hash_table_size = 1024;
	fc_state->style_raster_hash_table = push_array(fc_state->raster_arena, FC_Style_Raster_HT_Slot, fc_state->style_raster_hash_table_size);
}

Internal void fc_init(void) {
	Arena *arena = arena_create(MiB(64));
	fc_state = push_array(arena, FC_State, 1);
	fc_state->arena = arena;
	fc_state->raster_arena = arena_create(MiB(64));
	fc_state->font_hash_table_size = 64;
	fc_state->font_hash_table = push_array(arena, FC_Font_HT_Slot, fc_state->font_hash_table_size);
	fc_reset();
}

Internal FC_Tag fc_tag_from_path(String8 path) {
	FC_Tag result = {0};

	//- kti: Get a hash from the path.
	XXH128_hash_t xxhash = XXH3_128bits(path.str, path.len);
	memmove(&result, &xxhash, sizeof(result));

	//- kti: Find the slot for this path.
	L1 slot_idx = result.l1[1] % fc_state->font_hash_table_size;
	FC_Font_HT_Slot *slot = &fc_state->font_hash_table[slot_idx];

	//- kti: Check if this tag (font path) is already in the table.
	FC_Font_HT_Node *existing_node = 0;
	for (FC_Font_HT_Node *n = slot->first; n != 0; n = n->hash_next) {
		if (memcmp(&result, &n->tag, sizeof(FC_Tag)) == 0) {
			existing_node = n;
			break;
		}
	}

	//- kti: If we did not find the font, then we need to load it and put it in the hash table.
	if (existing_node == 0) {
		FP_Handle handle = fp_font_open(path);
		existing_node = push_array(fc_state->arena, FC_Font_HT_Node, 1);
		existing_node->tag = result;
		existing_node->handle = handle;
		existing_node->metrics = fp_metrics_from_font(handle);
		existing_node->path = push_str8_copy(fc_state->arena, path);
		SLLQueuePush_N(slot->first, slot->last, existing_node, hash_next);
	}

	//- kti: When the font is invalid. Return a zero tag.
	if (fp_handle_match(existing_node->handle, fp_handle_zero())) {
		memset(&result, 0, sizeof(FC_Tag));
	}

	return result;
}

Internal FP_Metrics fc_fp_metrics_from_tag(FC_Tag tag) {
	L1 slot_idx = tag.l1[1] % fc_state->font_hash_table_size;
	
	FC_Font_HT_Node *existing_node = 0;
	for (FC_Font_HT_Node *n = fc_state->font_hash_table[slot_idx].first; n != 0; n = n->hash_next) {
		if (memcmp(&tag, &n->tag, sizeof(FC_Tag)) == 0) {
			existing_node =n;
			break;
		}
	}
	
	FP_Metrics result = {0};
	if (existing_node != 0) {
		result = fp_metrics_from_font(existing_node->handle);
	}
	return result;
}

Internal FC_Metrics fc_metrics_from_tag_size(FC_Tag tag, F1 size) {
	FP_Metrics metrics = fc_fp_metrics_from_tag(tag);
	FC_Metrics result = {
		.ascent         = floor_F1(size) * metrics.ascent / metrics.design_units_per_em,
		.descent        = floor_F1(size) * metrics.descent / metrics.design_units_per_em,
		.line_gap       = floor_F1(size) * metrics.line_gap / metrics.design_units_per_em,
		.capital_height = floor_F1(size) * metrics.capital_height / metrics.design_units_per_em,
	};
	return result;
}

Internal FC_Style_Raster_HT_Node *fc_style_raster_from_tag_size(FC_Tag tag, F1 size) {
	D1 size_d1 = (D1)size;
	L1 buffer[] = {
		tag.l1[0],
		tag.l1[1],
		*(L1 *)&size_d1,
	};
	L1 style_hash = XXH3_64bits_withSeed(buffer, sizeof(buffer), 69420);

	L1 slot_idx = style_hash % fc_state->style_raster_hash_table_size;
	FC_Style_Raster_HT_Slot *slot = &fc_state->style_raster_hash_table[slot_idx];

	FC_Style_Raster_HT_Node *style_raster_node = 0;

	for (FC_Style_Raster_HT_Node *n = slot->first; n != 0; n = n->hash_next) {
		if (n->style_hash == style_hash) {
			style_raster_node = n;
			break;
		}
	}

	if (style_raster_node == 0) {
		FC_Metrics metrics = fc_metrics_from_tag_size(tag, size);
		style_raster_node = push_array(fc_state->raster_arena, FC_Style_Raster_HT_Node, 1);
		style_raster_node->style_hash = style_hash;
		style_raster_node->ascent = metrics.ascent;
		style_raster_node->descent = metrics.descent;
		style_raster_node->utf8_length1_direct_map = push_array(fc_state->raster_arena, FC_Raster_Info, 256);
		style_raster_node->raster_info_hash_table_size = 1024;
		style_raster_node->raster_info_hash_table = push_array(fc_state->raster_arena, FC_Raster_Info_HT_Slot, style_raster_node->raster_info_hash_table_size);
		DLLPushBack_NP(slot->first, slot->last, style_raster_node, hash_next, hash_prev);
	}

	return style_raster_node;
}

Internal void fc_font_run_from_string(FC_Tag tag, F1 size, F1 base_align_px, F1 tab_size_px, String8 string) {
	FC_Style_Raster_HT_Node *style_raster_node = fc_style_raster_from_tag_size(tag, size);

	// TODO: Setup run cache (if needed)
}

#endif
