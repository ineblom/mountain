#ifndef DRAW_H
#define DRAW_H

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
  M4F mesh_view_projection;
  F4 mesh_viewport_rect;
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
