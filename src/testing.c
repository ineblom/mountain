#if (TYP_)

typedef struct Window Window;
struct Window {
	Window *next;
	Window *prev;
	OS_Window *os;
	GFX_Window *gfx;
	UI_State *ui;
};

typedef struct State State;
struct State {
	Arena *arena;
	Window *first_window;
	Window *last_window;
	Window *free_window;
};

#endif

#if (ROM_)

Internal void draw_ui(void);

Global State *state = 0;

Internal Window *window_open(void) {
	Window *window = state->free_window;
	if (window != 0) {
		SLLStackPop(state->free_window);
		MemoryZeroStruct(window);
	} else {
		window = push_array(state->arena, Window, 1);
	}

	window->os = os_window_open(Str8_("Testing"), 1280, 720);
	window->gfx = gfx_window_equip(window->os);
	window->ui = ui_state_alloc();
	DLLPushBack(state->first_window, state->last_window, window);
	return window;
}

Internal void window_close(Window *window) {
	DLLRemove(state->first_window, state->last_window, window);
	SLLStackPush(state->free_window, window);
	ui_state_release(window->ui);
	gfx_window_unequip(window->gfx);
	os_window_close(window->os);
}

Internal Window *window_from_os_window(OS_Window *os) {
	Window *result = 0;

	for (Window *w = state->first_window; w != 0; w = w->next) {
		if (w->os == os) {
			result = w;
		}
	}

	return result;
}

Internal void lane(Arena *arena) {
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

		state = push_array(arena, State, 1);
		state->arena = arena;
		
	  window_open();
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

		OS_Event_List events = {0};
		if (lane_idx() == 0) {
			events = os_poll_events(scratch.arena);
			for (OS_Event *e = events.first; e != 0; e = e->next) {
				if (e->type == OS_EVENT_TYPE__WINDOW_CLOSE) {
					Window *window = window_from_os_window(e->window);
					if (window != 0) {
						window_close(window);
					}
				}
			}

			if (state->first_window == 0) {
				running = 0;
			}
		}
		lane_sync_L1(&running, 0);

		//- kti: UI
		if (lane_idx() == 0) {
			fc_frame();
			dr_begin_frame();
			FC_Tag prop_fnt = fc_tag_from_path(Str8_("/usr/share/fonts/bloomberg/Bloomberg-PropU_B.ttf"));
			FC_Tag fixed_fnt = fc_tag_from_path(Str8_("/usr/share/fonts/bloomberg/Bloomberg-FixedU_B.ttf"));

			for (Window *w = state->first_window; w != 0; w = w->next) {
				gfx_window_begin_frame(w->os, w->gfx);

				ui_state_equip(w->ui);
				ui_begin_build(w->os, events);
				ui_push_font(prop_fnt);
				ui_push_background_color((F4){0.2f, 0.0f, 0.0f, 1.0f});

				UI_Text_Align(UI_TEXT_ALIGN__CENTER)
				UI_Pref_Width(ui_text_dim(20.0f, 1.0f)) UI_Pref_Height(ui_em(2.0f, 1.0f))
				if (ui_button(Str8_("Open Window")).flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
					window_open();
				}

				ui_end_build();

				DR_Bucket *bucket = dr_bucket_make();
				dr_push_bucket(bucket);

				draw_ui();

				//- kti: fps counter
				// FC_Run run = fc_run_from_string(fixed_fnt, 14.0f, 0.0f, 100.0f, str8f(scratch.arena, "%.1f fps", fps));
				// F4 white = oklch(1.0, 0.0, 0, 1.0f);
				// dr_text_run(run, (F2){0.0f, run.ascent}, white);

				dr_submit_bucket(w->os, w->gfx, bucket);
				dr_pop_bucket();

				gfx_window_end_frame(w->os, w->gfx);
			}
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
		for (Window *w = state->first_window; w != 0; w = w->next) {
			window_close(w);
		}

		ProfShutdown();
	}
}

Internal void draw_ui(void) {
	for (UI_Box *box = ui_root(); !ui_box_is_nil(box);) {
		UI_Box_Rec rec = ui_box_rec_df_post(box, &ui_nil_box);

		// F1 softness = (box->corner_radii[0] > 0 || box->corner_radii[1] > 0 || box->corner_radii[2] > 0 || box->corner_radii[3] > 0) ? 1.0f : 0.0f;
		F1 softness = 1.0f;

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

			GFX_Rect_Instance *inst = dr_rect(box->rect, box->background_color, 0.0f, softness);
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
				GFX_Rect_Instance *inst = dr_rect(border_rect, (F4){0.0f}, 0.0f, softness);
				inst->corner_radii = b->corner_radii;
				inst->border_width = 1.0f;
				inst->border_color = b->border_color;
			}
		}

		box = rec.next;
	}
}

#endif
