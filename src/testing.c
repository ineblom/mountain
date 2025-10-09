
#if (CPU_ && RAM_)

F1 cam_dist;
F1 cam_rot_x;
F1 cam_rot_y;

#endif

#if (CPU_ && ROM_)

void lane(L1 arena) {
	if (lane_idx() == 0) {
		L1 window = os_window_open(arena, "Hello", 1280, 720);

		ramR->cam_dist = 3.0f;
		ramR->cam_rot_x = 35.0f;
		ramR->cam_rot_y = -45.0f;

		F1 sidebar_w = 400.0f;
		
		while (TR_(OS_Window, window)->should_close == 0) {
			F1 time = F1_(os_clock() / 1000000) / 1000.0f;

			F1 prev_mx = ramR->mouse_x;
			F1 prev_my = ramR->mouse_y;
			os_poll_events();

			F1 delta_mx = ramR->mouse_x - prev_mx;
			F1 delta_my = ramR->mouse_y - prev_my;
			I1 left_pressed = os_mouse_button(0);
			I1 right_pressed = os_mouse_button(1);
			
			I1 key_esc = os_key(1);
			I1 key_space = os_key(57);
			I1 key_w = os_key(17);
			I1 key_s = os_key(31);
			
			F1 window_width = F1_(TR_(OS_Window, window)->width);
			F1 window_height = F1_(TR_(OS_Window, window)->height);

			if (ramR->focused_window == window) {
				if (key_esc) {
					break;
				}
				if (right_pressed) {
					ramR->cam_rot_y += delta_mx * 0.2f;
					ramR->cam_rot_x += delta_my * 0.2f;
				}

				ramR->cam_dist += ramR->scroll_y/20.0f;
				if (key_w) {
					ramR->cam_dist -= 0.1f;
				}
				if (key_s) {
					ramR->cam_dist += 0.1f;
				}
			}

			os_window_get_context(window);

			glEnable(GL_DEPTH_TEST);
      glViewport(sidebar_w, 0, window_width-sidebar_w, window_height);
			F1 aspect = (window_width-sidebar_w)/window_height;
			
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			F1 near = 0.1f;
			F1 far = 40.0f;
			F1 scale_factor = near / 1.0f;
			glFrustum(
				-aspect*scale_factor, aspect*scale_factor,
				-1 * scale_factor, 1 * scale_factor,
				near, far);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(0, 0, -ramR->cam_dist);
			
			glRotatef(ramR->cam_rot_x, 1, 0, 0);
			glRotatef(ramR->cam_rot_y, 0, 1, 0);

		  // grid
		  glBegin(GL_LINES);
	  		glColor3f(0.2f, 0.2f, 0.2f);
	  		I1 grid_s = 25;
	  		F1 square_s = 0.25f;
	  		F1 half_grid_w = 0.5f*grid_s*square_s;
		  	for (I1 i = 0; i <= grid_s; i++) {
		  		F1 x = i * square_s - half_grid_w;
		  		glVertex3f(x, 0.0f, -half_grid_w);
		  		glVertex3f(x, 0.0f, half_grid_w);

		  		glVertex3f(-half_grid_w, 0.0f, x);
		  		glVertex3f(half_grid_w, 0.0f, x);
		  	}
		  glEnd();

			// axes
			glBegin(GL_LINES);
				glColor3f(0.0f, 0.0f, 1.0f);
				glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);

				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex3f(0, 0, 0); glVertex3f(0, 1, 0);

				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex3f(0, 0, 0); glVertex3f(0, 0, 1);
		  glEnd();

		  // triangle

		  glBegin(GL_TRIANGLES);
		  	glColor3f(1, 0, 0);
		  	glVertex3f(0.5f, 0.5f, 1.0f);

		  	glColor3f(0, 1, 0);
		  	glVertex3f(1.0f, 1.5f, 1.0f);

		  	glColor3f(0, 0, 1);
		  	glVertex3f(1.5f, 0.5f, 1.0f);
		  glEnd();

		  glViewport(0, 0, window_width, window_height);
		  glDisable(GL_DEPTH_TEST);

		  glMatrixMode(GL_PROJECTION);
		  glLoadIdentity();
		  glOrtho(0, window_width, window_height, 0.0f, -1.0f, 1.0f);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			// glEnable(GL_BLEND);
			// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

		  glBegin(GL_TRIANGLE_STRIP);
			  glColor3f(0.1f, 0.1f, 0.1f);
			  glVertex2f(0, 0);
			  glVertex2f(sidebar_w, 0);
			  glVertex2f(0, window_height);
			  glVertex2f(sidebar_w, window_height);
		  glEnd();

		  // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		  // glDisable(GL_BLEND);

		  glFlush();
			
			os_window_swap_buffers(window);
		}
		
		os_window_close(window);
	}
}


#endif
