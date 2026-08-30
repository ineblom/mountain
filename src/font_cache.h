#ifndef FONT_CACHE_H
#define FONT_CACHE_H

typedef struct FC_Tag FC_Tag;
struct FC_Tag {
  L1 l1[2];
};

typedef struct FC_Raster_Info FC_Raster_Info;
struct FC_Raster_Info {
  SW4 subrect;
  SW2 raster_dim;
  W1 atlas_num;
  F1 advance;
  F1 baseline;
  F1 raster_scale;
};

typedef struct FC_Piece FC_Piece;
struct FC_Piece {
  GFX_Texture *texture;
  SW4 subrect;
  F2 offset;
  F1 advance;
  F1 scale;
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

  F1 raster_scale;
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
