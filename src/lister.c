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

typedef I1 Lister_Apply;
enum {
  LISTER_APPLY__DELTA,
  LISTER_APPLY__SET,
};

typedef L1 Lister_Entry_Flags;
enum {
  LISTER_ENTRY_FLAG__NORMALIZE_F4 = 1 << 0,
};

typedef struct Lister_L1_Options Lister_L1_Options;
struct Lister_L1_Options {
  Lister_Apply apply;
  L1 default_value;
  L1 pixels_per_unit;
  L1 min;
  L1 max;
  Lister_Entry_Flags flags;
};

typedef struct Lister_F1_Options Lister_F1_Options;
struct Lister_F1_Options {
  Lister_Apply apply;
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

typedef struct Lister_Value Lister_Value;
struct Lister_Value {
  Lister_Value *next;
  String8 name;
  void *data;
  L1 *text_len;
  L1 text_capacity;
};

typedef struct Lister_Entry Lister_Entry;
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
Internal void lister_textedit(String8, String8, B1 *, L1 *, L1);
Internal void lister_l1_internal(String8, String8, void *, Lister_L1_Options);
Internal void lister_f1_internal(Lister_Entry_Kind, String8, String8, void *, Lister_F1_Options);
Internal void lister_enum(String8, String8, I1 *, String8 *, L1);
Internal Lister_Entry *lister_cmd(String8, Cmd);
Internal Lister_Entry *lister_progress(String8, F1);
Internal void lister_ui(void);

#define lister_L1(str, name, data, ...) lister_l1_internal((str), (name), (data), (Lister_L1_Options){__VA_ARGS__})
#define lister_F1(str, name, data, ...) lister_f1_internal(LISTER_ENTRY_KIND__F1, (str), (name), (data), (Lister_F1_Options){__VA_ARGS__})
#define lister_xyz(str, name, data, ...) lister_f1_internal(LISTER_ENTRY_KIND__XYZ, (str), (name), (data), (Lister_F1_Options){__VA_ARGS__})
#define lister_color(str, name, data, apply_mode) lister_f1_internal(LISTER_ENTRY_KIND__COLOR, (str), (name), (data), (Lister_F1_Options){.apply = (apply_mode), .max = 1.0f})
#define lister_value(value, T) ((T *)(value)->data)

#endif

#if (SOURCE)

Global Lister_State lister_state = {0};

Internal void lister_reset(Arena *arena) {
  lister_state.arena = arena;
  lister_state.entry_count = 0;
  MemoryZeroArray(lister_state.entry_hash_table);
}

Internal Lister_Entry *lister_entry_push(Lister_Entry_Kind kind) {
  Lister_Entry *entry = 0;

  if (lister_state.entry_count < ArrayCount(lister_state.entries)) {
    entry = &lister_state.entries[lister_state.entry_count];
    lister_state.entry_count += 1;
    MemoryZeroStruct(entry);
    entry->kind = kind;
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
    entry = lister_entry_push(kind);
    if (entry) {
      entry->str = str;
      entry->hash_next = slot[0];
      slot[0] = entry;
    }
  }
  return entry;
}

Internal void lister_header(String8 str) {
  lister_entry_get_or_push(LISTER_ENTRY_KIND__HEADER, str);
}

Internal Lister_Value *lister_value_push(Lister_Entry *entry, String8 name, void *data) {
  Lister_Value *value = 0;
  if (entry && data && lister_state.arena) {
    value = push_array(lister_state.arena, Lister_Value, 1);
    value->name = name;
    value->data = data;
    SLLQueuePush(entry->first_value, entry->last_value, value);
    entry->value_count += 1;
  }
  return value;
}

Internal void lister_textedit(String8 str, String8 name, B1 *data, L1 *len, L1 capacity) {
  Lister_Entry *entry = lister_entry_get_or_push(LISTER_ENTRY_KIND__TEXTEDIT, str);
  if (entry) {
    Lister_Value *value = lister_value_push(entry, name, data);
    if (value) {
      value->text_len = len;
      value->text_capacity = capacity;
    }
  }
}

Internal void lister_l1_internal(String8 str, String8 name, void *data, Lister_L1_Options options) {
  Lister_Entry *entry = lister_entry_get_or_push(LISTER_ENTRY_KIND__L1, str);
  if (entry) {
    entry->data.l1_options = options;
    lister_value_push(entry, name, data);
  }
}

Internal void lister_f1_internal(Lister_Entry_Kind kind, String8 str, String8 name, void *data, Lister_F1_Options options) {
  Lister_Entry *entry = lister_entry_get_or_push(kind, str);
  if (entry) {
    entry->data.f1_options = options;
    lister_value_push(entry, name, data);
  }
}

Internal void lister_apply_L1s(Lister_Entry *entry, L1 before, L1 after) {
  if (before != after) {
    for (Lister_Value *it = entry->first_value; it != 0; it = it->next) {
      if (entry->data.l1_options.apply == LISTER_APPLY__DELTA) {
        lister_value(it, L1)[0] += after - before;
      } else {
        lister_value(it, L1)[0] = after;
      }
    }
  }
}

Internal void lister_apply_F1s(Lister_Entry *entry, F1 before, F1 after) {
  if (before != after) {
    for (Lister_Value *it = entry->first_value; it != 0; it = it->next) {
      if (entry->data.f1_options.apply == LISTER_APPLY__DELTA) {
        lister_value(it, F1)[0] += after - before;
      } else {
        lister_value(it, F1)[0] = after;
      }
    }
  }
}

Internal void lister_apply_F4s(Lister_Entry *entry, F4 before, F4 after) {
  F4 delta = after - before;
  for (Lister_Value *it = entry->first_value; it != 0; it = it->next) {
    F4 old_value = lister_value(it, F4)[0];
    if (entry->data.f1_options.apply == LISTER_APPLY__DELTA) {
      lister_value(it, F4)[0] += delta;
    } else {
      // Preserve components that were not edited when setting multiple values.
      for (L1 component = 0; component < 4; component += 1) {
        if (delta[component] != 0.0f) {
          lister_value(it, F4)[0][component] = after[component];
        }
      }
    }

    if (entry->data.f1_options.flags & LISTER_ENTRY_FLAG__NORMALIZE_F4) {
      F4 value = F4_with_w(lister_value(it, F4)[0], 0.0f);
      F1 length_sq = length_sq_F4(value);
      if (length_sq > Square(0.00001f)) {
        lister_value(it, F4)[0] = value * (1.0f / sqrt_F1(length_sq));
      } else {
        old_value = F4_with_w(old_value, 0.0f);
        F1 old_length_sq = length_sq_F4(old_value);
        lister_value(it, F4)[0] = old_length_sq > Square(0.00001f) ? old_value * (1.0f / sqrt_F1(old_length_sq)) : (F4){0.0f, 0.0f, 1.0f, 0.0f};
      }
    }
  }
}

Internal void lister_enum(String8 str, String8 name, I1 *data, String8 *names, L1 enum_count) {
  Lister_Entry *entry = lister_entry_get_or_push(LISTER_ENTRY_KIND__ENUM, str);
  if (entry) {
    entry->data.enum_options.names = names;
    entry->data.enum_options.count = enum_count;
    lister_value_push(entry, name, data);
  }
}

Internal Lister_Entry *lister_cmd(String8 str, Cmd cmd) {
  Lister_Entry *entry = lister_entry_push(LISTER_ENTRY_KIND__CMD);
  if (entry) {
    entry->str = str;
    entry->data.cmd = cmd;
  }

  return entry;
}

Internal Lister_Entry *lister_progress(String8 str, F1 progress) {
  Lister_Entry *entry = lister_entry_push(LISTER_ENTRY_KIND__PROGRESS);
  if (entry) {
    entry->str = str;
    entry->data.progress = progress;
  }
  return entry;
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

    UI_Box_Flags top_side = (visible_entry_idx == 0)*UI_BOX_FLAG__DRAW_SIDE_TOP;
    visible_entry_idx += 1;
    CString f1_label_format = entry->value_count == 1 ? "%.*s %.2f" : "%.*s";
    CString l1_label_format = entry->value_count == 1 ? "%.*s %llu" : "%.*s";

    switch (entry->kind) {
      // Header
      case LISTER_ENTRY_KIND__HEADER: {
        ui_set_next_text_padding(10.0f);
        ui_set_next_background_color(oklch(0.192f, 0.0f, 0.0f, 1.0f));
        ui_set_next_text_color(oklch(0.507f, 0.208f, 29.2f, 1.0f));
        ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
         UI_BOX_FLAG__DRAW_BACKGROUND|
         UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
         top_side , entry->str);
      } break;

      // Textedit
      case LISTER_ENTRY_KIND__TEXTEDIT: {
        Lister_Value *first_value = entry->first_value;
        ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
        ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
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
      case LISTER_ENTRY_KIND__L1: {
        UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_f1%p", entry);
        UI_Parent(drag_box) {
          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
          UI_Text_Padding(10.0f)
          UI_Flags(UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side) {
            L1 before = lister_value(entry->first_value, L1)[0];
            L1 after = before;
            UI_Signal signal = ui_drag_L1_label(entry->str, l1_label_format,
              &after, entry->data.l1_options.default_value, entry->data.l1_options.pixels_per_unit,
              entry->data.l1_options.min, entry->data.l1_options.max);
            lister_apply_L1s(entry, before, after);
          }
        }
      } break;

      // F1
      case LISTER_ENTRY_KIND__F1: {
        UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_f1%p", entry);
        UI_Parent(drag_box) {
          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
          UI_Text_Padding(10.0f) {
            ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
            F1 before = lister_value(entry->first_value, F1)[0];
            F1 after = before;
            UI_Signal signal = ui_drag_F1_label(entry->str,
              f1_label_format,
              &after, entry->data.f1_options.default_value, entry->data.f1_options.pixels_per_unit,
              entry->data.f1_options.min, entry->data.f1_options.max);
            lister_apply_F1s(entry, before, after);
          }
        }
      } break;

      // XYZ
      case LISTER_ENTRY_KIND__XYZ: {
        UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_xyz%p", entry);
        UI_Parent(drag_box) {
          ui_set_next_text_padding(10.0f);
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
           UI_BOX_FLAG__DRAW_SIDE_RIGHT|
           UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side,
           entry->str);
          UI_Text_Align(UI_TEXT_ALIGN__CENTER) {
            F4 before = lister_value(entry->first_value, F4)[0];
            F4 after = before;
            F1 after_components[3] = {after[0], after[1], after[2]};
            String8 component_names[3] = {str8("X"), str8("Y"), str8("Z")};
            for (L1 component = 0; component < ArrayCount(component_names); component += 1) {
              UI_Box_Flags flags = UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side;
              if (component + 1 < ArrayCount(component_names)) {
                flags |= UI_BOX_FLAG__DRAW_SIDE_RIGHT;
              }
              ui_set_next_flags(flags);
              ui_drag_F1_label(component_names[component], f1_label_format,
                &after_components[component], entry->data.f1_options.default_value,
                entry->data.f1_options.pixels_per_unit, entry->data.f1_options.min, entry->data.f1_options.max);
              after[component] = after_components[component];
            }
            lister_apply_F4s(entry, before, after);
          }
        }
      } break;

      // Color
      case LISTER_ENTRY_KIND__COLOR: {
        UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_xyz%p", entry);
        UI_Parent(drag_box) {
          ui_set_next_text_padding(10.0f);
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
           UI_BOX_FLAG__DRAW_SIDE_RIGHT|
           UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side,
           entry->str);

          F4 before = lister_value(entry->first_value, F4)[0];
          F4 after = before;
          F1 after_components[3] = {after[0], after[1], after[2]};
          ui_set_next_background_color(oklch_from_linear_rgb(after));
          ui_set_next_pref_width(ui_px(30.0f, 1.0f));
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_BACKGROUND|
           UI_BOX_FLAG__DRAW_SIDE_RIGHT|
           UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
           top_side, str8("color_preview"));

          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
          UI_Pref_Width(ui_pct(0.75f/3.0f, 1.0f)) {
            F1 pixels_per_unit = 0.0f;
            String8 component_names[3] = {str8("R"), str8("G"), str8("B")};
            F4 component_colors[3] = {
              oklch(0.27f, 0.1f, 27.0f, 1.0f),
              oklch(0.27f, 0.09f, 143.0f, 1.0f),
              oklch(0.27f, 0.09f, 256.0f, 1.0f),
            };
            for (L1 component = 0; component < ArrayCount(component_names); component += 1) {
              UI_Box_Flags flags = UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side;
              if (component + 1 < ArrayCount(component_names)) {
                flags |= UI_BOX_FLAG__DRAW_SIDE_RIGHT;
              }
              ui_set_next_flags(flags);
              ui_set_next_background_color(component_colors[component]);
              ui_drag_F1_label(component_names[component], f1_label_format,
                &after_components[component], 0.0f, pixels_per_unit, 0.0f, 1.0f);
              after[component] = after_components[component];
            }
            lister_apply_F4s(entry, before, after);
          }
        }
      } break;

      // Enum
      case LISTER_ENTRY_KIND__ENUM: {
        ui_set_next_child_layout_axis(AXIS__X);
        UI_Box *enum_box = ui_build_box_from_stringf(0, "enum_%p", entry);
        UI_Parent(enum_box) {
          ui_set_next_text_padding(10.0f);
          ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
           UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
           top_side,
           entry->str);

          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
          UI_Pref_Width(ui_pct(1.0f/(F1)entry->data.enum_options.count, 1.0f)) {
            I1 enum_value = lister_value(entry->first_value, I1)[0];
            I1 enum_is_mixed = 0;
            for (Lister_Value *value = entry->first_value->next; value != 0; value = value->next) {
              if (lister_value(value, I1)[0] != enum_value) {
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
                UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                top_side,
                "%.*s##enum_%p_%llu",
                (int)name.len, name.str,
                entry, option);
              UI_Signal signal = ui_signal_from_box(option_box);
              if (signal.flags & UI_SIGNAL_FLAG__PRESSED) {
                for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
                  lister_value(value, I1)[0] = option;
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
          UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
          top_side,
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
        UI_Box *outer = ui_build_box_from_stringf(
          UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side,
          "progress_outer_%.*s", (int)entry->str.len, entry->str.str);
        UI_Parent(outer) {
          F1 progress = entry->data.progress;
          ui_set_next_pref_width(ui_pct(progress, 1.0f));
          ui_set_next_background_color(oklch(0.637f, 0.2f, 140.0f, 1.0f));
          ui_set_next_text_padding(10.0f);
          UI_Box *prog = ui_build_box_from_stringf(
            UI_BOX_FLAG__DRAW_TEXT|UI_BOX_FLAG__DRAW_BACKGROUND|UI_BOX_FLAG__DISABLE_TEXT_TRUNC,
            "%.*s %.2f%%", (int)entry->str.len, entry->str.str, progress*100.0f);
        }
      } break; 
    }
  }
}

#endif
