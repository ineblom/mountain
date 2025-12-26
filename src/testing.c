#if (CPU_ && ROM_)

Internal void lane(Arena *arena) {
  OS_Window *window = 0;
  GFX_Window *gfx_window = 0;

	L1 frame_count = 0;
	L1 total_frame_time = 0;
	L1 min_frame_time = L1_MAX;
	L1 max_frame_time = 0;

	////////////////////////////////
	//~ kti: Initialization.

	if (lane_idx() == 0) {
		//- kti: System init.
		prof_init();
		gfx_init();
    fp_init();
		fc_init();

		//- kti: Window
		window = os_window_open(arena, Str8_("Testing"), 1280, 720);
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

		//- kti: Wait for all lanes to finish their work.
		lane_sync();

		//- kti: Lane 0 renders the frame.
		if (lane_idx() == 0) {
		  F4 white = oklch(1.0, 0.0, 0, 1.0f);
		  F4 bg = oklch(0.186f, 0.027f, 343.0f, 1.0f);
		  F4 green = oklch(0.425f, 0.152f, 152.0f, 1.0f);

			fc_frame();
			gfx_window_begin_frame(window, gfx_window);
			dr_begin_frame();

			FC_Tag noto_tag = fc_tag_from_path(Str8_("/usr/share/fonts/noto/NotoSans-Regular.ttf"));
			FC_Run run = fc_run_from_string(noto_tag, 12.0f, 0.0f, 100.0f, Str8_("Hejsan!"));

			DR_Bucket *bucket = dr_bucket_make();
			dr_push_bucket(bucket);

			dr_rect((F4){0.0f, 0.0f, window->width, window->height}, bg, 0.0f, 0.0f);
			dr_rect((F4){100.0f, 100.0f, 100.0f, 100.0f}, green, 10.0f, 1.0f);

			dr_submit_bucket(window, gfx_window, bucket);

			gfx_window_end_frame(window, gfx_window);
		}

		scratch_end(scratch);

		ProfEnd();
		spall_buffer_flush(&spall_ctx, &spall_buffer);

		lane_sync();

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
	      printf("Avg: %.2fms  Min: %.2fms  Max: %.2fms  (%.1f fps)\n", avg_ms, min_ms, max_ms, 1000.0f/avg_ms);
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

		// lane_sync();
	}

	////////////////////////////////
	//~ kti: Shutdown

	spall_buffer_quit(&spall_ctx, &spall_buffer);

	lane_sync();

	if (lane_idx() == 0) {
		gfx_window_unequip(gfx_window);
		os_window_close(window);

		spall_quit(&spall_ctx);
	}
}

#endif
