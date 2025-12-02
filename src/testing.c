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
			gfx_begin_frame(gfx_window);
			gfx_end_frame(gfx_window);
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