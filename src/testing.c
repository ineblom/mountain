#if (TYP_)

typedef struct Panel Panel;
struct Panel {
	Panel *first;
	Panel *last;
	Panel *next;
	Panel *prev;
	Panel *parent;
	F1 pct_of_parent;
	UI_Axis split_axis;
};

typedef struct Panel_Rec Panel_Rec;
struct Panel_Rec {
	Panel *next;
	I1 push_count;
	I1 pop_count;
};

typedef I1 Dir;
enum {
	DIR__RIGHT = 0,
	DIR__UP,
	DIR__LEFT,
	DIR__DOWN,
	DIR_COUNT,
};

typedef I1 Cmd_Kind;
enum {
	CMD_KIND__NONE = 0,

	CMD_KIND__OPEN_RIGHT,

	CMD_KIND_COUNT,
};

typedef struct Cmd Cmd;
struct Cmd {
	Cmd_Kind kind;

	Panel *panel;
};

typedef struct Window Window;
struct Window {
	Window *next;
	Window *prev;
	OS_Window *os;
	GFX_Window *gfx;
	UI_State *ui;
	Arena *arena;
	Panel root_panel;
};

typedef struct State State;
struct State {
	Arena *arena;
	Window *first_window;
	Window *last_window;
	Window *free_window;
	Panel *free_panel;
};

#endif

#if (ROM_)

Global State *state = 0;

Internal Panel_Rec panel_rec_depth_first_pre_order(Panel *panel) {
	Panel_Rec rec = {0};

	if (panel->first) {
		rec.next = panel->first;
		rec.push_count = 1;
	} else for (Panel *p = panel; p != 0; p = p->parent) {
		if (p->next) {
			rec.next = p->next;
			break;
		}
		rec.pop_count += 1;
	}

	return rec;
}

Internal F4 panel_rect_from_parent_rect(Panel *child, F4 parent_rect) {
	F4 result = parent_rect;
	Panel *parent = child->parent;
	if (parent) {
		for (Panel *p = parent->first; p != child && p != 0; p = p->next) {
			result[parent->split_axis] += p->pct_of_parent * parent_rect[2+parent->split_axis];
		}
		result[2+parent->split_axis] = child->pct_of_parent * parent_rect[2+parent->split_axis];
	}
	return result;
}

Internal F4 panel_rect_from_root_rect(Panel *panel, F4 root_rect) {
	Temp_Arena scratch = scratch_begin(0, 0);

	typedef struct Walk_Node Walk_Node;
	struct Walk_Node {
		Walk_Node *next;
		Panel *child;
	};
	Walk_Node *first_walk_node = 0;
	for (Panel *p = panel; p != 0 && p->parent != 0; p = p->parent) {
		Walk_Node *node = push_array(scratch.arena, Walk_Node, 1);
		node->child = p;
		SLLStackPush(first_walk_node, node);
	}

	F4 result = root_rect;
	for (Walk_Node *n = first_walk_node; n != 0; n = n->next) {
		result = panel_rect_from_parent_rect(n->child, result);
	}

	scratch_end(scratch);

	return result; 
}

Internal Panel *panel_alloc() {
	Panel *result = state->free_panel;
	if (result != 0) {
		SLLStackPop(state->free_panel);
		MemoryZeroStruct(result);
	} else {
		result = push_array(state->arena, Panel, 1);
	}
	return result;
}

Internal void panel_insert(Panel *panel, Panel *at, UI_Axis split_axis) {
	Panel *parent = at->parent;
	if (parent == 0) {
		panel->parent = at;
		panel->pct_of_parent = 1.0f;
		DLLPushBack(at->first, at->last, panel);
	} else if (parent->split_axis == split_axis) {
		panel->parent = parent;
		panel->pct_of_parent = at->pct_of_parent = at->pct_of_parent*0.5f;
		DLLInsert(parent->first, parent->last, at, panel);
	} else {
		Panel *container = panel_alloc();
		container->split_axis = split_axis;
		container->parent = parent;
		container->pct_of_parent = at->pct_of_parent;

		DLLInsert(parent->first, parent->last, at, container);
		DLLRemove(parent->first, parent->last, at);

		DLLPushBack(container->first, container->last, at);
		DLLPushBack(container->first, container->last, panel);

		at->parent = container;
		panel->parent = container;
		at->pct_of_parent = panel->pct_of_parent = 0.5f;
	}
}

Internal void panel_close(Panel *root, Panel *panel) {
	if (panel->first != 0) {
		return;
	}

	Panel *parent = panel->parent;

	if (panel->prev) {
		panel->prev->pct_of_parent += panel->pct_of_parent;
	} else if (panel->next) {
		panel->next->pct_of_parent += panel->pct_of_parent;
	}

	DLLRemove(parent->first, parent->last, panel);

	if (parent->first && parent->first == parent->last && parent != root) {
		Panel *grandparent = parent->parent;
		Panel *survivor = parent->first;
		survivor->parent = grandparent;
		survivor->pct_of_parent = parent->pct_of_parent;
		DLLInsert(grandparent->first, grandparent->last, parent, survivor);
		DLLRemove(grandparent->first, grandparent->last, parent);
		SLLStackPush(state->free_panel, parent);
	}

	SLLStackPush(state->free_panel, panel);
}

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
	window->arena = arena_alloc(MiB(32));
	window->root_panel.split_axis = UI_AXIS__X;
	panel_insert(panel_alloc(), &window->root_panel, 0);

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
				ui_state_equip(w->ui);
				ui_begin_build(w->os, events);
				ui_push_font(prop_fnt);
				ui_push_background_color((F4){0.2f, 0.0f, 0.0f, 1.0f});
				ui_push_border_color((F4){0.5f, 0.0f, 0.0f, 1.0f});

				F4 root_plane_rect = {0, 0, w->os->width, w->os->height};

				//- kti: Non leaf panel ui
				for (Panel *panel = w->root_panel.first; panel != 0; panel = panel_rec_depth_first_pre_order(panel).next) {
					F4 panel_rect = panel_rect_from_root_rect(panel, root_plane_rect);

					for (Panel *child = panel->first; child != 0 && child->next != 0; child = child->next) {
						F4 child_rect = panel_rect_from_parent_rect(child, panel_rect);
						F4 boundary_rect = child_rect;
						F1 boundary_w = 8;
						boundary_rect[panel->split_axis] += child_rect[2+panel->split_axis] - boundary_w*0.5f;
						boundary_rect[2+panel->split_axis] = boundary_w;
						ui_set_next_fixed_rect(boundary_rect);
						UI_Box *boundary_box = ui_build_box_from_stringf(UI_BOX_FLAG__CLICKABLE|UI_BOX_FLAG__FLOATING, "##panel_boundary_%p", child);
						UI_Signal sig = ui_signal_from_box(boundary_box);
						if (sig.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
							Panel *min_child = child;
							Panel *max_child = child->next;
							if (sig.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
								F2 drag_data = {min_child->pct_of_parent, max_child->pct_of_parent};
								ui_store_drag_struct(&drag_data);
							}
							F2 drag_data = ui_get_drag_struct(F2)[0];
							F2 drag_delta = ui_drag_delta();
							F1 min_child_pct__pre_drag = drag_data[0];
							F1 max_child_pct__pre_drag = drag_data[1];
							F1 min_child_px__pre_drag = min_child_pct__pre_drag*panel_rect[2+panel->split_axis];
							F1 max_child_px__pre_drag = max_child_pct__pre_drag*panel_rect[2+panel->split_axis];
							F1 min_child_px__post_drag = min_child_px__pre_drag + drag_delta[panel->split_axis];
							F1 max_child_px__post_drag = max_child_px__pre_drag - drag_delta[panel->split_axis];
							min_child->pct_of_parent = min_child_px__post_drag/panel_rect[2+panel->split_axis];
							max_child->pct_of_parent = max_child_px__post_drag/panel_rect[2+panel->split_axis];
						}
					}
				}

				Panel *panel_to_close = 0;

				//- kti: build all leaf panel ui
				for (Panel *panel = w->root_panel.first; panel != 0; panel = panel_rec_depth_first_pre_order(panel).next) {
					F4 panel_rect = panel_rect_from_root_rect(panel, root_plane_rect);

					//- kti: Build ui
					if (panel->first == 0) {
						ui_set_next_fixed_rect(rect_pad(panel_rect, -4.0f));
						ui_set_next_child_layout_axis(UI_AXIS__X);
						UI_Box *box = ui_build_box_from_stringf(
								UI_BOX_FLAG__DRAW_BACKGROUND|
								UI_BOX_FLAG__DRAW_BORDER|
								UI_BOX_FLAG__CLICKABLE|
								UI_BOX_FLAG__FLOATING|
								UI_BOX_FLAG__CLIP,
								"##panel_box_%p", panel);
						UI_Parent(box)
						UI_Padding(ui_px(8.0f, 1.0f))
						UI_Pref_Width(ui_pct(1.0f, 0.0f))
						UI_Column() {


							UI_Row() {
								UI_Pref_Width(ui_text_dim(0.0f, 1.0f))
								ui_build_box_from_stringf(UI_BOX_FLAG__DRAW_TEXT, "%p", panel);

								ui_spacer(ui_px(10.0f, 1.0f));

								UI_Pref_Width(ui_text_dim(20.0f, 1.0f))
								UI_Text_Align(UI_TEXT_ALIGN__CENTER) {
									if (ui_button(Str8_("Split X")).flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
										panel_insert(panel_alloc(), panel, UI_AXIS__X);
									}
									ui_spacer(ui_px(10.0f, 1.0f));
									if (ui_button(Str8_("Split Y")).flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
										panel_insert(panel_alloc(), panel, UI_AXIS__Y);
									}
									ui_spacer(ui_px(10.0f, 1.0f));
									if (ui_button(Str8_("Close")).flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
										panel_to_close = panel;
									}
								}
							}
						}
					}
				}

				if (panel_to_close) {
					panel_close(&w->root_panel, panel_to_close);
				}

				if (w->root_panel.first == 0) {
					UI_Text_Align(UI_TEXT_ALIGN__CENTER)
					UI_Pref_Width(ui_text_dim(20.0f, 1.0f)) {
						ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, Str8_("Last panel closed."));
						if (ui_button(Str8_("Open Panel")).flags & UI_SIGNAL_FLAG__LEFT_CLICKED) {
							panel_insert(panel_alloc(), &w->root_panel, 0);
						}
					}
				}

				ui_end_build();

				gfx_window_begin_frame(w->os, w->gfx);
				DR_Bucket *bucket = dr_bucket_make();
				dr_push_bucket(bucket);

				ui_draw();

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
		while (state->first_window != 0) {
			window_close(state->first_window);
		}

		ProfShutdown();
	}
}

#endif
