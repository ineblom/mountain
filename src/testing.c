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

	F2 pane_pos = {30.0f, 50.0f};
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
			ui_set_next_pref_height(ui_pct(1.0f, 1.0f));
			UI_Child_Layout_Axis(UI_AXIS__Y)
			UI_Background_Color(oklch(0.0f, 0.0f, 0.0f, 1.0f))
			UI_Border_Color(oklch(1.0f, 0.0f, 0.0f, 1.0f))
			UI_Parent(ui_build_box_from_key(UI_BOX_FLAG__DRAW_BACKGROUND, ui_key_zero())) {

				ui_set_next_fixed_x(pane_pos[0]);
				ui_set_next_fixed_y(pane_pos[1]);
				ui_set_next_fixed_width(400.0f);
				ui_set_next_fixed_height(300.0f);
				ui_set_next_child_layout_axis(UI_AXIS__Y);
				ui_set_next_background_color(oklch(0.2f, 0.0f, 0.0f, 1.0f));
				UI_Box *pane = ui_build_box_from_string(
						UI_BOX_FLAG__FLOATING | UI_BOX_FLAG__CLICKABLE | UI_BOX_FLAG__DRAW_BACKGROUND |
						UI_BOX_FLAG__DRAW_BORDER | UI_BOX_FLAG__CLIP | UI_BOX_FLAG__VIEW_SCROLL | UI_BOX_FLAG__VIEW_CLAMP,
						Str8_("pane"));
				UI_Parent(pane) {
					ui_set_next_pref_width(ui_children_sum(1.0f));
					ui_set_next_pref_height(ui_children_sum(1.0f));
					UI_Row()
					UI_Padding(ui_em(1.0f, 1.0f)) {
						ui_set_next_pref_width(ui_children_sum(0.0f));
						ui_set_next_pref_height(ui_children_sum(1.0f));
						UI_Column()
						UI_Padding(ui_em(1.0f, 1.0f)) {
							UI_Text_Color(oklch(0.7706f, 0.1537f, 67.64f, 1.0f))
							UI_Pref_Width(ui_text_dim(0.0f, 1.0f))
							UI_Pref_Height(ui_text_dim(0.0f, 1.0f)) {
								ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, Str8_("My super awesome UI system."));
								ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, Str8_("This text is on another row."));
							}

							ui_spacer(ui_em(1.0f, 1.0f));

							UI_Pref_Width(ui_text_dim(20.0f, 1.0f))
							UI_Pref_Height(ui_em(2.0f, 1.0f))
							UI_Background_Color(oklch(0.335f, 0.151f, button_hue, 1.0f))
							UI_Text_Align(UI_TEXT_ALIGN__CENTER)
							UI_Corner_Radius(5.0f) {
								UI_Row() {
									if (ui_button(Str8_("Prev")).flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
										button_hue = fmodf(button_hue-30.0f, 360.0f);
									}
									ui_spacer(ui_px(10.0f, 1.0f));
									if (ui_button(Str8_("Next")).flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
										button_hue = fmodf(button_hue+30.0f, 360.0f);
									}
								}
							}

							ui_spacer(ui_em(1.0f, 1.0f));

							ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
							ui_set_next_pref_height(ui_children_sum(1.0f));
							ui_slider_F1(Str8_("HUE"), &button_hue, 0.0f, 360.0f);
						}
					}
				}

				UI_Signal pane_signal = ui_signal_from_box(pane);
				if (pane_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
					if (pane_signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
						ui_store_drag_struct(&pane_pos);
					}

					F2 original_pos = ui_get_drag_struct(F2)[0];
					pane_pos = original_pos + ui_drag_delta();
					pane->fixed_pos = pane_pos;
				}

				//- kti: Pane bounds chcek
				F1 pane_bounds_animation_rate = 0.3f;
				F2 window_size = {window->width, window->height};
				for EachIndex(axis, UI_AXIS_COUNT) {
					F1 max = ClampBot(0, window_size[axis] - pane->fixed_size[axis]);
					F1 target = Clamp(0.0f, pane_pos[axis], max);

					pane_pos[axis] += pane_bounds_animation_rate * (target - pane_pos[axis]);
					if (abs_F1(target-pane_pos[axis]) < 2.0f) {
						pane_pos[axis] = target;
					}
				}
			}

			ui_end_build();

			fc_frame();
			gfx_window_begin_frame(window, gfx_window);
			dr_begin_frame();

			DR_Bucket *bucket = dr_bucket_make();
			dr_push_bucket(bucket);

			//- kti: ui
			for (UI_Box *box = ui_root(); !ui_box_is_nil(box);) {
				UI_Box_Rec rec = ui_box_rec_df_post(box, &ui_nil_box);

				I1 draw_active_effect = (box->flags & UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS &&
						!ui_key_match(box->key, ui_key_zero()) &&
						ui_key_match(box->key, ui_active_key(OS_MOUSE_BUTTON__LEFT)));

				if (box->flags & UI_BOX_FLAG__DRAW_DROP_SHADOW) {
					F4 shadow = {
						box->rect[0] - 4,
						box->rect[1] - 4,
						box->rect[2] + 16,
						box->rect[3] + 16,
					};
					dr_rect(shadow, oklch(0, 0, 0, 1), 0.8f, 8.0f);
				}

				if (box->flags & UI_BOX_FLAG__DRAW_BACKGROUND) {
					I1 draw_hot_effect = (box->flags & UI_BOX_FLAG__DRAW_HOT_EFFECTS &&
							!ui_key_match(box->key, ui_key_zero()) &&
							ui_key_match(box->key, ui_hot_key()));
					if (draw_hot_effect && !draw_active_effect) {
						F4 shadow = {
							box->rect[0] - 4,
							box->rect[1] - 4,
							box->rect[2] + 16,
							box->rect[3] + 16,
						};
						dr_rect(shadow, oklch(0, 0, 0, 1), 0.8f, 8.0f);
					}

					GFX_Rect_Instance *inst = dr_rect(box->rect, box->background_color, 0.0f, 1.0f);
					inst->corner_radii = box->corner_radii;

					if (draw_active_effect) {
						inst->colors[0][0] = ClampBot(inst->colors[0][0]-0.1f, 0.0f);
						inst->colors[1][0] = ClampBot(inst->colors[1][0]-0.1f, 0.0f);
						inst->colors[2][0] = ClampTop(inst->colors[2][0]+0.1f, 1.0f);
						inst->colors[3][0] = ClampTop(inst->colors[3][0]+0.1f, 1.0f);
					} else if (draw_hot_effect) {
						inst->colors[0][0] = ClampTop(inst->colors[0][0]+0.1f, 1.0f);
						inst->colors[1][0] = ClampTop(inst->colors[1][0]+0.1f, 1.0f);
					}
				}

				if (box->flags & UI_BOX_FLAG__DRAW_TEXT) {
					for (DR_FRun_Node *n = box->display_fruns.first; n != 0; n = n->next) {
						F2 pos = ui_box_text_pos(box);
						F4 color = box->text_color;
						if (draw_active_effect) {
							color[0] = ClampBot(color[0]-0.2f, 0.0f);
						}
						dr_text_run(n->value.run, pos, color);
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
		  F4 white = oklch(1.0, 0.0, 0, 1.0f);
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
	      min_frame_time = L1_MAX;
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
