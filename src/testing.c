#if  (CPU_ && RAM_)

#endif

#if (CPU_ && ROM_)

Internal void lane(Arena *arena) {
	OS_Window *window = 0;
	GFX_Window *gfx_window = 0;
	GFX_Texture *texture = 0;

	////////////////////////////////
	//~ kti: Initialization.

	if (lane_idx() == 0) {

		gfx_init();

		window = os_window_open(arena, Str8_("Testing"), 1280, 720);
		gfx_window = gfx_window_equip(window);

		I1 pixels[256*256];
		for EachIndex(y, 256) {
			for EachIndex(x, 256) {
				pixels[y*256+x] = x<<16 | y | 0xFF000000;
			}
		}
		texture = gfx_tex2d_alloc(256, 256, pixels);
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

		  F4 top = oklch(1.0, 0.0, 0, 1.0f);
		  F4 bottom = oklch(1.0, 0.0, 0, 1.0f);
		  F4 border = oklch(1.00, 0.0, 0, 1.0f);

		  F1 height = 200.0f;
		  F1 width = height*GOLDEN_RATIO;
		  F1 x = (sinf(time)*0.5f+0.5f) * (window->width-width);
		  F1 y = 100.0f;

		  GFX_Rect_Instance first_instances[] = {
		    {
		      .src_rect = (F4){
		      	0, 0,
	          texture->width, texture->height
	        },
		      .dst_rect = (F4){
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
		    {
		    	.dst_rect = (F4){
		    		100.0f, 300.0f,
		    		400.0f, 300.0f,
		    	},
		    	.colors = {
		    		bg, bg,
		    		bg, bg,
		    	},
		    	.omit_texture = 1.0f,
		    }
		  };
		  GFX_Batch first_batch = {
		  	.texture = texture,
		  	.instances = first_instances,
		  	.instance_count = ArrayCount(first_instances),
		  };

		  GFX_Rect_Instance second_instances[] = {
		  	{
		  		.dst_rect = (F4){
		  			x, y+300.0f,
		  			width, height
		  		},
		  		.colors = {
		  			top, top,
		  			bottom, bottom,
		  		},
		  		.omit_texture = 1.0f,
		  	}
		  };
		  GFX_Batch second_batch = {
		  	.clip_rect = (F4){
		  		100.0f, 300.0f,
			  	400.0f, 300.0f
			  },
		  	.instances = second_instances,
		  	.instance_count = ArrayCount(second_instances),
		  };
		  first_batch.next = &second_batch;

		  GFX_BatchList batches = {
		  	.first = &first_batch,
		  	.last = &second_batch,
		  };

			gfx_window_submit(window, gfx_window, batches);

			gfx_window_end_frame(window, gfx_window);
		}

		scratch_end(scratch);

		lane_sync();

		L1 target_frame_time = 1000000000ULL / 60;
		L1 frame_end_time = os_clock();
		L1 frame_time = frame_end_time - frame_begin_time;

		// if (lane_idx() == 0) {
		// 	F1 frame_time_ms = (F1)(frame_time / 1000ULL) / 1000.0f;
		// 	printf("%.2fms\n", frame_time_ms);
		// }

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
		gfx_tex2d_free(texture);
		gfx_window_unequip(gfx_window);
		os_window_close(window);
	}
}

#endif