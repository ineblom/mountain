#if (TYP_)

typedef enum UI_Cmd_Kind
{
  UI_CMD_KIND__NULL,
  UI_CMD_KIND__TEXT,
  UI_CMD_KIND__NAVIGATE,
  UI_CMD_KIND__EDIT,
  UI_CMD_KIND__FILEDROP,
  UI_CMD_KIND__CANCEL,
  UI_CMD_KIND_COUNT
}
UI_Cmd_Kind;

typedef I1 UI_Cmd_Flags;

enum {
	UI_CMD_FLAG__EXPLICIT_DIRECTIONAL = (1<<0),
	UI_CMD_FLAG__ZERO_DELTA_ON_SELECT = (1<<1),
	UI_CMD_FLAG__CAP_AT_LINE          = (1<<2),
	UI_CMD_FLAG__PICK_SELECT_SIDE     = (1<<3),
	UI_CMD_FLAG__COPY                 = (1<<4),
	UI_CMD_FLAG__PASTE                = (1<<5),
	UI_CMD_FLAG__DELETE               = (1<<6),
	UI_CMD_FLAG__KEEP_MARK            = (1<<7),
};

typedef enum UI_Cmd_Delta_Unit {
  UI_CMD_DELTA_UNIT__NULL,
  UI_CMD_DELTA_UNIT__CHAR,
  UI_CMD_DELTA_UNIT__WORD,
  UI_CMD_DELTA_UNIT__LINE,
  UI_CMD_DELTA_UNIT__PAGE,
  UI_CMD_DELTA_UNIT__WHOLE,
  UI_CMD_DELTA_UNIT_COUNT
}
UI_Cmd_Delta_Unit;

typedef struct UI_Cmd UI_Cmd;
struct UI_Cmd {
  UI_Cmd *next;
  UI_Cmd *prev;

  UI_Cmd_Kind kind;
  UI_Cmd_Delta_Unit delta_unit;
	UI_Cmd_Flags flags;
  String8 string;
  String8_List paths;
  F2 pos;
  F2 delta_f2;
  SI2 delta_si2;
  L1 timestamp_ns;
};

typedef struct UI_Cmd_List UI_Cmd_List;
struct UI_Cmd_List {
  UI_Cmd *first;
  UI_Cmd *last;
  L1 count;
};

typedef struct Txt_Pt Txt_Pt;
struct Txt_Pt {
	L1 line;
	L1 column;
};

typedef struct Txt_Range Txt_Range;
struct Txt_Range {
	Txt_Pt min;
	Txt_Pt max;
};

typedef I1 UI_Txt_Op_Flags;
enum {
  UI_TXT_OP_FLAG__INVALID = (1<<0),
  UI_TXT_OP_FLAG__COPY    = (1<<1),
};

typedef struct UI_Txt_Op UI_Txt_Op;
struct UI_Txt_Op
{
  UI_Txt_Op_Flags flags;
  String8 replace;
  String8 copy;
  Txt_Range range;
  Txt_Pt cursor;
  Txt_Pt mark;
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

typedef I1 UI_Focus_Kind;
enum {
	UI_FOCUS_KIND__NULL,
	UI_FOCUS_KIND__OFF,
	UI_FOCUS_KIND__ON,
	UI_FOCUS_KIND__ROOT,
	UI_FOCUS_KIND_COUNT,
};

typedef L1 UI_Box_Flags;
enum {
	UI_BOX_FLAG__MOUSE_CLICKABLE          = (1LLU<<0),
	UI_BOX_FLAG__VIEW_SCROLL_X            = (1LLU<<1),
	UI_BOX_FLAG__VIEW_SCROLL_Y            = (1LLU<<2),
	UI_BOX_FLAG__DRAW_TEXT                = (1LLU<<3),
	UI_BOX_FLAG__DRAW_BORDER              = (1LLU<<4),
	UI_BOX_FLAG__DRAW_BACKGROUND          = (1LLU<<5),
	UI_BOX_FLAG__DRAW_DROP_SHADOW         = (1LLU<<6),
	UI_BOX_FLAG__CLIP                     = (1LLU<<7),
	UI_BOX_FLAG__DRAW_HOT_EFFECTS         = (1LLU<<8),
	UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS      = (1LLU<<9),
	UI_BOX_FLAG__DISABLED                 = (1LLU<<10),
	UI_BOX_FLAG__FLOATING_X               = (1LLU<<11),
	UI_BOX_FLAG__FLOATING_Y               = (1LLU<<12),
	UI_BOX_FLAG__FIXED_WIDTH              = (1LLU<<13),
	UI_BOX_FLAG__FIXED_HEIGHT             = (1LLU<<14),
	UI_BOX_FLAG__ALLOW_OVERFLOW_X         = (1LLU<<15),
	UI_BOX_FLAG__ALLOW_OVERFLOW_Y         = (1LLU<<16),
	UI_BOX_FLAG__SKIP_VIEW_OFF_X          = (1LLU<<17),
	UI_BOX_FLAG__SKIP_VIEW_OFF_Y          = (1LLU<<18),
	UI_BOX_FLAG__ROUND_CHILDREN_BY_PARENT = (1LLU<<19),
	UI_BOX_FLAG__HAS_DISPLAY_STRING       = (1LLU<<20),
	UI_BOX_FLAG__DISABLE_ID_STRING        = (1LLU<<21),
	UI_BOX_FLAG__FOCUS_HOT                = (1LLU<<22),
	UI_BOX_FLAG__FOCUS_ACTIVE             = (1LLU<<23),
	UI_BOX_FLAG__FOCUS_HOT_DISABLED       = (1LLU<<24),
	UI_BOX_FLAG__FOCUS_ACTIVE_DISABLED    = (1LLU<<25),
	UI_BOX_FLAG__SCROLL                   = (1LLU<<26),
	UI_BOX_FLAG__VIEW_CLAMP_X             = (1LLU<<27),
	UI_BOX_FLAG__VIEW_CLAMP_Y             = (1LLU<<28),
	UI_BOX_FLAG__DEFAULT_FOCUS_NAV_X      = (1LLU<<29),
	UI_BOX_FLAG__DEFAULT_FOCUS_NAV_Y      = (1LLU<<30),
	UI_BOX_FLAG__FOCUS_NAV_SKIP           = (1LLU<<31),
	UI_BOX_FLAG__KEYBOARD_CLICKABLE       = (1LLU<<33),
	UI_BOX_FLAG__CLICK_TO_FOCUS           = (1LLU<<34),
	UI_BOX_FLAG__DISABLE_FOCUS_OVERLAY    = (1LLU<<35),
	UI_BOX_FLAG__DISABLE_FOCUS_BORDER     = (1LLU<<36),
	UI_BOX_FLAG__DISABLE_TEXT_TRUNC       = (1LLU<<37),

	UI_BOX_FLAG__CLICKABLE         = (UI_BOX_FLAG__MOUSE_CLICKABLE|UI_BOX_FLAG__KEYBOARD_CLICKABLE),
	UI_BOX_FLAG__FLOATING          = (UI_BOX_FLAG__FLOATING_X|UI_BOX_FLAG__FLOATING_Y),
	UI_BOX_FLAG__VIEW_SCROLL       = (UI_BOX_FLAG__VIEW_SCROLL_X|UI_BOX_FLAG__VIEW_SCROLL_Y),
	UI_BOX_FLAG__VIEW_CLAMP        = (UI_BOX_FLAG__VIEW_CLAMP_X|UI_BOX_FLAG__VIEW_CLAMP_Y),
	UI_BOX_FLAG__DEFAULT_FOCUS_NAV = (UI_BOX_FLAG__DEFAULT_FOCUS_NAV_X|UI_BOX_FLAG__DEFAULT_FOCUS_NAV_Y),
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
	UI_Size pref_size[AXIS_COUNT];
	Axis child_layout_axis;
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

	UI_Key default_nav_focus_hot_key;
	UI_Key default_nav_focus_active_key;
	UI_Key default_nav_focus_next_hot_key;
	UI_Key default_nav_focus_next_active_key;
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

	UI_SIGNAL_FLAG__PRESSED = UI_SIGNAL_FLAG__LEFT_PRESSED|UI_SIGNAL_FLAG__KEYBOARD_PRESSED,
	UI_SIGNAL_FLAG__RELEASED = UI_SIGNAL_FLAG__LEFT_RELEASED,
	UI_SIGNAL_FLAG__CLICKED = UI_SIGNAL_FLAG__LEFT_CLICKED|UI_SIGNAL_FLAG__KEYBOARD_PRESSED,
	UI_SIGNAL_FLAG__DOUBLE_CLICKED = UI_SIGNAL_FLAG__LEFT_DOUBLE_CLICKED,
	UI_SIGNAL_FLAG__TRIPPLE_CLICKED = UI_SIGNAL_FLAG__LEFT_TRIPPLE_CLICKED,
	UI_SIGNAL_FLAG__DRAGGING = UI_SIGNAL_FLAG__LEFT_DRAGGING,
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
	UI_Key default_nav_root_key;
	L1 build_box_count;
	L1 last_build_box_count;
	OS_Window *window;
	F2 mouse;
	F2 drag_start_mouse;
	L1 last_time_mouse_moved;

	UI_Key hot_box_key;
	UI_Key active_box_key[OS_MOUSE_BUTTON_COUNT];

	UI_Cmd_List cmds;
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

Internal I1 txt_pt_match(Txt_Pt a, Txt_Pt b) {
	I1 result = (a.line == b.line && a.column == b.column);
	return result;
}

Internal I1 txt_pt_less_than(Txt_Pt a, Txt_Pt b) {
	I1 result = 0;
	if (a.line < b.line) {
		result = 1;
	} else if (a.line == b.line) {
		result = a.column < b.column;
	}
	return result;
}

Internal Txt_Pt txt_pt_min(Txt_Pt a, Txt_Pt b) {
	Txt_Pt result = b;
	if (txt_pt_less_than(a, b)) {
		result = a;
	}
	return result;
}

Internal Txt_Pt txt_pt_max(Txt_Pt a, Txt_Pt b) {
	Txt_Pt result = a;
	if (txt_pt_less_than(a, b)) {
		result = b;
	}
	return result;
}

Internal Txt_Range txt_range(Txt_Pt a, Txt_Pt b) {
	Txt_Range result = {txt_pt_min(a, b), txt_pt_max(a, b)};
	return result;
}

Internal UI_Key ui_key_zero(void) {
	UI_Key result = {0};
	return result;
}

Internal I1 ui_key_match(UI_Key a, UI_Key b) {
	I1 result = (a.l1[0] == b.l1[0]);
	return result;
}

Internal I1 ui_box_is_nil(UI_Box *box) {
	I1 result = (box == 0 || box == &ui_nil_box);
	return result;
}

Internal String8 ui_display_part_from_key_string(String8 key) {
	L1 end = key.len;

	for (L1 i = 0; i+1 < key.len; i += 1) {
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

Internal void ui_box_list_push(Arena *arena, UI_Box_List *list, UI_Box *box) {
	UI_Box_Node *n = push_array(arena, UI_Box_Node, 1);
	n->box = box;
	SLLQueuePush(list->first, list->last, n);
	list->count += 1;
}

Internal UI_State *ui_state_alloc(void) {
	Arena *arena = arena_alloc(MiB(64));
	UI_State *prev_state = ui_state;
	ui_state = push_array(arena, UI_State, 1);
	ui_state->arena = arena;
	ui_state->external_key = ui_key_from_string(ui_key_zero(), Str8_("external_interaction_key"));
	ui_state->build_arenas[0] = arena_alloc(MiB(64));
	ui_state->build_arenas[1] = arena_alloc(MiB(64));
	ui_state->drag_arena = arena_alloc(MiB(64));
	ui_state->box_table_size = 4096;
	ui_state->box_table = push_array(arena, UI_Box_HT_Slot, ui_state->box_table_size);
	UIInitStackNils();

	UI_State *new_state = ui_state;
	ui_state = prev_state;
	return new_state;
}

Internal void ui_state_release(UI_State *state) {
	if (ui_state == state) {
		ui_state = 0;
	}

	arena_release(state->build_arenas[0]);
	arena_release(state->build_arenas[1]);
	arena_release(state->drag_arena);
	arena_release(state->arena);
}

Internal void ui_state_equip(UI_State *state) {
	ui_state = state;
}

//- kti: Focus tree coloring.

Internal I1 ui_is_focus_active(void) {
	I1 result = (ui_top_focus_active() == UI_FOCUS_KIND__ON);
	if (result) {
		for (UI_Focus_Active_Node *n = ui_state->focus_active_stack.top; n != 0; n = n->next) {
			if (n->value == UI_FOCUS_KIND__ROOT) {
				break;
			}
			if (n->value == UI_FOCUS_KIND__OFF) {
				result = 0;
				break;
			}
		}
	}
	return result;
}


Internal I1 ui_is_focus_hot(void) {
	I1 result = (ui_top_focus_hot() == UI_FOCUS_KIND__ON);
	if (result) {
		for (UI_Focus_Hot_Node *n = ui_state->focus_hot_stack.top; n != 0; n = n->next) {
			if (n->value == UI_FOCUS_KIND__ROOT) {
				break;
			}
			if (n->value == UI_FOCUS_KIND__OFF) {
				result = 0;
				break;
			}
		}
	}
	return result;
}

//- kti: Implicit auto-managed tree-based focus state

Internal I1 ui_is_key_auto_focus_active(UI_Key key) {
	I1 result = 0;
	if (!ui_key_match(ui_key_zero(), key)) {
		for (UI_Box *p = ui_top_parent(); !ui_box_is_nil(p); p = p->parent) {
			if (p->flags & UI_BOX_FLAG__FOCUS_ACTIVE && ui_key_match(key, p->default_nav_focus_active_key)) {
				result = 1;
				break;
			}
		}
	}
	return result;
}

Internal I1 ui_is_key_auto_focus_hot(UI_Key key) {
	I1 result = 0;
	if (!ui_key_match(ui_key_zero(), key)) {
		for (UI_Box *p = ui_top_parent(); !ui_box_is_nil(p); p = p->parent) {
			if (p->flags & UI_BOX_FLAG__FOCUS_HOT &&
				((!(p->flags & UI_BOX_FLAG__FOCUS_HOT_DISABLED) && ui_key_match(key, p->default_nav_focus_hot_key)) ||
					ui_key_match(key, p->default_nav_focus_active_key))) {
				result = 1;
				break;
			}
		}
	}
	return result;
}

Internal void ui_set_auto_focus_active_key(UI_Key key) {
	for (UI_Box *p = ui_top_parent(); !ui_box_is_nil(p); p = p->parent) {
		if (p->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV) {
			p->default_nav_focus_next_active_key = key;
			break;
		}
	}
}

Internal void ui_set_auto_focus_hot_key(UI_Key key) {
	for (UI_Box *p = ui_top_parent(); !ui_box_is_nil(p); p = p->parent) {
		if (p->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV) {
			p->default_nav_focus_next_hot_key = key;
			break;
		}
	}
}

//- kti: Box building

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

	if (ui_is_focus_active() && (box->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV) && ui_key_match(ui_state->default_nav_root_key, ui_key_zero())) {
		ui_state->default_nav_root_key = box->key;
	}

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
		box->pref_size[AXIS__X] = ui_top_pref_width();
	}
	if (ui_state->fixed_height_stack.top != &ui_state->nil_fixed_height) {
		box->flags |= UI_BOX_FLAG__FIXED_HEIGHT;
		box->fixed_size[1] = ui_top_fixed_height();
	} else {
		box->pref_size[AXIS__Y] = ui_top_pref_height();
	}

	box->min_size[0] = ui_top_min_width();
	box->min_size[1] = ui_top_min_height();

	I1 is_auto_focus_active = ui_is_key_auto_focus_active(key);
	I1 is_auto_focus_hot = ui_is_key_auto_focus_hot(key);
	if (is_auto_focus_active) {
		ui_set_next_focus_active(UI_FOCUS_KIND__ON);
	}
	if (is_auto_focus_hot) {
		ui_set_next_focus_hot(UI_FOCUS_KIND__ON);
	}
	box->flags |= UI_BOX_FLAG__FOCUS_ACTIVE * (ui_top_focus_active() == UI_FOCUS_KIND__ON);
	box->flags |= UI_BOX_FLAG__FOCUS_HOT * (ui_top_focus_hot() == UI_FOCUS_KIND__ON);
	if (box->flags & UI_BOX_FLAG__FOCUS_HOT && !ui_is_focus_hot()) {
		box->flags |= UI_BOX_FLAG__FOCUS_HOT_DISABLED;
	}
	if (box->flags & UI_BOX_FLAG__FOCUS_ACTIVE && !ui_is_focus_active()) {
		box->flags |= UI_BOX_FLAG__FOCUS_ACTIVE_DISABLED;
	}

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

Internal UI_Cmd *ui_cmd_list_push(Arena *arena, UI_Cmd_List *list, UI_Cmd cmd) {
	UI_Cmd *out_cmd = push_array(arena, UI_Cmd, 1);
	memmove(out_cmd, &cmd, sizeof(UI_Cmd));
	out_cmd->string = push_str8_copy(arena, cmd.string);
	DLLPushBack(list->first, list->last, out_cmd);
	list->count += 1;
	return out_cmd;
}

Internal void ui_eat_cmd(UI_Cmd *cmd) {
	DLLRemove(ui_state->cmds.first, ui_state->cmds.last, cmd);
	ui_state->cmds.count -= 1;
}

Internal I1 ui_key_press(OS_Modifier_Flags modifiers, OS_Key key) {
	I1 result = 0;

	for (OS_Event *e = ui_state->events.first; e != 0; e = e->next) {
		if (e->kind == OS_EVENT_KIND__PRESS && e->key == key && e->modifiers == modifiers) {
			result = 1;
			ui_eat_event(e);
			break;
		}
	}

	return result;
}

Internal I1 ui_key_release(OS_Modifier_Flags modifiers, OS_Key key) {
	I1 result = 0;

	for (OS_Event *e = ui_state->events.first; e != 0; e = e->next) {
		if (e->kind == OS_EVENT_KIND__RELEASE && e->key == key && e->modifiers == modifiers) {
			result = 1;
			ui_eat_event(e);
			break;
		}
	}

	return result;
}

Internal UI_Signal ui_signal_from_box(UI_Box *box) {
	// TODO: Get double click time from os.
	L1 double_click_time = 500000000;

	I1 is_focus_hot = box->flags & UI_BOX_FLAG__FOCUS_HOT && !(box->flags & UI_BOX_FLAG__FOCUS_HOT_DISABLED);
	UI_Signal signal = {0};
	signal.box = box;
	signal.modifiers |= os_get_modifiers();

	F4 rect = box->rect;
	for (UI_Box *p = box->parent; !ui_box_is_nil(p); p = p->parent) {
		if (!ui_key_match(p->key, ui_key_zero()) && p->flags & UI_BOX_FLAG__CLIP) {
			rect = rect_overlap(p->rect, rect);
		}
	}

	I1 view_scrolled = 0;
	for (OS_Event *e = 0; ui_next_event(&e);) {
		I1 taken = 0;

		signal.modifiers |= e->modifiers;

		F2 evt_mouse = {floor_F1(e->x), floor_F1(e->y)};
		I1 evt_mouse_in_bounds = rect_contains(rect, evt_mouse); 
		I1 evt_key_is_mouse = (e->key == OS_MOUSE_BUTTON__LEFT ||
				e->key == OS_MOUSE_BUTTON__MIDDLE ||
				e->key == OS_MOUSE_BUTTON__RIGHT);
		I1 evt_mouse_idx = evt_key_is_mouse ?  e->key - OS_MOUSE_BUTTON__LEFT : 0;

		//- kti: Mouse down in bounds.
		if (box->flags & UI_BOX_FLAG__CLICKABLE &&
				e->kind == OS_EVENT_KIND__PRESS &&
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
				e->kind == OS_EVENT_KIND__RELEASE &&
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
				e->kind == OS_EVENT_KIND__RELEASE &&
				evt_key_is_mouse &&
				!evt_mouse_in_bounds &&
				ui_key_match(ui_state->active_box_key[evt_mouse_idx], box->key)) {
			ui_state->hot_box_key = ui_key_zero();
			ui_state->active_box_key[evt_mouse_idx] = ui_key_zero();
			signal.flags |= UI_SIGNAL_FLAG__LEFT_RELEASED << evt_mouse_idx;
			taken = 1;
		}

		//- kti: focus is hot & keyboard click -> mark signal
		// TODO: Cmd instead of hard coding enter?
		if (box->flags & UI_BOX_FLAG__KEYBOARD_CLICKABLE &&
			is_focus_hot && e->kind == OS_EVENT_KIND__PRESS && e->key == OS_KEY__ENTER) {
			signal.flags |= UI_SIGNAL_FLAG__KEYBOARD_PRESSED;
			taken = 1;
		}

		//- kti: Scrolling
		F1 scroll_mult = 4.0f;

		if (box->flags & UI_BOX_FLAG__SCROLL &&
				e->kind == OS_EVENT_KIND__SCROLL && 
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
				e->kind == OS_EVENT_KIND__SCROLL &&
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
			Max(0, box->view_bounds[0] - box->fixed_size[0]),
			Max(0, box->view_bounds[1] - box->fixed_size[1]),
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

	//- kti: get default nav ancestor
	UI_Box *default_nav_parent = &ui_nil_box;
	for (UI_Box *p = ui_top_parent(); !ui_box_is_nil(p); p = p->parent) {
		if (p->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV) {
			default_nav_parent = p;
			break;
		}
	}

	//- kti: clicking in default nav -> set navigation state to this box
	if (box->flags & UI_BOX_FLAG__CLICK_TO_FOCUS && signal.flags&UI_SIGNAL_FLAG__PRESSED && !ui_box_is_nil(default_nav_parent)) {
		default_nav_parent->default_nav_focus_next_hot_key = box->key;
		if (!ui_key_match(default_nav_parent->default_nav_focus_active_key, box->key)) {
			default_nav_parent->default_nav_focus_next_active_key = ui_key_zero();
		}
	}

	return signal;
}

Internal void ui_begin_build(OS_Window *window, OS_Event_List events, UI_Cmd_List cmds) {
	UIResetStacks();
	ui_state->root = &ui_nil_box;
	ui_state->last_build_box_count = ui_state->build_box_count;
	ui_state->build_box_count = 0;
	ui_state->window = window;
	ui_state->events = events;

	ui_state->cmds = cmds;

	//- kti: Mouse movement.
	for (OS_Event *e = ui_state->events.last; e != 0; e = e->prev) {
		if (e->kind == OS_EVENT_KIND__MOUSE_MOVE) {
			ui_state->mouse[0] = floor_F1(e->x);
			ui_state->mouse[1] = floor_F1(e->y);
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

	//- kti: default navigation
	Temp_Arena scratch = scratch_begin(0, 0);
	if (!ui_key_match(ui_state->default_nav_root_key, ui_key_zero())) {
		UI_Box *nav_root = ui_box_from_key(ui_state->default_nav_root_key);
		if (!ui_box_is_nil(nav_root)) {
			if (ui_key_match(ui_key_zero(), nav_root->default_nav_focus_active_key)) {
				for (;;) {
					I1 moved = 0;
					UI_Box *focus_box = ui_box_from_key(nav_root->default_nav_focus_next_hot_key);
					UI_Box_List next_focus_box_candidates = {0};

					I1 nav_next = 0;
					I1 nav_prev = 0;
					Axis axis_lock = AXIS__INVALID;

					if (ui_key_press(0, OS_KEY__TAB)) {
						nav_next = 1;
					}
					if (ui_key_press(OS_MODIFIER_FLAG__SHIFT, OS_KEY__TAB)) {
						nav_prev = 1;
					}

					for (UI_Cmd *cmd = ui_state->cmds.first, *next = 0; cmd != 0; cmd = next) {
						next = cmd->next;
						I1 taken = 0;
						if (cmd->delta_si2[0] == 0 && cmd->delta_si2[1] == 0) {
							continue;
						}
						if (((cmd->delta_si2[0] > 0 && nav_root->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV_X) || cmd->delta_si2[0] == 0) &&
								((cmd->delta_si2[1] > 0 && nav_root->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV_Y) || cmd->delta_si2[1] == 0)) {
							taken = 1;
							nav_next = 1;
						}
						if (((cmd->delta_si2[0] < 0 && nav_root->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV_X) || cmd->delta_si2[0] == 0) &&
								((cmd->delta_si2[1] < 0 && nav_root->flags & UI_BOX_FLAG__DEFAULT_FOCUS_NAV_Y) || cmd->delta_si2[1] == 0)) {
							taken = 1;
							nav_prev = 1;
						}
						if (cmd->flags & UI_CMD_FLAG__EXPLICIT_DIRECTIONAL) {
							axis_lock = cmd->delta_si2[0] != 0 ? AXIS__X : AXIS__Y;
						}
						if (taken) {
							ui_eat_cmd(cmd);
						}
					}

					if (nav_next) {
						UI_Box *search_start = ui_box_is_nil(focus_box) ? nav_root : focus_box;
						L1 moved_in_axis[AXIS_COUNT] = {0};
						moved = 1;
						for (UI_Box *box = search_start;;) {
							if (box != search_start && !(box->flags & UI_BOX_FLAG__FOCUS_NAV_SKIP) &&
								(box->flags & UI_BOX_FLAG__CLICKABLE || ui_box_is_nil(box)) &&
								(axis_lock == AXIS__INVALID || moved_in_axis[axis_lock] > 0)) {
								ui_box_list_push(scratch.arena, &next_focus_box_candidates, box);
								if (axis_lock == AXIS__INVALID || moved_in_axis[axis_lock] > 1) {
									break;
								}
							}

							UI_Box *last_box = box;
							if (!ui_box_is_nil(box->first)) {
								moved_in_axis[box->child_layout_axis] += 1;
								box = box->first;
							} else for (UI_Box *p = box; !ui_box_is_nil(p) && p != nav_root; p = p->parent) {
								if (!ui_box_is_nil(p->next)) {
									moved_in_axis[p->parent->child_layout_axis] += 1;
									box = p->next;
									break;
								}
							}
							if (last_box == box) {
								ui_box_list_push(scratch.arena, &next_focus_box_candidates, &ui_nil_box);
								break;
							}
						}
					}

					if (nav_prev) {
						UI_Box *search_start = ui_box_is_nil(focus_box) ? nav_root : focus_box;
						L1 moved_in_axis[AXIS_COUNT] = {0};
						moved = 1;
						for (UI_Box *box = search_start;;) {
							if (box != search_start && !(box->flags & UI_BOX_FLAG__FOCUS_NAV_SKIP) &&
								(box->flags & UI_BOX_FLAG__CLICKABLE || ui_box_is_nil(box)) &&
								(axis_lock == AXIS__INVALID || moved_in_axis[axis_lock] > 0)) {
								ui_box_list_push(scratch.arena, &next_focus_box_candidates, box);
								if (axis_lock == AXIS__INVALID || moved_in_axis[axis_lock] > 1) {
									break;
								}
							}
							UI_Box *last_box = box;
							UI_Box *root_descendant = &ui_nil_box;
							if (box == nav_root && box == search_start) {
								for (UI_Box *d = box->last; !ui_box_is_nil(d); d = d->last) {
									moved_in_axis[d->parent->child_layout_axis] += 1;
									root_descendant = d;
								}
							}
							UI_Box *prev_descendant = &ui_nil_box;
							for (UI_Box *d = box->prev; !ui_box_is_nil(d); d = d->last) {
								moved_in_axis[d->parent->child_layout_axis] += 1;
								prev_descendant = d;
							}
							if (!ui_box_is_nil(root_descendant)) {
								box = root_descendant;
							} else if (!ui_box_is_nil(prev_descendant)) {
								box = prev_descendant;
							} else if (box->parent != nav_root) {	
								moved_in_axis[box->parent->child_layout_axis] += 1;
								box = box->parent;
							}
							if (box == last_box) {
								ui_box_list_push(scratch.arena, &next_focus_box_candidates, &ui_nil_box);
								break;
							}
						}
					}

					//- kti: scan candidates and grab next focus box.
					UI_Box *next_focus_box = focus_box;
					F1 best_distance_from_start = 1000000;
					for (UI_Box_Node *n = next_focus_box_candidates.first; n != 0; n = n->next) {
						UI_Box *box = n->box;
						F1 distance_from_start = 0;
						if (axis_lock != AXIS__INVALID) {
							distance_from_start = abs_F1(rect_center(box->rect)[axis_flip(axis_lock)] - rect_center(focus_box->rect)[axis_flip(axis_lock)]);
						}
						if (distance_from_start < best_distance_from_start && box != focus_box) {
							next_focus_box = box;
							best_distance_from_start = distance_from_start;
						}
					}

					//- kti: commit next focus box.
					nav_root->default_nav_focus_next_hot_key = next_focus_box->key;

					//- kti: no movement -> break.
					if (moved == 0) {
						break;
					}
				}
			}

			if (!ui_key_match(ui_key_zero(), nav_root->default_nav_focus_active_key)) {
				for (UI_Cmd *cmd = ui_state->cmds.first, *next = 0; cmd != 0; cmd = next) {
					next = cmd->next;
					if (cmd->kind == UI_CMD_KIND__CANCEL) {
						ui_eat_cmd(cmd);
						
						UI_Box *prev_focus_root = nav_root;
						for(UI_Box *focus_root = ui_box_from_key(nav_root->default_nav_focus_active_key); !ui_box_is_nil(focus_root);) {
							UI_Box *next_focus_root = ui_box_from_key(focus_root->default_nav_focus_active_key);
							if(ui_box_is_nil(next_focus_root)) {
								prev_focus_root->default_nav_focus_next_active_key = ui_key_zero();
								break;
							} else {
								prev_focus_root = focus_root;
								focus_root = next_focus_root;
							}
						}
					}
				}
			}
		}
	}
	ui_state->default_nav_root_key = ui_key_zero();
	scratch_end(scratch);

	//- kti: next-default-nav-focus keys -> current-default-nav-focus-keys
	for (L1 slot_idx = 0; slot_idx < ui_state->box_table_size; slot_idx += 1) {
		for (UI_Box *box = ui_state->box_table[slot_idx].first; !ui_box_is_nil(box); box = box->hash_next) {
			box->default_nav_focus_hot_key = box->default_nav_focus_next_hot_key;
			box->default_nav_focus_active_key = box->default_nav_focus_next_active_key;
		}
	}

	//- kti: Setup root.
	ui_set_next_fixed_width(window->width);
	ui_set_next_fixed_height(window->height);
	ui_set_next_child_layout_axis(AXIS__X);
	UI_Box *root = ui_build_box_from_stringf(0, "%llu", (L1)window);
	ui_push_parent(root);
	ui_state->root = root;
	
	//- kti: Reset active key when the box is disabled or nil.
	for (L1 k = 0; k < OS_MOUSE_BUTTON_COUNT; k += 1) {
		if (!ui_key_match(ui_state->active_box_key[k], ui_key_zero())) {
			UI_Box *box = ui_box_from_key(ui_state->active_box_key[k]);
			if (ui_box_is_nil(box) ||
					!(box->flags & UI_BOX_FLAG__CLICKABLE) ||
					box->flags & UI_BOX_FLAG__DISABLED) {
				ui_state->active_box_key[k] = ui_key_zero();
			}
		}
	}

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

Internal void ui_calc_sizes_standalone__in_place(UI_Box *root, Axis axis) {
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

Internal void ui_calc_sizes_upwards_dependent__in_place(UI_Box *root, Axis axis) {
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

Internal void ui_calc_sizes_downwards_dependent__in_place(UI_Box *root, Axis axis) {
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

Internal void ui_layout_enforce_constraints__in_place(UI_Box *root, Axis axis) {
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
						fixup_size_this_child = Max(0, fixup_size_this_child);

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
			child->fixed_size[axis] = Max(child->min_size[axis], child->fixed_size[axis]);
		}
	}

	scratch_end(scratch);
}

Internal void ui_layout_position__in_place(UI_Box *root, Axis axis) {
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

Internal void ui_layout_root(UI_Box *root, Axis axis) {
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

	for EachIndex(axis, AXIS_COUNT) {
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
	FP_Metrics font_metrics = fc_metrics_from_tag_size(font, font_size);
	result[1] = floor_F1((box->rect[1]+box->rect[3]*0.5f) + font_metrics.ascent*0.5f - font_metrics.descent*0.5f);
	switch (box->text_align) {
		default:
		case UI_TEXT_ALIGN__LEFT: {
			result[0] = box->rect[0] + box->text_padding;
		} break;
		case UI_TEXT_ALIGN__CENTER: {
			F2 text_dim = box->display_fruns.dim;
			result[0] = round_F1(box->rect[0]+box->rect[2]*0.5f - text_dim[0]*0.5f);
			result[0] = Max(result[0], box->rect[0]);
		} break;
		case UI_TEXT_ALIGN__RIGHT: {
			F2 text_dim = box->display_fruns.dim;
			result[0] = round_F1(box->rect[0]+box->rect[2] - text_dim[0] - box->text_padding);
			result[0] = Max(result[0], box->rect[0]);
		} break;
	}
	result[0] = floor_F1(result[0]);
	return result;
}

Internal L1 ui_box_char_pos_from_xy(UI_Box *box, F2 pos) {
	String8 line = ui_box_display_string(box);
	L1 result = fc_char_pos_from_tag_size_string_p(box->font, box->font_size, 0, box->tab_size, line, pos[0] - ui_box_text_pos(box)[0]);
	return result;
}

Internal void ui_kill_action(void) {
	for EachIndex(k, OS_MOUSE_BUTTON_COUNT) {
		ui_state->active_box_key[k] = ui_key_zero();
	}
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

#define UI_Corner_Radius(v) DeferLoop(ui_push_corner_radius((v)), ui_pop_corner_radius())
#define UI_Focus(v) DeferLoop((ui_push_focus_hot(v), ui_push_focus_active(v)), (ui_pop_focus_active(), ui_pop_focus_hot()))

Internal void ui_set_next_fixed_rect(F4 rect) {
	ui_set_next_fixed_x(rect[0]);
	ui_set_next_fixed_y(rect[1]);
	ui_set_next_fixed_width(rect[2]);
	ui_set_next_fixed_height(rect[3]);
}

Internal void ui_push_pref_size(Axis axis, UI_Size size) {
	(axis == AXIS__X ? ui_push_pref_width : ui_push_pref_height)(size);
}

Internal void ui_pop_pref_size(Axis axis) {
	(axis == AXIS__X ? ui_pop_pref_width : ui_pop_pref_height)();
}

Internal void ui_set_next_pref_size(Axis axis, UI_Size size) {
	(axis == AXIS__X ? ui_set_next_pref_width : ui_set_next_pref_height)(size);
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
	ui_set_next_child_layout_axis(AXIS__Y);
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
	ui_set_next_child_layout_axis(AXIS__X);
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

// TODO(kti): Look at alignment.
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
				UI_Box *box = ui_build_box_from_stringf(UI_BOX_FLAG__CLICKABLE|UI_BOX_FLAG__DRAW_BORDER, "slider_%.*s", (int)str.len, str.str);
				UI_Key bg_key = ui_key_from_string(box->key, Str8_("bg"));
				UI_Key knob_key = ui_key_from_string(box->key, Str8_("knob"));

				box->group_key = box->key;

				UI_Group_Key(box->group_key)
				UI_Parent(box) {
					F1 knob_inset = floor_F1((height-knob_size)*0.5f);
					F1 min = knob_size+knob_inset*2.0f;
					F1 w = floor_F1(min+(box->rect[2]-min)*pct);
					ui_set_next_background_color(oklch(0.195f, 0.100f, 17.0f, 1.0f));
					ui_set_next_pref_width(ui_px(w, 1.0f));
					ui_build_box_from_key(
						UI_BOX_FLAG__DRAW_BACKGROUND|
						UI_BOX_FLAG__DRAW_BORDER|
						UI_BOX_FLAG__DRAW_HOT_EFFECTS|
						UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS,
						bg_key);
					UI_Box *bg_box = ui_box_from_key(bg_key);
					UI_Parent(bg_box) {
						ui_spacer(ui_px(knob_inset, 1.0f));
						ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
						ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
						UI_Row() {
							ui_spacer(ui_pct(1.0f, 0.0f));

							UI_Pref_Width(ui_px(knob_size, 1.0f))
							UI_Pref_Height(ui_pct(1.0f, 0.0f))
							UI_Column()
							UI_Padding(ui_pct(1.0f, 0.0f))
							UI_Background_Color(oklch(0.7f, 0.0f, 0.0f, 1.0f))
							UI_Pref_Width(ui_px(knob_size, 1.0f))
							UI_Pref_Height(ui_px(knob_size, 1.0f))
							UI_Corner_Radius(knob_size*0.5f-2.0f) {
								ui_build_box_from_key(
									UI_BOX_FLAG__DRAW_BACKGROUND|
									UI_BOX_FLAG__DRAW_HOT_EFFECTS|
									UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS,
									knob_key);
							}

							ui_spacer(ui_px(knob_inset, 1.0f));
						}
					}
				}

				//- kti: Signal handling
				signal = ui_signal_from_box(box);

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

	FP_Metrics metrics = fc_metrics_from_tag_size(ui_top_font(), ui_top_font_size());
	F1 size = metrics.ascent + metrics.descent;
	F1 check_inset = size * 0.15f;

	ui_set_next_pref_height(ui_px(size, 1.0f));
	UI_Row() {
		//- kti: Checkbox square. outer is Y-layout so inner check can be centered with padding.
		ui_set_next_fixed_width(size);
		ui_set_next_fixed_height(size);
		ui_set_next_child_layout_axis(AXIS__Y);
		UI_Corner_Radius(check_inset * 0.5f) {
			ui_set_next_background_color(oklch(0.195f, 0.1f, 17.0f, 1.0f));
			ui_set_next_border_color(oklch(0.5f, 0.0f, 0.0f, 1.0f));
			UI_Box *outer = ui_build_box_from_stringf(
					UI_BOX_FLAG__CLICKABLE|
					UI_BOX_FLAG__DRAW_HOT_EFFECTS|
					UI_BOX_FLAG__DRAW_BACKGROUND|
					UI_BOX_FLAG__DRAW_BORDER,
					"##checkbox_%.*s", (int)str.len, str.str);
			UI_Box *check = &ui_nil_box;

			if (value[0]) {
				UI_Parent(outer)
				UI_Background_Color(oklch(0.7f, 0.0f, 0.0f, 1.0f))
				UI_Padding(ui_pct(1.0f, 0.0f)) {
					UI_Pref_Height(ui_px(size-check_inset*2, 1.0f))
					UI_Row()
					UI_Padding(ui_pct(1.0f, 0.0f))
					UI_Pref_Width(ui_px(size-check_inset*2, 1.0f)) {
						check = ui_build_box_from_key(UI_BOX_FLAG__DRAW_BACKGROUND, ui_key_zero());
					}
				}
			}

			signal = ui_signal_from_box(outer);
			if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
				value[0] = !value[0];
			}
			if (!ui_box_is_nil(check) && signal.flags & UI_SIGNAL_FLAG__HOVERING) {
				check->background_color[0] = Min(1.0f, check->background_color[0]+0.1f);
			}
		}

		ui_spacer(ui_px(size * 0.5f, 1.0f));

		//- kti: Label. Text is vertically centered by ui_box_text_pos.
		UI_Pref_Width(ui_text_dim(0.0f, 1.0f)) {
			ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
			ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str);
			ui_spacer(ui_em(1.0f, 1.0f));
		}
	}

	return signal;
}

Internal UI_Signal ui_label(String8 string) {
  UI_Box *box = ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, (String8){0});
  ui_box_equip_display_string(box, string);
  UI_Signal interact = ui_signal_from_box(box);
  return interact;
}

Internal L1 ui_scanned_column_from_column(String8 string, L1 start_column, Side side) {
	L1 new_column = start_column;
	SL1 delta = (!!side)*2 - 1;
	I1 found_text = 0;
	I1 found_non_space = 0;
	SL1 start_off = delta < 0 ? delta : 0;
	for (SL1 col = (SL1)start_column+start_off; 0 <= col && col <= (SL1)string.len; col += delta) {
		B1 byte = (col < (SL1)string.len) ? string.str[col] : 0;
		I1 is_non_space = !char_is_space(byte);
		I1 is_name = (char_is_alpha(byte) || char_is_digit(byte, 10) || byte == '_');
		if (((side == SIDE__MIN) && (col == 0)) ||
				((side == SIDE__MAX) && (col == (SL1)string.len)) ||
				(found_non_space && !is_non_space) ||
				(found_text && !is_name)) {
			new_column = col + (!side && col != 0);
			break;
		} else if (!found_text && is_name) {
			found_text = 1;
		} else if (!found_non_space && is_non_space) {
			found_non_space = 1;
		}
	}
	return new_column;
}

Internal String8 ui_push_string_replace_range(Arena *arena, String8 string, L1 min, L1 max, String8 replace) {
	if (min > string.len) {
		min = 0;
	}
	if (max > string.len) {
		max = string.len;
	}

	L1 old_len = string.len;
	L1 new_len = old_len - (max-min) + replace.len;

	B1 *push_base = push_array(arena, B1, new_len);
	memmove(push_base, string.str, min);
	memmove(push_base+min+replace.len, string.str+max, string.len-max);
	if (replace.str != 0) {
		memmove(push_base+min, replace.str, replace.len);
	}

	String8 result = {push_base, new_len};
	return result;
}

Internal UI_Txt_Op ui_single_line_txt_op_from_cmd(Arena *arena, UI_Cmd *cmd, String8 string, Txt_Pt cursor, Txt_Pt mark) {
	Txt_Pt next_cursor = cursor;
	Txt_Pt next_mark = mark;
	Txt_Range range = {0};
	String8 replace = {0};
	String8 copy = {0};
	UI_Txt_Op_Flags flags = 0;
	SI2 delta = cmd->delta_si2;
	SI2 original_delta = delta;

	switch (cmd->delta_unit) {
		default: {} break;
		case UI_CMD_DELTA_UNIT__CHAR: {
			// TODO: Multi byte characters in UTF-8.
		} break;
		case UI_CMD_DELTA_UNIT__WORD: {
			delta[0] = (SL1)ui_scanned_column_from_column(string, cursor.column, delta[0] > 0 ? SIDE__MAX : SIDE__MIN) - (SL1)cursor.column;
		} break;
		case UI_CMD_DELTA_UNIT__LINE:
		case UI_CMD_DELTA_UNIT__WHOLE:
		case UI_CMD_DELTA_UNIT__PAGE: {
			L1 first_nonwhitespace_column = 0;
			for (L1 idx = 0; idx < string.len; idx += 1) {
				if (!char_is_space(string.str[idx])) {
					first_nonwhitespace_column = idx;
					break;
				}
			}
			L1 home_dest_column = (cursor.column == first_nonwhitespace_column) ? 0 : first_nonwhitespace_column;
			delta[0] = (SL1)((delta[0] > 0) ? string.len : home_dest_column) - (SL1)cursor.column;
		} break;
	}

	if (!txt_pt_match(cursor, mark) && cmd->flags & UI_CMD_FLAG__ZERO_DELTA_ON_SELECT) {
		delta = (SI2){0};
	}

	if (txt_pt_match(cursor, mark) || !(cmd->flags & UI_CMD_FLAG__ZERO_DELTA_ON_SELECT)) {
		SL1 next_column = (SL1)next_cursor.column + delta[0];
		next_cursor.column = (L1)Max(0, next_column);
	}

	if (cmd->flags & UI_CMD_FLAG__CAP_AT_LINE) {
		next_cursor.column = Clamp(0, next_cursor.column, string.len);
	}

	if (!txt_pt_match(cursor, mark) && cmd->flags & UI_CMD_FLAG__PICK_SELECT_SIDE) {
		if (original_delta[0] < 0 || original_delta[1] < 0) {
			next_cursor = next_mark = txt_pt_min(cursor, mark);
		} else if (original_delta[0] > 0 || original_delta[1] > 0) {
			next_cursor = next_mark = txt_pt_max(cursor, mark);
		}
	}

	if (cmd->flags & UI_CMD_FLAG__COPY) {
		if (cursor.line == mark.line) {
			copy = str8_substr(string, cursor.column, mark.column);
			flags |= UI_TXT_OP_FLAG__COPY;
		} else {
			flags |= UI_TXT_OP_FLAG__INVALID;
		}
	}

	if (cmd->flags & UI_CMD_FLAG__PASTE) {
		range = txt_range(cursor, mark);
		// TODO: replace = <Get clipboard text>
		next_cursor = next_mark = (Txt_Pt){range.min.line, range.min.column+replace.len};
	}

	if (cmd->flags & UI_CMD_FLAG__DELETE) {
		Txt_Pt new_pos = txt_pt_min(next_cursor, next_mark);
		range = txt_range(next_cursor, next_mark);
		replace = Str8_("");
		next_cursor = next_mark = new_pos;
	}

	if (!(cmd->flags & UI_CMD_FLAG__KEEP_MARK)) {
		next_mark = next_cursor;
	}

	if (cmd->string.len != 0) {
		range = txt_range(cursor, mark);
		replace = push_str8_copy(arena, cmd->string);
		next_cursor = next_mark = (Txt_Pt){range.min.line, range.min.column + cmd->string.len};
	}

	if (next_cursor.column > string.len || cmd->delta_si2[1] != 0) {
		flags |= UI_TXT_OP_FLAG__INVALID;
	}
	next_cursor.column = Clamp(0, next_cursor.column, string.len+replace.len);
	next_mark.column = Clamp(0, next_mark.column, string.len+replace.len);

	UI_Txt_Op op = {0};
	op.flags = flags;
	op.replace = replace;
	op.copy = copy;
	op.range = range;
	op.cursor = next_cursor;
	op.mark = next_mark;
	return op;
}

Internal UI_Signal ui_textedit(Txt_Pt *cursor, Txt_Pt *mark, B1 *edit_buffer, L1 edit_buffer_size, L1 *edit_string_size_out, String8 pre_edit_value, String8 string) {
	//- kti: Makbox->e key.
	UI_Key key = ui_key_from_string(ui_active_seed_key(), string);

	//- kti: Calculate focus.
	I1 is_auto_focus_hot = ui_is_key_auto_focus_hot(key);
	I1 is_auto_focus_active = ui_is_key_auto_focus_active(key);
	ui_push_focus_hot(is_auto_focus_hot ? UI_FOCUS_KIND__ON : UI_FOCUS_KIND__NULL);
	ui_push_focus_active(is_auto_focus_active ? UI_FOCUS_KIND__ON : UI_FOCUS_KIND__NULL);
	I1 is_focus_hot = ui_is_focus_hot();
	I1 is_focus_active = ui_is_focus_active();
	I1 is_focus_hot_disabled = (!is_focus_hot && ui_top_focus_hot() == UI_FOCUS_KIND__ON);
	I1 is_focus_active_disabled = (!is_focus_active && ui_top_focus_active() == UI_FOCUS_KIND__ON);

	//- kti: Build top level box.
	ui_set_next_border_color(oklch(0.7f, 0.0f, 0.0f, 1.0f));
	UI_Box *box = ui_build_box_from_key(UI_BOX_FLAG__DRAW_BACKGROUND |
																			UI_BOX_FLAG__DRAW_BORDER |
																			UI_BOX_FLAG__MOUSE_CLICKABLE |
																			UI_BOX_FLAG__CLICK_TO_FOCUS |
																		  ((is_auto_focus_hot || is_auto_focus_active)*UI_BOX_FLAG__KEYBOARD_CLICKABLE) |
																			UI_BOX_FLAG__DRAW_HOT_EFFECTS |
																		  (is_focus_active || is_focus_active_disabled)*(UI_BOX_FLAG__CLIP | UI_BOX_FLAG__ALLOW_OVERFLOW_X | UI_BOX_FLAG__VIEW_CLAMP),
																			key);

	//- kti: handle text manipulation.
	if (is_focus_active) {
		Temp_Arena scratch = scratch_begin(0, 0);
		for (UI_Cmd *cmd = ui_state->cmds.first, *next; cmd != 0; cmd = next) {
			next = cmd->next;

			String8 edit_string = (String8){.str = edit_buffer, edit_string_size_out[0]};

			//- kti: Skip non single-line operations.
			if ((cmd->kind != UI_CMD_KIND__EDIT && cmd->kind != UI_CMD_KIND__NAVIGATE && cmd->kind != UI_CMD_KIND__TEXT) || cmd->delta_si2[1] != 0) {
				continue;
			}

			UI_Txt_Op op = ui_single_line_txt_op_from_cmd(scratch.arena, cmd, edit_string, cursor[0], mark[0]);

			if (!txt_pt_match(op.range.min, op.range.max) || op.replace.len != 0) {
				String8 new_string = ui_push_string_replace_range(scratch.arena, edit_string, op.range.min.column, op.range.max.column, op.replace);
				new_string.len = Min(edit_buffer_size, new_string.len);
				memmove(edit_buffer, new_string.str, new_string.len);
				edit_string_size_out[0] = new_string.len;
			}

			if (op.flags & UI_TXT_OP_FLAG__COPY) {
				// TODO: Copy
			}

			cursor[0] = op.cursor;
			mark[0] = op.mark;

			ui_eat_cmd(cmd);
		}
		scratch_end(scratch);
	}

	//- kti: build contents
	Txt_Pt mouse_pt = {0};
	F1 cursor_off = 0;
	UI_Parent(box) {
		String8 edit_string = (String8){.str = edit_buffer, edit_string_size_out[0]};
		if (!is_focus_active && !is_focus_active_disabled) {
			String8 display_string = ui_display_part_from_key_string(string);
			if (pre_edit_value.len != 0) {
				display_string = pre_edit_value;
			}
			ui_label(display_string);
		} else {
			F1 total_text_width = fc_dim_from_tag_size_string(ui_top_font(), ui_top_font_size(), 0, ui_top_tab_size(), edit_string)[0];
			ui_set_next_pref_width(ui_px(total_text_width+ui_top_font_size()*5, 1.0f));
			UI_Box *editstr_box = ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT | UI_BOX_FLAG__DISABLE_TEXT_TRUNC, Str8_("###editstr"));
			ui_box_equip_display_string(editstr_box, edit_string);
			// TODO: custom draw.
			mouse_pt = (Txt_Pt){0, ui_box_char_pos_from_xy(editstr_box, ui_mouse())};
			cursor_off = fc_dim_from_tag_size_string(ui_top_font(), ui_top_font_size(), 0, ui_top_tab_size(), str8_prefix(edit_string, cursor->column))[0];
		}
	}

	//- kti: Interact
	UI_Signal signal = ui_signal_from_box(box);
	if (!is_focus_active && signal.flags&(UI_SIGNAL_FLAG__DOUBLE_CLICKED|UI_SIGNAL_FLAG__KEYBOARD_PRESSED)) {
		String8 edit_string = pre_edit_value;
		edit_string.len = Min(edit_buffer_size, pre_edit_value.len);
		memmove(edit_buffer, edit_string.str, edit_string.len);
		edit_string_size_out[0] = edit_string.len;
		ui_set_auto_focus_active_key(key);
		ui_kill_action();
		cursor[0] = (Txt_Pt){0, edit_string.len};
		mark[0] = (Txt_Pt){0, 0};
	}
	if (is_focus_active && signal.flags&UI_SIGNAL_FLAG__KEYBOARD_PRESSED) {
		ui_set_auto_focus_active_key(ui_key_zero());
		signal.flags |= UI_SIGNAL_FLAG__COMMIT;
	}
	if (is_focus_active && signal.flags&UI_SIGNAL_FLAG__DRAGGING) {
		if (signal.flags&UI_SIGNAL_FLAG__PRESSED) {
			mark[0] = mouse_pt;
		}
		cursor[0] = mouse_pt;
	}

	//- kti: Focus cursor.
	// if (is_focus_active && box->rect[2] > 0) {
		F2 cursor_range_px = {cursor_off-ui_top_font_size()*2.0f, cursor_off+ui_top_font_size()*2.0f};
		F2 visible_range_px = {box->view_off_target[0], box->view_off_target[0]+box->rect[2]};
		cursor_range_px[0] = Max(0, cursor_range_px[0]);
		cursor_range_px[1] = Max(0, cursor_range_px[1]);
		F1 min_delta = Min(0, cursor_range_px[0]-visible_range_px[0]);
		F1 max_delta = Max(0, cursor_range_px[1]-visible_range_px[1]);
		box->view_off_target[0] += min_delta;
		box->view_off_target[0] += max_delta;
	// }

	ui_pop_focus_hot();
	ui_pop_focus_active();

	return signal;
}

Internal void ui_draw(void) {
	for (UI_Box *box = ui_root(); !ui_box_is_nil(box);) {
		UI_Box_Rec rec = ui_box_rec_df_post(box, &ui_nil_box);
		UI_Box *hot_box = ui_box_from_key(ui_hot_key());
		UI_Box *active_box = ui_box_from_key(ui_active_key(OS_MOUSE_BUTTON__LEFT));

		// F1 softness = (box->corner_radii[0] > 0 || box->corner_radii[1] > 0 || box->corner_radii[2] > 0 || box->corner_radii[3] > 0) ? 1.0f : 0.0f;
		F1 softness = 1.0f;

		I1 hot_by_key = !ui_key_match(box->key, ui_key_zero()) && ui_key_match(box->key, ui_hot_key());
		I1 active_by_key = !ui_key_match(box->key, ui_key_zero()) && ui_key_match(box->key, ui_active_key(OS_MOUSE_BUTTON__LEFT));
		I1 hot_by_group = (!ui_key_match(box->group_key, ui_key_zero()) && !ui_box_is_nil(hot_box) && ui_key_match(box->group_key, hot_box->group_key));
		I1 active_by_group = (!ui_key_match(box->group_key, ui_key_zero()) && !ui_box_is_nil(active_box) && ui_key_match(box->group_key, active_box->group_key));
		I1 draw_active_effect = (box->flags & UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS && (active_by_key || active_by_group));
		I1 draw_hot_effect = (box->flags & UI_BOX_FLAG__DRAW_HOT_EFFECTS && (hot_by_key || hot_by_group));

		if (box->flags & UI_BOX_FLAG__DRAW_DROP_SHADOW) {
			F4 shadow = {
				box->rect[0] - 4,
				box->rect[1] - 4,
				box->rect[2] + 16,
				box->rect[3] + 16,
			};
			dr_rect(shadow, oklch(0, 0, 0, 1), 0.8f, 8.0f);
		}

		if (box->flags & UI_BOX_FLAG__DRAW_BACKGROUND) {
			if (draw_hot_effect && !draw_active_effect) {
				F4 shadow = {
					box->rect[0] - 4,
					box->rect[1] - 4,
					box->rect[2] + 16,
					box->rect[3] + 16,
				};
				dr_rect(shadow, oklch(0, 0, 0, 1), 0.8f, 8.0f);
			}

			GFX_Rect_Instance *inst = dr_rect(box->rect, box->background_color, 0.0f, softness);
			inst->corner_radii = box->corner_radii;

			if (draw_active_effect) {
				inst->colors[0][0] = inst->colors[0][0]*0.9f;
				inst->colors[1][0] = inst->colors[1][0]*0.9f;
				inst->colors[2][0] = Min(inst->colors[2][0]+0.1f, 1.0f);
				inst->colors[3][0] = Min(inst->colors[3][0]+0.1f, 1.0f);
			} else if (draw_hot_effect) {
				inst->colors[0][0] = Min(inst->colors[0][0]+0.1f, 1.0f);
				inst->colors[1][0] = Min(inst->colors[1][0]+0.1f, 1.0f);
			}
		}

		if (box->flags & UI_BOX_FLAG__DRAW_TEXT) {
			I1 truncate_text = !(box->flags & UI_BOX_FLAG__DISABLE_TEXT_TRUNC);
			if (truncate_text) {
				F4 text_clip = box->rect;
				F4 top_clip = dr_top_clip();
				if (top_clip[2] != 0 || top_clip[3] != 0) {
					text_clip = rect_overlap(text_clip, top_clip);
				}
				dr_push_clip(text_clip);
			}
			for (DR_FRun_Node *n = box->display_fruns.first; n != 0; n = n->next) {
				F2 pos = ui_box_text_pos(box);
				F4 color = box->text_color;
				if (draw_active_effect) {
					color[0] = Max(color[0]-0.2f, 0.0f);
				} else if (draw_hot_effect) {
					color[0] = Max(color[0]+0.1f, 1.0f);
				}
				dr_text_run(n->value.run, pos, color);
			}
			if (truncate_text) {
				dr_pop_clip();
			}
		}

		if (box->flags & UI_BOX_FLAG__CLIP) {
			F4 top_clip = dr_top_clip();
			F4 new_clip = (F4){box->rect[0]+1, box->rect[1]+1, box->rect[2]-2, box->rect[3]-2};
			if (top_clip[2] != 0 || top_clip[3] != 0) {
				new_clip = rect_overlap(new_clip, top_clip);
			}
			dr_push_clip(new_clip);
		}

		L1 pop_idx = 0;
		for (UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent) {
			pop_idx += 1;
			if (b == box && rec.push_count != 0) {
				continue;
			}

			if (b->flags & UI_BOX_FLAG__CLIP) {
				dr_pop_clip();
			}

			if (b->flags & UI_BOX_FLAG__DRAW_BORDER) {
				F4 border_rect = (F4){b->rect[0]-1, b->rect[1]-1, b->rect[2]+2, b->rect[3]+2};
				GFX_Rect_Instance *inst = dr_rect(border_rect, (F4){0.0f}, 0.0f, softness);
				inst->corner_radii = b->corner_radii;
				inst->border_width = 1.0f;
				inst->border_color = b->border_color;
			}

			I1 is_focus_hot = !!(b->flags & UI_BOX_FLAG__FOCUS_HOT) && !(b->flags & UI_BOX_FLAG__FOCUS_HOT_DISABLED);
			I1 is_focus_active = !!(b->flags & UI_BOX_FLAG__FOCUS_ACTIVE) && !(b->flags & UI_BOX_FLAG__FOCUS_ACTIVE_DISABLED);

			//- kti: Focus Overlay
			if (b->flags & UI_BOX_FLAG__CLICKABLE && !(b->flags & UI_BOX_FLAG__DISABLE_FOCUS_OVERLAY) && is_focus_hot) {
				GFX_Rect_Instance *inst = dr_rect(b->rect, (F4){1, 0, 0, 0.01f}, 0.0f, 0.0f);
				inst->corner_radii = b->corner_radii;
			}

			//- kti: Focus border
			if (b->flags & UI_BOX_FLAG__CLICKABLE && !(b->flags & UI_BOX_FLAG__DISABLE_FOCUS_BORDER) && is_focus_active) {
				F4 rect = rect_pad(b->rect, 1.0f);
				GFX_Rect_Instance *inst = dr_rect(rect, (F4){0}, 0.0f, softness);
				inst->border_color = oklch(0.428f, 0.176f, 29.234f, 1.0f);
				inst->border_width = 1.0f;
				inst->corner_radii = b->corner_radii;
			}

		}

		box = rec.next;
	}
}

#endif
