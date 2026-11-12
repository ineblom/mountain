typedef struct N_Block N_Block;
struct N_Block
{
  N_Block *next;
  N_Block *prev;

  B1 *buffer;
  L1  length;
  L1  capacity;
};

typedef struct N_State N_State;
struct N_State
{
  Arena *arena;

  N_Block *first_block;
  N_Block *last_block;
  N_Block *free_block;

  Txt_Pt cursor;
  Txt_Pt mark;
};

//- kti: Block ops
typedef I1 N_Op_Kind;
enum
{
  N_OP_KIND__NONE,
  N_OP_KIND__SPLIT,
  N_OP_KIND__MERGE_PREV,
  N_OP_KIND__MERGE_NEXT,
  N_OP_KIND__MOVE_PREV,
  N_OP_KIND__MOVE_NEXT,
};

typedef struct N_Op N_Op;
struct N_Op
{
  N_Op_Kind kind;
  N_Block  *block;

  I1 is_vertical;
};

////////////////////////////////
//~ kti: Blocks

Internal N_Block *n_block_alloc(N_State *state)
{
  N_Block *block = state->free_block;

  if (block != 0)
  {
    state->free_block = block->next;
  }
  else
  {
    block            =  push_array(state->arena, N_Block, 1);
    block->capacity  =  KiB(4);
    block->buffer    =  push_array(state->arena, B1, block->capacity);
  }

  block->next    =  0;
  block->prev    =  0;
  block->length  =  0;

  return block;
}

Internal void n_block_release(N_State *state, N_Block *block)
{
  DLLRemove(state->first_block, state->last_block, block);

  block->prev        =  0;
  block->next        =  state->free_block;
  state->free_block  =  block;
}

Internal N_Block *n_block_split(N_State *state, N_Block *block, L1 min, L1 max)
{
  max  =  Min(max, block->length);
  min  =  Min(min, max);

  N_Block *new_block = n_block_alloc(state);

  new_block->length = Min(block->length - max, new_block->capacity);

  memmove(new_block->buffer, block->buffer + max, new_block->length);

  block->length = min;

  DLLInsert(state->first_block, state->last_block, block, new_block);

  return new_block;
}

Internal L1 n_block_merge(N_State *state, N_Block *dst, N_Block *src)
{
  L1 at       = dst->length;
  L1 copy_len = Min(src->length, dst->capacity - dst->length);

  memmove(dst->buffer + dst->length, src->buffer, copy_len);

  dst->length += copy_len;

  n_block_release(state, src);

  return at;
}

Internal String8 n_key_string_from_block(Arena *arena, N_Block *block)
{
  return str8f(arena, "##block%p", block);
}

void notes_main(void *)
{
  Arena *arena = worker_arena();

  //- kti: Init.

  gfx_init();
  fp_init();
  fc_init();

  OS_Window  *os_window  = os_window_open(str8("Notes"), 1280, 720);
  GFX_Window *gfx_window = gfx_window_equip(os_window);
  UI_State   *ui_state   = ui_state_alloc();

  UI_Theme_Pattern theme_patterns[] = {
    UI_ThemeColor(0.01f, 0.01f, 0.01f, 1.0f,  str8("background")),
    UI_ThemeColor(0.10f, 0.10f, 0.10f, 1.0f,  str8("border")),
    UI_ThemeColor(0.60f, 0.00f, 0.00f, 1.0f,  str8("text")),
    UI_ThemeColor(0.60f, 0.60f, 0.60f, 1.0f,  str8("cursor")),

    //- kti: Header
    UI_ThemeColor(1.00f, 1.00f, 1.00f, 1.0f,  str8("header"), str8("text")),

    //- kti: Button
    UI_ThemeColor(0.60f, 0.60f, 0.60f, 1.0f,  str8("button"), str8("text")),

    //- kti: Textedit
    UI_ThemeColor(0.10f, 0.30f, 0.90f, 0.2f,  str8("selection")),
    UI_ThemeColor(0.60f, 0.60f, 0.60f, 1.0f,  str8("textedit"), str8("text")),
    UI_ThemeColor(0.10f, 0.30f, 0.90f, 1.0f,  str8("textedit"), str8("focused"), str8("border")),
  };

  UI_Theme theme = {
    .patterns = theme_patterns,
    .pattern_count = ArrayCount(theme_patterns),
  };

  L1 frames_requested = 2;
  
  N_State state = {.arena = arena};

  //- kti: First block.
  {
    N_Block *block = n_block_alloc(&state);

    DLLPushBack(state.first_block, state.last_block, block);
  }

  N_Block *focus_block = state.first_block;

  L1 running = 1;

  while (running)
  {
    F1 animation_dt = 1.0f / 60.0f;

    Temp_Arena scratch = scratch_begin(0, 0);

    UI_Cmd_List ui_cmds = {0};

    OS_Event_List events = os_poll_events(scratch.arena, frames_requested == 0 ? -1 : 0);

    for (OS_Event *e = events.first; e != 0; e = e->next)
    {
      if (e->kind == OS_EVENT_KIND__WINDOW_CLOSE)
      {
        running = 0;
      }

      //- kti: Escape
      if (e->kind == OS_EVENT_KIND__PRESS && e->key == OS_KEY__ESC)
      {
        UI_Cmd cmd = {
          .kind = UI_CMD_KIND__CANCEL,
          .timestamp_ns = e->timestamp_ns,
        };
        ui_cmd_list_push(scratch.arena, &ui_cmds, cmd);
      }

      //- kti: Text.
      if (e->kind == OS_EVENT_KIND__TEXT)
      {
        UI_Cmd cmd = {
          .kind = UI_CMD_KIND__TEXT,
          .string = {e->text, e->text_len},
          .timestamp_ns = e->timestamp_ns,
        };
        ui_cmd_list_push(scratch.arena, &ui_cmds, cmd);
      }

      //- kti: Shift + Enter -> line break.
      if (e->kind == OS_EVENT_KIND__PRESS && e->key == OS_KEY__ENTER && e->modifiers == OS_MODIFIER_FLAG__SHIFT)
      {
        UI_Cmd cmd = {
          .kind = UI_CMD_KIND__TEXT,
          .string = str8("\n"),
          .timestamp_ns = e->timestamp_ns,
        };
        ui_cmd_list_push(scratch.arena, &ui_cmds, cmd);
      }

      //- kti: Up & Down
      if (e->kind == OS_EVENT_KIND__PRESS && (e->key == OS_KEY__UP || e->key == OS_KEY__DOWN))
      {
        UI_Cmd_Flags flags = 0;

        if (e->modifiers & OS_MODIFIER_FLAG__SHIFT)
        {
          flags |= UI_CMD_FLAG__KEEP_MARK;
        }

        UI_Cmd cmd = {
          .kind         = UI_CMD_KIND__NAVIGATE,
          .delta_unit   = UI_CMD_DELTA_UNIT__CHAR,
          .flags        = flags,
          .delta_si2    = {{0, (e->key == OS_KEY__UP) ? -1 : 1}},
          .timestamp_ns = e->timestamp_ns,
        };

        ui_cmd_list_push(scratch.arena, &ui_cmds, cmd);
      }

      //- kti: Left & Right
      if (e->kind == OS_EVENT_KIND__PRESS && (e->key == OS_KEY__LEFT || e->key == OS_KEY__RIGHT))
      {
        UI_Cmd_Delta_Unit delta_unit = UI_CMD_DELTA_UNIT__CHAR;
        UI_Cmd_Flags      flags      = UI_CMD_FLAG__CAP_AT_LINE;

        if (e->modifiers & OS_MODIFIER_FLAG__CTRL)
        {
          delta_unit = UI_CMD_DELTA_UNIT__WORD;
        }

        if (e->modifiers & OS_MODIFIER_FLAG__SHIFT)
        {
          flags |= UI_CMD_FLAG__KEEP_MARK;
        }
        else
        {
          flags |= UI_CMD_FLAG__PICK_SELECT_SIDE;
        }

        UI_Cmd cmd = {
          .kind         = UI_CMD_KIND__NAVIGATE,
          .delta_unit   = delta_unit,
          .flags        = flags,
          .delta_si2    = {{(e->key == OS_KEY__LEFT) ? -1 : 1, 0}},
          .timestamp_ns = e->timestamp_ns,
        };

        ui_cmd_list_push(scratch.arena, &ui_cmds, cmd);
      }

      //- kti: Home & End
      if (e->kind == OS_EVENT_KIND__PRESS && (e->key == OS_KEY__HOME || e->key == OS_KEY__END))
      {
        UI_Cmd_Flags flags = UI_CMD_FLAG__CAP_AT_LINE;

        if (e->modifiers & OS_MODIFIER_FLAG__SHIFT)
        {
          flags |= UI_CMD_FLAG__KEEP_MARK;
        }

        UI_Cmd cmd = {
          .kind         = UI_CMD_KIND__NAVIGATE,
          .delta_unit   = UI_CMD_DELTA_UNIT__LINE,
          .flags        = flags,
          .delta_si2    = {{(e->key == OS_KEY__HOME) ? -1 : 1, 0}},
          .timestamp_ns = e->timestamp_ns,
        };

        ui_cmd_list_push(scratch.arena, &ui_cmds, cmd);
      }

      //- kti: Backspace & Delete
      if (e->kind == OS_EVENT_KIND__PRESS && (e->key == OS_KEY__BACKSPACE || e->key == OS_KEY__DELETE))
      {
        UI_Cmd_Delta_Unit delta_unit = UI_CMD_DELTA_UNIT__CHAR;

        if (e->modifiers & OS_MODIFIER_FLAG__CTRL)
        {
          delta_unit = UI_CMD_DELTA_UNIT__WORD;
        }

        UI_Cmd cmd = {
          .kind         = UI_CMD_KIND__EDIT,
          .delta_unit   = delta_unit,
          .flags        = UI_CMD_FLAG__CAP_AT_LINE | UI_CMD_FLAG__ZERO_DELTA_ON_SELECT | UI_CMD_FLAG__DELETE,
          .delta_si2    = {{(e->key == OS_KEY__BACKSPACE) ? -1 : 1, 0}},
          .timestamp_ns = e->timestamp_ns,
        };

        ui_cmd_list_push(scratch.arena, &ui_cmds, cmd);
      }
    }

    ////////////////////////////////
    //~ kti: Build UI

    fc_frame();

    CString user_home = getenv("HOME");
    String8 prop_font_path  = str8f(scratch.arena, "%s/.local/share/fonts/Bloomberg-PropU_N.ttf", user_home);
    String8 fixed_font_path = str8f(scratch.arena, "%s/.local/share/fonts/Bloomberg-FixedU_N.ttf", user_home);

    FC_Tag prop_font  = fc_tag_from_path(prop_font_path);
    FC_Tag fixed_font = fc_tag_from_path(fixed_font_path);

    ui_state_equip(ui_state);
    ui_begin_build(os_window, events, ui_cmds, &theme, animation_dt);
    ui_push_font(prop_font);

    ui_spacer(ui_pct(0.5f, 0.0f));

    UI_Box *main_box = 0;

    N_Op    op              =  {0};
    FC_Tag  text_font       =  {0};
    F1      text_font_size  =  0;
    F1      text_tab_size   =  0;

    UI_Focus(UI_FOCUS_KIND__ON)
    {
      ui_set_next_pref_width(ui_pct(0.90f, 1.0f));
      ui_set_next_pref_height(ui_pct(1.0f, 1.0f));
      ui_set_next_child_layout_axis(AXIS__Y);
      main_box = ui_build_box_from_string(UI_BOX_FLAG__DEFAULT_FOCUS_NAV, str8("main_section"));
    }

    UI_Parent(main_box)
    UI_Pref_Width(ui_pct(1.0f, 1.0f))
    {
      UI_Font_Size(24.0f)
      UI_Text_Align(UI_TEXT_ALIGN__CENTER)
      UI_Pref_Height(ui_text_dim(20.0f, 1.0f))
      UI_Tag(str8("header"))
      ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|UI_BOX_FLAG__DRAW_SIDE_BOTTOM, str8("Notes App"));

      ui_spacer(ui_px(25.0f, 1.0f));

      text_font       =  ui_top_font();
      text_font_size  =  ui_top_font_size();
      text_tab_size   =  ui_top_tab_size();

      for (N_Block *block = state.first_block; block != 0; block = block->next)
      {
        String8  key_string  =  n_key_string_from_block(scratch.arena, block);
        UI_Key   key         =  ui_key_from_string(ui_active_seed_key(), key_string);
        I1       is_focused  =  ui_is_key_auto_focus_active(key);

        //- kti: Edits & moves leaving the block.
        if (is_focused && op.kind == N_OP_KIND__NONE)
        {
          UI_Cmd *cmd = ui_state->cmds.first;

          for (; cmd != 0; cmd = cmd->next)
          {
            if (cmd->kind == UI_CMD_KIND__EDIT || cmd->kind == UI_CMD_KIND__NAVIGATE || cmd->kind == UI_CMD_KIND__TEXT)
            {
              break;
            }
          }

          if (cmd != 0 && cmd->delta_si2.y == 0 && txt_pt_match(state.cursor, state.mark))
          {
            I1  at_start   =  (state.cursor.column == 0);
            I1  at_end     =  (state.cursor.column >= block->length);
            I1  is_delete  =  (cmd->kind == UI_CMD_KIND__EDIT && cmd->flags & UI_CMD_FLAG__DELETE);
            I1  is_move    =  (cmd->kind == UI_CMD_KIND__NAVIGATE
                               && !(cmd->flags & UI_CMD_FLAG__KEEP_MARK)
                               && (cmd->delta_unit == UI_CMD_DELTA_UNIT__CHAR || cmd->delta_unit == UI_CMD_DELTA_UNIT__WORD));

            if (cmd->delta_si2.x < 0 && at_start && block->prev != 0)
            {
              if (is_delete) { op = (N_Op){N_OP_KIND__MERGE_PREV, block}; }
              if (is_move)   { op = (N_Op){N_OP_KIND__MOVE_PREV, block}; }
            }

            if (cmd->delta_si2.x > 0 && at_end && block->next != 0)
            {
              if (is_delete) { op = (N_Op){N_OP_KIND__MERGE_NEXT, block}; }
              if (is_move)   { op = (N_Op){N_OP_KIND__MOVE_NEXT, block}; }
            }

            if (op.kind != N_OP_KIND__NONE)
            {
              ui_eat_cmd(cmd);
            }
          }
        }

        UI_Signal signal = {0};

        UI_Pref_Height(ui_em(1.8f, 1.0f))
        UI_Text_Padding(6.0f)
        {
          signal = ui_textedit(UI_TEXTEDIT_FLAG__FOCUS_ON_PRESS | UI_TEXTEDIT_FLAG__KEEP_FOCUS_ON_COMMIT |
                               UI_TEXTEDIT_FLAG__MULTILINE | UI_TEXTEDIT_FLAG__CURSOR_AT_MOUSE,
                               &state.cursor, &state.mark,
                               block->buffer, block->capacity,
                               &block->length,
                               (String8){block->buffer, block->length},
                               key_string);
        }

        if (is_focused && op.kind == N_OP_KIND__NONE)
        {
          //- kti: Enter -> split.
          if (signal.flags & UI_SIGNAL_FLAG__COMMIT)
          {
            op = (N_Op){N_OP_KIND__SPLIT, block};
          }

          //- kti: Up & down leaving the block.
          for (UI_Cmd *cmd = ui_state->cmds.first; cmd != 0 && op.kind == N_OP_KIND__NONE; cmd = cmd->next)
          {
            if (cmd->kind != UI_CMD_KIND__NAVIGATE || cmd->delta_si2.y == 0)
            {
              continue;
            }

            I1       is_up      =  (cmd->delta_si2.y < 0);
            N_Block  *neighbor  =  is_up ? block->prev : block->next;
            L1       edge       =  is_up ? 0 : block->length;

            if (cmd->flags & UI_CMD_FLAG__KEEP_MARK)
            {
              //- kti: Selections stay inside the block.
              state.cursor.column = edge;
            }
            else if (neighbor != 0)
            {
              op = (N_Op){is_up ? N_OP_KIND__MOVE_PREV : N_OP_KIND__MOVE_NEXT, block, .is_vertical = 1};
            }
            else
            {
              state.cursor.column  =  edge;
              state.mark           =  state.cursor;
            }

            ui_eat_cmd(cmd);
            break;
          }
        }
      }
    }

    ui_spacer(ui_pct(0.5f, 0.0f));

    ui_end_build();

    ////////////////////////////////
    //~ kti: Apply block edits

    if (op.kind != N_OP_KIND__NONE)
    {
      N_Block  *block      =  op.block;
      N_Block  *cursor_in  =  block;
      L1       cursor_at   =  state.cursor.column;
      I1       keep_x      =  0;

      switch (op.kind)
      {
        default: {} break;

        case N_OP_KIND__SPLIT:
        {
          cursor_in  =  n_block_split(&state, block, Min(state.cursor.column, state.mark.column), Max(state.cursor.column, state.mark.column));
          cursor_at  =  0;
        } break;

        case N_OP_KIND__MERGE_PREV:
        {
          cursor_in  =  block->prev;
          cursor_at  =  n_block_merge(&state, block->prev, block);
        } break;

        case N_OP_KIND__MERGE_NEXT:
        {
          cursor_at = n_block_merge(&state, block, block->next);
        } break;

        case N_OP_KIND__MOVE_PREV:
        case N_OP_KIND__MOVE_NEXT:
        {
          I1 is_prev = (op.kind == N_OP_KIND__MOVE_PREV);

          cursor_in  =  is_prev ? block->prev : block->next;
          cursor_at  =  is_prev ? cursor_in->length : 0;

          if (op.is_vertical)
          {
            String8            text   =  {cursor_in->buffer, cursor_in->length};
            UI_Txt_Line_Array  lines  =  ui_txt_lines_from_string(scratch.arena, text, 1);
            UI_Txt_Line        line   =  lines.v[is_prev ? lines.count - 1 : 0];

            cursor_at = line.min + fc_byte_offset_from_tag_size_string_x_offset(text_font,
                                                                                text_font_size,
                                                                                (F1)os_window->pixel_ratio,
                                                                                0,
                                                                                text_tab_size,
                                                                                str8_substr(text, line.min, line.max),
                                                                                ui_state->textedit_preferred_x);
            keep_x = 1;
          }
        } break;
      }

      state.cursor  =  (Txt_Pt){0, cursor_at};
      state.mark    =  state.cursor;
      focus_block   =  cursor_in;

      if (!keep_x)
      {
        String8            text   =  {cursor_in->buffer, cursor_in->length};
        UI_Txt_Line_Array  lines  =  ui_txt_lines_from_string(scratch.arena, text, 1);
        UI_Txt_Line        line   =  lines.v[ui_txt_line_idx_from_off(lines, cursor_at)];

        ui_state->textedit_preferred_x = fc_x_offset_from_tag_size_string_byte_offset(text_font,
                                                                                      text_font_size,
                                                                                      (F1)os_window->pixel_ratio,
                                                                                      0,
                                                                                      text_tab_size,
                                                                                      str8_substr(text, line.min, line.max),
                                                                                      cursor_at - line.min);
      }
    }

    //- kti: Hand over focus.
    if (focus_block != 0)
    {
      UI_Key key = ui_key_from_string(main_box->key, n_key_string_from_block(scratch.arena, focus_block));

      main_box->default_nav_focus_next_hot_key     =  key;
      main_box->default_nav_focus_next_active_key  =  key;

      focus_block = 0;
    }

    ////////////////////////////////
    //~ kti: Draw

    dr_begin_frame();
    gfx_window_begin_frame(os_window, gfx_window);

    DR_Bucket *bucket = dr_bucket_make();
    dr_push_bucket(bucket);

    ui_draw();

    dr_submit_bucket(os_window, gfx_window, bucket);
    dr_pop_bucket();
    gfx_window_end_frame(os_window, gfx_window);

    scratch_end(scratch);

    if (frames_requested > 0)
    {
      frames_requested -= 1;
    }

    if (ui_state->animation_active || events.count > 0)
    {
      frames_requested = 1;
    }
  }

  gfx_window_unequip(gfx_window);
  os_window_close(os_window);

}

int main(void)
{
  Worker_Group_Params main_group_params = {
    .count = 1,
    .proc = notes_main,

    .arena_size = GiB(1),
    .scratch_size = MiB(64),

    .worker_zero_on_caller = 1,
  };

  worker_group_launch(main_group_params);

  return 0;
}
