
#if (CPU_ && TYP_)

typedef struct Entity Entity;
struct Entity {
	F3 pos;
	F3 size;
};

#define MAX_ENTITY_COUNT 128

#endif

#if (CPU_ && RAM_)

F1 cam_dist;
F1 cam_rot_x;
F1 cam_rot_y;

L1 entity_count;
Entity entities[MAX_ENTITY_COUNT];

#endif

#if (CPU_ && ROM_)

void lane(L1 arena) {
	if (lane_idx() == 0) {
		L1 window = os_window_open(arena, "Hello", 1280, 720);
		os_window_get_context(window);

		ramR->cam_dist = 3.0f;
		ramR->cam_rot_x = 35.0f;
		ramR->cam_rot_y = -45.0f;

		ramR->entities[ramR->entity_count].pos = (F3){0, 0, 0};
		ramR->entities[ramR->entity_count].size = (F3){1, 1, 1};
		ramR->entity_count += 1;

		F1 sidebar_w = 400.0f;
		F1 near = 0.1f;
		F1 far = 40.0f;
		F1 scale_factor = near / 1.0f;

		// ---- FONT ----
		I1 num_letters = 26;
		I1 font_tex_w = 8*num_letters;
		I1 font_tex_h = 8;
		L1 tex_data[] = {
			0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FFFFFFFFFF0000, 0xFFFFFF0000000000, 0x0000FF000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0xFF0000000000FF00, 0x00FFFFFFFFFFFF00,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000, 0x000000FF0000FF00, 0x000000000000FF00, 0x00FFFF0000FFFF00, 0x00FF000000FFFF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0xFF0000000000FF00, 0x000000000000FF00,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000, 0x00000000FF00FF00, 0x000000000000FF00, 0x00FFFFFFFFFFFF00, 0x00FF0000FF00FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF000000FF0000, 0x0000000000FF0000,
			0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFFFF00, 0x000000000000FF00, 0x00FFFFFFFFFFFF00, 0x000000FF00000000, 0x00FF000000000000, 0x0000000000FFFF00, 0x000000000000FF00, 0x00FF00FFFF00FF00, 0x00FF00FF0000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF00000000FF00, 0x000000FFFF000000, 0x0000FF00FF000000, 0x00000000FF000000,
			0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FFFFFF0000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x00000000FF00FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FFFF000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF00FFFF00FF00, 0x0000FF0000FF0000, 0x000000FF00000000, 0x000000FF00000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x000000FF0000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x0000FF0000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x0000FF000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FFFF000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x000000FFFF000000, 0x00FFFF0000FFFF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000,
			0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x000000000000FF00, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FFFFFFFFFF0000, 0x0000FFFFFF000000, 0x00FF00000000FF00, 0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFF0000, 0x000000000000FF00, 0xFFFFFFFFFFFF0000, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x000000FF00000000, 0x0000FFFFFFFF0000, 0x000000FFFF000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FFFFFFFFFFFF00,
		};

		glEnable(GL_TEXTURE_2D);

		I1 font_tex = 0;
		glGenTextures(1, &font_tex);
		glBindTexture(GL_TEXTURE_2D, font_tex);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font_tex_w, font_tex_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tex_data);
		glBindTexture(GL_TEXTURE_2D, 0);

		// ---- MAIN LOOP ----

		I1 left_was_pressed = os_mouse_button(0);
		I1 right_was_pressed = os_mouse_button(0);

		while (TR_(OS_Window, window)->should_close == 0) {
			F1 time = F1_(os_clock() / 1000000) / 1000.0f;

			F1 prev_mx = ramR->mouse_x;
			F1 prev_my = ramR->mouse_y;
			os_poll_events();

			F1 delta_mx = ramR->mouse_x - prev_mx;
			F1 delta_my = ramR->mouse_y - prev_my;
			I1 left_pressed = os_mouse_button(0);
			I1 left_just_pressed = left_pressed && !left_was_pressed;
			I1 right_pressed = os_mouse_button(1);
			I1 right_just_pressed = right_pressed && !right_was_pressed;
			left_was_pressed = left_pressed;
			right_was_pressed = right_pressed;

			I1 key_esc = os_key(1);
			I1 key_space = os_key(57);
			I1 key_w = os_key(17);
			I1 key_s = os_key(31);

			F1 window_width = F1_(TR_(OS_Window, window)->width);
			F1 window_height = F1_(TR_(OS_Window, window)->height);

			F1 viewport_width = window_width-sidebar_w;
			F1 viewport_height = window_height;
			F1 viewport_aspect = viewport_width/viewport_height;

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

			// ---- SELECTION ----
			if (left_just_pressed) {
				F1 viewport_mx = ramR->mouse_x-sidebar_w;
				F1 viewport_my = ramR->mouse_y;
				F3 ray_origin = ;
				F3 ray_direction = ;

				for EachIndex(entity_index, ramR->entity_count) {
					Entity e = ramR->entities[entity_index];
					F3 aabb_min = e.pos - e.size;
					F3 aabb_max = e.pos + e.size;

					F1 t = ray_aabb_intersect(ray_origin, ray_direction, aabb_min, aabb_max);
					if (t > 0.0001f) {
						printf("Intersection\n");
					}
					printf("Missed\n");
				}
			}

			// ---- RENDERING ----

			glEnable(GL_DEPTH_TEST);
			glViewport(sidebar_w, 0, viewport_width, viewport_height);

			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(
				-viewport_aspect*scale_factor, viewport_aspect*scale_factor,
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

			// ---- WORLD AXES ----
			glBegin(GL_LINES);
				glColor3f(0.0f, 0.0f, 1.0f);
				glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);

				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex3f(0, 0, 0); glVertex3f(0, 1, 0);

				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex3f(0, 0, 0); glVertex3f(0, 0, 1);
			glEnd();

			// ---- ENTITIES ----
			for EachIndex(entity_index, ramR->entity_count) {
				Entity e = ramR->entities[entity_index];

				glBegin(GL_TRIANGLES);
					glColor3f(1, 0, 0);
					glVertex3f(e.pos.x + -0.5f, e.pos.y, e.pos.z);

					glColor3f(0, 1, 0);
					glVertex3f(e.pos.x, e.pos.y+1.0f, e.pos.z);

					glColor3f(0, 0, 1);
					glVertex3f(e.pos.x+0.5f, e.pos.y, e.pos.z);
				glEnd();
			}

			// ---- SIDE PANEL UI ----

			glViewport(0, 0, window_width, window_height);
			glDisable(GL_DEPTH_TEST);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, window_width, window_height, 0.0f, -1.0f, 1.0f);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			// sidebar background
			glBegin(GL_QUADS);
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex2f(0, 0);
				glVertex2f(sidebar_w, 0);
				glVertex2f(sidebar_w, window_height);
				glVertex2f(0, window_height);
			glEnd();

			// border line
			glBegin(GL_LINES);
				glColor3f(0.8f, 1.0f, 0.8f);
				glVertex2f(sidebar_w, 0);
				glVertex2f(sidebar_w, window_height);
			glEnd();

			// enable blending for text rendering
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
			glBindTexture(GL_TEXTURE_2D, font_tex);

			glBegin(GL_QUADS);
				glColor3f(1.0f, 0.0f, 0.0f);

				F1 x = 10.0f;
				F1 y = 10.0f;
				F1 w = 16.0f;
				F1 h = 16.0f;
				CString msg = "ENTITY";

				for EachIndex(i, cstr_len(msg)) {
					B1 c_i = msg[i] - 'A';
					F1 tw = 1.0f/F1_(num_letters);
					F1 t0 = c_i*tw;
					F1 t1 = c_i*tw+tw;
					glTexCoord2f(t0, 0); glVertex2f(x, y);
					glTexCoord2f(t1, 0); glVertex2f(x+w, y);
					glTexCoord2f(t1, 1); glVertex2f(x+w, y+h);
					glTexCoord2f(t0, 1); glVertex2f(x, y+h);

					x += w;
					if (i != 0 && i % 7 == 0) {
						x = 0;
						y += h + 10;
					}
				}

			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDisable(GL_BLEND);

			glFlush();

			os_window_swap_buffers(window);
		}

		os_window_close(window);
	}
}


#endif
