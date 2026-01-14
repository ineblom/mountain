#if (CPU_ && ROM_)

Internal void lane(Arena *arena) {
  OS_Window *window = 0;
  GFX_Window *gfx_window = 0;

	L1 frame_count = 0;
	L1 total_frame_time = 0;
	L1 min_frame_time = L1_MAX;
	L1 max_frame_time = 0;
	F1 fps = 0.0f;

	////////////////////////////////
	//~ kti: Initialization.

	if (lane_idx() == 0) {
		//- kti: System init.
		ProfInit();
		gfx_init();
		fp_init();
		fc_init();
		ui_init();

		//- kti: Window
		window = os_window_open(Str8_("Testing"), 1280, 720);
		gfx_window = gfx_window_equip(window);
	}

	lane_sync();

	L1 running = 1;

	////////////////////////////////
	//~ kti: Main loop

  while (running) {
		ProfBegin("Frame");

		L1 frame_begin_time = os_clock();
		F1 time = (F1)(frame_begin_time/1000000ULL) / 1000.0f;

		Temp_Arena scratch = scratch_begin(0, 0);

		if (lane_idx() == 0) {
			OS_Event *first_event = os_poll_events(scratch.arena);
			for (OS_Event *e = first_event; e != 0; e = e->next) {
				if (e->type == OS_EVENT_TYPE__WINDOW_CLOSE) {
					running = 0;
				}
			}
		}
		lane_sync_L1(&running, 0);

		//- kti: UI
		if (lane_idx() == 0) {
			ui_begin_build(window);

			UI_Box *b1 = ui_build_box_from_string(UI_BOX_FLAG__DRAW_BACKGROUND, Str8_("left container"));
			b1->pref_size[UI_AXIS__X] = (UI_Size){ .kind = UI_SIZE_KIND__PERCENT_OF_PARENT, .value = 0.5f };
			b1->pref_size[UI_AXIS__Y] = (UI_Size){ .kind = UI_SIZE_KIND__PERCENT_OF_PARENT, .value = 1.0f };

			ui_set_next_child_layout_axis(UI_AXIS__Y);
			UI_Box *b2 = ui_build_box_from_string(UI_BOX_FLAG__DRAW_BACKGROUND, Str8_("other container"));
			b2->pref_size[UI_AXIS__X] = (UI_Size){ .kind = UI_SIZE_KIND__CHILDREN_SUM };
			b2->pref_size[UI_AXIS__Y] = (UI_Size){ .kind = UI_SIZE_KIND__CHILDREN_SUM };

			ui_push_parent(b2);

			ui_set_next_fixed_width(200);
			ui_set_next_fixed_height(100);
			UI_Box *b3 = ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, Str8_("Hejsan!"));

			ui_set_next_fixed_width(100);
			ui_set_next_fixed_height(30);
			UI_Box *b4 = ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, Str8_("VA!"));

			ui_pop_parent();

			ui_end_build();
		}

		//- kti: Lane 0 renders the frame.
		if (lane_idx() == 0) {
		  F4 white = oklch(1.0, 0.0, 0, 1.0f);
		  F4 bg = oklch(0.186f, 0.027f, 343.0f, 1.0f);

			fc_frame();
			gfx_window_begin_frame(window, gfx_window);
			dr_begin_frame();

			FC_Tag noto_tag = fc_tag_from_path(Str8_("/usr/share/fonts/noto/NotoSans-Regular.ttf"));

			DR_Bucket *bucket = dr_bucket_make();
			dr_push_bucket(bucket);

			// kti: bg
			dr_rect((F4){0.0f, 0.0f, window->width, window->height}, bg, 0.0f, 0.0f);

			// kti: fps counter
			FC_Run run = fc_run_from_string(noto_tag, 16.0f, 0.0f, 100.0f, str8f(scratch.arena, "%.1f fps", fps));
			dr_text_run(run, (F2){0.0f, run.ascent}, white);

			dr_submit_bucket(window, gfx_window, bucket);
			dr_pop_bucket();

			gfx_window_end_frame(window, gfx_window);
		}

		scratch_end(scratch);

		ProfEnd();
		ProfFlush();

		//- kti: Calculate time spent and sleep until target frame time is met.
		L1 target_frame_time = 1000000000ULL / 60;
		L1 frame_end_time = os_clock();
		L1 frame_time = frame_end_time - frame_begin_time;

		if (lane_idx() == 0) {
			frame_count += 1;
			total_frame_time += frame_time;
			min_frame_time = Min(min_frame_time, frame_time);
			max_frame_time = Max(max_frame_time, frame_time);

			if (frame_count % 60 == 0) {
				F1 avg_ms = (total_frame_time / 60) / 1000000.0f;
	      F1 min_ms = min_frame_time / 1000000.0f;
	      F1 max_ms = max_frame_time / 1000000.0f;
				fps = 1000.0f/avg_ms;
	      printf("Avg: %.2fms  Min: %.2fms  Max: %.2fms  (%.1f fps)\n", avg_ms, min_ms, max_ms, fps);
	      total_frame_time = 0;
	      min_frame_time = UINT64_MAX;
	      max_frame_time = 0;
			}
		}

		if (frame_time < target_frame_time) {
			L1 remainder = target_frame_time - frame_time;
			if (remainder > 2000000ULL) {
				os_sleep(remainder - 2000000ULL);
			}
			while (os_clock() - frame_begin_time < target_frame_time) {}
		}

		lane_sync();
	}

	////////////////////////////////
	//~ kti: Shutdown

	lane_sync();

	if (lane_idx() == 0) {
		gfx_window_unequip(gfx_window);
		os_window_close(window);

		ProfShutdown();
	}
}

#endif
