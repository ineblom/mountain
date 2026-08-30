#ifndef UI_H
#define UI_H

typedef enum UI_Cmd_Kind
{
  UI_CMD_KIND__NULL,
  UI_CMD_KIND__TEXT,
  UI_CMD_KIND__NAVIGATE,
  UI_CMD_KIND__EDIT,
  UI_CMD_KIND__FILEDROP,
  UI_CMD_KIND__CANCEL,
  UI_CMD_KIND_COUNT
} UI_Cmd_Kind;

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

////////////////////////////////
//~ kti: Themes

typedef struct UI_Theme_Pattern UI_Theme_Pattern;
struct UI_Theme_Pattern {
  String8_Array tags;
  F4 linear;
};

typedef struct UI_Theme UI_Theme;
struct UI_Theme {
  UI_Theme_Pattern *patterns;
  L1 pattern_count;
};

typedef I1 UI_Focus_Kind;
enum {
  UI_FOCUS_KIND__NULL,
  UI_FOCUS_KIND__OFF,
  UI_FOCUS_KIND__ON,
  UI_FOCUS_KIND__ROOT,
  UI_FOCUS_KIND_COUNT,
};

typedef I1 UI_Box_Custom_Draw_Kind;
enum {
  UI_BOX_CUSTOM_DRAW_KIND__NONE,
  UI_BOX_CUSTOM_DRAW_KIND__LINE_EDIT,
};

typedef struct UI_Line_Edit_Draw_Data UI_Line_Edit_Draw_Data;
struct UI_Line_Edit_Draw_Data {
  String8 edited_string;
  Txt_Pt cursor;
  Txt_Pt mark;
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
  UI_BOX_FLAG__DRAW_SIDE_TOP            = (1LLU<<38),
  UI_BOX_FLAG__DRAW_SIDE_BOTTOM         = (1LLU<<39),
  UI_BOX_FLAG__DRAW_SIDE_LEFT           = (1LLU<<40),
  UI_BOX_FLAG__DRAW_SIDE_RIGHT          = (1LLU<<41),
  UI_BOX_FLAG__INSET_FOCUS_BORDER       = (1LLU<<42),

  UI_BOX_FLAG__CLICKABLE         = (UI_BOX_FLAG__MOUSE_CLICKABLE|UI_BOX_FLAG__KEYBOARD_CLICKABLE),
  UI_BOX_FLAG__FLOATING          = (UI_BOX_FLAG__FLOATING_X|UI_BOX_FLAG__FLOATING_Y),
  UI_BOX_FLAG__VIEW_SCROLL       = (UI_BOX_FLAG__VIEW_SCROLL_X|UI_BOX_FLAG__VIEW_SCROLL_Y),
  UI_BOX_FLAG__VIEW_CLAMP        = (UI_BOX_FLAG__VIEW_CLAMP_X|UI_BOX_FLAG__VIEW_CLAMP_Y),
  UI_BOX_FLAG__DEFAULT_FOCUS_NAV = (UI_BOX_FLAG__DEFAULT_FOCUS_NAV_X|UI_BOX_FLAG__DEFAULT_FOCUS_NAV_Y),
  UI_BOX_FLAG__DRAW_SIDES        = (UI_BOX_FLAG__DRAW_SIDE_TOP|UI_BOX_FLAG__DRAW_SIDE_BOTTOM|UI_BOX_FLAG__DRAW_SIDE_LEFT|UI_BOX_FLAG__DRAW_SIDE_RIGHT),
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
  UI_Key tags_key;
  UI_Box_Flags flags;
  String8 string;
  F2 fixed_pos;
  F2 fixed_size;
  F2 min_size;
  F2 view_off;
  F2 view_off_target;
  F2 view_bounds;
  F4 rect;
  UI_Size pref_size[AXIS2_COUNT];
  Axis child_layout_axis;
  L1 first_touch_build_index;
  L1 last_touch_build_index;

  FC_Tag font;
  F1 font_size;
  F1 tab_size;
  UI_Text_Align text_align;
  F4 corner_radii;
  F4 background_colors[4];
  F4 text_color;
  F4 border_color;
  F1 text_padding;
  DR_FRun_List display_fruns;
  DR_FStr_List display_fstrs;
  UI_Box_Custom_Draw_Kind custom_draw_kind;
  void *custom_draw_user_data;

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

typedef struct UI_Tags_Cache_Node UI_Tags_Cache_Node;
struct UI_Tags_Cache_Node {
  UI_Tags_Cache_Node *next;
  UI_Key key;
  String8_Array tags;
};

typedef struct UI_Tags_Cache_Slot UI_Tags_Cache_Slot;
struct UI_Tags_Cache_Slot {
  UI_Tags_Cache_Node *first;
  UI_Tags_Cache_Node *last;
};

typedef struct UI_Tags_Key_Stack_Node UI_Tags_Key_Stack_Node;
struct UI_Tags_Key_Stack_Node {
  UI_Tags_Key_Stack_Node *next;
  UI_Key key;
};

typedef struct UI_Theme_Pattern_Cache_Node UI_Theme_Pattern_Cache_Node;
struct UI_Theme_Pattern_Cache_Node {
  UI_Theme_Pattern_Cache_Node *slot_next;
  UI_Theme_Pattern_Cache_Node *slot_prev;
  UI_Theme_Pattern_Cache_Node *lru_next;
  UI_Theme_Pattern_Cache_Node *lru_prev;
  L1 last_build_index_accessed;
  UI_Key key;
  F4 target_rgba;
  F4 current_rgba;
};

typedef struct UI_Theme_Pattern_Cache_Slot UI_Theme_Pattern_Cache_Slot;
struct UI_Theme_Pattern_Cache_Slot {
  UI_Theme_Pattern_Cache_Node *first;
  UI_Theme_Pattern_Cache_Node *last;
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

  Arena *drag_arena[OS_MOUSE_BUTTON_COUNT];
  String8 drag_data[OS_MOUSE_BUTTON_COUNT];

  Arena *build_arenas[2];
  L1 build_index;

  UI_Box *first_free_box;
  L1 box_table_size;
  UI_Box_HT_Slot *box_table;

  UI_Tags_Key_Stack_Node *tags_key_stack_top;
  UI_Tags_Key_Stack_Node *tags_key_stack_free;
  L1 tags_cache_slot_count;
  UI_Tags_Cache_Slot *tags_cache_slots;

  L1 theme_pattern_cache_slot_count;
  UI_Theme_Pattern_Cache_Slot *theme_pattern_cache_slots;
  UI_Theme_Pattern_Cache_Node *theme_pattern_cache_node_free;
  UI_Theme_Pattern_Cache_Node *lru_theme_pattern_cache_node;
  UI_Theme_Pattern_Cache_Node *mru_theme_pattern_cache_node;
  UI_Theme *theme;
  L1 theme_build_index;
  F1 animation_dt;

  UI_Box *root;
  UI_Key default_nav_root_key;
  L1 build_box_count;
  L1 last_build_box_count;
  OS_Window *window;
  F2 mouse;
  F2 drag_start_mouse[OS_MOUSE_BUTTON_COUNT];
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

#define UI_Tag(v) DeferLoop(ui_push_tag((v)), ui_pop_tag())
#define UI_TagF(...) DeferLoop(ui_push_tagf(__VA_ARGS__), ui_pop_tag())

#endif
