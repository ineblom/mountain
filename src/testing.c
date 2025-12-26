#if  (CPU_ && RAM_)

#endif

#if (CPU_ && ROM_)

Internal void lane(Arena *arena) {
  OS_Window *window = 0;
  GFX_Window *gfx_window = 0;
	FC_Tag noto_tag = {0};

	L1 frame_count = 0;
	L1 total_frame_time = 0;
	L1 min_frame_time = L1_MAX;
	L1 max_frame_time = 0;

	////////////////////////////////
	//~ kti: Initialization.

	if (lane_idx() == 0) {
		//- kti: System init.
		gfx_init();
    fp_init();
		fc_init();

		//- kti: Window
		window = os_window_open(arena, Str8_("Testing"), 1280, 720);
		gfx_window = gfx_window_equip(window);

		//- kti: Font cache
    noto_tag = fc_tag_from_path(Str8_("/usr/share/fonts/noto/NotoSans-Regular.ttf"));
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
		lane_sync_L1((void *)&window, 0);

		//- kti: Wait for all lanes to finish their work.
		lane_sync();

		//- kti: Lane 0 renders the frame.
		if (lane_idx() == 0) {
			fc_frame();
			gfx_window_begin_frame(window, gfx_window);

		  F4 white = oklch(1.0, 0.0, 0, 1.0f);
		  F4 bg = oklch(0.186f, 0.027f, 343.0f, 1.0f);
		  F4 blue = oklch(0.425f, 0.152f, 152.0f, 1.0f);


			FC_Run run = fc_run_from_string(noto_tag, 32.0f, 0.0f, 100.0f, Str8_("Gå iväg!"));
			run = fc_run_from_string(noto_tag, 32.0f, 0.0f, 100.0f, Str8_("Gå iväg!"));

			L1 instance_count = 0;
		  GFX_Rect_Instance instances[16];

			instances[instance_count] = (GFX_Rect_Instance){
				.dst_rect = {
					0, 0,
					window->width, window->height
				},
				.colors = {
					bg, bg,
					bg, bg,
				},
				.omit_texture = 1.0f,
			};
			instance_count += 1;

			F2 cursor = {100.0f, 100.0f};
			instances[instance_count] = (GFX_Rect_Instance){
				.dst_rect = {
					cursor.x, cursor.y - (run.ascent+run.descent),
					run.dim.x, run.dim.y
				},
				.colors = {
					blue, blue,
					blue, blue,
				},
				.omit_texture = 1.0f,
			};
			instance_count += 1;
			
			GFX_Texture *texture = run.pieces.v[0].texture;
			for EachIndex(i, run.pieces.count) {
				SW4 subrect = run.pieces.v[i].subrect;
				SW2 offset = run.pieces.v[i].offset;
				instances[instance_count]	= (GFX_Rect_Instance){
					.dst_rect = {
						cursor.x+offset.x, cursor.y+offset.y,
						subrect.z, subrect.w
					},
					.src_rect = {
						subrect.x, subrect.y,
						subrect.z, subrect.w
					},
					.colors = { white, white, white, white },
				};
				instance_count += 1;
				cursor.x += run.pieces.v[i].advance;
			}

		  GFX_Batch first_batch = {
		  	.texture = texture,
		  	.instances = instances,
		  	.instance_count = instance_count,
		  };
		  GFX_BatchList batches = {
		  	.first = &first_batch,
		  	.last = &first_batch,
		  };
			gfx_window_submit(window, gfx_window, batches);

			gfx_window_end_frame(window, gfx_window);
		}

		scratch_end(scratch);

		lane_sync();

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

	lane_sync();

	////////////////////////////////
	//~ kti: Shutdown

	if (lane_idx() == 0) {
		gfx_window_unequip(gfx_window);
		os_window_close(window);
	}
}

#endif
