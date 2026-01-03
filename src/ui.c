#if (CPU_ && TYP_)

enum {
	UI_AXIS__X,
	UI_AXIS__Y,
	UI_AXIS_COUNT,
};

typedef struct UI_Key UI_Key;
struct UI_Key {
	L1 l1[1];
};

typedef I1 UI_Size_Kind;
enum {
	UI_SIZE_KIND__NULL,
	UI_SIZE_KIND__PIXELS,
	UI_SIZE_KIND__TEXT_CONTENT,
	UI_SIZE_KIND__PERCENT_OF_PARENT,
	UI_SIZE_KIND__CHILDREN_SUM,
};

typedef struct UI_Size UI_Size;
struct UI_Size {
	UI_Size_Kind kind;
	F1 value;
	F1 strictness;
};

typedef L1 UI_Box_Flags;
enum {
	UI_BOX_FLAG__CLICKABLE        = (1<<0),
	UI_BOX_FLAG__VIEW_SCROLL      = (1<<1),
	UI_BOX_FLAG__DRAW_TEXT        = (1<<2),
	UI_BOX_FLAG__DRAW_BORDER      = (1<<3),
	UI_BOX_FLAG__DRAW_BACKGROUND  = (1<<4),
	UI_BOX_FLAG__DRAW_DROP_SHADOW = (1<<5),
	UI_BOX_FLAG__CLIP             = (1<<6),
	UI_BOX_FLAG__HOT_ANIMATION    = (1<<7),
	UI_BOX_FLAG__ACTIVE_ANIMATION = (1<<8),
	UI_BOX_FLAG__DISABLED         = (1<<9),
	UI_BOX_FLAG__FLOATING_X       = (1<<10),
	UI_BOX_FLAG__FLOATING_Y       = (1<<11),
	UI_BOX_FLAG__FIXED_WIDTH      = (1<<12),
	UI_BOX_FLAG__FIXED_HEIGHT     = (1<<13),
};

typedef struct UI_Box UI_Box;
struct UI_Box {
	UI_Box *hash_next;
	UI_Box *hash_prev;

	UI_Box *first;
	UI_Box *last;
	UI_Box *next;
	UI_Box *prev;
	UI_Box *parent;
	L1 child_count;

	UI_Key key;
	UI_Box_Flags flags;
	String8 string;
	F2 fixed_pos;
	F2 fixed_size;
	F2 min_size;
	UI_Size pref_size[UI_AXIS_COUNT];
	I1 child_layout_axis;
	L1 first_touch_build_index;
	L1 last_touch_build_index;

	FC_Tag font;
	F1 font_size;
	F1 tab_size;
	F1 corner_radii[4];
	F4 background_color;
	F4 text_color;
	F4 border_color;
	DR_Bucket *draw_bucket;

	F1 hot_t;
	F1 active_t;
};

typedef struct UI_Box_Node UI_Box_Node;
struct UI_Box_Node {
	UI_Box_Node *next;
	UI_Box *box;
};

typedef struct UI_Box_List UI_Box_List;
struct UI_Box_List {
	UI_Box_Node *first;
	UI_Box_Node *last;
	L1 count;
};

typedef struct UI_Box_HT_Slot UI_Box_HT_Slot;
struct UI_Box_HT_Slot {
	UI_Box *first;
	UI_Box *last;
};

typedef L1 UI_Signal_Flags;
enum {
	UI_SIGNAL_FLAG__LEFT_PRESSED   = (1<<0),
	UI_SIGNAL_FLAG__MIDDLE_PRESSED = (1<<1),
	UI_SIGNAL_FLAG__RIGHT_PRESSED  = (1<<2),

	UI_SIGNAL_FLAG__LEFT_DRAGGING   = (1<<3),
	UI_SIGNAL_FLAG__MIDDLE_DRAGGING = (1<<4),
	UI_SIGNAL_FLAG__RIGHT_DRAGGING  = (1<<5),

	UI_SIGNAL_FLAG__LEFT_DOUBLE_DRAGGING   = (1<<6),
	UI_SIGNAL_FLAG__MIDDLE_DOUBLE_DRAGGING = (1<<7),
	UI_SIGNAL_FLAG__RIGHT_DOUBLE_DRAGGING  = (1<<8),

	UI_SIGNAL_FLAG__LEFT_TRIPPLE_DRAGGING   = (1<<9),
	UI_SIGNAL_FLAG__MIDDLE_TRIPPLE_DRAGGING = (1<<10),
	UI_SIGNAL_FLAG__RIGHT_TRIPPLE_DRAGGING  = (1<<11),

	UI_SIGNAL_FLAG__LEFT_RELEASED   = (1<<12),
	UI_SIGNAL_FLAG__MIDDLE_RELEASED = (1<<13),
	UI_SIGNAL_FLAG__RIGHT_RELEASED  = (1<<14),

	UI_SIGNAL_FLAG__LEFT_CLICKED   = (1<<15),
	UI_SIGNAL_FLAG__MIDDLE_CLICKED = (1<<16),
	UI_SIGNAL_FLAG__RIGHT_CLICKED  = (1<<17),

	UI_SIGNAL_FLAG__LEFT_DOUBLE_CLICKED   = (1<<18),
	UI_SIGNAL_FLAG__MIDDLE_DOUBLE_CLICKED = (1<<19),
	UI_SIGNAL_FLAG__RIGHT_DOUBLE_CLICKED  = (1<<20),

	UI_SIGNAL_FLAG__LEFT_TRIPPLE_CLICKED   = (1<<21),
	UI_SIGNAL_FLAG__MIDDLE_TRIPPLE_CLICKED = (1<<22),
	UI_SIGNAL_FLAG__RIGHT_TRIPPLE_CLICKED  = (1<<23),

	UI_SIGNAL_FLAG__KEYBOARD_PRESSED = (1<<24),

	UI_SIGNAL_FLAG__HOVERING   = (1<<25),
	UI_SIGNAL_FLAG__MOUSE_OVER = (1<<26),

	UI_SIGNAL_FLAG__COMMIT = (1<<27),
};

typedef struct UI_Signal UI_Signal;
struct UI_Signal {
	UI_Box *box;
	OS_Modifier_Flags modifiers;
	F2 scroll;
	UI_Signal_Flags flags;
};

#include "ui.meta.h"

typedef struct UI_State UI_State;
struct UI_State {
	Arena *arena;
	UI_Key external_key;

	Arena *build_arenas[2];
	L1 build_index;

	UI_Box *first_free_box;
	L1 box_table_size;
	UI_Box_HT_Slot *box_table;

	UI_Box *root;
	L1 build_box_count;
	L1 last_build_box_count;
	OS_Window *window;

	UI_Key hot_box_key;
	UI_Key active_box_key;

	UIStacks;
};

#endif

#if (CPU_ && ROM_)

Global UI_Box ui_nil_box = {
	&ui_nil_box,
	&ui_nil_box,

	&ui_nil_box,
	&ui_nil_box,
	&ui_nil_box,
	&ui_nil_box,
	&ui_nil_box,
};

Global UI_State *ui_state = 0;

Inline UI_Size ui_px(F1 value, F1 strictness) {
	UI_Size result = {
		.kind = UI_SIZE_KIND__PIXELS,
		.value = value,
		.strictness = strictness,
	};
	return result;
}

Internal Arena *ui_build_arena(void) {
	Arena *arena = ui_state->build_arenas[ui_state->build_index%ArrayCount(ui_state->build_arenas)];
	return arena;
}

#include "ui.meta.c"

Internal UI_Key ui_key_zero(void) {
	UI_Key result = {0};
	return result;
}

Internal I1 ui_key_match(UI_Key a, UI_Key b) {
	I1 result = (a.l1[0] == b.l1[0]);
	return result;
}

Internal UI_Key ui_key_from_string(UI_Key seed_key, String8 string) {
	UI_Key result = {0};
	if (string.len != 0) {
		result.l1[0] = XXH3_64bits_withSeed(string.str, string.len, seed_key.l1[0]);
	}
	return result;
}

Internal void ui_init(void) {
	Arena *arena = arena_create(MiB(64));
	ui_state = push_array(arena, UI_State, 1);
	ui_state->arena = arena;
	ui_state->external_key = ui_key_from_string(ui_key_zero(), Str8_("external_interaction_key"));
	ui_state->build_arenas[0] = arena_create(MiB(64));
	ui_state->build_arenas[1] = arena_create(MiB(64));
	ui_state->box_table_size = 4096;
	ui_state->box_table = push_array(arena, UI_Box_HT_Slot, ui_state->box_table_size);
	UIInitStackNils();
}

Internal I1 ui_box_is_nil(UI_Box *box) {
	I1 result = (box == 0 || box == &ui_nil_box);
	return result;
}

Internal UI_Box *ui_box_from_key(UI_Key key) {
	UI_Box *result = &ui_nil_box;

	if (!ui_key_match(key, ui_key_zero())) {
		UI_Box_HT_Slot *slot = &ui_state->box_table[key.l1[0]%ui_state->box_table_size];
		for (UI_Box *b = slot->first; !ui_box_is_nil(b); b = b->hash_next) {
			if (ui_key_match(b->key, key)) {
				result = b;
				break;
			}
		}
	}

	return result;
}

Internal UI_Key ui_active_seed_key(void) {
	UI_Box *keyed_ancestor = &ui_nil_box;
	for (UI_Box *p = ui_top_parent(); !ui_box_is_nil(p); p = p->parent) {
		if (!ui_key_match(p->key, ui_key_zero())) {
			keyed_ancestor = p;
			break;
		}
	}
	return keyed_ancestor->key;
}

Internal UI_Box *ui_build_box_from_key(UI_Box_Flags flags, UI_Key key) {
	ui_state->build_box_count += 1;

	UI_Box *parent = ui_top_parent();

	UI_Box *box = ui_box_from_key(key);
	I1 box_first_frame = ui_box_is_nil(box);
	I1 last_flags = box->flags;

	//- kti: zero key and create new on key duplicate.
	if (!box_first_frame && box->last_touch_build_index == ui_state->build_index) {
		box = &ui_nil_box;
		key = ui_key_zero();
		box_first_frame = 1;
	}

	// NOTE(kti): Transient = box lasts for 1 build only.
	I1 box_is_transient = ui_key_match(key, ui_key_zero());

	//- kti: Allocate box if needed.
	if (box_first_frame) {
		box = !box_is_transient ? ui_state->first_free_box : 0;
		if (ui_box_is_nil(box)) {
			box = push_array_no_zero(box_is_transient ? ui_build_arena() : ui_state->arena, UI_Box, 1);
		} else {
			SLLStackPop(ui_state->first_free_box);
		}
		MemoryZeroStruct(box);
	}

	//- kti: Zero per frame state.
	box->first = box->last = box->next = box->prev = box->parent = &ui_nil_box;
	box->child_count = 0;
	box->flags = 0;
	MemoryZeroArray(box->pref_size);
	box->draw_bucket = 0;

	//- kti: Add to persistent table.
	if (box_first_frame && !box_is_transient) {
		UI_Box_HT_Slot *slot = &ui_state->box_table[key.l1[0] % ui_state->box_table_size];
		DLLInsert_NPZ(&ui_nil_box, slot->first, slot->last, slot->last, box, hash_next, hash_prev);
	}

	//- kti: Add to per-frame tree structure.
	if (!ui_box_is_nil(parent)) {
		DLLPushBack_NPZ(&ui_nil_box, parent->first, parent->last, box, next, prev);
		parent->child_count += 1;
		box->parent = parent;
	}

	//- kti: Fill box
	box->key = key;
	box->flags = flags; // TODO: Flags & Omit flags stack.
	if (box_first_frame) {
		box->first_touch_build_index = ui_state->build_index;
	}
	box->last_touch_build_index = ui_state->build_index;

	if (ui_state->fixed_x_stack.top != &ui_state->nil_fixed_x) {
		box->flags |= UI_BOX_FLAG__FLOATING_X;
		box->fixed_pos[0] = ui_top_fixed_x();
	}
	if (ui_state->fixed_y_stack.top != &ui_state->nil_fixed_y) {
		box->flags |= UI_BOX_FLAG__FLOATING_Y;
		box->fixed_pos[1] = ui_top_fixed_y();
	}

	if (ui_state->fixed_width_stack.top != &ui_state->nil_fixed_width) {
		box->flags |= UI_BOX_FLAG__FIXED_WIDTH;
		box->fixed_size[0] = ui_top_fixed_width();
	} else {
		box->pref_size[UI_AXIS__X] = ui_top_pref_width();
	}
	if (ui_state->fixed_height_stack.top != &ui_state->nil_fixed_height) {
		box->flags |= UI_BOX_FLAG__FIXED_HEIGHT;
		box->fixed_size[1] = ui_top_fixed_height();
	} else {
		box->pref_size[UI_AXIS__Y] = ui_top_pref_height();
	}

	box->min_size[0] = ui_top_min_width();
	box->min_size[1] = ui_top_min_height();
	box->child_layout_axis = ui_top_child_layout_axis();
	box->tab_size = ui_top_tab_size();
	box->font = ui_top_font();
	box->font_size = ui_top_font_size();

	if (flags & UI_BOX_FLAG__DRAW_BACKGROUND) {
		box->background_color = ui_top_background_color();
	}

	if (flags & UI_BOX_FLAG__DRAW_TEXT) {
		box->text_color = ui_top_text_color();
	}

	UIAutoPopStacks();

	return box;
}

Internal UI_Box *ui_build_box_from_string(UI_Box_Flags flags, String8 string) {
	UI_Key key = ui_key_from_string(ui_active_seed_key(), string);
	UI_Box *box = ui_build_box_from_key(flags, key);
	// TODO: if draw text flag equip display string.
	return box;
}

Internal UI_Box *ui_build_box_from_stringf(UI_Box_Flags flags, CString fmt, ...) {
	Temp_Arena scratch = scratch_begin(0, 0);
	va_list args;
	va_start(args, fmt);
	String8 string = str8fv(scratch.arena, fmt, args);
	va_end(args);
	UI_Box *box = ui_build_box_from_string(flags, string);
	scratch_end(scratch);
	return box;
}

Internal UI_Signal ui_signal_from_box(UI_Box *box) {
	UI_Signal result = {0};

	return result;
}

Internal void ui_begin_build(OS_Window *window) {
	UIResetStacks();
	ui_state->root = &ui_nil_box;
	ui_state->last_build_box_count = ui_state->build_box_count;
	ui_state->build_box_count = 0;
	ui_state->window = window;

	// TODO: Detect external press & holds
	// This sets the active box key to the external key.
	// Handling the user interacting with something outside this ui system.

	ui_set_next_fixed_width(window->width);
	ui_set_next_fixed_height(window->height);
	ui_set_next_child_layout_axis(UI_AXIS__X);
	UI_Box *root = ui_build_box_from_stringf(0, "%llu", (L1)window);
	ui_push_parent(root);
	ui_state->root = root;
	
	//- kti: Reset hot key if we don't have an active box.
	if (ui_key_match(ui_state->active_box_key, ui_key_zero())) {
		ui_state->hot_box_key = ui_key_zero();
	}

	//- kti: Reset active key if box is disabled.
	if (!ui_key_match(ui_state->active_box_key, ui_key_zero())) {
		UI_Box *box = ui_box_from_key(ui_state->active_box_key);
		if (!ui_box_is_nil(box) && box->flags & UI_BOX_FLAG__DISABLED) {
			ui_state->active_box_key = ui_key_zero();
		}
	}

	//- kti: Reset active key if box is nil.
	if (!ui_key_match(ui_state->active_box_key, ui_key_zero())) {
		UI_Box *box = ui_box_from_key(ui_state->active_box_key);
		if (ui_box_is_nil(box)) {
			ui_state->active_box_key = ui_key_zero();
		}
	}
}

Internal void ui_end_build(void) {
	ui_state->build_index += 1;
	arena_clear(ui_build_arena());
}

Internal UI_Signal ui_button(String8 string) {
	UI_Box *box = ui_build_box_from_string(
			UI_BOX_FLAG__CLICKABLE |
			UI_BOX_FLAG__DRAW_TEXT |
			UI_BOX_FLAG__DRAW_BORDER |
			UI_BOX_FLAG__DRAW_BACKGROUND |
			UI_BOX_FLAG__HOT_ANIMATION |
			UI_BOX_FLAG__ACTIVE_ANIMATION,
			string);
	UI_Signal signal = ui_signal_from_box(box);
	return signal;
}

#endif
