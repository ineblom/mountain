#if (HEADER)

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
#define lister_color(str, data) lister_f1_internal(LISTER_ENTRY_KIND__COLOR, (str), (data), (Lister_F1_Options){.max = 1.0f})

#endif

#if (SOURCE)

Global Lister_State lister_state = {0};

Internal void lister_reset(Arena *arena) {
  lister_state.arena = arena;
  lister_state.entry_count = 0;
  MemoryZeroArray(lister_state.entry_hash_table);
}

Internal Lister_Entry *lister_entry_push(Lister_Entry_Kind kind, String8 str) {
  Lister_Entry *entry = 0;

  if (lister_state.entry_count < ArrayCount(lister_state.entries)) {
    entry = &lister_state.entries[lister_state.entry_count];
    lister_state.entry_count += 1;
    MemoryZeroStruct(entry);
    entry->kind = kind;
    entry->str = str;
  }

  return entry;
}

Internal Lister_Entry *lister_entry_get_or_push(Lister_Entry_Kind kind, String8 str) {
  L1 hash = str8_hash(str) ^ (5381*kind);
  L1 slot_idx = hash%ArrayCount(lister_state.entry_hash_table);
  Lister_Entry **slot = &lister_state.entry_hash_table[slot_idx];
  Lister_Entry *entry = 0;

  for (Lister_Entry *it = slot[0]; it != 0; it = it->hash_next) {
    if (it->kind == kind && str8_match(it->str, str)) {
      entry = it;
      break;
    }
  }

  if (entry == 0) {
    entry = lister_entry_push(kind, str);
    if (entry) {
      entry->hash_next = slot[0];
      slot[0] = entry;
    }
  }
  return entry;
}

Internal void lister_header(String8 str) {
  lister_entry_get_or_push(LISTER_ENTRY_KIND__HEADER, str);
}

Internal Lister_Value *lister_value(Lister_Entry_Kind kind, String8 str, void *data) {
  Lister_Value *value = 0;
  Lister_Entry *entry = lister_entry_get_or_push(kind, str);
  if (entry && data && lister_state.arena) {
    value = push_array(lister_state.arena, Lister_Value, 1);
    value->entry = entry;
    value->data = data;
    SLLQueuePush(entry->first_value, entry->last_value, value);
    entry->value_count += 1;
  }
  return value;
}

Internal void lister_textedit(String8 str, B1 *data, L1 *len, L1 capacity) {
  Lister_Value *value = lister_value(LISTER_ENTRY_KIND__TEXTEDIT, str, data);
  if (value) {
    value->text_len = len;
    value->text_capacity = capacity;
  }
}

Internal void lister_enum(String8 str, I1 *data, String8 *names, L1 enum_count) {
  Lister_Value *value = lister_value(LISTER_ENTRY_KIND__ENUM, str, data);
  if (value) {
    value->entry->data.enum_options.names = names;
    value->entry->data.enum_options.count = enum_count;
  }
}

Internal Lister_Entry *lister_cmd(String8 str, Cmd cmd) {
  Lister_Entry *entry = lister_entry_push(LISTER_ENTRY_KIND__CMD, str);
  if (entry) {
    entry->data.cmd = cmd;
  }

  return entry;
}

Internal Lister_Entry *lister_progress(String8 str, F1 progress) {
  Lister_Entry *entry = lister_entry_push(LISTER_ENTRY_KIND__PROGRESS, str);
  if (entry) {
    entry->data.progress = progress;
  }
  return entry;
}

Internal void lister_l1_internal(String8 str, void *data, Lister_L1_Options options) {
  Lister_Value *value = lister_value(LISTER_ENTRY_KIND__L1, str, data);
  if (value) {
    value->entry->data.l1_options = options;
  }
}

Internal void lister_f1_internal(Lister_Entry_Kind kind, String8 str, void *data, Lister_F1_Options options) {
  Lister_Value *value = lister_value(kind, str, data);
  if (value) {
    value->entry->data.f1_options = options;
  }
}

typedef struct Lister_Drag Lister_Drag;
struct Lister_Drag {
  UI_Box *box;
  UI_Signal signal;
};

Internal Lister_Drag lister_drag_begin(Lister_Entry *entry, L1 component) {
  Lister_Drag result = {0};

  UI_Key key = ui_key_from_string(ui_active_seed_key(),
    str8f(ui_build_arena(), "lister_drag_%p_%llu", entry, component));
  result.box = ui_build_box_from_key(UI_BOX_FLAG__MOUSE_CLICKABLE|
                                      UI_BOX_FLAG__DRAW_BACKGROUND|
                                      UI_BOX_FLAG__DRAW_HOT_EFFECTS,
                                      key);
  result.signal = ui_signal_from_box(result.box);

  return result;
}

#define as(value, T) ((T *)(value)->data)

Internal void lister_drag_L1(Lister_Entry *entry) {
  Lister_L1_Options options = entry->data.l1_options;
  L1 before = as(entry->first_value, L1)[0];
  L1 after = before;

  L1 true_max = options.max != 0 ? options.max : L1_MAX;
  Lister_Drag drag = lister_drag_begin(entry, 0);

  F1 pixels_per_unit = options.pixels_per_unit;
  if (pixels_per_unit == 0.0f) {
    pixels_per_unit = 5.0f;

    // Auto-calculate pixels per unit when possible.
    if (options.max > options.min && drag.box->rect[2] > 0.0f) {
      L1 range = options.max - options.min;
      pixels_per_unit = (F1)((D1)drag.box->rect[2] / (D1)range);
    }
  }

  if (drag.signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
    if (drag.signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
      ui_store_drag_struct(&before);
    }
    L1 initial_value = ui_get_drag_struct(L1)[0];

    initial_value = Clamp(options.min, initial_value, true_max);

    SL1 delta = (SL1)(ui_drag_delta()[0] / pixels_per_unit);
    if (delta < 0) {
      L1 magnitude = (L1)-delta;
      after = magnitude > initial_value - options.min ? options.min : initial_value - magnitude;
    } else {
      L1 magnitude = (L1)delta;
      after = magnitude > true_max - initial_value ? true_max : initial_value + magnitude;
    }
  }

  if (drag.signal.flags & UI_SIGNAL_FLAG__MIDDLE_PRESSED) {
    after = options.default_value;
  }

  if (before != after) {
    for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
      as(value, L1)[0] += after - before;
    }
  }

  UI_Parent(drag.box) {
    ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
    CString format = entry->value_count == 1 ? "%.*s %llu" : "%.*s";
    ui_label(str8f(ui_build_arena(), format, (int)entry->str.len, entry->str.str, after));
  }
}

Internal void lister_drag_F1(Lister_Entry *entry, L1 component, String8 label) {
  Lister_F1_Options options = entry->data.f1_options;
  I1 is_scalar = entry->kind == LISTER_ENTRY_KIND__F1;
  F1 before = is_scalar ?
    as(entry->first_value, F1)[0] :
    as(entry->first_value, F4)[0][component];
  F1 after = before;

  Lister_Drag drag = lister_drag_begin(entry, component);

  F1 pixels_per_unit = options.pixels_per_unit;
  if (pixels_per_unit == 0.0f) {
    pixels_per_unit = 50.0f;

    // Auto-calculate pixels per unit when possible.
    if (options.max > options.min && drag.box->rect[2] > 0.0f) {
      D1 range = (D1)options.max - (D1)options.min;
      pixels_per_unit = (F1)((D1)drag.box->rect[2] / range);
    }
  }

  // handle dragging
  if (drag.signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
    if (drag.signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
      ui_store_drag_struct(&before);
    }
    F1 initial_value = ui_get_drag_struct(F1)[0];
    after = initial_value + ui_drag_delta()[0] / pixels_per_unit;
  }

  // default value
  if (drag.signal.flags & UI_SIGNAL_FLAG__MIDDLE_PRESSED) {
    after = options.default_value;
  }

  // clamp
  if (options.min != 0) after = Max(options.min, after);
  if (options.max != 0) after = Min(options.max, after);

  // update entry values
  if (before != after) {
    F1 delta = after - before;

    for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
      if (is_scalar) {
        as(value, F1)[0] += delta;
      } else {
        as(value, F4)[0][component] += delta;

        if (options.flags & LISTER_ENTRY_FLAG__NORMALIZE_F4) {
          // TODO: Implement.
        }
      }
    }
  }

  UI_Parent(drag.box) {
    ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
    CString format = entry->value_count == 1 ? "%.*s %.2f" : "%.*s";
    ui_label(str8f(ui_build_arena(), format, (int)label.len, label.str, after));
  }
}

Internal void lister_ui(void) {
  L1 visible_entry_idx = 0;
  for (L1 i = 0; i < lister_state.entry_count; i += 1) {
    Lister_Entry *entry = &lister_state.entries[i];

    //- kti: Skip entries without values.
    if (entry->kind != LISTER_ENTRY_KIND__HEADER &&
      entry->kind != LISTER_ENTRY_KIND__CMD &&
      entry->kind != LISTER_ENTRY_KIND__PROGRESS &&
      entry->value_count == 0) {
      continue;
    }

    UI_Box_Flags row_sides = UI_BOX_FLAG__DRAW_SIDE_BOTTOM|(visible_entry_idx == 0)*UI_BOX_FLAG__DRAW_SIDE_TOP;

    visible_entry_idx += 1;
    switch (entry->kind) {
      // Header
      case LISTER_ENTRY_KIND__HEADER: {
        ui_set_next_text_padding(10.0f);
        // ui_set_next_background_color(oklch(0.192f, 0.0f, 0.0f, 1.0f));
        ui_set_next_text_color(oklch(0.507f, 0.208f, 29.2f, 1.0f));
        ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|UI_BOX_FLAG__DRAW_BACKGROUND|row_sides , entry->str);
      } break;

      // Textedit
      case LISTER_ENTRY_KIND__TEXTEDIT: {
        Lister_Value *first_value = entry->first_value;
        ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
        ui_set_next_flags(row_sides);
        ui_set_next_omit_flags(UI_BOX_FLAG__DRAW_BORDER);
        UI_Text_Padding(10.0f) {
          UI_Signal signal = ui_textedit(
            &lister_state.textedit_cursor,
            &lister_state.textedit_mark,
            lister_state.textedit_buffer,
            sizeof(lister_state.textedit_buffer),
            &lister_state.textedit_buffer_len,
            (String8){first_value->data, first_value->text_len[0]},
            str8f(ui_build_arena(), "###lister_textedit_%p", first_value->data));

          if (signal.flags & UI_SIGNAL_FLAG__COMMIT) {
            for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
              value->text_len[0] = Min(lister_state.textedit_buffer_len, value->text_capacity);
              memmove(value->data, lister_state.textedit_buffer, value->text_len[0]);
            }
          }
        }
      } break;

      // L1
      case LISTER_ENTRY_KIND__L1:
      case LISTER_ENTRY_KIND__F1: {
        UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_l1%p", entry);
        UI_Parent(drag_box)
        UI_Text_Align(UI_TEXT_ALIGN__CENTER)
        UI_Text_Padding(10.0f)
        UI_Background_Color(oklch(0.1f, 0.0f, 0.0f, 1.0f))
        UI_Flags(row_sides) {
          if (entry->kind == LISTER_ENTRY_KIND__L1) lister_drag_L1(entry);
          else lister_drag_F1(entry, 0, entry->str);
        }
      } break;

      // XYZ
      case LISTER_ENTRY_KIND__XYZ: {
        UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_xyz%p", entry);
        UI_Parent(drag_box) {
          ui_set_next_text_padding(10.0f);
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|UI_BOX_FLAG__DRAW_SIDE_RIGHT|row_sides, entry->str);
          UI_Text_Align(UI_TEXT_ALIGN__CENTER) {
            String8 component_names[3] = {str8("X"), str8("Y"), str8("Z")};
            for (L1 component = 0; component < 3; component += 1) {
              I1 not_last = (component < 2);
              ui_set_next_flags(row_sides|(UI_BOX_FLAG__DRAW_SIDE_RIGHT*not_last));
              lister_drag_F1(entry, component, component_names[component]);
            }
          }
        }
      } break;

      // Color
      case LISTER_ENTRY_KIND__COLOR: {
        UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_xyz%p", entry);
        UI_Parent(drag_box) {
          ui_set_next_text_padding(10.0f);
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|UI_BOX_FLAG__DRAW_SIDE_RIGHT|row_sides, entry->str);

          // preview
          F4 color = as(entry->first_value, F4)[0];
          ui_set_next_background_color(oklch_from_linear_rgb(color));
          ui_set_next_pref_width(ui_px(30.0f, 1.0f));
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_BACKGROUND|UI_BOX_FLAG__DRAW_SIDE_RIGHT|row_sides, str8("color_preview"));

          // components
          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
          UI_Pref_Width(ui_pct(0.75f/3.0f, 1.0f)) {
            String8 component_names[3] = {str8("R"), str8("G"), str8("B")};
            F4 component_colors[3] = {
              oklch(0.27f, 0.1f, 27.0f, 1.0f),
              oklch(0.27f, 0.09f, 143.0f, 1.0f),
              oklch(0.27f, 0.09f, 256.0f, 1.0f),
            };
            for (L1 component = 0; component < 3; component += 1) {
              I1 not_last = (component < 2);
              ui_set_next_flags(row_sides|(UI_BOX_FLAG__DRAW_SIDE_RIGHT*not_last));
              ui_set_next_background_color(component_colors[component]);
              lister_drag_F1(entry, component, component_names[component]);
            }
          }
        }
      } break;

      // Enum
      case LISTER_ENTRY_KIND__ENUM: {
        ui_set_next_child_layout_axis(AXIS__X);
        UI_Box *enum_box = ui_build_box_from_stringf(0, "enum_%p", entry);
        UI_Parent(enum_box) {
          ui_set_next_text_padding(10.0f);
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|row_sides,entry->str);

          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
          UI_Pref_Width(ui_pct(1.0f/(F1)entry->data.enum_options.count, 1.0f)) {
            I1 enum_value = as(entry->first_value, I1)[0];
            I1 enum_is_mixed = 0;
            for (Lister_Value *value = entry->first_value->next; value != 0; value = value->next) {
              if (as(value, I1)[0] != enum_value) {
                enum_is_mixed = 1;
                break;
              }
            }
            for (L1 option = 0; option < entry->data.enum_options.count; option += 1) {
              if (!enum_is_mixed && enum_value == option) {
                ui_set_next_background_color(oklch(0.35f, 0.08f, 252.0f, 1.0f));
              }

              String8 name = entry->data.enum_options.names[option];
              UI_Box *option_box = ui_build_box_from_stringf(
                UI_BOX_FLAG__CLICKABLE|
                UI_BOX_FLAG__DRAW_TEXT|
                UI_BOX_FLAG__DRAW_BACKGROUND|
                UI_BOX_FLAG__DRAW_HOT_EFFECTS|
                UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS|
                UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                row_sides,
                "%.*s##enum_%p_%llu",
                (int)name.len, name.str,
                entry, option);
              UI_Signal signal = ui_signal_from_box(option_box);
              if (signal.flags & UI_SIGNAL_FLAG__PRESSED) {
                for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
                  as(value, I1)[0] = option;
                }
              }
            }
          }
        }
      } break;

      // Cmd
      case LISTER_ENTRY_KIND__CMD: {
        ui_set_next_background_color(oklch(0.2f, 0.1, 27.0f, 1.0f));
        ui_set_next_text_align(UI_TEXT_ALIGN__CENTER);
        UI_Box *cmd_box = ui_build_box_from_stringf(
          UI_BOX_FLAG__CLICKABLE|
          UI_BOX_FLAG__DRAW_TEXT|
          UI_BOX_FLAG__DRAW_BACKGROUND|
          UI_BOX_FLAG__DRAW_HOT_EFFECTS|
          UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS|
          row_sides,
          "%.*s##lister_cmd_%p",
          (int)entry->str.len, entry->str.str,
          (void *)entry);
        UI_Signal signal = ui_signal_from_box(cmd_box);
        if (signal.flags & UI_SIGNAL_FLAG__PRESSED) {
          cmd_push(entry->data.cmd);
        }
      } break;

      // Progress
      case LISTER_ENTRY_KIND__PROGRESS: {
        UI_Box *outer = ui_build_box_from_stringf(row_sides, "progress_outer_%.*s", (int)entry->str.len, entry->str.str);
        UI_Parent(outer) {
          F1 progress = entry->data.progress;
          ui_set_next_pref_width(ui_pct(progress, 1.0f));
          ui_set_next_background_color(oklch(0.637f, 0.2f, 140.0f, 1.0f));
          ui_set_next_text_padding(10.0f);
          ui_build_box_from_stringf(
            UI_BOX_FLAG__DRAW_TEXT|UI_BOX_FLAG__DRAW_BACKGROUND|UI_BOX_FLAG__DISABLE_TEXT_TRUNC,
            "%.*s %.2f%%", (int)entry->str.len, entry->str.str, progress*100.0f);
        }
      } break; 
    }
  }
}

#undef as

#endif
