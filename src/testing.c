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

		if (lane_idx() == 0) {
			gfx_window_begin_frame(window, gfx_window);

		  F4 bg = srgb_F4((F4){0.1f, 0.1f, 0.1f, 1.0f});

		  F4 dark_red = oklch(0.191f, 0.029f, 25.0f, 1.0f);
		  F4 light_red = oklch(0.1f, 0.5f, 20.0f, 1.0f);
		  F4 border = oklch(0.933f, 0.063f, 25.0f, 1.0f);

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
		      	100.0f, 100.0f,
	          300.0f, 500.0f
	        },
		      .colors = {
		        dark_red, dark_red,
		        light_red, light_red,
		      },
		      .corner_radii = (F4){10.0f, 10.0f, 10.0f, 10.0f},
		      .border_color = border,
		      .border_width = 1,
		    },
		  };

			gfx_window_submit(window, gfx_window, ArrayCount(instances), instances);

			gfx_window_end_frame(window, gfx_window);
		}

		scratch_end(scratch);
		lane_sync();
	}

	////////////////////////////////
	//~ kti: Shutdown

	if (lane_idx() == 0) {
		gfx_window_unequip(gfx_window);
		os_window_close(window);
	}
}

#endif