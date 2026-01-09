#if (CPU_ && DEF_)

# include "meow_hash_x64_aesni.h"

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

typedef struct FC_Raster_Info FC_Raster_Info;
struct FC_Raster_Info {
	SW4 subrect;
	SW2 raster_dim;
	W1 atlas_num;
	F1 advance;
};

typedef struct FC_Piece FC_Piece;
struct FC_Piece {
	GFX_Texture *texture;
	SW4 subrect;
	SW2 offset;
	F1 advance;
	W1 decode_size;
};

typedef struct FC_Piece_Array FC_Piece_Array;
struct FC_Piece_Array {
	FC_Piece *v;
	L1 count;
};

typedef struct FC_Run FC_Run;
struct FC_Run {
	FC_Piece_Array pieces;
	F2 dim;
	F1 ascent;
	F1 descent;
};

//- kti: Piece Chunk List
typedef struct FC_Piece_Chunk_Node FC_Piece_Chunk_Node;
struct FC_Piece_Chunk_Node {
	FC_Piece_Chunk_Node *next;
	FC_Piece *v;
	L1 count;
	L1 cap;
};

typedef struct FC_Piece_Chunk_List FC_Piece_Chunk_List;
struct FC_Piece_Chunk_List {
	FC_Piece_Chunk_Node *first;
	FC_Piece_Chunk_Node *last;
	L1 node_count;
	L1 total_piece_count;
};

//- kti: Font Hash Table

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

//- kti: Run Table

typedef struct FC_Run_Cache_Node FC_Run_Cache_Node;
struct FC_Run_Cache_Node {
	FC_Run_Cache_Node *next;
	String8 string;
	FC_Run run;
};

typedef struct FC_Run_Cache_Slot FC_Run_Cache_Slot;
struct FC_Run_Cache_Slot {
	FC_Run_Cache_Node *first;
	FC_Run_Cache_Node *last;
};

//- kti: Raster Info Hash Table

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

//- kti: Style Raster Hash Table

typedef struct FC_Style_Raster_HT_Node FC_Style_Raster_HT_Node;
struct FC_Style_Raster_HT_Node {
	FC_Style_Raster_HT_Node *hash_next;
	FC_Style_Raster_HT_Node *hash_prev;

	L1 style_hash;

	F1 ascent;
	F1 descent;
	F1 column_width;

	FC_Raster_Info *utf8_length1_direct_map; // len = 256.
	L1 utf8_length1_direct_map_mask[4]; // 64bit * 4 = 256bit mask.
	L1 raster_info_hash_table_size;
	FC_Raster_Info_HT_Slot *raster_info_hash_table;

	L1 run_cache_size;
	FC_Run_Cache_Slot *run_cache;
	L1 run_cache_frame_index;
};

typedef struct FC_Style_Raster_HT_Slot FC_Style_Raster_HT_Slot;
struct FC_Style_Raster_HT_Slot {
	FC_Style_Raster_HT_Node *first;
	FC_Style_Raster_HT_Node *last;
};


//- kti: Atlas

typedef I1 FC_Atlas_Region_Node_Flags;
enum {
	FC_ATLAS_REGION_NODE_FLAG__TAKEN = (1 << 0),
};

typedef struct FC_Atlas_Region_Node FC_Atlas_Region_Node;
struct FC_Atlas_Region_Node {
	FC_Atlas_Region_Node *parent;
	FC_Atlas_Region_Node *children[4];
	SW2 max_free_size[4];
	FC_Atlas_Region_Node_Flags flags;
	L1 num_allocated_descendants;
};

typedef struct FC_Atlas FC_Atlas;
struct FC_Atlas {
	FC_Atlas *next;
	FC_Atlas *prev;
	GFX_Texture *texture;
	SW2 root_dim;
	FC_Atlas_Region_Node *root;
};

//- kti: State

typedef struct FC_State FC_State;
struct FC_State {
	Arena *arena;
	Arena *raster_arena;
	Arena *frame_arena;
	L1 frame_index;

	L1 font_hash_table_size;
	FC_Font_HT_Slot *font_hash_table;

	L1 style_raster_hash_table_size;
	FC_Style_Raster_HT_Slot *style_raster_hash_table;

	FC_Atlas *first_atlas;
	FC_Atlas *last_atlas;
};

#endif

#if (CPU_ && ROM_)

Global FC_State *fc_state = 0;

Internal void fc_reset(void) {
	for (FC_Atlas *a = fc_state->first_atlas; a != 0; a = a->next) {
		gfx_tex2d_free(a->texture);
	}
	fc_state->first_atlas = fc_state->last_atlas = 0;
	arena_clear(fc_state->raster_arena);
	fc_state->style_raster_hash_table_size = 1024;
	fc_state->style_raster_hash_table = push_array(fc_state->raster_arena, FC_Style_Raster_HT_Slot, fc_state->style_raster_hash_table_size);
}

Internal void fc_init(void) {
	Arena *arena = arena_create(MiB(64));
	fc_state = push_array(arena, FC_State, 1);
	fc_state->arena = arena;
	fc_state->raster_arena = arena_create(MiB(64));
	fc_state->frame_arena = arena_create(MiB(64));
	fc_state->font_hash_table_size = 64;
	fc_state->font_hash_table = push_array(arena, FC_Font_HT_Slot, fc_state->font_hash_table_size);
	fc_reset();
	fc_state->frame_index = 1;
}

Internal void fc_frame(void) {
	fc_state->frame_index += 1;
	arena_clear(fc_state->frame_arena);
}

Internal FC_Piece *fc_piece_chunk_list_push_new(Arena *arena, FC_Piece_Chunk_List *list, L1 cap) {
	FC_Piece_Chunk_Node *node = list->last;
	if (node == 0 || node->count >= node->cap) {
		node = push_array(arena, FC_Piece_Chunk_Node, 1);
		node->v = push_array_no_zero(arena, FC_Piece, cap);
		node->cap = cap;
		SLLQueuePush(list->first, list->last, node);
		list->node_count += 1;
	}
	FC_Piece *result = node->v + node->count;
	node->count += 1;
	list->total_piece_count += 1;

	return result;
}

Internal FC_Piece_Array fc_piece_array_from_chunk_list(Arena *arena, FC_Piece_Chunk_List *list) {
	FC_Piece_Array array = {0};
	array.count = list->total_piece_count;
	array.v = push_array_no_zero(arena, FC_Piece, array.count);
	L1 write_idx = 0;
	for (FC_Piece_Chunk_Node *n = list->first; n != 0; n = n->next) {
		memmove(array.v + write_idx, n->v, sizeof(FC_Piece) * n->count);
		write_idx += n->count;
	}
	return array;
}

Internal FC_Tag fc_tag_from_path(String8 path) {
	ProfFuncBegin();

	FC_Tag result = {0};

	//- kti: Get a hash from the path.
	meow_u128 hash = MeowHash(MeowDefaultSeed, path.len, path.str);
	result.l1[0] = MeowU64From(hash, 0);
	result.l1[1] = MeowU64From(hash, 1);

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
		MemoryZeroStruct(&result);
	}

	ProfEnd();
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
	ProfFuncBegin();

	D1 size_d1 = (D1)size;
	L1 buffer[] = {
		tag.l1[0],
		tag.l1[1],
		*(L1 *)&size_d1,
	};
	meow_u128 hash = MeowHash(MeowDefaultSeed, sizeof(buffer), buffer);
	L1 style_hash = MeowU64From(hash, 0);

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

	ProfEnd();
	return style_raster_node;
}

Internal SI2 fc_vertex_from_corner(I1 corner) {
	LocalPersist SI2 vertices[4] = {
		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1},
	};
	SI2 result = {0};
	if (corner < 4) {
		result = vertices[corner];
	}
	return result;
}

Internal SW4 fc_atlas_region_alloc(Arena *arena, FC_Atlas *atlas, SW2 needed_size) {
	ProfFuncBegin();

	SW2 region_p = {0};
	SW2 region_s = {0};
	I1 node_corner = I1_MAX;
	FC_Atlas_Region_Node *node = 0;

	//- kti: Walk down quad tree until children can't fit the needed size.
	SW2 n_size = atlas->root_dim;
	for (FC_Atlas_Region_Node *n = atlas->root, *next = 0; n != 0; n = next, next = 0) {
		if (n->flags & FC_ATLAS_REGION_NODE_FLAG__TAKEN) {
			break;
		}

		I1 n_can_be_allocated = (n->num_allocated_descendants == 0);
		if (n_can_be_allocated) {
			region_s = n_size;
		}

		SW2 child_size = n_size/2;

		FC_Atlas_Region_Node *best_child = 0;
		if (child_size[0] >= needed_size[0] && child_size[1] >= needed_size[1]) {
			for (I1 corner = 0; corner < 4; corner += 1) {
				if (n->children[corner] == 0) {
					n->children[corner] = push_array(arena, FC_Atlas_Region_Node, 1);
					n->children[corner]->parent = n;
					n->children[corner]->max_free_size[0] =
						n->children[corner]->max_free_size[1] =
						n->children[corner]->max_free_size[2] =
						n->children[corner]->max_free_size[3] = child_size/2;
				}
				if (n->max_free_size[corner][0] >= needed_size[0] &&
						n->max_free_size[corner][1] >= needed_size[1]) {
					best_child = n->children[corner];
					node_corner = corner;
					SI2 side_vertex = fc_vertex_from_corner(corner);
					region_p[0] += side_vertex[0]*child_size[0];
					region_p[1] += side_vertex[1]*child_size[1];
					break;
				}
			}
		}

		if (n_can_be_allocated && best_child == 0) {
			node = n;
		} else {
			next = best_child;
			n_size = child_size;
		}
	}

	if (node != 0 && node_corner != I1_MAX) {
		//- kti: Mark this region as taken.
		node->flags |= FC_ATLAS_REGION_NODE_FLAG__TAKEN;
		if (node->parent != 0) {
			MemoryZeroStruct(&node->parent->max_free_size[node_corner]);
		}

		//- kti: Walk up tree and re-calculate max free size.
		for (FC_Atlas_Region_Node *p = node->parent; p != 0; p = p->parent) {
			p->num_allocated_descendants += 1;
			FC_Atlas_Region_Node *parent = p->parent;
			if (parent != 0) {
				I1 p_corner =
					(p == parent->children[0] ? 0 :
					p == parent->children[1] ? 1 :
					p == parent->children[2] ? 2 :
					p == parent->children[3] ? 3 :
					I1_MAX);
				Assert(p_corner != I1_MAX);
				parent->max_free_size[p_corner][0] = Max(
						Max(p->max_free_size[0][0], p->max_free_size[1][0]),
						Max(p->max_free_size[2][0], p->max_free_size[3][0]));
				parent->max_free_size[p_corner][1] = Max(
						Max(p->max_free_size[0][1], p->max_free_size[1][1]),
						Max(p->max_free_size[2][1], p->max_free_size[3][1]));

			}
		}
	}

	ProfEnd();

	SW4 result = {region_p[0], region_p[1], region_s[0], region_s[1]};
	return result;
}

Internal FC_Run fc_run_from_string(FC_Tag tag, F1 size, F1 base_align_px, F1 tab_size_px, String8 string) {
	ProfFuncBegin();

	FC_Style_Raster_HT_Node *style_raster_node = fc_style_raster_from_tag_size(tag, size);

	//- kti: Create run cache if on new frame.
	if (style_raster_node->run_cache_frame_index != fc_state->frame_index) {
		style_raster_node->run_cache_size = 1024;
		style_raster_node->run_cache = push_array(fc_state->frame_arena, FC_Run_Cache_Slot, style_raster_node->run_cache_size);
		style_raster_node->run_cache_frame_index = fc_state->frame_index;
	}

	//- kti: Unpack run params.
	meow_u128 hash = MeowHash(MeowDefaultSeed, string.len, string.str);
	L1 run_hash = MeowU64From(hash, 0);
	L1 run_cache_slot_idx = run_hash % style_raster_node->run_cache_size;
	FC_Run_Cache_Slot *run_slot = &style_raster_node->run_cache[run_cache_slot_idx];

	//- kti: Check cache for existing run node for this string.
	FC_Run_Cache_Node *run_node = 0;
	for (FC_Run_Cache_Node *n = run_slot->first; n != 0; n = n->next) {
		if (str8_match(n->string, string)) {
			run_node = n;
			break;
		}
	}

	I1 run_is_cacheable = 1;
	FC_Run run = {0};
	if (run_node != 0) {
		//- kti: Run is in cache, we can return that. And don't cache it again ofc.
		run = run_node->run;
		run_is_cacheable = 0;
	} else {
		//- kti: Cache miss.
		
		F2 dim = {0};
		FC_Piece_Chunk_List piece_chunks = {0};
		I1 font_handle_is_set = 0;
		FP_Handle font_handle = {0};
		L1 piece_substring_begin_idx = 0;
		L1 piece_substring_end_idx = 0;
		for (L1 i = 0; i <= string.len;) {
			//- kti: Get the piece substring indices.
			B1 byte = (i < string.len ? string.str[i] : 0);
			if (byte == 0) {
				i += 1;
			} else  {
				Unicode_Decode decode = utf8_decode(string.str+i, string.len-i);
				i += decode.inc;
				piece_substring_end_idx += decode.inc;
			}

			//- kti: Do we need to parse another byte?
			if (piece_substring_begin_idx == piece_substring_end_idx) {
				continue;
			}

			//- kti: Grab substring.
			String8 piece_substring = str8_substr(string, piece_substring_begin_idx, piece_substring_end_idx);
			piece_substring_begin_idx = i;
			piece_substring_end_idx = i;

			//- kti: Use space glyph for tabs. Don't cache. Tab width may vary between calls. 
			I1 is_tab = (piece_substring.len == 1 && piece_substring.str[0] == '\t');
			if (is_tab) {
				run_is_cacheable = 0;
				piece_substring = Str8_(" ");
			}

			//- kti: Get raster info..
			FC_Raster_Info *info = 0;
			L1 piece_hash = 0;

			// kti: fast path for cached utf8 length 1 glyphs.
			if (piece_substring.len == 1 &&
					style_raster_node->utf8_length1_direct_map_mask[piece_substring.str[0]/64] & (1ull<<piece_substring.str[0]%64)) {
				info = &style_raster_node->utf8_length1_direct_map[piece_substring.str[0]];
			}

			// kii: slower path for other glyphs.
			if (piece_substring.len > 1) {
				meow_u128 hash = MeowHash(MeowDefaultSeed, piece_substring.len, piece_substring.str);
				piece_hash = MeowU64From(hash, 0);
				L1 slot_idx = piece_hash%style_raster_node->raster_info_hash_table_size;
				FC_Raster_Info_HT_Slot *slot = &style_raster_node->raster_info_hash_table[slot_idx];
				for (FC_Raster_Info_HT_Node *n = slot->first; n != 0; n = n->hash_next) {
					if (n->hash == piece_hash) {
						info = &n->info;
						break;
					}
				}
			}

			//- kti: No info found? -> cache miss. Fill in info in cache.
			if (info == 0) {
				Temp_Arena scratch = scratch_begin(0, 0);

				// kti: Grab font handle.
				if (font_handle_is_set == 0) {
					font_handle_is_set = 1;

					L1 font_slot_idx = tag.l1[1] % fc_state->font_hash_table_size;
					FC_Font_HT_Slot *slot = &fc_state->font_hash_table[font_slot_idx];

					FC_Font_HT_Node *existing_node = 0;
					for (FC_Font_HT_Node *n = slot->first; n != 0; n = n->hash_next) {
						if (memcmp(&n->tag, &tag, sizeof(FC_Tag)) == 0) {
							existing_node = n;
							break;
						}
					}

					if (existing_node != 0) {
						font_handle = existing_node->handle;
					}
				}

				// kti: Rasterize
				FP_Raster_Result raster = {0};
				if (size > 0) {
					raster = fp_raster(scratch.arena, font_handle, size, piece_substring);
				}

				// kti: Allocate portion of atlas.
				SW1 chosen_atlas_num = 0;
				FC_Atlas *chosen_atlas = 0;
				SW4 chosen_atlas_region = {0};
				if (raster.atlas_dim[0] != 0 && raster.atlas_dim[1] != 0) {
					L1 num_atlases = 0;
					for (FC_Atlas *atlas = fc_state->first_atlas;; atlas = atlas->next, num_atlases += 1) {
						// kit: Allocate atlas if needed.
						if (atlas == 0 && num_atlases < 64) {
							atlas = push_array(fc_state->raster_arena, FC_Atlas, 1);
							DLLPushBack(fc_state->first_atlas, fc_state->last_atlas, atlas);
							atlas->root_dim = (SW2){1024, 1024};
							atlas->root = push_array(fc_state->raster_arena, FC_Atlas_Region_Node, 1);
							atlas->root->max_free_size[0] =
								atlas->root->max_free_size[1] =
								atlas->root->max_free_size[2] =
								atlas->root->max_free_size[3] = (SW2){atlas->root_dim[0]/2, atlas->root_dim[1]/2};
							atlas->texture = gfx_tex2d_alloc(GFX_TEXTURE_USAGE__DYNAMIC, atlas->root_dim[0], atlas->root_dim[1], 0);
						}

						// kti: Allocate from atlas.
						if (atlas != 0) {
							SW2 needed_dimensions = {raster.atlas_dim[0] + 2, raster.atlas_dim[1] + 2};
							chosen_atlas_region = fc_atlas_region_alloc(fc_state->raster_arena, atlas, needed_dimensions);
							if (chosen_atlas_region[2] != 0 && chosen_atlas_region[3] != 0) {
								chosen_atlas = atlas;
								chosen_atlas_num = (SW1)num_atlases;
								break;
							}
						} else {
							break;
						}
					}
				}

				if (chosen_atlas != 0) {
					SI4 subregion = {
						chosen_atlas_region[0],
						chosen_atlas_region[1],
						raster.atlas_dim[0],
						raster.atlas_dim[1],
					};
					gfx_fill_tex2d_region(chosen_atlas->texture, subregion, raster.atlas);
				}

				if (piece_substring.len == 1) {
					info = &style_raster_node->utf8_length1_direct_map[piece_substring.str[0]];
					style_raster_node->utf8_length1_direct_map_mask[piece_substring.str[0]/64] |= (1ull << (piece_substring.str[0]%64));
				} else {
					L1 slot_idx = piece_hash%style_raster_node->raster_info_hash_table_size;
					FC_Raster_Info_HT_Slot *slot = &style_raster_node->raster_info_hash_table[slot_idx];
					FC_Raster_Info_HT_Node *node = push_array_no_zero(fc_state->raster_arena, FC_Raster_Info_HT_Node, 1);
					DLLPushBack_NP(slot->first, slot->last, node, hash_next, hash_prev);
					node->hash = piece_hash;
					info = &node->info;
				}

				if (info != 0) {
					info->subrect = chosen_atlas_region;
					info->atlas_num = chosen_atlas_num;
					info->raster_dim = raster.atlas_dim;
					info->advance = raster.advance;
				}

				scratch_end(scratch);
			}

			if (info != 0) {
				//- kti: Find atlas.
				FC_Atlas *atlas = 0;
				if (info->subrect[2] != 0 && info->subrect[3] != 0) {
					L1 atlas_num = 0;
					for (FC_Atlas *a = fc_state->first_atlas; a != 0; a = a->next, atlas_num += 1) {
						if (atlas_num == info->atlas_num) {
							atlas = a;
							break;
						}
					}
				}

				F1 advance = info->advance;
				if (is_tab) {
					advance = floor_F1(tab_size_px) - mod_F1(floor_F1(base_align_px), floor_F1(tab_size_px));
				}

				//- kti: Create piece.
				FC_Piece *piece = fc_piece_chunk_list_push_new(fc_state->frame_arena, &piece_chunks, string.len);
				piece->texture = atlas ? atlas->texture : 0;
				piece->subrect = (SW4){
					info->subrect[0],
					info->subrect[1],
					info->raster_dim[0],
					info->raster_dim[1]
				};
				piece->advance = advance;
				piece->decode_size = piece_substring.len;
				piece->offset = (SW2){0, -(style_raster_node->ascent + style_raster_node->descent)};

				base_align_px += advance;
				dim[0] += piece->advance;
				dim[1] = Max(dim[1], info->raster_dim[1]);
			}
		}

		if (piece_chunks.node_count == 1) {
			run.pieces.v = piece_chunks.first->v;
			run.pieces.count = piece_chunks.first->count;
		} else {
			run.pieces = fc_piece_array_from_chunk_list(fc_state->frame_arena, &piece_chunks);
		}
		run.dim = dim;
		run.ascent = style_raster_node->ascent;
		run.descent = style_raster_node->descent;

	}

	//- kti: Build cache node for cacheable runs.
	if (run_is_cacheable) {
		run_node = push_array(fc_state->frame_arena, FC_Run_Cache_Node, 1);
		SLLQueuePush(run_slot->first, run_slot->last, run_node);
		run_node->string = push_str8_copy(fc_state->frame_arena, string);
		run_node->run = run;
	}

	ProfEnd();
	return run;
}

#endif
