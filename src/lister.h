#ifndef LISTER_H
#define LISTER_H

typedef enum Lister_Entry_Kind {
  LISTER_ENTRY_KIND__HEADER,
  LISTER_ENTRY_KIND__TEXTEDIT,
  LISTER_ENTRY_KIND__F1,
  LISTER_ENTRY_KIND__L1,
  LISTER_ENTRY_KIND__XYZ,
  LISTER_ENTRY_KIND__COLOR,
  LISTER_ENTRY_KIND__ENUM,
  LISTER_ENTRY_KIND__CMD,
  LISTER_ENTRY_KIND__PROGRESS,
} Lister_Entry_Kind;

typedef L1 Lister_Entry_Flags;
enum {
  LISTER_ENTRY_FLAG__NORMALIZE_F4 = 1 << 0,
};

typedef struct Lister_L1_Options Lister_L1_Options;
struct Lister_L1_Options {
  L1 default_value;
  L1 pixels_per_unit;
  L1 min;
  L1 max;
  Lister_Entry_Flags flags;
};

typedef struct Lister_F1_Options Lister_F1_Options;
struct Lister_F1_Options {
  F1 default_value;
  F1 pixels_per_unit;
  F1 min;
  F1 max;
  Lister_Entry_Flags flags;
};

typedef struct Lister_Enum_Options Lister_Enum_Options;
struct Lister_Enum_Options {
  String8 *names;
  L1 count;
};

typedef struct Lister_Entry Lister_Entry;
typedef struct Lister_Value Lister_Value;
struct Lister_Value {
  Lister_Entry *entry;
  Lister_Value *next;
  void *data;
  L1 *text_len;
  L1 text_capacity;
};

struct Lister_Entry {
  Lister_Entry *hash_next;
  Lister_Entry_Kind kind;
  String8 str;
  Lister_Value *first_value;
  Lister_Value *last_value;
  L1 value_count;

  union {
    Lister_F1_Options f1_options;
    Lister_L1_Options l1_options;
    Lister_Enum_Options enum_options;
    Cmd cmd;
    F1 progress;
  } data;
};

typedef struct Lister_State Lister_State;
struct Lister_State {
  Arena *arena;
  L1 entry_count;
  Lister_Entry entries[512];
  Lister_Entry *entry_hash_table[128];

  L1 open_color_key;
  F1 color_hue;

  L1 textedit_buffer_len;
  B1 textedit_buffer[512];
  Txt_Pt textedit_cursor;
  Txt_Pt textedit_mark;
};

Internal void lister_reset(Arena *);
Internal void lister_header(String8);
Internal void lister_textedit(String8, B1 *, L1 *, L1);
Internal void lister_l1_internal(String8, void *, Lister_L1_Options);
Internal void lister_f1_internal(Lister_Entry_Kind, String8, void *, Lister_F1_Options);
Internal void lister_enum(String8, I1 *, String8 *, L1);
Internal Lister_Entry *lister_cmd(String8, Cmd);
Internal Lister_Entry *lister_progress(String8, F1);
Internal void lister_ui(void);

#define lister_L1(str, data, ...) lister_l1_internal((str), (data), (Lister_L1_Options){__VA_ARGS__})
#define lister_F1(str, data, ...) lister_f1_internal(LISTER_ENTRY_KIND__F1, (str), (data), (Lister_F1_Options){__VA_ARGS__})
#define lister_xyz(str, data, ...) lister_f1_internal(LISTER_ENTRY_KIND__XYZ, (str), (data), (Lister_F1_Options){__VA_ARGS__})
#define lister_color(str, data, ...) lister_f1_internal(LISTER_ENTRY_KIND__COLOR, (str), (data), (Lister_F1_Options){__VA_ARGS__})

#endif
