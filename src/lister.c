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

Internal L1 lister_color_key(Lister_Entry *entry) {
  L1 result = str8_hash(entry->str) ^ (5381*entry->kind);
  if (result == 0) result = 1;
  return result;
}

Internal F4 lister_hsv_from_rgb(F4 color) {
  F1 r = color[0];
  F1 g = color[1];
  F1 b = color[2];
  F1 max_component = Max(r, Max(g, b));
  F1 min_component = Min(r, Min(g, b));
  F1 delta = max_component - min_component;

  F1 hue = 0.0f;
  if (delta > 0.00001f) {
    if (max_component == r) {
      hue = mod_F1((g-b)/delta, 6.0f);
    } else if (max_component == g) {
      hue = (b-r)/delta + 2.0f;
    } else {
      hue = (r-g)/delta + 4.0f;
    }
    hue /= 6.0f;
    if (hue < 0.0f) hue += 1.0f;
  }

  F1 saturation = max_component > 0.00001f ? delta/max_component : 0.0f;
  F4 result = {hue, saturation, max_component, color[3]};
  return result;
}

Internal F4 lister_rgb_from_hsv(F4 hsv) {
  F1 hue = mod_F1(hsv[0], 1.0f);
  if (hue < 0.0f) hue += 1.0f;

  F1 saturation = clamp01_F1(hsv[1]);
  F1 value = clamp01_F1(hsv[2]);
  F1 h = hue*6.0f;
  L1 sector = (L1)floor_F1(h);
  F1 fraction = h - floor_F1(h);
  F1 p = value*(1.0f-saturation);
  F1 q = value*(1.0f-saturation*fraction);
  F1 t = value*(1.0f-saturation*(1.0f-fraction));

  F4 result = {0.0f, 0.0f, 0.0f, hsv[3]};
  switch (sector%6) {
    case 0: result = (F4){value, t, p, hsv[3]}; break;
    case 1: result = (F4){q, value, p, hsv[3]}; break;
    case 2: result = (F4){p, value, t, hsv[3]}; break;
    case 3: result = (F4){p, q, value, hsv[3]}; break;
    case 4: result = (F4){t, p, value, hsv[3]}; break;
    case 5: result = (F4){value, p, q, hsv[3]}; break;
  }
  return result;
}

Internal void lister_color_apply(Lister_Entry *entry, F4 hsv) {
  F4 rgb = lister_rgb_from_hsv(hsv);
  for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
    F1 alpha = as(value, F4)[0][3];
    as(value, F4)[0] = rgb;
    as(value, F4)[0][3] = alpha;
  }
}

Internal void lister_color_sv_marker(UI_Box *box, F1 saturation, F1 value) {
  F1 marker_size = 12.0f;
  saturation = clamp01_F1(saturation);
  value = clamp01_F1(value);
  F1 x = saturation*box->rect[2] - marker_size*0.5f;
  F1 y = (1.0f-value)*box->rect[3] - marker_size*0.5f;

  UI_Parent(box) {
    ui_set_next_fixed_x(x);
    ui_set_next_fixed_y(y);
    ui_set_next_fixed_width(marker_size);
    ui_set_next_fixed_height(marker_size);
    ui_set_next_corner_radius(marker_size*0.5f);
    ui_set_next_border_color((F4){0.0f, 0.0f, 0.0f, 1.0f});
    ui_build_box_from_string(UI_BOX_FLAG__DRAW_BORDER, str8("sv_marker_outer"));

    ui_set_next_fixed_x(x+1.0f);
    ui_set_next_fixed_y(y+1.0f);
    ui_set_next_fixed_width(marker_size-2.0f);
    ui_set_next_fixed_height(marker_size-2.0f);
    ui_set_next_corner_radius((marker_size-2.0f)*0.5f);
    ui_set_next_border_color((F4){1.0f, 1.0f, 1.0f, 1.0f});
    ui_build_box_from_string(UI_BOX_FLAG__DRAW_BORDER, str8("sv_marker_inner"));
  }
}

Internal void lister_color_hue_marker(UI_Box *box, F1 hue) {
  F1 marker_height = 6.0f;
  hue = clamp01_F1(hue);
  F1 y = hue*box->rect[3] - marker_height*0.5f;

  UI_Parent(box) {
    ui_set_next_fixed_x(-2.0f);
    ui_set_next_fixed_y(y);
    ui_set_next_fixed_width(box->rect[2]+4.0f);
    ui_set_next_fixed_height(marker_height);
    ui_set_next_corner_radius(2.0f);
    ui_set_next_border_color((F4){0.0f, 0.0f, 0.0f, 1.0f});
    ui_build_box_from_string(UI_BOX_FLAG__DRAW_BORDER, str8("hue_marker_outer"));

    ui_set_next_fixed_x(-1.0f);
    ui_set_next_fixed_y(y+1.0f);
    ui_set_next_fixed_width(box->rect[2]+2.0f);
    ui_set_next_fixed_height(marker_height-2.0f);
    ui_set_next_corner_radius(1.0f);
    ui_set_next_border_color((F4){1.0f, 1.0f, 1.0f, 1.0f});
    ui_build_box_from_string(UI_BOX_FLAG__DRAW_BORDER, str8("hue_marker_inner"));
  }
}

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
  if (options.min != 0 || options.max != 0) after = Max(options.min, after);
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
          as(value, F4)[0] = normalize_F4(as(value, F4)[0]);
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
        // ui_set_next_background_color((F4){0.007077888f, 0.007077888f, 0.007077888f, 1.0f});
        ui_set_next_text_color((F4){0.526210363f, 0.000019104f, 0.000108155f, 1.0f});
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
        UI_Background_Color(((F4){0.008101465f, 0.017500665f, 0.023206312f, 1.0f}))
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
        L1 color_key = lister_color_key(entry);
        F4 color = as(entry->first_value, F4)[0];
        F4 hsv = lister_hsv_from_rgb(color);

        ui_set_next_child_layout_axis(AXIS__Y);
        ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
        ui_set_next_pref_height(ui_children_sum(1.0f));
        UI_Box *color_column = ui_build_box_from_stringf(row_sides, "color_column_%p", entry);
        UI_Box *preview_box = 0;
        UI_Box *sv_box = 0;

        UI_Parent(color_column) {
          ui_set_next_child_layout_axis(AXIS__X);
          ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
          ui_set_next_pref_height(ui_px(30.0f, 1.0f));
          UI_Box *header = ui_build_box_from_stringf(0, "color_header_%p", entry);
          UI_Parent(header) {
            ui_set_next_pref_width(ui_pct(0.75f, 1.0f));
            ui_set_next_text_padding(10.0f);
            ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|UI_BOX_FLAG__DRAW_SIDE_RIGHT, entry->str);

            ui_set_next_pref_width(ui_pct(0.25f, 1.0f));
            ui_set_next_background_color(color);
            preview_box = ui_build_box_from_stringf(
              UI_BOX_FLAG__MOUSE_CLICKABLE|
              UI_BOX_FLAG__DRAW_BACKGROUND|
              UI_BOX_FLAG__DRAW_HOT_EFFECTS|
              UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS,
              "color_preview_%p", entry);

            UI_Signal preview_signal = ui_signal_from_box(preview_box);
            if (preview_signal.flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
              if (lister_state.open_color_key == color_key) {
                lister_state.open_color_key = 0;
              } else {
                lister_state.open_color_key = color_key;
                if (hsv[1] > 0.00001f && hsv[2] > 0.00001f) {
                  lister_state.color_hue = hsv[0];
                }
              }
            }
          }

          if (lister_state.open_color_key == color_key) {
            if (hsv[1] > 0.00001f && hsv[2] > 0.00001f) {
              lister_state.color_hue = hsv[0];
            } else {
              hsv[0] = lister_state.color_hue;
            }

            ui_set_next_child_layout_axis(AXIS__Y);
            ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
            ui_set_next_pref_height(ui_children_sum(1.0f));
            UI_Box *picker_body = ui_build_box_from_stringf(0, "color_picker_body_%p", entry);
            UI_Parent(picker_body) {
              ui_spacer(ui_px(8.0f, 1.0f));

              ui_set_next_child_layout_axis(AXIS__X);
              ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
              ui_set_next_pref_height(ui_px(140.0f, 1.0f));
              UI_Box *picker_row = ui_build_box_from_stringf(0, "color_picker_row_%p", entry);
              UI_Parent(picker_row)
              UI_Padding(ui_px(8.0f, 1.0f)) {
                ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
                ui_set_next_pref_height(ui_pct(1.0f, 1.0f));
                ui_set_next_corner_radius(3.0f);
                sv_box = ui_build_box_from_stringf(
                  UI_BOX_FLAG__MOUSE_CLICKABLE|
                  UI_BOX_FLAG__DRAW_BACKGROUND,
                  "color_sv_%p", entry);

                UI_Signal sv_signal = ui_signal_from_box(sv_box);
                if (sv_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
                  hsv[1] = clamp01_F1((ui_mouse()[0]-sv_box->rect[0])/Max(sv_box->rect[2], 1.0f));
                  hsv[2] = 1.0f-clamp01_F1((ui_mouse()[1]-sv_box->rect[1])/Max(sv_box->rect[3], 1.0f));
                  hsv[0] = lister_state.color_hue;
                  lister_color_apply(entry, hsv);
                }
                lister_color_sv_marker(sv_box, hsv[1], hsv[2]);

                ui_spacer(ui_px(8.0f, 1.0f));

                ui_set_next_child_layout_axis(AXIS__Y);
                ui_set_next_pref_width(ui_px(18.0f, 1.0f));
                ui_set_next_pref_height(ui_pct(1.0f, 1.0f));
                UI_Box *hue_box = ui_build_box_from_stringf(
                  UI_BOX_FLAG__MOUSE_CLICKABLE,
                  "color_hue_%p", entry);

                UI_Signal hue_signal = ui_signal_from_box(hue_box);
                if (hue_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
                  lister_state.color_hue = clamp01_F1((ui_mouse()[1]-hue_box->rect[1])/Max(hue_box->rect[3], 1.0f));
                  hsv[0] = lister_state.color_hue;
                  lister_color_apply(entry, hsv);
                }

                UI_Parent(hue_box) {
                  lister_color_hue_marker(hue_box, lister_state.color_hue);
                  for (L1 segment_idx = 0; segment_idx < 6; segment_idx += 1) {
                    F4 segment_top = lister_rgb_from_hsv((F4){(F1)segment_idx/6.0f, 1.0f, 1.0f, 1.0f});
                    F4 segment_bottom = lister_rgb_from_hsv((F4){(F1)(segment_idx+1)/6.0f, 1.0f, 1.0f, 1.0f});
                    ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
                    ui_set_next_pref_height(ui_pct(1.0f/6.0f, 1.0f));
                    UI_Box *segment = ui_build_box_from_stringf(
                      UI_BOX_FLAG__DRAW_BACKGROUND,
                      "color_hue_segment_%p_%llu", entry, segment_idx);
                    ui_box_equip_background_colors(segment,
                      segment_top, segment_top,
                      segment_bottom, segment_bottom);
                  }
                }
              }

              ui_spacer(ui_px(8.0f, 1.0f));
            }
          }
        }

        F4 hue_color = lister_rgb_from_hsv((F4){lister_state.color_hue, 1.0f, 1.0f, 1.0f});
        if (sv_box != 0) {
          ui_box_equip_background_colors(sv_box,
            (F4){1.0f, 1.0f, 1.0f, 1.0f}, hue_color,
            (F4){0.0f, 0.0f, 0.0f, 1.0f}, (F4){0.0f, 0.0f, 0.0f, 1.0f});
        }
        if (preview_box != 0) {
          ui_box_equip_background_color(preview_box, as(entry->first_value, F4)[0]);
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
                ui_set_next_background_color((F4){0.008402845f, 0.044952845f, 0.123825366f, 1.0f});
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
        ui_set_next_background_color((F4){0.038710978f, -0.002232542f, -0.000659834f, 1.0f});
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
          ui_set_next_background_color((F4){0.037265774f, 0.381785296f, 0.004780161f, 1.0f});
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
