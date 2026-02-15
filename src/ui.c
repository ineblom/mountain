#if (TYP_)

typedef I1 UI_Axis;

enum {
	UI_AXIS__X,
	UI_AXIS__Y,
	UI_AXIS_COUNT,
};

typedef I1 UI_Text_Align;
enum {
	UI_TEXT_ALIGN__LEFT = 0,
	UI_TEXT_ALIGN__RIGHT,
	UI_TEXT_ALIGN__CENTER,

	UI_TEXT_ALIGN_COUNT,
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
	UI_BOX_FLAG__CLICKABLE                = (1<<0),
	UI_BOX_FLAG__VIEW_SCROLL_X            = (1<<1),
	UI_BOX_FLAG__VIEW_SCROLL_Y            = (1<<2),
	UI_BOX_FLAG__DRAW_TEXT                = (1<<3),
	UI_BOX_FLAG__DRAW_BORDER              = (1<<4),
	UI_BOX_FLAG__DRAW_BACKGROUND          = (1<<5),
	UI_BOX_FLAG__DRAW_DROP_SHADOW         = (1<<6),
	UI_BOX_FLAG__CLIP                     = (1<<7),
	UI_BOX_FLAG__DRAW_HOT_EFFECTS         = (1<<8),
	UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS      = (1<<9),
	UI_BOX_FLAG__DISABLED                 = (1<<10),
	UI_BOX_FLAG__FLOATING_X               = (1<<11),
	UI_BOX_FLAG__FLOATING_Y               = (1<<12),
	UI_BOX_FLAG__FIXED_WIDTH              = (1<<13),
	UI_BOX_FLAG__FIXED_HEIGHT             = (1<<14),
	UI_BOX_FLAG__ALLOW_OVERFLOW_X         = (1<<15),
	UI_BOX_FLAG__ALLOW_OVERFLOW_Y         = (1<<16),
	UI_BOX_FLAG__SKIP_VIEW_OFF_X          = (1<<17),
	UI_BOX_FLAG__SKIP_VIEW_OFF_Y          = (1<<18),
	UI_BOX_FLAG__ROUND_CHILDREN_BY_PARENT = (1<<19),
	UI_BOX_FLAG__HAS_DISPLAY_STRING       = (1<<20),
	UI_BOX_FLAG__DISABLE_ID_STRING        = (1<<21),
	UI_BOX_FLAG__FOCUS_HOT                = (1<<22),
	UI_BOX_FLAG__FOCUS_ACTIVE             = (1<<23),
	UI_BOX_FLAG__FOCUS_HOT_DISABLED       = (1<<24),
	UI_BOX_FLAG__FOCUS_ACTIVE_DISABLED    = (1<<25),
	UI_BOX_FLAG__SCROLL                   = (1<<26),
	UI_BOX_FLAG__VIEW_CLAMP_X             = (1<<27),
	UI_BOX_FLAG__VIEW_CLAMP_Y             = (1<<28),

	UI_BOX_FLAG__FLOATING    = (UI_BOX_FLAG__FLOATING_X|UI_BOX_FLAG__FLOATING_Y),
	UI_BOX_FLAG__VIEW_SCROLL = (UI_BOX_FLAG__VIEW_SCROLL_X|UI_BOX_FLAG__VIEW_SCROLL_Y),
	UI_BOX_FLAG__VIEW_CLAMP  = (UI_BOX_FLAG__VIEW_CLAMP_X|UI_BOX_FLAG__VIEW_CLAMP_Y),
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
	UI_Key group_key;
	UI_Box_Flags flags;
	String8 string;
	F2 fixed_pos;
	F2 fixed_size;
	F2 min_size;
	F2 view_off;
	F2 view_off_target;
	F2 view_bounds;
	F4 rect;
	UI_Size pref_size[UI_AXIS_COUNT];
	UI_Axis child_layout_axis;
	L1 first_touch_build_index;
	L1 last_touch_build_index;

	FC_Tag font;
	F1 font_size;
	F1 tab_size;
	UI_Text_Align text_align;
	F4 corner_radii;
	F4 background_color;
	F4 text_color;
	F4 border_color;
	F1 text_padding;
	DR_FRun_List display_fruns;
	DR_FStr_List display_fstrs;

	F1 hot_t;
	F1 active_t;
};

typedef struct UI_Box_Rec UI_Box_Rec;
struct UI_Box_Rec {
	L1 push_count;
	L1 pop_count;
	UI_Box *next;
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

	Arena *drag_arena;
	String8 drag_data;

	Arena *build_arenas[2];
	L1 build_index;

	UI_Box *first_free_box;
	L1 box_table_size;
	UI_Box_HT_Slot *box_table;

	UI_Box *root;
	L1 build_box_count;
	L1 last_build_box_count;
	OS_Window *window;
	F2 mouse;
	F2 drag_start_mouse;
	L1 last_time_mouse_moved;

	UI_Key hot_box_key;
	UI_Key active_box_key[OS_MOUSE_BUTTON_COUNT];

	OS_Event_List events;
	UI_Key press_key_history[OS_MOUSE_BUTTON_COUNT][3];
	L1 press_timestamp_history[OS_MOUSE_BUTTON_COUNT][3];
	F2 press_pos_history[OS_MOUSE_BUTTON_COUNT][3];

	UIStacks;
};

#endif

#if (ROM_)

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

Internal String8 ui_display_part_from_key_string(String8 key) {
	L1 end = key.len;

	for EachIndex(i, key.len-1) {
		if (key.str[i] == '#' && key.str[i+1] == '#') {
			end = i;
			break;
		}
	}

	return str8_substr(key, 0, end);
}

Internal UI_Key ui_key_from_string(UI_Key seed_key, String8 string) {
	UI_Key result = {0};
	if (string.len != 0) {
		meow_u128 hash = MeowHash(MeowDefaultSeed, string.len, string.str);
		result.l1[0] = MeowU64From(hash, 0) ^ seed_key.l1[0];
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
	ui_state->drag_arena = arena_create(MiB(64));
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
	box->group_key = ui_top_group_key();

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
	box->text_align = ui_top_text_align();
	box->text_padding = ui_top_text_padding();
	box->font = ui_top_font();
	box->font_size = ui_top_font_size();
	box->corner_radii[0] = ui_top_tl_corner_radius();
	box->corner_radii[1] = ui_top_tr_corner_radius();
	box->corner_radii[2] = ui_top_bl_corner_radius();
	box->corner_radii[3] = ui_top_br_corner_radius();
	box->background_color = ui_top_background_color();
	box->text_color = ui_top_text_color();
	box->border_color = ui_top_border_color();

	UIAutoPopStacks();

	return box;
}

Internal String8 ui_box_display_string(UI_Box *box) {
	String8 result = box->string;
	if (!(box->flags & UI_BOX_FLAG__DISABLE_ID_STRING)) {
		result = ui_display_part_from_key_string(result);
	}
	return result;
}

Internal void ui_box_equip_display_string(UI_Box *box, String8 string) {
	box->string = push_str8_copy(ui_build_arena(), string);
	box->flags |= UI_BOX_FLAG__HAS_DISPLAY_STRING;
	F4 text_color = box->text_color;
	if (box->flags & UI_BOX_FLAG__DRAW_TEXT) {
		String8 display_string = ui_box_display_string(box);
		DR_FStr_Node fstr_n = {0, {display_string, {box->font, text_color, box->font_size, 0, 0}}};
		DR_FStr_List fstrs = {&fstr_n, &fstr_n, 1};
		box->display_fstrs = dr_fstrs_copy(ui_build_arena(), &fstrs);
		box->display_fruns = dr_fruns_from_fstrs(ui_build_arena(), box->tab_size, &box->display_fstrs);
	}
}


Internal UI_Box *ui_build_box_from_string(UI_Box_Flags flags, String8 string) {
	UI_Key key = ui_key_from_string(ui_active_seed_key(), string);
	UI_Box *box = ui_build_box_from_key(flags, key);
	if (flags & UI_BOX_FLAG__DRAW_TEXT) {
		ui_box_equip_display_string(box, string);
	}
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

Internal I1 ui_next_event(OS_Event **ev) {
	OS_Event_List events = ui_state->events;
	OS_Event *start_node = events.first;

	if (ev[0] != 0) {
		start_node = ev[0]->next;
		ev[0] = 0;
	}
	if (start_node != 0) {
		// TODO: Permissions?
		// Have permissions stack.
		// Skip events that don't fit permisisons.
		//
		// for now just return first node...
		ev[0] = start_node;
	}

	I1 result = !!ev[0];
	return result;
}

Internal void ui_eat_event(OS_Event *e) {
	DLLRemove(ui_state->events.first, ui_state->events.last, e);
	ui_state->events.count -= 1;
}

Internal UI_Signal ui_signal_from_box(UI_Box *box) {
	// TODO: Get from os.
	L1 double_click_time = 500000000;

	I1 is_focus_hot = box->flags & UI_BOX_FLAG__FOCUS_HOT && !(box->flags & UI_BOX_FLAG__FOCUS_HOT_DISABLED);
	UI_Signal signal = {0};
	signal.box = box;
	signal.modifiers |= os_get_modifiers();

	F4 rect = box->rect;
	for (UI_Box *p = box->parent; !ui_box_is_nil(p); p = p->parent) {
		if (!ui_key_match(p->key, ui_key_zero()) && p->flags & UI_BOX_FLAG__CLIP) {
			rect = intersect_rects(p->rect, rect);
		}
	}

	I1 view_scrolled = 0;
	for (OS_Event *e = 0; ui_next_event(&e);) {
		I1 taken = 0;

		signal.modifiers |= e->modifiers;

		F2 evt_mouse = {e->x, e->y};
		I1 evt_mouse_in_bounds = rect_contains(rect, evt_mouse); 
		I1 evt_key_is_mouse = (e->key == OS_MOUSE_BUTTON__LEFT ||
				e->key == OS_MOUSE_BUTTON__MIDDLE ||
				e->key == OS_MOUSE_BUTTON__RIGHT);
		I1 evt_mouse_idx = evt_key_is_mouse ?  e->key - OS_MOUSE_BUTTON__LEFT : 0;

		//- kti: Mouse down in bounds.
		if (box->flags & UI_BOX_FLAG__CLICKABLE &&
				e->type == OS_EVENT_TYPE__PRESS &&
				evt_mouse_in_bounds &&
				evt_key_is_mouse) {

			ui_state->hot_box_key = box->key;
			ui_state->active_box_key[evt_mouse_idx] = box->key;
			signal.flags |= UI_SIGNAL_FLAG__LEFT_PRESSED << evt_mouse_idx;
			ui_state->drag_start_mouse = evt_mouse;

			if (ui_key_match(box->key, ui_state->press_key_history[evt_mouse_idx][0]) &&
					e->timestamp_ns-ui_state->press_timestamp_history[evt_mouse_idx][0] <= double_click_time) {
				signal.flags |= UI_SIGNAL_FLAG__LEFT_DOUBLE_CLICKED << evt_mouse_idx;
			}

			if (ui_key_match(box->key, ui_state->press_key_history[evt_mouse_idx][0]) &&
					ui_key_match(box->key, ui_state->press_key_history[evt_mouse_idx][1]) &&
					e->timestamp_ns-ui_state->press_timestamp_history[evt_mouse_idx][0] <= double_click_time &&
					ui_state->press_timestamp_history[evt_mouse_idx][0]-ui_state->press_timestamp_history[evt_mouse_idx][1] <= double_click_time) {
				signal.flags |= UI_SIGNAL_FLAG__LEFT_TRIPPLE_CLICKED << evt_mouse_idx;
			}

			// Move history buffers back and fill in latest.
			memmove(&ui_state->press_key_history[evt_mouse_idx][1], &ui_state->press_key_history[evt_mouse_idx][0],
							sizeof(ui_state->press_key_history[evt_mouse_idx][0])*(ArrayCount(ui_state->press_key_history[evt_mouse_idx])-1));
			memmove(&ui_state->press_timestamp_history[evt_mouse_idx][1], &ui_state->press_timestamp_history[evt_mouse_idx][0],
							sizeof(ui_state->press_timestamp_history[evt_mouse_idx][0])*(ArrayCount(ui_state->press_timestamp_history[evt_mouse_idx])-1));
			memmove(&ui_state->press_pos_history[evt_mouse_idx][1], &ui_state->press_pos_history[evt_mouse_idx][0],
							sizeof(ui_state->press_pos_history[evt_mouse_idx][0])*(ArrayCount(ui_state->press_pos_history[evt_mouse_idx])-1));
			ui_state->press_key_history[evt_mouse_idx][0] = box->key;
			ui_state->press_timestamp_history[evt_mouse_idx][0] = e->timestamp_ns;
			ui_state->press_pos_history[evt_mouse_idx][0] = (F2){e->x, e->y};

			taken = 1;
		}

		//- kti: Mouse released in bounds. Triggers click.
		if (box->flags & UI_BOX_FLAG__CLICKABLE &&
				e->type == OS_EVENT_TYPE__RELEASE &&
				evt_mouse_in_bounds &&
				evt_key_is_mouse &&
				ui_key_match(ui_state->active_box_key[evt_mouse_idx], box->key)) {
			ui_state->active_box_key[evt_mouse_idx] = ui_key_zero();
			signal.flags |= UI_SIGNAL_FLAG__LEFT_RELEASED << evt_mouse_idx;
			signal.flags |= UI_SIGNAL_FLAG__LEFT_CLICKED << evt_mouse_idx;
			taken = 1;
		}

		//- kti: Mouse released outside of bounds.
		if (box->flags & UI_BOX_FLAG__CLICKABLE &&
				e->type == OS_EVENT_TYPE__RELEASE &&
				evt_key_is_mouse &&
				!evt_mouse_in_bounds &&
				ui_key_match(ui_state->active_box_key[evt_mouse_idx], box->key)) {
			ui_state->hot_box_key = ui_key_zero();
			ui_state->active_box_key[evt_mouse_idx] = ui_key_zero();
			signal.flags |= UI_SIGNAL_FLAG__LEFT_RELEASED << evt_mouse_idx;
			taken = 1;
		}

		//- kti: Scrolling
		F1 scroll_mult = 4.0f;

		if (box->flags & UI_BOX_FLAG__SCROLL &&
				e->type == OS_EVENT_TYPE__SCROLL && 
				(e->modifiers == 0 || e->modifiers == OS_MODIFIER_FLAG__SHIFT) &&
				evt_mouse_in_bounds) {
			F2 delta = (F2){e->delta_x, e->delta_y} * scroll_mult;
			if (e->modifiers == OS_MODIFIER_FLAG__SHIFT) {
				Swap(delta[0], delta[1]);
			}
			signal.scroll += delta;
			taken = 1;
		}

		if (box->flags & UI_BOX_FLAG__VIEW_SCROLL &&
				box->first_touch_build_index != box->last_touch_build_index &&
				e->type == OS_EVENT_TYPE__SCROLL &&
				(e->modifiers == 0 || e->modifiers == OS_MODIFIER_FLAG__SHIFT) &&
				evt_mouse_in_bounds) {
			F2 delta = (F2){e->delta_x, e->delta_y} * scroll_mult;
			if (e->modifiers == OS_MODIFIER_FLAG__SHIFT) {
				Swap(delta[0], delta[1]);
			}
			if (!(box->flags & UI_BOX_FLAG__VIEW_SCROLL_X)) {
				if (delta[1] == 0) {
					delta[1] = delta[0];
				}
				delta[0] = 0;
			}
			if (!(box->flags & UI_BOX_FLAG__VIEW_SCROLL_Y)) {
				if (delta[0] == 0) {
					delta[0] = delta[1];
				}
				delta[1] = 0;
			}
			box->view_off_target += delta;
			view_scrolled = 1;
			taken = 1;
		}

		if (taken) {
			ui_eat_event(e);
		}
	}

	//- kti: View clamp.
	if (box->flags & UI_BOX_FLAG__VIEW_CLAMP && view_scrolled) {
		F2 max_view_off_target = {
			ClampBot(0, box->view_bounds[0] - box->fixed_size[0]),
			ClampBot(0, box->view_bounds[1] - box->fixed_size[1]),
		};
		if (box->flags & UI_BOX_FLAG__VIEW_CLAMP_X) {
			box->view_off_target[0] = Clamp(0, box->view_off_target[0], max_view_off_target[0]);
		}
		if (box->flags & UI_BOX_FLAG__VIEW_CLAMP_Y) {
			box->view_off_target[1] = Clamp(0, box->view_off_target[1], max_view_off_target[1]);
		}
	}

	//- kti: Dragging
	if (box->flags & UI_BOX_FLAG__CLICKABLE) {
		for EachIndex(k, OS_MOUSE_BUTTON_COUNT) {
			if (ui_key_match(box->key, ui_state->active_box_key[k]) || signal.flags & (UI_SIGNAL_FLAG__LEFT_PRESSED<<k)) {
				signal.flags |= (UI_SIGNAL_FLAG__LEFT_DRAGGING<<k);

				if (ui_key_match(box->key, ui_state->press_key_history[k][0]) &&
						ui_key_match(box->key, ui_state->press_key_history[k][1]) &&
						ui_state->press_timestamp_history[k][0] - ui_state->press_timestamp_history[k][1] <= double_click_time &&
						length_sq_F2(ui_state->press_pos_history[k][0] - ui_state->press_pos_history[k][1]) < 10*10) {
					signal.flags |= (UI_SIGNAL_FLAG__LEFT_DOUBLE_DRAGGING<<k);

					if (ui_key_match(box->key, ui_state->press_key_history[k][2]) &&
							ui_state->press_timestamp_history[k][1] - ui_state->press_timestamp_history[k][2] <= double_click_time &&
							length_sq_F2(ui_state->press_pos_history[k][1] - ui_state->press_pos_history[k][2]) < 10*10) {
						signal.flags |= (UI_SIGNAL_FLAG__LEFT_TRIPPLE_DRAGGING<<k);
					}
				}
			}
		}
	}

	//- kti: Mouse over.
	if (rect_contains(rect, ui_state->mouse)) {
		signal.flags |= UI_SIGNAL_FLAG__MOUSE_OVER;
	}

	//- kti: Hovering.
  // TODO: The huge check making sure that one mouse button is available (or active for this box) may be unecessary.
	// This will almost never fail. Maybe at least make it a for loop. 
	if (box->flags & UI_BOX_FLAG__CLICKABLE &&
			rect_contains(rect, ui_state->mouse) &&
			(ui_key_match(ui_state->hot_box_key, ui_key_zero()) || ui_key_match(ui_state->hot_box_key, box->key)) &&
			(ui_key_match(ui_state->active_box_key[0], ui_key_zero()) || ui_key_match(ui_state->active_box_key[0], box->key)) &&
			(ui_key_match(ui_state->active_box_key[1], ui_key_zero()) || ui_key_match(ui_state->active_box_key[1], box->key)) &&
			(ui_key_match(ui_state->active_box_key[2], ui_key_zero()) || ui_key_match(ui_state->active_box_key[2], box->key))) {
		ui_state->hot_box_key = box->key;
		signal.flags |= UI_SIGNAL_FLAG__HOVERING;
	}

	//- kti: Group hovering.
	if (box->flags & UI_BOX_FLAG__CLICKABLE &&
			rect_contains(rect, ui_state->mouse) &&
			!ui_key_match(box->group_key, ui_key_zero())) {
		for (L1 k = 0; k < OS_MOUSE_BUTTON_COUNT; k += 1) {
			UI_Box *active_box = ui_box_from_key(ui_state->active_box_key[k]);
			if (ui_key_match(active_box->group_key, box->group_key)) {
				ui_state->hot_box_key = box->key;
				ui_state->active_box_key[k] = box->key;
				signal.flags |= UI_SIGNAL_FLAG__HOVERING|(UI_SIGNAL_FLAG__LEFT_DRAGGING<<k);
			}
		}
	}

	return signal;
}

Internal void ui_begin_build(OS_Window *window, OS_Event_List events) {
	UIResetStacks();
	ui_state->root = &ui_nil_box;
	ui_state->last_build_box_count = ui_state->build_box_count;
	ui_state->build_box_count = 0;
	ui_state->window = window;

	//- kti: Copy events.
	MemoryZeroStruct(&ui_state->events);
	L1 event_pool_idx = 0;
	OS_Event *event_pool = push_array_no_zero(ui_build_arena(), OS_Event, events.count);
	for (OS_Event *e = events.first; e != 0; e = e->next) {
		OS_Event *new_e = &event_pool[event_pool_idx];
		event_pool_idx += 1;
		memmove(new_e, e, sizeof(OS_Event));
		new_e->prev = 0;
		new_e->next = 0;
		DLLPushBack(ui_state->events.first, ui_state->events.last, new_e);
		ui_state->events.count += 1;
	}

	//- kti: Mouse movement.
	for (OS_Event *e = events.last; e != 0; e = e->prev) {
		if (e->type == OS_EVENT_TYPE__MOUSE_MOVE) {
			ui_state->mouse[0] = e->x;
			ui_state->mouse[1] = e->y;
			ui_state->last_time_mouse_moved = e->timestamp_ns;
			break;
		}
	}

	L1 now = os_clock();
	if (os_hovered_window() != window && now-ui_state->last_time_mouse_moved > 500000000) {
		ui_state->mouse[0] = -100.0f;
		ui_state->mouse[1] = -100.0f;
	}

	for (L1 k = 0; k < OS_MOUSE_BUTTON_COUNT; k += 1) {
		if (ui_key_match(ui_state->active_box_key[k], ui_key_zero()) && os_key_is_down(OS_MOUSE_BUTTON__LEFT+k)) {
			ui_state->active_box_key[k] = ui_state->external_key;
		} else if (ui_key_match(ui_state->active_box_key[k], ui_state->external_key) && !os_key_is_down(OS_MOUSE_BUTTON__LEFT+k)) {
			ui_state->active_box_key[k] = ui_key_zero();
		}
	}

	//- kti: Setup root.
	ui_set_next_fixed_width(window->width);
	ui_set_next_fixed_height(window->height);
	ui_set_next_child_layout_axis(UI_AXIS__X);
	UI_Box *root = ui_build_box_from_stringf(0, "%llu", (L1)window);
	ui_push_parent(root);
	ui_state->root = root;
	
	//- kti: Reset hot key if we don't have an active box.
	L1 has_active = 0;
	for (L1 k = 0; k < OS_MOUSE_BUTTON_COUNT; k += 1) {
		if (!ui_key_match(ui_state->active_box_key[k], ui_key_zero())) {
			has_active = 1;
			break;
		}
	}
	if (!has_active) {
		ui_state->hot_box_key = ui_key_zero();
	}

	//- kti: Reset active key when the box is disabled or nil.
	for (L1 k = 0; k < OS_MOUSE_BUTTON_COUNT; k += 1) {
		if (!ui_key_match(ui_state->active_box_key[k], ui_key_zero())) {
			UI_Box *box = ui_box_from_key(ui_state->active_box_key[k]);
			if (ui_box_is_nil(box) || box->flags & UI_BOX_FLAG__DISABLED) {
				ui_state->active_box_key[k] = ui_key_zero();
			}
		}
	}
}

Internal UI_Box_Rec ui_box_rec_df(UI_Box *box, UI_Box *root, L1 sib_member_off, L1 child_member_off) {
	UI_Box_Rec result = {0};
	result.next = &ui_nil_box;
	if (!ui_box_is_nil(*MemberFromOffset(UI_Box **, box, child_member_off))) {
		result.next = *MemberFromOffset(UI_Box **, box, child_member_off);
		result.push_count = 1;
	}
	else for (UI_Box *p = box; !ui_box_is_nil(p) && p != root; p = p->parent) {
		if (!ui_box_is_nil(*MemberFromOffset(UI_Box **, p, sib_member_off))) {
			result.next = *MemberFromOffset(UI_Box **, p, sib_member_off);
			break;
		}
		result.pop_count += 1;
	}

	return result;
}

#define ui_box_rec_df_pre(box, root) ui_box_rec_df(box, root, OffsetOf(UI_Box, next), OffsetOf(UI_Box, first))
#define ui_box_rec_df_post(box, root) ui_box_rec_df(box, root, OffsetOf(UI_Box, prev), OffsetOf(UI_Box, last))

Internal void ui_calc_sizes_standalone__in_place(UI_Box *root, UI_Axis axis) {
	for (UI_Box *b = root; !ui_box_is_nil(b); b = ui_box_rec_df_pre(b, root).next) {
		switch (b->pref_size[axis].kind) {
			default: {} break;
			case UI_SIZE_KIND__PIXELS: {
				b->fixed_size[axis] = b->pref_size[axis].value;
			} break;
			case UI_SIZE_KIND__TEXT_CONTENT: {
				F1 padding = b->pref_size[axis].value;
				F1 text_size = b->display_fruns.dim[axis];
				b->fixed_size[axis] = padding + text_size + b->text_padding * 2;
			} break;
		}
	}
}

Internal void ui_calc_sizes_upwards_dependent__in_place(UI_Box *root, UI_Axis axis) {
	for (UI_Box *b = root; !ui_box_is_nil(b); b = ui_box_rec_df_pre(b, root).next) {
		switch (b->pref_size[axis].kind) {
			default: {} break;
			case UI_SIZE_KIND__PERCENT_OF_PARENT: {
				UI_Box *fixed_parent = &ui_nil_box;
				for (UI_Box *p = b->parent; !ui_box_is_nil(p); p = p->parent) {
					if (p->flags & (UI_BOX_FLAG__FIXED_WIDTH<<axis) ||
							p->pref_size[axis].kind == UI_SIZE_KIND__PIXELS ||
							p->pref_size[axis].kind == UI_SIZE_KIND__TEXT_CONTENT ||
							p->pref_size[axis].kind == UI_SIZE_KIND__PERCENT_OF_PARENT) {
						fixed_parent = p;
						break;
					}
				}

				F1 size = fixed_parent->fixed_size[axis] * b->pref_size[axis].value;
				b->fixed_size[axis] = size;

			} break;
		}
	}
}

Internal void ui_calc_sizes_downwards_dependent__in_place(UI_Box *root, UI_Axis axis) {
	UI_Box_Rec rec = {0};
	for (UI_Box *box = root; !ui_box_is_nil(box); box = rec.next) {
		rec = ui_box_rec_df_pre(box, root);

		L1 pop_idx = 0;
		for (UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent, pop_idx += 1) {
			if (b->pref_size[axis].kind == UI_SIZE_KIND__CHILDREN_SUM) {
				F1 sum = 0;
				for (UI_Box *child = b->first; !ui_box_is_nil(child); child = child->next) {
					if (!(child->flags & (UI_BOX_FLAG__FLOATING_X<<axis))) {
						if (axis == b->child_layout_axis) {
							sum += child->fixed_size[axis];
						} else {
							sum = Max(sum, child->fixed_size[axis]);
						}
					}
				}
				b->fixed_size[axis] = sum;
			}
		}
	}
}

Internal void ui_layout_enforce_constraints__in_place(UI_Box *root, UI_Axis axis) {
	Temp_Arena scratch = scratch_begin(0, 0);

	for (UI_Box *box = root; !ui_box_is_nil(box); box = ui_box_rec_df_pre(box, root).next) {

		//- kti: Non layout axis
		if (axis != box->child_layout_axis && !(box->flags & (UI_BOX_FLAG__ALLOW_OVERFLOW_X << axis))) {
			F1 allowed_size = box->fixed_size[axis];
			for (UI_Box *child = box->first; !ui_box_is_nil(child); child = child->next) {
				if (!(child->flags & (UI_BOX_FLAG__FLOATING_X<<axis))) {
					F1 child_size = child->fixed_size[axis];
					F1 violation = child_size - allowed_size;
					F1 max_fixup = child_size;
					F1 fixup = Clamp(0, violation, max_fixup);
					child->fixed_size[axis] -= fixup;
				}
			}
		}

		//- kti: Layout axis
		if (axis == box->child_layout_axis && !(box->flags & (UI_BOX_FLAG__ALLOW_OVERFLOW_X<<axis))) {
			F1 total_allowed_size = box->fixed_size[axis];
			F1 total_size = 0;
			F1 total_weighted_size = 0;
			for (UI_Box *child = box->first; !ui_box_is_nil(child); child = child->next) {
				if (!(child->flags & (UI_BOX_FLAG__FLOATING_X<<axis))) {
					total_size += child->fixed_size[axis];
					total_weighted_size += child->fixed_size[axis] * (1-child->pref_size[axis].strictness);
				}
			}

			F1 violation = total_size - total_allowed_size;
			if (violation > 0 && total_weighted_size > 0) {
				for (UI_Box *child = box->first; !ui_box_is_nil(child); child = child->next) {
					if (!(child->flags & (UI_BOX_FLAG__FLOATING_X<<axis))) {
						F1 fixup_size_this_child = child->fixed_size[axis] * (1 - child->pref_size[axis].strictness);
						fixup_size_this_child = ClampBot(0, fixup_size_this_child);

						F1 fixup_pct = (violation / total_weighted_size);
						fixup_pct = Clamp(0, fixup_pct, 1);

						child->fixed_size[axis] -= fixup_size_this_child * fixup_pct;
					}
				}
			}
		}

		//- kti: fixup upwards-relative sizes
		if (box->flags & (UI_BOX_FLAG__ALLOW_OVERFLOW_X<<axis)) {
			for (UI_Box *child = box->first; !ui_box_is_nil(child); child = child->next) {
				if (child->pref_size[axis].kind == UI_SIZE_KIND__PERCENT_OF_PARENT) {
					child->fixed_size[axis] = box->fixed_size[axis] * child->pref_size[axis].value;
				}
			}
		}

		//- kti: enforce clamps
		for (UI_Box *child = box->first; !ui_box_is_nil(child); child = child->next) {
			child->fixed_size[axis] = ClampBot(child->min_size[axis], child->fixed_size[axis]);
		}
	}

	scratch_end(scratch);
}

Internal void ui_layout_position__in_place(UI_Box *root, UI_Axis axis) {
	for (UI_Box *box = root; !ui_box_is_nil(box); box = ui_box_rec_df_pre(box, root).next) {
		F1 layout_position = 0;

		F1 bounds = 0;
		for (UI_Box *child = box->first; !ui_box_is_nil(child); child = child->next) {
			if (!(child->flags & (UI_BOX_FLAG__FLOATING_X<<axis))) {
				child->fixed_pos[axis] = layout_position;
				if (box->child_layout_axis == axis) {
					layout_position += child->fixed_size[axis];
					bounds += child->fixed_size[axis];
				} else {
					bounds = Max(bounds, child->fixed_size[axis]);
				}
			}

			// TODO: Should rects be xy,wh or xy,xy?
			child->rect[axis] = box->rect[axis] + child->fixed_pos[axis] - !(child->flags&(UI_BOX_FLAG__SKIP_VIEW_OFF_X<<axis))*floor_F1(box->view_off[axis]);
			child->rect[2+axis] = child->fixed_size[axis];

			child->rect[0] = floor_F1(child->rect[0]);
			child->rect[1] = floor_F1(child->rect[1]);
			child->rect[2] = floor_F1(child->rect[2]);
			child->rect[3] = floor_F1(child->rect[3]);
		}

		box->view_bounds[axis] = bounds;
	}
}

Internal void ui_layout_root(UI_Box *root, UI_Axis axis) {
	ui_calc_sizes_standalone__in_place(root, axis);
	ui_calc_sizes_upwards_dependent__in_place(root, axis);
	ui_calc_sizes_downwards_dependent__in_place(root, axis);
	ui_layout_enforce_constraints__in_place(root, axis);
	ui_layout_position__in_place(root, axis);
}

Internal void ui_end_build(void) {
	//- kti: Prune untouched or transient boxes.
	for EachIndex(slot_idx, ui_state->box_table_size) {
		UI_Box_HT_Slot *slot = &ui_state->box_table[slot_idx];
		for (UI_Box *box = slot->first; !ui_box_is_nil(box); box = box->hash_next) {
			if (box->last_touch_build_index < ui_state->build_index ||
					ui_key_match(box->key, ui_key_zero())) {
				DLLRemove_NPZ(&ui_nil_box, slot->first, slot->last, box, hash_next, hash_prev);
				SLLStackPush(ui_state->first_free_box, box);
			}
		}
	}

	for EachIndex(axis, UI_AXIS_COUNT) {
		ui_layout_root(ui_state->root, axis);
	}

	//- kti: Enforce child rounding.
	for EachIndex(slot_idx, ui_state->box_table_size) {
		for (UI_Box *box = ui_state->box_table[slot_idx].first; !ui_box_is_nil(box); box = box->hash_next) {
			if (box->flags & UI_BOX_FLAG__ROUND_CHILDREN_BY_PARENT) {
				for (UI_Box *b = box; !ui_box_is_nil(b); b = ui_box_rec_df_pre(b, box).next) {
					if (floor_F1(b->rect[0]) <= floor_F1(box->rect[0]) &&
							floor_F1(b->rect[1]) <= floor_F1(box->rect[1])) {
						b->corner_radii[0] = box->corner_radii[0];
					}
					if (floor_F1(b->rect[0]+b->rect[2]) >= floor_F1(box->rect[0]+box->rect[2]) &&
							floor_F1(b->rect[1]) <= floor_F1(box->rect[1])) {
						b->corner_radii[1] = box->corner_radii[1];
					}
					if (floor_F1(b->rect[0]) <= floor_F1(box->rect[0]) &&
							floor_F1(b->rect[1]+b->rect[3]) >= floor_F1(box->rect[1]+box->rect[3])) {
						b->corner_radii[2] = box->corner_radii[2];
					}
					if (floor_F1(b->rect[0]+b->rect[2]) >= floor_F1(box->rect[0]+box->rect[2]) &&
							floor_F1(b->rect[1]+b->rect[3]) >= floor_F1(box->rect[1]+box->rect[3])) {
						b->corner_radii[2] = box->corner_radii[2];
					}
				}
				box->first->corner_radii[0] = box->corner_radii[0];
				box->first->corner_radii[1] = box->corner_radii[1];
				box->last->corner_radii[2] = box->corner_radii[2];
				box->last->corner_radii[3] = box->corner_radii[3];
			}
		}
	}

	//- kti: Animate
	for EachIndex(slot_idx, ui_state->box_table_size) {
		UI_Box_HT_Slot *slot = &ui_state->box_table[slot_idx];
		for (UI_Box *b = slot->first; !ui_box_is_nil(b); b = b->hash_next) {
			F1 scroll_animation_rate = 0.2f;

			b->view_off += scroll_animation_rate * (b->view_off_target - b->view_off);
			if (abs_F1(b->view_off_target[0] - b->view_off[0]) < 2.0f) {
				b->view_off[0] = b->view_off_target[0];
			}
			if (abs_F1(b->view_off_target[1] - b->view_off[1]) < 2.0f) {
				b->view_off[1] = b->view_off_target[1];
			}
		}
	}

	ui_state->build_index += 1;
	arena_clear(ui_build_arena());
}

Internal F2 ui_box_text_pos(UI_Box *box) {
	F2 result = {0};
	FC_Tag font = box->font;
	F1 font_size = box->font_size;
	FC_Metrics font_metrics = fc_metrics_from_tag_size(font, font_size);
	result[1] = floor_F1((box->rect[1]+box->rect[3]*0.5f) + font_metrics.ascent*0.5f - font_metrics.descent*0.5f);
	switch (box->text_align) {
		default:
		case UI_TEXT_ALIGN__LEFT: {
			result[0] = box->rect[0] + box->text_padding;
		} break;
		case UI_TEXT_ALIGN__CENTER: {
			F2 text_dim = box->display_fruns.dim;
			result[0] = round_F1(box->rect[0]+box->rect[2]*0.5f - text_dim[0]*0.5f);
			result[0] = ClampBot(result[0], box->rect[0]);
		} break;
		case UI_TEXT_ALIGN__RIGHT: {
			F2 text_dim = box->display_fruns.dim;
			result[0] = round_F1(box->rect[0]+box->rect[2] - text_dim[0] - box->text_padding);
			result[0] = ClampBot(result[0], box->rect[0]);
		} break;
	}
	result[0] = floor_F1(result[0]);
	return result;
}

Internal F2 ui_mouse(void) {
	return ui_state->mouse;
}

Internal F2 ui_drag_start_mouse(void) {
	F2 pos = ui_state->drag_start_mouse;
	return pos;
}

Internal F2 ui_drag_delta(void) {
	F2 delta = ui_mouse() - ui_state->drag_start_mouse;
	return delta;
}

Internal void ui_store_drag_data(String8 data) {
	arena_clear(ui_state->drag_arena);
	ui_state->drag_data = push_str8_copy(ui_state->drag_arena, data);
}

Internal String8 ui_get_drag_data(L1 min_required_size) {
	if (ui_state->drag_data.len < min_required_size) {
		Temp_Arena scratch = scratch_begin(0, 0);
		String8 data = {
			.str = push_array(scratch.arena, B1, min_required_size),
			.len = min_required_size,
		};
		ui_store_drag_data(data);
		scratch_end(scratch);
	}
	return ui_state->drag_data;
}

#define ui_store_drag_struct(ptr) ui_store_drag_data((String8){.str = (B1 *)(ptr), .len = sizeof(*(ptr)) })
#define ui_get_drag_struct(type) ((type *)ui_get_drag_data(sizeof(type)).str)

Internal UI_Box *ui_root(void) {
	UI_Box *root = ui_state->root;
	return root;
}

Internal UI_Key ui_hot_key(void) {
	UI_Key result = ui_state->hot_box_key;
	return result;
}

Internal UI_Key ui_active_key(I1 mouse_button) {
	// TODO: Maybe add UI mouse buttons enum that goes from 0 to BUTTON COUNT.
	Assert(mouse_button >= OS_MOUSE_BUTTON__LEFT && mouse_button < OS_MOUSE_BUTTON__LEFT+OS_MOUSE_BUTTON_COUNT);
	UI_Key result = ui_state->active_box_key[mouse_button-OS_MOUSE_BUTTON__LEFT];
	return result;
}

Internal void ui_push_corner_radius(F1 v) {
	ui_push_tl_corner_radius(v);
	ui_push_tr_corner_radius(v);
	ui_push_bl_corner_radius(v);
	ui_push_br_corner_radius(v);
}

Internal void ui_set_next_corner_radius(F1 v) {
	ui_set_next_tl_corner_radius(v);
	ui_set_next_tr_corner_radius(v);
	ui_set_next_bl_corner_radius(v);
	ui_set_next_br_corner_radius(v);
}

Internal void ui_pop_corner_radius(void) {
	ui_pop_tl_corner_radius();
	ui_pop_tr_corner_radius();
	ui_pop_bl_corner_radius();
	ui_pop_br_corner_radius();
}

#define UI_Corner_Radius(v) DeferLoop(ui_push_corner_radius(v), ui_pop_corner_radius())

Internal void ui_push_pref_size(UI_Axis axis, UI_Size size) {
	(axis == UI_AXIS__X ? ui_push_pref_width : ui_push_pref_height)(size);
}

Internal void ui_pop_pref_size(UI_Axis axis) {
	(axis == UI_AXIS__X ? ui_pop_pref_width : ui_pop_pref_height)();
}

Internal void ui_set_next_pref_size(UI_Axis axis, UI_Size size) {
	(axis == UI_AXIS__X ? ui_set_next_pref_width : ui_set_next_pref_height)(size);
}

Internal UI_Signal ui_spacer(UI_Size size) {
	UI_Box *parent = ui_top_parent();
	ui_set_next_pref_size(parent->child_layout_axis, size);
	UI_Box *box = ui_build_box_from_key(0, ui_key_zero());
	UI_Signal signal = ui_signal_from_box(box);
	return signal;
}

Internal UI_Size ui_size(UI_Size_Kind kind, F1 value, F1 strictness) {
	UI_Size result = {
		.kind = kind,
		.value = value,
		.strictness = strictness,
	};
	return result;
}

#define ui_px(value, strictness) ui_size(UI_SIZE_KIND__PIXELS, value, strictness)
#define ui_em(value, strictness) ui_size(UI_SIZE_KIND__PIXELS, (value)*ui_top_font_size(), strictness)
#define ui_text_dim(padding, strictness) ui_size(UI_SIZE_KIND__TEXT_CONTENT, padding, strictness)
#define ui_pct(value, strictness) ui_size(UI_SIZE_KIND__PERCENT_OF_PARENT, value, strictness)
#define ui_children_sum(strictness) ui_size(UI_SIZE_KIND__CHILDREN_SUM, 0.0f, strictness)

#define UI_Column() DeferLoop(ui_column_begin(), ui_column_end())
#define UI_Row() DeferLoop(ui_row_begin(), ui_row_end())
#define UI_Padding(v) DeferLoop(ui_spacer(v), ui_spacer(v))

Internal UI_Box *ui_named_column_begin(String8 name) {
	ui_set_next_child_layout_axis(UI_AXIS__Y);
	UI_Box *box = ui_build_box_from_string(0, name);
	ui_push_parent(box);
	return box;
}

Internal UI_Signal ui_named_column_end(void) {
	UI_Box *box = ui_pop_parent();
	UI_Signal signal = ui_signal_from_box(box);
	return signal;
}

Internal UI_Box *ui_named_row_begin(String8 name) {
	ui_set_next_child_layout_axis(UI_AXIS__X);
	UI_Box *box = ui_build_box_from_string(0, name);
	ui_push_parent(box);
	return box;
}

Internal UI_Signal ui_named_row_end(void) {
	UI_Box *box = ui_pop_parent();
	UI_Signal signal = ui_signal_from_box(box);
	return signal;
}

Internal UI_Box *ui_column_begin(void) { return ui_named_column_begin(Str8_("")); }
Internal UI_Box *ui_row_begin(void) { return ui_named_row_begin(Str8_("")); }
Internal UI_Signal ui_column_end(void) { return ui_named_column_end(); }
Internal UI_Signal ui_row_end(void) { return ui_named_row_end(); }

Internal UI_Signal ui_button(String8 string) {
	UI_Box *box = ui_build_box_from_string(
			UI_BOX_FLAG__CLICKABLE |
			UI_BOX_FLAG__DRAW_TEXT |
			UI_BOX_FLAG__DRAW_BORDER |
			UI_BOX_FLAG__DRAW_BACKGROUND |
			UI_BOX_FLAG__DRAW_HOT_EFFECTS |
			UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS,
			string);
	UI_Signal signal = ui_signal_from_box(box);
	return signal;
}

Internal UI_Signal ui_buttonf(CString fmt, ...) {
  Temp_Arena scratch = scratch_begin(0, 0);
  va_list args;
  va_start(args, fmt);
  String8 string = str8fv(scratch.arena, fmt, args);
  va_end(args);
  UI_Signal result = ui_button(string);
  scratch_end(scratch);
  return result;
}

Internal UI_Box *ui_pane_begin(F4 rect, String8 string) {
	ui_set_next_fixed_x(rect[0]);
	ui_set_next_fixed_y(rect[1]);
	ui_set_next_fixed_width(rect[2]);
	ui_set_next_fixed_height(rect[3]);
	ui_set_next_child_layout_axis(UI_AXIS__Y);
	UI_Box *box = ui_build_box_from_string(UI_BOX_FLAG__CLICKABLE|UI_BOX_FLAG__CLIP|UI_BOX_FLAG__DRAW_BORDER|UI_BOX_FLAG__DRAW_BACKGROUND, string);
	ui_push_parent(box);
	ui_push_pref_width(ui_pct(1.0f, 0.0f));
	return box;
}

Internal UI_Signal ui_pane_end(void) {
	ui_pop_pref_width();
	UI_Box *box = ui_pop_parent();
	UI_Signal signal = ui_signal_from_box(box);
	return signal;
}

Internal UI_Signal ui_slider_F1(String8 str, F1 *value, F1 min, F1 max) {
	UI_Signal signal = {0};

	F1 range = max - min;
	F1 pct = (value[0]-min) / range;
	F1 height = 20.0f;

	ui_set_next_pref_height(ui_px(height, 1.0f));
	UI_Row() {
		//- kti: Label
		UI_Pref_Width(ui_text_dim(0.0f, 1.0f))
		UI_Pref_Height(ui_text_dim(0.0f, 1.0f)) {
			ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str);
			ui_spacer(ui_em(1.0f, 1.0f));
		}

		//- kti: Value
		Temp_Arena scratch = scratch_begin(0, 0);
		F1 value_font_size = 10.0f;
		FC_Tag value_font = ui_top_font();
		F1 value_tab_px = ui_top_tab_size();
		String8 min_str = str8f(scratch.arena, "%.2f", min);
		String8 max_str = str8f(scratch.arena, "%.2f", max);
		F1 value_box_width = Max(
			fc_run_from_string(value_font, value_font_size, 0.0f, value_tab_px, min_str).dim[0],
			fc_run_from_string(value_font, value_font_size, 0.0f, value_tab_px, max_str).dim[0]);

		ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
		UI_Pref_Width(ui_px(value_box_width, 1.0f))
		UI_Column() UI_Padding(ui_pct(1.0f, 0.0f)) {
			UI_Font_Size(value_font_size)
			UI_Text_Align(UI_TEXT_ALIGN__RIGHT)
			UI_Pref_Width(ui_px(value_box_width, 1.0f))
			UI_Pref_Height(ui_text_dim(0.0f, 1.0f)) {
				ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str8f(scratch.arena, "%.2f", value[0]));
			}
		}

		scratch_end(scratch);
		ui_spacer(ui_px(10.f, 1.0f));

		//- kti: Slider
		F1 knob_size = 14.0f;
		ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
		UI_Pref_Width(ui_pct(1.0f, 0.0f))
		UI_Column() UI_Padding(ui_pct(1, 0)) {
			UI_Pref_Height(ui_px(height, 1.0f))
			UI_Corner_Radius(height*0.5f-1.0f)
			UI_Border_Color(oklch(0.5f, 0.0f, 0.0f, 1.0f)) {
				UI_Box *knob = &ui_nil_box;
				UI_Box *bg_box = &ui_nil_box;
				UI_Box *box = ui_build_box_from_stringf(UI_BOX_FLAG__CLICKABLE|UI_BOX_FLAG__DRAW_BORDER, "slider_%.*s", (int)str.len, str.str);
				UI_Parent(box) {
					F1 knob_inset = floor_F1((height-knob_size)*0.5f);
					F1 min = knob_size+knob_inset*2.0f;
					F1 w = floor_F1(min+(box->rect[2]-min)*pct);
					ui_set_next_background_color(oklch(0.195f, 0.100f, 17.0f, 1.0f));
					ui_set_next_pref_width(ui_px(w, 1.0f));
					bg_box = ui_build_box_from_key(UI_BOX_FLAG__DRAW_BACKGROUND|UI_BOX_FLAG__DRAW_BORDER, ui_key_zero());
					UI_Parent(bg_box) {
						ui_spacer(ui_px(knob_inset, 1.0f));
						ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
						ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
						UI_Row() {
							ui_spacer(ui_pct(1.0f, 0.0f));

							ui_set_next_background_color(oklch(0.7f, 0.0f, 0.0f, 1.0f));
							ui_set_next_pref_width(ui_px(knob_size, 1.0f));
							ui_set_next_pref_height(ui_px(knob_size, 1.0f));
							ui_set_next_corner_radius(knob_size*0.5f-2.0f);
							knob = ui_build_box_from_key(UI_BOX_FLAG__DRAW_BACKGROUND, ui_key_zero());

							ui_spacer(ui_px(knob_inset, 1.0f));
						}
					}
				}

				//- kti: Signal handling
				signal = ui_signal_from_box(box);
				if ((signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING || signal.flags & UI_SIGNAL_FLAG__HOVERING)
						&& !ui_box_is_nil(knob) && !ui_box_is_nil(bg_box)) {
					knob->background_color[0] = ClampTop(knob->background_color[0]+0.1f, 1.0f);
					bg_box->background_color[0] = ClampTop(bg_box->background_color[0]+0.04f, 1.0f);
				}

				if (signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
					if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
						ui_store_drag_struct(&pct);
					}

					F1 initial_pct = ui_get_drag_struct(F1)[0];
					F1 pct_change = ui_drag_delta()[0] / (box->rect[2]-knob_size);
					value[0] = min + Clamp(0, initial_pct+pct_change, 1.0f)*range;
				}
			}
		}
	}

	return signal;
}

Internal UI_Signal ui_checkbox(String8 str, I1 *value) {
	UI_Signal signal = {0};
	
	F1 size = 20.0f;
	F1 check_inset = 3.0f;

	UI_Row() {
		UI_Pref_Width(ui_px(size, 1.0f))
		UI_Column()
		UI_Pref_Height(ui_px(size, 1.0f))
		UI_Corner_Radius(check_inset*0.5f) 
		UI_Padding(ui_pct(1.0f, 0.0f)) {
			ui_set_next_background_color(oklch(0.195f, 0.1f, 17.0f, 1.0f));
			ui_set_next_border_color(oklch(0.5f, 0.0f, 0.0f, 1.0f));
			UI_Box *inset_box = &ui_nil_box;
			UI_Box *box = ui_build_box_from_stringf(UI_BOX_FLAG__CLICKABLE|UI_BOX_FLAG__DRAW_HOT_EFFECTS|UI_BOX_FLAG__DRAW_BACKGROUND|UI_BOX_FLAG__DRAW_BORDER, "##checkbox_%.*s", (int)str.len, str.str);

			if (value[0]) {
				UI_Parent(box)
				// UI_Background_Color(oklch(0.2f, 0.5f, 17.0f, 1.0f))
				UI_Background_Color(oklch(0.7f, 0.0f, 0.0f, 1.0f))
				UI_Padding(ui_pct(1.0f, 0.0f)) {
					UI_Pref_Height(ui_px(size-check_inset*2, 1.0f))
					UI_Row()
					UI_Padding(ui_pct(1.0f, 0.0f))
					UI_Pref_Width(ui_px(size-check_inset*2, 1.0f)) {
						inset_box = ui_build_box_from_key(UI_BOX_FLAG__DRAW_BACKGROUND, ui_key_zero());
					}
				}
			}

			signal = ui_signal_from_box(box);
			if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
				value[0] = !value[0];
			}
			if (!ui_box_is_nil(inset_box) && signal.flags & UI_SIGNAL_FLAG__HOVERING) {
				inset_box->background_color[0] = ClampTop(1.0f, inset_box->background_color[0]+0.1f);
			}
		}
		
		ui_spacer(ui_px(10.0f, 1.0f));

		//- kti: Label
		UI_Pref_Width(ui_text_dim(0.0f, 1.0f))
		UI_Pref_Height(ui_text_dim(0.0f, 1.0f)) {
			ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str);
			ui_spacer(ui_em(1.0f, 1.0f));
		}
	}

	return signal;
}

Internal void ui_textbox(String8 label, String8 *str, L1 buffer_size) {
	UI_Border_Color(oklch(0.7f, 0.0f, 0.0f, 1.0f)) {
		UI_Box *box = ui_build_box_from_string(UI_BOX_FLAG__CLICKABLE | UI_BOX_FLAG__DRAW_BORDER | UI_BOX_FLAG__CLIP, Str8_("textbox"));

	}

}

#endif
