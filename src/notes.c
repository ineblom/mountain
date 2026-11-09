typedef struct N_Block N_Block;
struct N_Block
{
  N_Block *next;
  N_Block *prev;
  String8 contents;
};

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
  Txt_Pt cursor = {0};
  Txt_Pt mark = {0};
  B1 input_buffer[1024] = {0};
  L1 input_buffer_len = 0;

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

      String8 welcome = str8("Welcome to the notes app. This is an example of a block!");
      ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, welcome);

      ui_spacer(ui_px(10.0f, 1.0f));

      UI_Row()
      UI_Pref_Height(ui_em(1.8f, 1.0f))
      {
        //- kti: Input
        I1 committed_input = 0;

        UI_Pref_Width(ui_pct(1.0f, 0.0f))
        UI_Text_Padding(6.0f)
        {
          UI_Signal input_signal = ui_textedit(&cursor, &mark,
                                               input_buffer, sizeof(input_buffer),
                                               &input_buffer_len,
                                               (String8){input_buffer, input_buffer_len},
                                               str8("##notes_input"));
          committed_input = input_signal.flags & UI_SIGNAL_FLAG__COMMIT;
        }

        ui_spacer(ui_px(10.0f, 1.0f));

        //- kti: Push button.
        UI_Pref_Width(ui_text_dim(20.0f, 1.0f))
        UI_Text_Align(UI_TEXT_ALIGN__CENTER)
        UI_Corner_Radius(2.5f)
        {
          UI_Signal button_signal = ui_button(str8("Push"));
          I1 pressed_push = button_signal.flags & UI_SIGNAL_FLAG__PRESSED;

          if (pressed_push || committed_input)
          {


            //- kti: Clear input.
            MemoryZeroArray(input_buffer);
            input_buffer_len = 0;
          }
        }
      }
    }

    ui_spacer(ui_pct(0.5f, 0.0f));

    ui_end_build();

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

    if (ui_state->animation_active || events.count > 0)
    {
      frames_requested = 2;
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
