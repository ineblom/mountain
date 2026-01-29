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

	F1 button_hue = 0.0f;
	L1 running = 1;

	////////////////////////////////
	//~ kti: Main loop

  while (running) {
		ProfBegin("Frame");

		L1 frame_begin_time = os_clock();
		F1 time = (F1)(frame_begin_time/1000000ULL) / 1000.0f;

		Temp_Arena scratch = scratch_begin(0, 0);

		OS_Event_List events = {0};
		if (lane_idx() == 0) {
			events = os_poll_events(scratch.arena);
			for (OS_Event *e = events.first; e != 0; e = e->next) {
				if (e->type == OS_EVENT_TYPE__WINDOW_CLOSE) {
					running = 0;
				}
			}
		}
		lane_sync_L1(&running, 0);

		//- kti: UI
		if (lane_idx() == 0) {
			ui_begin_build(window, events);

			FC_Tag prop_fnt = fc_tag_from_path(Str8_("/usr/share/fonts/bloomberg/Bloomberg-PropU_B.ttf"));
			FC_Tag fixed_fnt = fc_tag_from_path(Str8_("/usr/share/fonts/bloomberg/Bloomberg-FixedU_B.ttf"));
			ui_push_font(prop_fnt);

			ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
			ui_set_next_pref_height(ui_children_sum(1.0f));
			ui_set_next_child_layout_axis(UI_AXIS__Y);
			UI_Box *b1 = ui_build_box_from_key(0, ui_key_zero());
			ui_push_parent(b1);

			ui_spacer(ui_em(1.8f, 1.0f));

			ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
			ui_set_next_pref_height(ui_children_sum(1.0f));
			ui_set_next_child_layout_axis(UI_AXIS__X);
			UI_Box *row = ui_build_box_from_key(0, ui_key_zero());
			ui_push_parent(row); {
				ui_spacer(ui_pct(1.0f, 0.0f));
				
				ui_set_next_pref_width(ui_text_dim(20.0f, 1.0f));
				ui_set_next_pref_height(ui_em(2.0f, 1.0f));
				ui_set_next_border_color(oklch(0.933f, 0.063f, 25.0f, 1.0f));
				ui_set_next_background_color(oklch(0.435f, 0.151f, button_hue, 1.0f));
				ui_set_next_text_align(UI_TEXT_ALIGN__CENTER);
				ui_set_next_corner_radius(5.0f);

				if (ui_button(Str8_("Press me")).flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
					button_hue = fmodf(button_hue+40.0f, 360.0f);
				}

				ui_spacer(ui_pct(1.0f, 0.0f));
			} ui_pop_parent();

			ui_pop_parent();
			ui_end_build();

		  F4 white = oklch(1.0, 0.0, 0, 1.0f);
		  F4 bg = oklch(0.186f, 0.027f, 343.0f, 1.0f);

			fc_frame();
			gfx_window_begin_frame(window, gfx_window);
			dr_begin_frame();

			DR_Bucket *bucket = dr_bucket_make();
			dr_push_bucket(bucket);

			//- kti: bg
			dr_rect((F4){0.0f, 0.0f, window->width, window->height}, bg, 0.0f, 0.0f);

			//- kti: ui
			for (UI_Box *box = ui_root(); !ui_box_is_nil(box);) {
				UI_Box_Rec rec = ui_box_rec_df_post(box, &ui_nil_box);

				if (box->flags & UI_BOX_FLAG__DRAW_BACKGROUND) {
					F4 color = box->background_color;
					if (ui_key_match(box->key, ui_hot_key())) {
						color[0] = ClampTop(color[0]+0.1f, 1.0f);
					}
					GFX_Rect_Instance *inst = dr_rect(box->rect, color, 0.0f, 1.0f);
					inst->corner_radii = box->corner_radii;
				}

				if (box->flags & UI_BOX_FLAG__DRAW_TEXT) {
					for (DR_FRun_Node *n = box->display_fruns.first; n != 0; n = n->next) {
						F2 pos = ui_box_text_pos(box);
						dr_text_run(n->value.run, pos, box->text_color);
					}
				}

				if (box->flags & UI_BOX_FLAG__CLIP) {
					F4 top_clip = dr_top_clip();
					F4 new_clip = (F4){box->rect[0]+1, box->rect[1]+1, box->rect[2]-2, box->rect[3]-2};
					if (top_clip[2] != 0 || top_clip[3] != 0) {
						new_clip = intersect_rects(new_clip, top_clip);
					}
					dr_push_clip(new_clip);
				}

			  L1 pop_idx = 0;
				for (UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent) {
					pop_idx += 1;
					if (b == box && rec.push_count != 0) {
						continue;
					}

					if (b->flags & UI_BOX_FLAG__CLIP) {
						dr_pop_clip();
					}

					if (b->flags & UI_BOX_FLAG__DRAW_BORDER) {
						F4 border_rect = (F4){b->rect[0]-1, b->rect[1]-1, b->rect[2]+2, b->rect[3]+2};
						GFX_Rect_Instance *inst = dr_rect(border_rect, (F4){0.0f}, 0.0f, 1.0f);
						inst->corner_radii = b->corner_radii;
						inst->border_width = 1.0f;
						inst->border_color = b->border_color;
					}
				}

				box = rec.next;
			}

			//- kti: fps counter
			FC_Run run = fc_run_from_string(fixed_fnt, 14.0f, 0.0f, 100.0f, str8f(scratch.arena, "%.1f fps", fps));
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
