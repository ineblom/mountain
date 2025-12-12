#if  (CPU_ && RAM_)

#endif

#if (CPU_ && ROM_)

Internal void lane(Arena *arena) {
	OS_Window *window = 0;
	GFX_Window *gfx_window = 0;

	////////////////////////////////
	//~ kti: Initialization.

	if (lane_idx() == 0) {

		gfx_init(arena);

		window = os_window_open(arena, Str8_("Testing"), 1280, 720);
		gfx_window = gfx_window_equip(arena, window);
	}

	lane_sync();

	L1 running = 1;

	////////////////////////////////
	//~ kti: Main loop

	while (running) {
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

		//- kti: Wait for all lanes to finish their work.
		lane_sync();

		//- kti: Lane 0 renders the frame.
		if (lane_idx() == 0) {
			gfx_window_begin_frame(window, gfx_window);

		  F4 bg = oklch(0.181f, 0.028f, 252.0, 1.0f);

		  F4 top    = oklch(0.4, 0.18, 30, 1.0f);
		  F4 bottom = oklch(0.3f, 0.15f, 25.0f, 1.0f);
		  F4 border = oklch(0.65, 0.22, 45, 1.0f);

		  F1 height = 200.0f;
		  F1 width = height*GOLDEN_RATIO;
		  F1 x = (sinf(time)*0.5f+0.5f) * (window->width-width);
		  F1 y = 100.0f;

		  GFX_Rect_Instance instances[] = {
		    {
		      .rect = (F4){
		      	0.0f, 0.0f,
	          window->width, window->height
	        },
		      .colors = { bg, bg, bg, bg },
		    },
		    {
		      .rect = (F4){
		      	x, y,
	          width, height
	        },
		      .colors = {
		        top, top,
		        bottom, bottom,
		      },
		      .corner_radii = (F4){10.0f, 10.0f, 10.0f, 10.0f},
		      .border_color = border,
		      .border_width = 1.0f,
		      .softness = 1.0f,
		    },
		  };

			gfx_window_submit(window, gfx_window, ArrayCount(instances), instances);

			gfx_window_end_frame(window, gfx_window);
		}

		scratch_end(scratch);

		lane_sync();

		L1 target_frame_time = 1000000000ULL / 60;
		L1 frame_end_time = os_clock();
		L1 frame_time = frame_end_time - frame_begin_time;
		if (frame_time < target_frame_time) {
			L1 remainder = target_frame_time - frame_time;
			os_sleep(remainder);
		}

		// lane_sync();
	}

	lane_sync();

	////////////////////////////////
	//~ kti: Shutdown

	if (lane_idx() == 0) {
		gfx_window_unequip(gfx_window);
		os_window_close(window);
	}
}

#endif