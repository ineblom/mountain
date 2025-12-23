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

typedef struct FC_Font_Hash_Node FC_Font_Hash_Node;
struct FC_Font_Hash_Node {
	FC_Font_Hash_Node *hash_next;
	FC_Tag tag;
	FP_Handle handle;
	FP_Metrics metrics;
	String8 path;
};

typedef struct FC_Font_Hash_Slot FC_Font_Hash_Slot;
struct FC_Font_Hash_Slot {
	FC_Font_Hash_Node *first;
	FC_Font_Hash_Node *last;
};

typedef struct FC_State FC_State;
struct FC_State {
	Arena *arena;

	L1 font_hash_table_size;
	FC_Font_Hash_Slot *font_hash_table;
};

#endif

#if (CPU_ && ROM_)

Global FC_State *fc_state = 0;

Internal FC_Tag fc_tag_from_path(String8 path) {
	FC_Tag result = {0};

	XXH128_hash_t xxhash = XXH3_128bits(path.str, path.len);
	memmove(&result, &xxhash, sizeof(result));

	result.l1[1] |= 1llu << 63;

	L1 slot_idx = result.l1[1] % fc_state->font_hash_table_size;

	FC_Font_Hash_Node *existing_node = 0;
	for (FC_Font_Hash_Node *n = fc_state->font_hash_table[slot_idx].first; n != 0; n = n->hash_next) {
		if (memcmp(&result, &n->tag, sizeof(FC_Tag)) == 0) {
			existing_node = n;
			break;
		}
	}

	if (existing_node == 0) {
		FP_Handle handle = fp_font_open(path);
		FC_Font_Hash_Slot *slot = &fc_state->font_hash_table[slot_idx];
		existing_node = push_array(fc_state->arena, FC_Font_Hash_Node, 1);
		existing_node->tag = result;
		existing_node->handle = handle;
		existing_node->metrics = fp_metrics_from_font(handle);
		existing_node->path = push_str8_copy(fc_state->arena, path);
		SLLQueuePush_N(slot->first, slot->last, existing_node, hash_next);
	}

	if (fp_handle_match(existing_node->handle, fp_handle_zero())) {
		memset(&result, 0, sizeof(FC_Tag));
	}

	return result;
}

Internal void fc_init(void) {
	Arena *arena = arena_create(MiB(64));
	fc_state = push_array(arena, FC_State, 1);
	fc_state->arena = arena;
	fc_state->font_hash_table_size = 64;
	fc_state->font_hash_table = push_array(arena, FC_Font_Hash_Slot, fc_state->font_hash_table_size);
}

#endif
