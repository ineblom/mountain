
#if (CPU_ && DEF_)

#define MAX_ENTITY_COUNT 128

#endif

#if (CPU_ && TYP_)

enum {
	ENTITY_FLAG__SHAPE = 1 << 0,
	ENTITY_FLAG__CAMERA = 1 << 1,
};

enum {
	SHAPE__BOX = 0,
	SHAPE__SPHERE,
	SHAPE__COUNT,
};

typedef struct Entity Entity;
struct Entity {
	L1 flags;
	String8 name;
	L1 shape;
	F3 pos;
	F3 size;
	F3 color;
};

enum {
	AXIS_MODE__POS = 0,
	AXIS_MODE__SIZE,
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

I1 hot_axis;  // 0=none, 1=X, 2=Y, 3=Z
I1 active_axis;
F3 drag_start_pos;
I1 axis_mode;

#endif

#if (CPU_ && ROM_)

String8 shape_strs[] = {
	[SHAPE__BOX] = Str8_("BOX"),
	[SHAPE__SPHERE] = Str8_("SPHERE"),
};
StaticAssert(ArrayCount(shape_strs) == SHAPE__COUNT);

#define sidebar_w 400.0f

// 4x4 matrix stored in column-major order
typedef struct Mat4 Mat4;
struct Mat4 {
	F1 m[16];
};

Inline Mat4 mat4_identity(void) {
	Mat4 result = {0};
	result.m[0] = 1.0f;
	result.m[5] = 1.0f;
	result.m[10] = 1.0f;
	result.m[15] = 1.0f;
	return result;
}

Inline Mat4 mat4_frustum(F1 left, F1 right, F1 bottom, F1 top, F1 near, F1 far) {
	Mat4 result = {0};
	result.m[ 0] = (2.0f * near) / (right - left);
	result.m[ 5] = (2.0f * near) / (top - bottom);
	result.m[ 8] = (right + left) / (right - left);
	result.m[ 9] = (top + bottom) / (top - bottom);
	result.m[10] = -(far + near) / (far - near);
	result.m[11] = -1.0f;
	result.m[14] = -(2.0f * far * near) / (far - near);
	return result;
}

Inline Mat4 mat4_translate(F1 x, F1 y, F1 z) {
	Mat4 result = mat4_identity();
	result.m[12] = x;
	result.m[13] = y;
	result.m[14] = z;
	return result;
}

Inline Mat4 mat4_rotate_x(F1 angle_deg) {
	F1 rad = angle_deg * PI / 180.0f;
	F1 c = cosf(rad);
	F1 s = sinf(rad);
	Mat4 result = mat4_identity();
	result.m[5] = c;
	result.m[6] = s;
	result.m[9] = -s;
	result.m[10] = c;
	return result;
}

Inline Mat4 mat4_rotate_y(F1 angle_deg) {
	F1 rad = angle_deg * PI / 180.0f;
	F1 c = cosf(rad);
	F1 s = sinf(rad);
	Mat4 result = mat4_identity();
	result.m[0] = c;
	result.m[2] = -s;
	result.m[8] = s;
	result.m[10] = c;
	return result;
}

Inline Mat4 mat4_multiply(Mat4 a, Mat4 b) {
	Mat4 result = {0};
	for (I1 row = 0; row < 4; row++) {
		for (I1 col = 0; col < 4; col++) {
			F1 sum = 0.0f;
			for (I1 i = 0; i < 4; i++) {
				sum += a.m[row + i * 4] * b.m[i + col * 4];
			}
			result.m[row + col * 4] = sum;
		}
	}
	return result;
}

Inline Mat4 mat4_invert(Mat4 m) {
	Mat4 inv = {0};
	F1R mm = m.m;
	F1R dst = inv.m;

	dst[ 0] =  mm[5]*mm[10]*mm[15] - mm[5]*mm[11]*mm[14] - mm[9]*mm[6]*mm[15] + mm[9]*mm[7]*mm[14] + mm[13]*mm[6]*mm[11] - mm[13]*mm[7]*mm[10];
	dst[ 4] = -mm[4]*mm[10]*mm[15] + mm[4]*mm[11]*mm[14] + mm[8]*mm[6]*mm[15] - mm[8]*mm[7]*mm[14] - mm[12]*mm[6]*mm[11] + mm[12]*mm[7]*mm[10];
	dst[ 8] =  mm[4]*mm[ 9]*mm[15] - mm[4]*mm[11]*mm[13] - mm[8]*mm[5]*mm[15] + mm[8]*mm[7]*mm[13] + mm[12]*mm[5]*mm[11] - mm[12]*mm[7]*mm[ 9];
	dst[12] = -mm[4]*mm[ 9]*mm[14] + mm[4]*mm[10]*mm[13] + mm[8]*mm[5]*mm[14] - mm[8]*mm[6]*mm[13] - mm[12]*mm[5]*mm[10] + mm[12]*mm[6]*mm[ 9];
	dst[ 1] = -mm[1]*mm[10]*mm[15] + mm[1]*mm[11]*mm[14] + mm[9]*mm[2]*mm[15] - mm[9]*mm[3]*mm[14] - mm[13]*mm[2]*mm[11] + mm[13]*mm[3]*mm[10];
	dst[ 5] =  mm[0]*mm[10]*mm[15] - mm[0]*mm[11]*mm[14] - mm[8]*mm[2]*mm[15] + mm[8]*mm[3]*mm[14] + mm[12]*mm[2]*mm[11] - mm[12]*mm[3]*mm[10];
	dst[ 9] = -mm[0]*mm[ 9]*mm[15] + mm[0]*mm[11]*mm[13] + mm[8]*mm[1]*mm[15] - mm[8]*mm[3]*mm[13] - mm[12]*mm[1]*mm[11] + mm[12]*mm[3]*mm[ 9];
	dst[13] =  mm[0]*mm[ 9]*mm[14] - mm[0]*mm[10]*mm[13] - mm[8]*mm[1]*mm[14] + mm[8]*mm[2]*mm[13] + mm[12]*mm[1]*mm[10] - mm[12]*mm[2]*mm[ 9];
	dst[ 2] =  mm[1]*mm[ 6]*mm[15] - mm[1]*mm[ 7]*mm[14] - mm[5]*mm[2]*mm[15] + mm[5]*mm[3]*mm[14] + mm[13]*mm[2]*mm[ 7] - mm[13]*mm[3]*mm[ 6];
	dst[ 6] = -mm[0]*mm[ 6]*mm[15] + mm[0]*mm[ 7]*mm[14] + mm[4]*mm[2]*mm[15] - mm[4]*mm[3]*mm[14] - mm[12]*mm[2]*mm[ 7] + mm[12]*mm[3]*mm[ 6];
	dst[10] =  mm[0]*mm[ 5]*mm[15] - mm[0]*mm[ 7]*mm[13] - mm[4]*mm[1]*mm[15] + mm[4]*mm[3]*mm[13] + mm[12]*mm[1]*mm[ 7] - mm[12]*mm[3]*mm[ 5];
	dst[14] = -mm[0]*mm[ 5]*mm[14] + mm[0]*mm[ 6]*mm[13] + mm[4]*mm[1]*mm[14] - mm[4]*mm[2]*mm[13] - mm[12]*mm[1]*mm[ 6] + mm[12]*mm[2]*mm[ 5];
	dst[ 3] = -mm[1]*mm[ 6]*mm[11] + mm[1]*mm[ 7]*mm[10] + mm[5]*mm[2]*mm[11] - mm[5]*mm[3]*mm[10] - mm[ 9]*mm[2]*mm[ 7] + mm[ 9]*mm[3]*mm[ 6];
	dst[ 7] =  mm[0]*mm[ 6]*mm[11] - mm[0]*mm[ 7]*mm[10] - mm[4]*mm[2]*mm[11] + mm[4]*mm[3]*mm[10] + mm[ 8]*mm[2]*mm[ 7] - mm[ 8]*mm[3]*mm[ 6];
	dst[11] = -mm[0]*mm[ 5]*mm[11] + mm[0]*mm[ 7]*mm[ 9] + mm[4]*mm[1]*mm[11] - mm[4]*mm[3]*mm[ 9] - mm[ 8]*mm[1]*mm[ 7] + mm[ 8]*mm[3]*mm[ 5];
	dst[15] =  mm[0]*mm[ 5]*mm[10] - mm[0]*mm[ 6]*mm[ 9] - mm[4]*mm[1]*mm[10] + mm[4]*mm[2]*mm[ 9] + mm[ 8]*mm[1]*mm[ 6] - mm[ 8]*mm[2]*mm[ 5];

	F1 det = mm[0]*dst[0] + mm[1]*dst[4] + mm[2]*dst[8] + mm[3]*dst[12];
	det = 1.0f / det;
	for (I1 i = 0; i < 16; i++) {
		dst[i] *= det;
	}

	return inv;
}

Inline F3 mat4_transform_point(Mat4 m, F3 p) {
	F3 result;
	F1 w = m.m[3] * p.x + m.m[7] * p.y + m.m[11] * p.z + m.m[15];
	result.x = (m.m[0] * p.x + m.m[4] * p.y + m.m[8] * p.z + m.m[12]) / w;
	result.y = (m.m[1] * p.x + m.m[5] * p.y + m.m[9] * p.z + m.m[13]) / w;
	result.z = (m.m[2] * p.x + m.m[6] * p.y + m.m[10] * p.z + m.m[14]) / w;
	return result;
}

// Project 3D world point to screen space
Inline F3 project_to_screen(F3 world_pos, Mat4 view_projection, F1 viewport_width, F1 viewport_height) {
	F3 ndc = mat4_transform_point(view_projection, world_pos);
	F3 screen;
	screen.x = (ndc.x + 1.0f) * 0.5f * viewport_width + sidebar_w;
	screen.y = (1.0f - ndc.y) * 0.5f * viewport_height;
	screen.z = ndc.z;
	return screen;
}

// Check if mouse is near a line segment in screen space
Inline F1 distance_to_line_segment(F1 mx, F1 my, F3 p0_screen, F3 p1_screen) {
	F1 dx = p1_screen.x - p0_screen.x;
	F1 dy = p1_screen.y - p0_screen.y;
	F1 len_sq = dx*dx + dy*dy;

	if (len_sq < 0.0001f) {
		F1 ddx = mx - p0_screen.x;
		F1 ddy = my - p0_screen.y;
		return sqrtf(ddx*ddx + ddy*ddy);
	}

	F1 t = ((mx - p0_screen.x) * dx + (my - p0_screen.y) * dy) / len_sq;
	t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

	F1 proj_x = p0_screen.x + t * dx;
	F1 proj_y = p0_screen.y + t * dy;

	F1 ddx = mx - proj_x;
	F1 ddy = my - proj_y;
	return sqrtf(ddx*ddx + ddy*ddy);
}

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
	F3 color = (F3){0.8f, 0.8f, 0.8f};

	ramR->panel_current_y += 10.0f;
	F1 height = draw_text(msg, 10.0f, ramR->panel_current_y, 16.0f, color);
	ramR->panel_current_y += height;
}

Internal String8 cut_label(String8 label) {
	// cut label at ##.
	String8 result = label;
	for EachIndex(i, result.len-1) {
		if (B1R_(result.str)[i] == '#' && B1R_(result.str)[i+1] == '#') {
			result.len = i;
			break;
		}
	}

	return result;
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

	F3 color = (F3){0.8f, 0.8f, 0.8f};
	if (ramR->hot_hash == hash || ramR->active_hash == hash) {
		color *= 1.5f;
	}

	label = cut_label(label);

	ramR->panel_current_y += 10.0f;
	String8 text = str8f(arena, "%.*s %f", label.len, label.str, value[0]);
	F1 height = draw_text(text, 10.0f, ramR->panel_current_y, 16.0f, color);

	ramR->panel_current_y += height;
}

Internal I1 ui_select(String8 label, L1 alternatives, L1 alternative_count, L1R value) {
	I1 did_change_value = 0;

	ramR->panel_current_y += 10.0f;

	F3 text_color = (F3){0.8f, 0.8f, 0.8f};
	F1 text_size = 16.0f;
	F1 pad_y = 5.0f;
	F1 left = 10.0f;
	F1 right = sidebar_w - left * 2.0f;
	F1 alt_h = text_size+pad_y*2.0f;
	F1 alts_h = alternative_count * alt_h;
	F1 top = ramR->panel_current_y;
	F1 bottom =  top + text_size + 10.0f + alts_h;

	L1 base_hash = str8_hash(label);
	label = cut_label(label);

	top += draw_text(label, 10.0f, ramR->panel_current_y, text_size, text_color);
	top += 10.0f;

	L1 hover_alt_i = (ramR->mouse_y-top) / alt_h;
	if ((ramR->active_hash == 0 || ramR->active_hash == base_hash + hover_alt_i) &&
		ramR->mouse_x > left && ramR->mouse_x < right &&
		ramR->mouse_y > top && ramR->mouse_y < bottom) {
		ramR->hot_hash = base_hash + hover_alt_i;
	}

	for EachIndex(i, alternative_count) {
		L1 hash = base_hash + i;

		F1 red = 0.2f;
		if (i == value[0]) {
			red = 0.8f;
		} else if (ramR->active_hash == hash) {
			red = 0.5f;

			if (ramR->left_just_released) {
				ramR->active_hash = 0;
				if (ramR->hot_hash == hash) {
					value[0] = i;
					did_change_value = 1;
				}
			}
		} else if (ramR->hot_hash == hash) {
			red = 0.3f;

			if (ramR->left_just_pressed) {
				ramR->active_hash = hash;
			}
		}

		glBegin(GL_QUADS);
		glColor3f(red, 0.0f, 0.0f);
		glVertex2f(left, top);
		glVertex2f(right, top);
		glVertex2f(right, top+alt_h);
		glVertex2f(left, top+alt_h);
		glEnd();

		top += pad_y;
		top += draw_text(TR_(String8, alternatives)[i], left, top, 16.0f, text_color);
		top += pad_y;
	}

	ramR->panel_current_y = bottom;

	return did_change_value;
}

Internal I1 ui_button(String8 label) {
	I1 return_value = 0;

	L1 hash = str8_hash(label);

	F3 text_color = (F3){0.8f, 0.8f, 0.8f};
	F1 text_size = 16.0f;
	F1 padding_x = 10.0f;
	F1 padding_y = 5.0f;
	F1 left = 10.0f;
	F1 right = left + text_size*label.len + padding_x*2;
	F1 top = ramR->panel_current_y+10.0f;
	F1 bottom = top + text_size + padding_y*2;

	if ((ramR->active_hash == 0 || ramR->active_hash == hash) &&
			ramR->mouse_x > left && ramR->mouse_x < right &&
			ramR->mouse_y > top && ramR->mouse_y < bottom) {
		ramR->hot_hash = hash;
	}

	if (ramR->active_hash == hash) {
		if (ramR->left_just_released) {
			ramR->active_hash = 0;
			if (ramR->hot_hash == hash) {
				return_value = 1;
			}
		}
	} else if (ramR->hot_hash == hash) {
		if (ramR->left_just_pressed) {
			ramR->active_hash = hash;
		}
	}

	label = cut_label(label);

	F1 red = 0.2f;
	if (ramR->active_hash == hash) {
		red = 0.5f;
	} else if (ramR->hot_hash == hash) {
		red = 0.3f;
	}

	glBegin(GL_QUADS);
	glColor3f(red, 0, 0);
	glVertex2f(left, top);
	glVertex2f(right, top);
	glVertex2f(right, bottom);
	glVertex2f(left, bottom);
	glEnd();

	ramR->panel_current_y = top;
	draw_text(label, left+padding_x, top+padding_y, text_size, text_color);
	ramR->panel_current_y = bottom;

	return return_value;
}

Internal void draw_cube(F3 pos, F3 size, F3 color) {
	glColor3f(color.x, color.y, color.z);

	glBegin(GL_QUADS);
		// TOP
		glNormal3d(0, 1, 0);
		glVertex3f(pos.x+size.x, pos.y+size.y, pos.z+size.z);
		glVertex3f(pos.x+size.x, pos.y+size.y, pos.z-size.z);
		glVertex3f(pos.x-size.x, pos.y+size.y, pos.z-size.z);
		glVertex3f(pos.x-size.x, pos.y+size.y, pos.z+size.z);

		// FRONT
		glNormal3d(0, 0, 1);
		glVertex3f(pos.x-size.x, pos.y+size.y, pos.z+size.z);
		glVertex3f(pos.x+size.x, pos.y+size.y, pos.z+size.z);
		glVertex3f(pos.x+size.x, pos.y-size.y, pos.z+size.z);
		glVertex3f(pos.x-size.x, pos.y-size.y, pos.z+size.z);

		// BACK
		glNormal3d(0, 0,-1);
		glVertex3f(pos.x-size.x, pos.y+size.y, pos.z-size.z);
		glVertex3f(pos.x+size.x, pos.y+size.y, pos.z-size.z);
		glVertex3f(pos.x+size.x, pos.y-size.y, pos.z-size.z);
		glVertex3f(pos.x-size.x, pos.y-size.y, pos.z-size.z);

		// LEFT
		glNormal3d(-1, 0, 0);
		glVertex3f(pos.x-size.x, pos.y+size.y, pos.z+size.z);
		glVertex3f(pos.x-size.x, pos.y+size.y, pos.z-size.z);
		glVertex3f(pos.x-size.x, pos.y-size.y, pos.z-size.z);
		glVertex3f(pos.x-size.x, pos.y-size.y, pos.z+size.z);

		// RIGHT
		glNormal3d(1, 0, 0);
		glVertex3f(pos.x+size.x, pos.y+size.y, pos.z+size.z);
		glVertex3f(pos.x+size.x, pos.y+size.y, pos.z-size.z);
		glVertex3f(pos.x+size.x, pos.y-size.y, pos.z-size.z);
		glVertex3f(pos.x+size.x, pos.y-size.y, pos.z+size.z);


		// BOTTOM
		glNormal3d(0,-1, 0);
		glVertex3f(pos.x+size.x, pos.y-size.y, pos.z+size.z);
		glVertex3f(pos.x+size.x, pos.y-size.y, pos.z-size.z);
		glVertex3f(pos.x-size.x, pos.y-size.y, pos.z-size.z);
		glVertex3f(pos.x-size.x, pos.y-size.y, pos.z+size.z);

	glEnd();
}

// Source - https://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
// Posted by Constantinius
// Retrieved 2025-11-06, License - CC BY-SA 4.0
Internal void draw_sphere(F3 pos, F1 radius, F3 color) {
	L1 lats = 32;
	L1 longs = 32;
  for(L1 i = 0; i <= lats; i++) {
      F1 lat0 = M_PI * (-0.5 + F1_(i - 1) / lats);
      F1 z0  = sinf(lat0);
      F1 zr0 =  cosf(lat0);

      F1 lat1 = M_PI * (-0.5 + F1_(i) / lats);
      F1 z1 = sinf(lat1);
      F1 zr1 = cosf(lat1);

      glBegin(GL_QUAD_STRIP);
      for(L1 j = 0; j <= longs; j++) {
          F1 lng = 2 * M_PI * F1_(j - 1) / longs;
          F1 x = cosf(lng);
          F1 y = sinf(lng);

          glNormal3f(x * zr0, y * zr0, z0);
          glVertex3f(pos.x + radius*x*zr0, pos.y + radius*y*zr0, pos.z + radius*z0);
          glNormal3f(x * zr1, y * zr1, z1);
          glVertex3f(pos.x + radius*x*zr1, pos.y + radius*y*zr1, pos.z + radius*z1);
      }
      glEnd();
  }
}

Internal L1 entity_create(L1 flags, String8 name) {
	L1 idx = ramR->entity_count;
	ramR->entity_count += 1;

	ramR->entities[idx] = (Entity){
		.flags = flags,
		.name = name,
		.shape = SHAPE__BOX,
		.pos = {0.0f, 0.0f, 0.0f},
		.size = {0.5f, 0.5f, 0.5f},
		.color = {0.8f, 0.8f, 0.8f},
	};
	return idx;
}

Internal void lane(L1 arena) {
	L1 should_render = 0;

	if (lane_idx() == 0) {
		L1 window = os_window_open(arena, Str8_("Hello"), 1280, 720);
		os_window_get_context(window);

		ramR->cam_dist = 3.0f;
		ramR->cam_rot_x = 35.0f;
		ramR->cam_rot_y = -45.0f;

		L1 camera = entity_create(ENTITY_FLAG__CAMERA, Str8_("CAMERA"));
		ramR->entities[camera].pos = (F3){0.0f, 0.5f, -5.0f};

		L1 starting_cube = entity_create(ENTITY_FLAG__SHAPE, Str8_("CUBE"));
		ramR->selected_entity_idx = starting_cube;

		ramR->axis_mode = AXIS_MODE__POS;

		F1 near = 0.1f;
		F1 far = 40.0f;
		F1 scale_factor = near / 1.0f;

		// ---- FONT ----
		ramR->font_num_letters = 26 + 10 + 2;
		I1 font_tex_w = 8*ramR->font_num_letters;
		I1 font_tex_h = 8;
		L1 tex_data[] = {
			0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x00FFFFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FFFFFFFFFF0000, 0xFFFFFF0000000000, 0x0000FF000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x00FFFFFFFFFF0000, 0x00FFFFFFFFFF0000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0xFF0000000000FF00, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x000000FFFF000000, 0x0000FFFFFFFF0000, 0x0000FFFFFFFF0000, 0x0000FF0000000000, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x00FFFFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x0000FFFFFFFF0000, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000, 0x000000FF0000FF00, 0x000000000000FF00, 0x00FFFF0000FFFF00, 0x00FF000000FFFF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0xFF0000000000FF00, 0x00FF000000000000, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FFFF00000000, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000000000, 0x00000000FF00FF00, 0x000000000000FF00, 0x00FFFFFFFFFFFF00, 0x00FF0000FF00FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000FF00000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF000000FF0000, 0x0000FF0000000000, 0x00FF000000FFFF00, 0x000000FF00000000, 0x00FF000000000000, 0x00FF000000000000, 0x0000FF00FF000000, 0x000000000000FF00, 0x000000000000FF00, 0x0000FF0000000000, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFFFF00, 0x000000000000FF00, 0x00FFFFFFFFFFFF00, 0x000000FF00000000, 0x00FF000000000000, 0x0000000000FFFF00, 0x000000000000FF00, 0x00FF00FFFF00FF00, 0x00FF00FF0000FF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x00FF00000000FF00, 0x0000FFFFFFFFFF00, 0x0000FFFFFFFF0000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF00000000FF00, 0x000000FFFF000000, 0x0000FF00FF000000, 0x000000FF00000000, 0x00FF0000FF00FF00, 0x000000FF00000000, 0x0000FF0000000000, 0x0000FFFFFF000000, 0x0000FF0000FF0000, 0x0000FFFFFFFF0000, 0x0000FFFFFFFFFF00, 0x000000FF00000000, 0x0000FFFFFFFF0000, 0x00FFFFFFFFFF0000, 0x00FFFFFFFFFFFF00, 0x0000000000000000,
			0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FFFFFF0000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x00000000FF00FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FFFF000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FF00FFFF00FF00, 0x0000FF0000FF0000, 0x000000FF00000000, 0x00000000FF000000, 0x00FF00FF0000FF00, 0x000000FF00000000, 0x000000FFFF000000, 0x00FF000000000000, 0x0000FF000000FF00, 0x00FF000000000000, 0x00FF00000000FF00, 0x00000000FF000000, 0x00FF00000000FF00, 0x00FF000000000000, 0x00FFFFFFFFFFFF00, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x000000FF0000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x0000FF0000FF0000, 0x00FFFFFFFFFFFF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x0000000000FF0000, 0x00FFFF000000FF00, 0x000000FF00000000, 0x0000000000FF0000, 0x00FF000000000000, 0x00FFFFFFFFFFFF00, 0x00FF000000000000, 0x00FF00000000FF00, 0x00000000FF000000, 0x00FF00000000FF00, 0x00FF000000000000, 0x0000000000000000, 0x0000000000000000,
			0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x00FF000000FF0000, 0x0000FF000000FF00, 0x000000000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x00FF00000000FF00, 0x000000000000FF00, 0x00FFFF000000FF00, 0x00FF00000000FF00, 0x00FF000000000000, 0x000000FF00000000, 0x00FF00000000FF00, 0x000000FFFF000000, 0x00FFFF0000FFFF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x000000000000FF00, 0x00FF00000000FF00, 0x000000FF00000000, 0x000000000000FF00, 0x00FF00000000FF00, 0x0000FF0000000000, 0x00FF000000000000, 0x00FF00000000FF00, 0x00000000FF000000, 0x00FF00000000FF00, 0x00FF000000000000, 0x0000000000000000, 0x000000FFFF000000,
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

			I1 key_space = os_key(57);
			I1 key_w = os_key(17);
			I1 key_s = os_key(31);

			F1 window_width = F1_(TR_(OS_Window, window)->width);
			F1 window_height = F1_(TR_(OS_Window, window)->height);

			F1 viewport_width = window_width-sidebar_w;
			F1 viewport_height = window_height;
			F1 viewport_aspect = viewport_width/viewport_height;

			if (right_pressed) {
				ramR->cam_rot_y += ramR->delta_mx * 0.2f;
				ramR->cam_rot_x += delta_my * 0.2f;
			}

			if (key_w) {
				ramR->axis_mode = AXIS_MODE__POS;
			} else if (key_s) {
				ramR->axis_mode = AXIS_MODE__SIZE;
			}

			ramR->cam_dist += ramR->scroll_y/20.0f;

			Mat4 projection = mat4_frustum(
				-viewport_aspect*scale_factor, viewport_aspect*scale_factor,
				-1 * scale_factor, 1 * scale_factor,
				near, far);

			Mat4 view = mat4_translate(0, 0, -ramR->cam_dist);
			view = mat4_multiply(view, mat4_rotate_x(ramR->cam_rot_x));
			view = mat4_multiply(view, mat4_rotate_y(ramR->cam_rot_y));
			F3 camera_right = {view.m[0], view.m[4], view.m[8]};
			F3 camera_up = {view.m[1], view.m[5], view.m[9]};

			// ---- SELECTION (DEPTH BUFFER UNPROJECTION) ----
			if (ramR->left_just_pressed && ramR->mouse_x >= sidebar_w && ramR->hot_axis == 0) {
				ramR->selected_entity_idx = L1_MAX;

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

				// Only proceed if we clicked on geometry (not the background)
				if (depth < 1.0f) {
					F1 viewport_mx = ramR->mouse_x - sidebar_w;
					F1 viewport_my = ramR->mouse_y;

					// Convert to normalized device coordinates
					F1 ndc_x = (viewport_mx / viewport_width) * 2.0f - 1.0f;
					F1 ndc_y = 1.0f - (viewport_my / viewport_height) * 2.0f;
					F1 ndc_z = depth * 2.0f - 1.0f; // Convert [0,1] to [-1,1]

					// Unproject: transform from NDC to world space
					Mat4 view_projection = mat4_multiply(projection, view);
					Mat4 inverse_vp = mat4_invert(view_projection);

					F3 ndc_point = {ndc_x, ndc_y, ndc_z};
					F3 world_pos = mat4_transform_point(inverse_vp, ndc_point);

					// Find which entity's AABB contains or is closest to this world position
					F1 closest_dist = F1_MAX;
					for EachIndex(entity_index, ramR->entity_count) {
						Entity e = ramR->entities[entity_index];
						F3 aabb_min = e.pos - e.size;
						F3 aabb_max = e.pos + e.size;

						// Find closest point on AABB to world_pos
						F3 closest_point;
						closest_point.x = world_pos.x < aabb_min.x ? aabb_min.x : Min(world_pos.x, aabb_max.x);
						closest_point.y = world_pos.y < aabb_min.y ? aabb_min.y : Min(world_pos.y, aabb_max.y);
						closest_point.z = world_pos.z < aabb_min.z ? aabb_min.z : Min(world_pos.z, aabb_max.z);

						F3 diff = world_pos - closest_point;
						F1 dist = F3_length(diff);
						if (dist < closest_dist) {
							closest_dist = dist;
							ramR->selected_entity_idx = entity_index;
						}
					}
				}
			}

			// ---- AXIS DETECTION ----
			Mat4 view_projection = mat4_multiply(projection, view);
			ramR->hot_axis = 0;
			if (ramR->selected_entity_idx != L1_MAX && ramR->active_axis == 0 && ramR->mouse_x >= sidebar_w) {
				TR(Entity) e = &ramR->entities[ramR->selected_entity_idx];

				// Project axis endpoints to screen space
				F3 x_end = e->pos; x_end.x += 1.0f;
				F3 y_end = e->pos; y_end.y += 1.0f;
				F3 z_end = e->pos; z_end.z += 1.0f;
				F3 origin_screen = project_to_screen(e->pos, view_projection, viewport_width, viewport_height);
				F3 x_screen = project_to_screen(x_end, view_projection, viewport_width, viewport_height);
				F3 y_screen = project_to_screen(y_end, view_projection, viewport_width, viewport_height);
				F3 z_screen = project_to_screen(z_end, view_projection, viewport_width, viewport_height);

				F1 dist_x = distance_to_line_segment(ramR->mouse_x, ramR->mouse_y, origin_screen, x_screen);
				F1 dist_y = distance_to_line_segment(ramR->mouse_x, ramR->mouse_y, origin_screen, y_screen);
				F1 dist_z = distance_to_line_segment(ramR->mouse_x, ramR->mouse_y, origin_screen, z_screen);

				// Find which axis is closest
				F1 min_dist = dist_x;
				ramR->hot_axis = 1;
				if (dist_y < min_dist) {
					min_dist = dist_y;
					ramR->hot_axis = 2;
				}
				if (dist_z < min_dist) {
					min_dist = dist_z;
					ramR->hot_axis = 3;
				}

				F1 threshold = 10.0f; // pixels
				if (min_dist > threshold) {
					ramR->hot_axis = 0;
				}
			}

			// ---- RENDERING ----

			glEnable(GL_DEPTH_TEST);
			glViewport(sidebar_w, 0, viewport_width, viewport_height);

			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(projection.m);

			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(view.m);

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

			GLfloat white[] = {1, 1, 1};
			glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
			glLightfv(GL_LIGHT0, GL_SPECULAR, white);
			glMaterialf(GL_FRONT, GL_SHININESS, 30);
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);

			GLfloat light_pos[] = {2, 6,-3, 1};
			glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

			for EachIndex(entity_index, ramR->entity_count) {
				TR(Entity) e = &ramR->entities[entity_index];

			 	GLfloat color[4] = {e->color.x, e->color.y, e->color.z, 1.0};
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
				glMaterialfv(GL_FRONT, GL_SPECULAR, color);

				if (e->flags & ENTITY_FLAG__SHAPE) {
					switch (e->shape) {
						case SHAPE__BOX: {
							draw_cube(e->pos, e->size, e->color);
						} break;

						case SHAPE__SPHERE: {
							draw_sphere(e->pos, e->size.x, e->color);
						} break;
					}
				}

				if (e->flags & ENTITY_FLAG__CAMERA) {
					glDisable(GL_LIGHTING);

					F3 tl = e->pos - camera_right*0.3f + camera_up*0.2f;
					F3 tr = e->pos + camera_right*0.3f + camera_up*0.2f;
					F3 bl = e->pos - camera_right*0.3f - camera_up*0.2f;
					F3 br = e->pos + camera_right*0.3f - camera_up*0.2f;
					F3 mt = e->pos - camera_right*0.3f + camera_up*0.1f;
					F3 mb = e->pos - camera_right*0.3f - camera_up*0.1f;
					F3 ot = e->pos - camera_right*0.5f + camera_up*0.15f;
					F3 ob = e->pos - camera_right*0.5f - camera_up*0.15f;

					glLineWidth(3.0f);
					glBegin(GL_LINE_STRIP);
						glColor3f(0.8f, 0.8f, 0.8f);
						glVertex3f(ot.x, ot.y, ot.z);
						glVertex3f(mt.x, mt.y, mt.z);
						glVertex3f(tl.x, tl.y, tl.z);
						glVertex3f(tr.x, tr.y, tr.z);
						glVertex3f(br.x, br.y, br.z);
						glVertex3f(bl.x, bl.y, bl.z);
						glVertex3f(mb.x, mb.y, mb.z);
						glVertex3f(ob.x, ob.y, ob.z);
						glVertex3f(ot.x, ot.y, ot.z);
					glEnd();
					glLineWidth(1.0f);

					glEnable(GL_LIGHTING);
				}
			}

			glDisable(GL_LIGHTING);

			// ---- AXES ----
			if (ramR->selected_entity_idx != L1_MAX) {
				TR(Entity) e = &ramR->entities[ramR->selected_entity_idx];

				// Handle axis dragging
				if (ramR->active_axis != 0) {
					I1 is_x = (ramR->active_axis == 1);
					I1 is_y = (ramR->active_axis == 2);
					I1 is_z = (ramR->active_axis == 3);

					F3 axis_dir = {is_x, is_y, is_z};

					F3 axis_end_world = e->pos + axis_dir;
					F3 pos_screen = project_to_screen(e->pos, view_projection, viewport_width, viewport_height);
					F3 axis_end_screen = project_to_screen(axis_end_world, view_projection, viewport_width, viewport_height);

					F1 screen_dx = axis_end_screen.x - pos_screen.x;
					F1 screen_dy = axis_end_screen.y - pos_screen.y;
					F1 screen_len = sqrtf(screen_dx*screen_dx + screen_dy*screen_dy);

					if (screen_len > 0.0001f) {
						// Project mouse delta onto screen-space axis direction
						F1 mouse_proj = (ramR->delta_mx * screen_dx + delta_my * screen_dy) / screen_len;
						F1 world_delta = mouse_proj / screen_len;
						F3 delta_v = (F3){world_delta * is_x, world_delta * is_y, world_delta * is_z};

						if (ramR->axis_mode == AXIS_MODE__POS) {
							e->pos += delta_v;
						} else if (ramR->axis_mode == AXIS_MODE__SIZE) {
							if (e->shape == SHAPE__SPHERE) {
								e->size.x += world_delta;
							} else {
								e->size += delta_v;
							}
						}

					}

					if (ramR->left_just_released) {
						ramR->active_axis = 0;
					}
				} else if (ramR->hot_axis != 0 && ramR->left_just_pressed) {
					ramR->active_axis = ramR->hot_axis;
					ramR->drag_start_pos = e->pos;
				}

				glDisable(GL_DEPTH_TEST);

				F1 base_brightness = 0.7f;
				F1 hot_brightness = 1.0f;
				F1 base_width = 3.0f;
				F1 hot_width = 5.0f;

				// X axis (blue)
				F1 x_brightness = (ramR->hot_axis == 1 || ramR->active_axis == 1) ? hot_brightness : base_brightness;
				F1 x_width = (ramR->hot_axis == 1 || ramR->active_axis == 1) ? hot_width : base_width;
				glLineWidth(x_width);
				glBegin(GL_LINES);
				glColor3f(0.0f, 0.0f, x_brightness);
				glVertex3f(e->pos.x, e->pos.y, e->pos.z); glVertex3f(e->pos.x+1, e->pos.y, e->pos.z);
				glEnd();

				// Y axis (red)
				F1 y_brightness = (ramR->hot_axis == 2 || ramR->active_axis == 2) ? hot_brightness : base_brightness;
				F1 y_width = (ramR->hot_axis == 2 || ramR->active_axis == 2) ? hot_width : base_width;
				glLineWidth(y_width);
				glBegin(GL_LINES);
				glColor3f(y_brightness, 0.0f, 0.0f);
				glVertex3f(e->pos.x, e->pos.y, e->pos.z); glVertex3f(e->pos.x, e->pos.y+1, e->pos.z);
				glEnd();

				// Z axis (green)
				F1 z_brightness = (ramR->hot_axis == 3 || ramR->active_axis == 3) ? hot_brightness : base_brightness;
				F1 z_width = (ramR->hot_axis == 3 || ramR->active_axis == 3) ? hot_width : base_width;
				glLineWidth(z_width);
				glBegin(GL_LINES);
				glColor3f(0.0f, z_brightness, 0.0f);
				glVertex3f(e->pos.x, e->pos.y, e->pos.z); glVertex3f(e->pos.x, e->pos.y, e->pos.z+1);
				glEnd();

				if (ramR->axis_mode == AXIS_MODE__SIZE) {
					F1 s = 0.05f;
					draw_cube((F3){e->pos.x+1, e->pos.y, e->pos.z}, (F3){s,s,s}, (F3){0, 0, x_brightness});
					draw_cube((F3){e->pos.x, e->pos.y+1, e->pos.z}, (F3){s,s,s}, (F3){y_brightness, 0, 0});
					draw_cube((F3){e->pos.x, e->pos.y, e->pos.z+1}, (F3){s,s,s}, (F3){0, z_brightness, 0});
				}

				glLineWidth(1.0f);
				glEnable(GL_DEPTH_TEST);
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
				glColor3f(0.05f, 0.05f, 0.05f);
				glVertex2f(0, 0);
				glVertex2f(sidebar_w, 0);
				glVertex2f(sidebar_w, window_height);
				glVertex2f(0, window_height);
			glEnd();

			ramR->panel_current_y = 0.0f;
			ramR->hot_hash = 0;

			if (ramR->selected_entity_idx == L1_MAX) {
				if (ui_button(Str8_("CREATE ENTITY"))) {
					String8 new_entity_name = str8f(arena, "%d", ramR->entity_count);
					entity_create(ENTITY_FLAG__SHAPE, new_entity_name);
				}

				if (ui_button(Str8_("RENDER"))) {
					should_render = 1;
					break;
				}

				ui_text(Str8_("ENTITIES"));

				for EachIndex(entity_index, ramR->entity_count) {
					if (ui_button(ramR->entities[entity_index].name)) {
						ramR->selected_entity_idx = entity_index;
					}
				}
			} else {
				TR(Entity) e = &ramR->entities[ramR->selected_entity_idx];

				ui_text(e->name);

				if (ui_button(Str8_("DELETE"))) {
					ramR->entities[ramR->selected_entity_idx] = ramR->entities[ramR->entity_count-1];
					ramR->selected_entity_idx = L1_MAX;
					ramR->entity_count -= 1;
				}

				L1 pos = arena_pos(arena);

				ui_text(Str8_("POSITION"));
				F1 x = e->pos.x, y = e->pos.y, z = e->pos.z;
				ui_drag(arena, Str8_(" X##pos"), &x);
				ui_drag(arena, Str8_(" Y##pos"), &y);
				ui_drag(arena, Str8_(" Z##pos"), &z);
				e->pos = (F3){x, y, z};

				ui_text(Str8_("SIZE"));
				x = e->size.x, y = e->size.y, z = e->size.z;
				ui_drag(arena, Str8_(" X##size"), &x);
				if (e->shape != SHAPE__SPHERE) {
					ui_drag(arena, Str8_(" Y##size"), &y);
					ui_drag(arena, Str8_(" Z##size"), &z);
				}
				e->size = (F3){Max(0, x), Max(0, y), Max(z, 0)};

				ui_text(Str8_("COLOR"));
				x = e->color.x, y = e->color.y, z = e->color.z;
				ui_drag(arena, Str8_(" X##color"), &x);
				ui_drag(arena, Str8_(" Y##color"), &y);
				ui_drag(arena, Str8_(" Z##color"), &z);
				e->color = F3_clamp01((F3){x, y, z});

				ui_select(Str8_("SHAPE"), L1_(shape_strs), ArrayCount(shape_strs), &e->shape);

				arena_pop_to(arena, pos);
			}

			glFlush();

			os_window_swap_buffers(window);
		}

		os_window_close(window);
	}

	lane_sync_L1(L1_(&should_render), 0);

	if (should_render) {
		RT_Material materials[] = {
			{ // ground
				.base_color  = (F3){0.63f, 0.53f, 0.13f},
				.metallic = 0.3f,
				.roughness = 0.5f,
			},
			{ // left
				.base_color  = (F3){1.0f, 1.0f, 1.0f},
				.metallic = 1.0f,
				.roughness = 0.40f,
			},
			{ // right
				.base_color  = (F3){0.1f, 1.0f, 1.0f},
				.metallic = 0.0f,
				.roughness = 0.10f,
			},
			{ // pink glow
				.base_color  = (F3){1.0f, 0.2f, 1.0f},
				.emissive = (F3){2.0f, 0.6f, 2.0f},
				.metallic = 0.1f,
				.roughness = 1.00f,
			},
			{ // blue glow
				.base_color  = (F3){1.0f, 1.0f, 1.0f},
				.emissive = (F3){1.0f, 1.0f, 6.0f},
			},
		};

		RT_Plane planes[] = {
			{
				.n = (F3){0, 0, 1},
				.d = 1.0f,
				.material_idx = 0
			}
		};

		RT_Sphere spheres[] = {
			{
				.p = (F3){-2.0f, 2.0f, 0.0f},
				.r = 1.0f,
				.material_idx = 1,
			},
			{
				.p = (F3){2.0f, 0.0f, 0.0f},
				.r = 1.0f,
				.material_idx = 2,
			},
			{
				.p = (F3){0.0f, 20.0f, 0.0f},
				.r = 1.0f,
				.material_idx = 3,
			},
		};

		RT_Box boxes[] = {
			{
				.min = {-2.6f, -2.0f,-1.0},
				.max = {-2.5f,  0.7f,-0.5},
				.material_idx = 4,
		 	}
		};

		RT_Camera camera = {0};
		camera.pos = (F3){0.0f, -5.0f, 0.5f};
		camera.forward = F3_normalize(camera.pos);
		camera.aperture_radius =  0.02f;
		camera.focal_distance = 5.0f;

		RT_Scene scene = {
			.output_filename = Str8_("output.bmp"),
			.output_width    = 1280,
			.output_height   = 720,

			.rays_per_pixel  = 128,
			.max_num_bounces = 8,

			.bloom_pass_count       = 8,
			.bloom_threshold        = 0.5f,
			.bloom_strength         = 0.4f,
			.bloom_knee             = 0.5f,
			.bloom_overlay_filename = Str8_("lens-dirt.bmp"),
			.bloom_overlay_strength = 2.0f,

			.camera = camera,

			.materials = L1_(materials),
			.material_count = ArrayCount(materials),

			.spheres = L1_(spheres),
			.sphere_count = ArrayCount(spheres),

			.planes = L1_(planes),
			.plane_count = ArrayCount(planes),

			.boxes = L1_(boxes),
			.box_count = ArrayCount(boxes),
		};

		trace_scene(arena, scene);
	}
}


#endif
