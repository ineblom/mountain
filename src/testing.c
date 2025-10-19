
#if (CPU_ && DEF_)

#define MAX_ENTITY_COUNT 128

#endif

#if (CPU_ && TYP_)

typedef struct Entity Entity;
struct Entity {
	F3 pos;
	F3 size;
	F3 color;
};

#endif

#if (CPU_ && RAM_)

F1 delta_mx;
I1 left_just_pressed;
I1 left_just_released;

L1 font_num_letters;
I1 font_tex;

F1 cam_dist;
F1 cam_rot_x;
F1 cam_rot_y;

L1 selected_entity_idx;
L1 entity_count;
Entity entities[MAX_ENTITY_COUNT];

L1 hot_hash;
L1 active_hash;

F1 panel_current_y;

#endif

#if (CPU_ && ROM_)

#define sidebar_w 400.0f

Internal F1 draw_text(String8 msg, F1 x, F1 y, F1 size, F3 color) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, ramR->font_tex);

	glBegin(GL_QUADS);
		glColor3f(color.x, color.y, color.z);

		F1 current_x = x;
		F1 current_y = y;

		for EachIndex(i, msg.len) {
			B1 c = B1R_(msg.str)[i];
			L1 c_i = 0;
			if (c >= '0' && c <= '9') {
				c_i = c - '0' + 26;
			} else if (c >= 'A' && c <= 'Z') {
				c_i = c - 'A';
			} else if (c == '-') {
				c_i = ramR->font_num_letters - 2;
			} else if (c == '.') {
				c_i = ramR->font_num_letters - 1;
			} else {
				current_x += size;
				continue;
			}

			F1 tw = 1.0f/F1_(ramR->font_num_letters);
			F1 t0 = c_i*tw;
			F1 t1 = c_i*tw+tw;

			glTexCoord2f(t0, 0); glVertex2f(current_x,      current_y);
			glTexCoord2f(t1, 0); glVertex2f(current_x+size, current_y);
			glTexCoord2f(t1, 1); glVertex2f(current_x+size, current_y+size);
			glTexCoord2f(t0, 1); glVertex2f(current_x,      current_y+size);

			current_x += size;
			if (i != 0 && i % 22 == 0) {
				current_x = x;
				y += size; // TODO: + line gap
			}
		}

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_BLEND);

	return current_y - y + size;
}

Internal void ui_text(String8 msg) {
	F3 color = (F3){0.5f, 0.0f, 0.0f};

	ramR->panel_current_y += 10.0f;
	F1 height = draw_text(msg, 10.0f, ramR->panel_current_y, 16.0f, color);
	ramR->panel_current_y += height;
}

Internal void ui_drag(L1 arena, String8 label, F1R value) {
	L1 hash = str8_hash(label);

	if (ramR->active_hash == 0 &&
			ramR->mouse_x > 10.0f && ramR->mouse_x < sidebar_w &&
			ramR->mouse_y > ramR->panel_current_y+10.0f && ramR->mouse_y < ramR->panel_current_y+10.0f+16.0f) {
		ramR->hot_hash = hash;
	}

	if (ramR->active_hash == hash) {
		value[0] += ramR->delta_mx*0.01f;

		if (ramR->left_just_released) {
			ramR->active_hash = 0;
		}
	} else if (ramR->hot_hash == hash) {
		if (ramR->left_just_pressed) {
			ramR->active_hash = hash;
		}
	}

	F3 color = (F3){0.3f, 0.0f, 0.8f};
	if (ramR->hot_hash == hash || ramR->active_hash == hash) {
		color *= 1.5f;
	}

	// cut label at the first instance of ##.
	for EachIndex(i, label.len-1) {
		if (B1R_(label.str)[i] == '#' && B1R_(label.str)[i+1] == '#') {
			label.len = i;
			break;
		}
	}

	ramR->panel_current_y += 10.0f;
	String8 text = str8f(arena, "%.*s %f", label.len, label.str, value[0]);
	F1 height = draw_text(text, 10.0f, ramR->panel_current_y, 16.0f, color);

	ramR->panel_current_y += height;
}

Internal void lane(L1 arena) {
	if (lane_idx() == 0) {
		L1 window = os_window_open(arena, "Hello", 1280, 720);
		os_window_get_context(window);

		ramR->cam_dist = 3.0f;
		ramR->cam_rot_x = 35.0f;
		ramR->cam_rot_y = -45.0f;

		ramR->entities[ramR->entity_count].pos = (F3){0, 0, 0};
		ramR->entities[ramR->entity_count].size = (F3){0.5f, 0.5f, 0.5f};
		ramR->entities[ramR->entity_count].color = (F3){1.0f, 1.0f, 0.0f};
		ramR->entity_count += 1;

		ramR->selected_entity_idx = L1_MAX;

		F1 near = 0.1f;
		F1 far = 40.0f;
		F1 scale_factor = near / 1.0f;

		// ---- FONT ----
		ramR->font_num_letters = 26 + 10 + 2;
		I1 font_tex_w = 8*ramR->font_num_letters;
		I1 font_tex_h = 8;
		L1 tex_data[] = {
			0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FFFFFFFFFF0000, 0xFFFFFF0000000000, 0x0000FF000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0xFF0000000000FF00, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x000000FFFF000000, 0x0000FFFFFFFF0000, 0x0000FFFFFFFF0000, 0x0000FF0000000000, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFF0000, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000, 0x000000FF0000FF00, 0x000000000000FF00, 0x00FFFF0000FFFF00, 0x00FF000000FFFF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0xFF0000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FFFF00000000, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000, 0x00000000FF00FF00, 0x000000000000FF00, 0x00FFFFFFFFFFFF00, 0x00FF0000FF00FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF000000FF0000, 0x0000000000FF0000, 0x00FF000000FFFF00, 0x000000FF00000000, 0x00FF000000000000, 0x00FF000000000000, 0x0000FF00FF000000, 0x000000000000FF00, 0x000000000000FF00, 0x0000FF0000000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFFFF00, 0x000000000000FF00, 0x00FFFFFFFFFFFF00, 0x000000FF00000000, 0x00FF000000000000, 0x0000000000FFFF00, 0x000000000000FF00, 0x00FF00FFFF00FF00, 0x00FF00FF0000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF00000000FF00, 0x000000FFFF000000, 0x0000FF00FF000000, 0x00000000FF000000, 0x00FF0000FF00FF00, 0x000000FF00000000, 0x0000FF0000000000, 0x0000FFFFFF000000, 0x0000FF0000FF0000, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x000000FF00000000, 0x0000FFFFFFFF0000, 0x00FFFFFFFFFF0000, 0x00FFFFFFFFFFFF00, 0x0000000000000000,
			0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FFFFFF0000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x00000000FF00FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FFFF000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF00FFFF00FF00, 0x0000FF0000FF0000, 0x000000FF00000000, 0x000000FF00000000, 0x00FF00FF0000FF00, 0x000000FF00000000, 0x000000FFFF000000, 0x00FF000000000000, 0x0000FF000000FF00, 0x00FF000000000000, 0x00FF00000000FF00, 0x00000000FF000000, 0x00FF00000000FF00, 0x00FF000000000000, 0x00FFFFFFFFFFFF00, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x000000FF0000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x0000FF0000000000, 0x00FFFF000000FF00, 0x000000FF00000000, 0x0000000000FF0000, 0x00FF000000000000, 0x00FFFFFFFFFFFF00, 0x00FF000000000000, 0x00FF00000000FF00, 0x00000000FF000000, 0x00FF00000000FF00, 0x00FF000000000000, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x0000FF000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FFFF000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x000000FFFF000000, 0x00FFFF0000FFFF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000, 0x00FF00000000FF00, 0x000000FF00000000, 0x000000000000FF00, 0x00FF00000000FF00, 0x0000FF0000000000, 0x00FF000000000000, 0x00FF00000000FF00, 0x00000000FF000000, 0x00FF00000000FF00, 0x00FF000000000000, 0x0000000000000000, 0x000000FFFF000000,
			0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x000000000000FF00, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FFFFFFFFFF0000, 0x0000FFFFFF000000, 0x00FF00000000FF00, 0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFF0000, 0x000000000000FF00, 0xFFFFFFFFFFFF0000, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x000000FF00000000, 0x0000FFFFFFFF0000, 0x000000FFFF000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x0000FFFFFF000000, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x0000FF0000000000, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x00000000FF000000, 0x0000FFFFFFFF0000, 0x0000FFFFFFFF0000, 0x0000000000000000, 0x000000FFFF000000,
		};

		glEnable(GL_TEXTURE_2D);

		glGenTextures(1, &ramR->font_tex);
		glBindTexture(GL_TEXTURE_2D, ramR->font_tex);
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

			ramR->delta_mx = ramR->mouse_x - prev_mx;
			F1 delta_my = ramR->mouse_y - prev_my;
			I1 left_pressed = os_mouse_button(0);
			ramR->left_just_pressed = left_pressed && !left_was_pressed;
			ramR->left_just_released = !left_pressed && left_was_pressed;
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
				ramR->cam_rot_y += ramR->delta_mx * 0.2f;
				ramR->cam_rot_x += delta_my * 0.2f;
			}

			ramR->cam_dist += ramR->scroll_y/20.0f;
			if (key_w) {
				ramR->cam_dist -= 0.1f;
			}
			if (key_s) {
				ramR->cam_dist += 0.1f;
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

			// ---- GRID ----
			glBegin(GL_LINES);
				glColor3f(0.1f, 0.1f, 0.1f);
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

			// ---- ENTITIES ----
			for EachIndex(entity_index, ramR->entity_count) {
				Entity e = ramR->entities[entity_index];

				glBegin(GL_QUADS);
					F3 color = e.color;
					glColor3f(color.x, color.y, color.z);

					glVertex3f(e.pos.x-e.size.x, e.pos.y+e.size.y, e.pos.z+e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y+e.size.y, e.pos.z+e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y-e.size.y, e.pos.z+e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y-e.size.y, e.pos.z+e.size.z);

					glVertex3f(e.pos.x-e.size.x, e.pos.y+e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y+e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y-e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y-e.size.y, e.pos.z-e.size.z);

					glVertex3f(e.pos.x-e.size.x, e.pos.y+e.size.y, e.pos.z+e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y+e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y-e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y-e.size.y, e.pos.z+e.size.z);

					glVertex3f(e.pos.x+e.size.x, e.pos.y+e.size.y, e.pos.z+e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y+e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y-e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y-e.size.y, e.pos.z+e.size.z);

					glVertex3f(e.pos.x+e.size.x, e.pos.y+e.size.y, e.pos.z+e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y+e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y+e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y+e.size.y, e.pos.z+e.size.z);

					glVertex3f(e.pos.x+e.size.x, e.pos.y-e.size.y, e.pos.z+e.size.z);
					glVertex3f(e.pos.x+e.size.x, e.pos.y-e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y-e.size.y, e.pos.z-e.size.z);
					glVertex3f(e.pos.x-e.size.x, e.pos.y-e.size.y, e.pos.z+e.size.z);

				glEnd();
			}

			// ---- SELECTION (DEPTH BUFFER PICKING) ----
			if (ramR->left_just_pressed && ramR->mouse_x >= sidebar_w) {
				// Read depth value at mouse position
				F1 depth;
				glReadPixels(
					I1_(ramR->mouse_x),
					I1_(window_height - ramR->mouse_y), // OpenGL y is bottom-up
					1, 1,
					GL_DEPTH_COMPONENT,
					GL_FLOAT,
					&depth
				);

				ramR->selected_entity_idx = L1_MAX;

				// Only proceed if we clicked on geometry (not the background)
				if (depth < 1.0f) {
					F1 viewport_mx = ramR->mouse_x - sidebar_w;
					F1 viewport_my = ramR->mouse_y;

					// Convert to normalized device coordinates
					F1 ndc_x = (viewport_mx / viewport_width) * 2.0f - 1.0f;
					F1 ndc_y = 1.0f - (viewport_my / viewport_height) * 2.0f;
					F1 ndc_z = depth * 2.0f - 1.0f; // Convert [0,1] to [-1,1]

					// Unproject to get world position
					// We need to invert: clip = projection * view * world
					// For our simple camera setup:

					// First, get view space position (inverse projection)
					F1 view_x = ndc_x * viewport_aspect * scale_factor / near * -1.0f; // perspective divide
					F1 view_y = ndc_y * scale_factor / near * -1.0f;
					// Linearize depth from perspective projection
					F1 view_z = -(2.0f * far * near) / (far + near - ndc_z * (far - near));

					// Scale by actual depth
					view_x *= view_z;
					view_y *= view_z;

					// Now transform from view space to world space (inverse view transform)
					F1 rot_x_rad = ramR->cam_rot_x * PI / 180.0f;
					F1 rot_y_rad = ramR->cam_rot_y * PI / 180.0f;
					F1 cos_x = cosf(rot_x_rad);
					F1 sin_x = sinf(rot_x_rad);
					F1 cos_y = cosf(rot_y_rad);
					F1 sin_y = sinf(rot_y_rad);

					// Inverse rotation: first inverse X rotation, then inverse Y rotation
					F1 temp_y = view_y * cos_x + view_z * sin_x;
					F1 temp_z = -view_y * sin_x + view_z * cos_x;

					F3 world_pos;
					world_pos.x = view_x * cos_y - temp_z * sin_y;
					world_pos.y = temp_y;
					world_pos.z = view_x * sin_y + temp_z * cos_y;

					// Inverse translation
					F3 cam_forward = {
						sin_y * cos_x,
						sin_x,
						-cos_y * cos_x
					};
					F3 cam_pos = cam_forward * -ramR->cam_dist;
					world_pos = world_pos + cam_pos;

					// Find which AABB contains or is closest to this world position
					F1 closest_dist = F1_MAX;
					for EachIndex(entity_index, ramR->entity_count) {
						Entity e = ramR->entities[entity_index];
						F3 aabb_min = e.pos - e.size;
						F3 aabb_max = e.pos + e.size;

						// Otherwise find closest AABB
						F3 closest_point;
						closest_point.x = world_pos.x < aabb_min.x ? aabb_min.x : (world_pos.x > aabb_max.x ? aabb_max.x : world_pos.x);
						closest_point.y = world_pos.y < aabb_min.y ? aabb_min.y : (world_pos.y > aabb_max.y ? aabb_max.y : world_pos.y);
						closest_point.z = world_pos.z < aabb_min.z ? aabb_min.z : (world_pos.z > aabb_max.z ? aabb_max.z : world_pos.z);

						F3 diff = world_pos - closest_point;
						F1 dist = F3_length(diff);

						if (dist < closest_dist) {
							closest_dist = dist;
							ramR->selected_entity_idx = entity_index;
						}
					}
				}
			}

			// ---- WORLD AXES ----
			glDisable(GL_DEPTH_TEST);
			glLineWidth(3.0f);
			glBegin(GL_LINES);
				glColor3f(0.0f, 0.0f, 1.0f);
				glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);

				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex3f(0, 0, 0); glVertex3f(0, 1, 0);

				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex3f(0, 0, 0); glVertex3f(0, 0, 1);
			glEnd();
			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);

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
				glColor3f(0.5f, 0.0f, 0.0f);
				glVertex2f(sidebar_w, 0);
				glVertex2f(sidebar_w, window_height);
			glEnd();

			ramR->panel_current_y = 0.0f;
			ramR->hot_hash = 0;

			L1 pos = arena_pos(arena);

			if (ramR->selected_entity_idx == L1_MAX) {
				ui_text(Str8_("NO ENTITY SELECTED"));
			} else {
				TR(Entity) e = &ramR->entities[ramR->selected_entity_idx];

				ui_text(str8f(arena, "ENTITY %llu", ramR->selected_entity_idx));

				ramR->panel_current_y += 16.0f;

				ui_text(Str8_("POSITION"));
				F1 x = e->pos.x, y = e->pos.y, z = e->pos.z;
				ui_drag(arena, Str8_(" X##pos"), &x);
				ui_drag(arena, Str8_(" Y##pos"), &y);
				ui_drag(arena, Str8_(" Z##pos"), &z);
				e->pos = (F3){x, y, z};

				ui_text(Str8_("SIZE"));
				x = e->size.x, y = e->size.y, z = e->size.z;
				ui_drag(arena, Str8_(" X##size"), &x);
				ui_drag(arena, Str8_(" Y##size"), &y);
				ui_drag(arena, Str8_(" Z##size"), &z);
				e->size = (F3){Max(0, x), Max(0, y), Max(z, 0)};

				ui_text(Str8_("COLOR"));
				x = e->color.x, y = e->color.y, z = e->color.z;
				ui_drag(arena, Str8_(" X##color"), &x);
				ui_drag(arena, Str8_(" Y##color"), &y);
				ui_drag(arena, Str8_(" Z##color"), &z);
				e->color = F3_clamp01((F3){x, y, z});
			}

			arena_pop_to(arena, pos);

			glFlush();

			os_window_swap_buffers(window);
		}

		os_window_close(window);
	}
}


#endif
