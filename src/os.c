#if (DEF_)

#include <wayland-client.h>
#include <wayland-cursor.h>
#include "xdg-shell-protocol.c"
#include "xdg-shell-protocol.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <poll.h>

#endif

#if (TYP_)

typedef void *ThreadFunc(void *);

typedef struct OS_Thread OS_Thread;
struct OS_Thread {
  pthread_t handle;
};

typedef struct OS_Barrier OS_Barrier;
struct OS_Barrier {
  pthread_barrier_t handle;
};

typedef struct OS_Window OS_Window;
struct OS_Window {
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct xdg_surface_listener xdg_surface_listener;
  struct xdg_toplevel_listener xdg_toplevel_listener;

  I1 configured;
  SI1 width;
  SI1 height;

  OS_Window *prev;
  OS_Window *next;
};

enum {
  OS_EVENT_TYPE__NULL,
  OS_EVENT_TYPE__PRESS,
  OS_EVENT_TYPE__RELEASE,
  OS_EVENT_TYPE__MOUSE_MOVE,
  OS_EVENT_TYPE__SCROLL,
  OS_EVENT_TYPE__WINDOW_CLOSE,
  OS_EVENT_TYPE_COUNT,
};

typedef I1 OS_Modifier_Flags;

enum {
  OS_MODIFIER_FLAG__CTRL  = (1<<0),
  OS_MODIFIER_FLAG__SHIFT = (1<<1),
  OS_MODIFIER_FLAG__ALT   = (1<<2),
};

typedef struct OS_Event OS_Event;
struct OS_Event {
  OS_Event *next;
  OS_Event *prev;
  OS_Window *window;
  L1 timestamp_ns;
  I1 type;
  I1 key;
  I1 modifiers;
	// TODO: Are ints or doubles better here?
  D1 x, y;
	D1 delta_x, delta_y;
};

typedef struct OS_Event_List OS_Event_List;
struct OS_Event_List {
	OS_Event *first;
	OS_Event *last;
	L1 count;
};

Internal void *os_reserve(L1);
Internal void os_commit(void *, L1);
Internal void os_release(void *, L1);
Internal L1 os_clock(void);

enum {
  OS_KEY__NULL,
  OS_KEY__ESC,
  OS_KEY__F1,
  OS_KEY__F2,
  OS_KEY__F3,
  OS_KEY__F4,
  OS_KEY__F5,
  OS_KEY__F6,
  OS_KEY__F7,
  OS_KEY__F8,
  OS_KEY__F9,
  OS_KEY__F10,
  OS_KEY__F11,
  OS_KEY__F12,
  OS_KEY__F13,
  OS_KEY__F14,
  OS_KEY__F15,
  OS_KEY__F16,
  OS_KEY__F17,
  OS_KEY__F18,
  OS_KEY__F19,
  OS_KEY__F20,
  OS_KEY__F21,
  OS_KEY__F22,
  OS_KEY__F23,
  OS_KEY__F24,
  OS_KEY__TICK,
  OS_KEY__0,
  OS_KEY__1,
  OS_KEY__2,
  OS_KEY__3,
  OS_KEY__4,
  OS_KEY__5,
  OS_KEY__6,
  OS_KEY__7,
  OS_KEY__8,
  OS_KEY__9,
  OS_KEY__MINUS,
  OS_KEY__EQUAL,
  OS_KEY__BACKSPACE,
  OS_KEY__TAB,
  OS_KEY__Q,
  OS_KEY__W,
  OS_KEY__E,
  OS_KEY__R,
  OS_KEY__T,
  OS_KEY__Y,
  OS_KEY__U,
  OS_KEY__I,
  OS_KEY__O,
  OS_KEY__P,
  OS_KEY__LEFT_BRACKET,
  OS_KEY__RIGHT_BRACKET,
  OS_KEY__BACK_SLASH,
  OS_KEY__CAPS_LOCK,
  OS_KEY__A,
  OS_KEY__S,
  OS_KEY__D,
  OS_KEY__F,
  OS_KEY__G,
  OS_KEY__H,
  OS_KEY__J,
  OS_KEY__K,
  OS_KEY__L,
  OS_KEY__SEMICOLON,
  OS_KEY__QUOTE,
  OS_KEY__RETURN,
  OS_KEY__SHIFT,
  OS_KEY__Z,
  OS_KEY__X,
  OS_KEY__C,
  OS_KEY__V,
  OS_KEY__B,
  OS_KEY__N,
  OS_KEY__M,
  OS_KEY__COMMA,
  OS_KEY__PERIOD,
  OS_KEY__SLASH,
  OS_KEY__CTRL,
  OS_KEY__ALT,
  OS_KEY__SPACE,
  OS_KEY__MENU,
  OS_KEY__SCROLL_LOCK,
  OS_KEY__PAUSE,
  OS_KEY__INSERT,
  OS_KEY__HOME,
  OS_KEY__PAGE_UP,
  OS_KEY__DELETE,
  OS_KEY__END,
  OS_KEY__PAGE_DOWN,
  OS_KEY__UP,
  OS_KEY__LEFT,
  OS_KEY__DOWN,
  OS_KEY__RIGHT,
  OS_KEY__EX0,
  OS_KEY__EX1,
  OS_KEY__EX2,
  OS_KEY__EX3,
  OS_KEY__EX4,
  OS_KEY__EX5,
  OS_KEY__EX6,
  OS_KEY__EX7,
  OS_KEY__EX8,
  OS_KEY__EX9,
  OS_KEY__EX10,
  OS_KEY__EX11,
  OS_KEY__EX12,
  OS_KEY__EX13,
  OS_KEY__EX14,
  OS_KEY__EX15,
  OS_KEY__EX16,
  OS_KEY__EX17,
  OS_KEY__EX18,
  OS_KEY__EX19,
  OS_KEY__EX20,
  OS_KEY__EX21,
  OS_KEY__EX22,
  OS_KEY__EX23,
  OS_KEY__EX24,
  OS_KEY__EX25,
  OS_KEY__EX26,
  OS_KEY__EX27,
  OS_KEY__EX28,
  OS_KEY__EX29,
  OS_KEY__NUM_LOCK,
  OS_KEY__NUM_SLASH,
  OS_KEY__NUM_STAR,
  OS_KEY__NUM_MINUS,
  OS_KEY__NUM_PLUS,
  OS_KEY__NUM_PERIOD,
  OS_KEY__NUM0,
  OS_KEY__NUM1,
  OS_KEY__NUM2,
  OS_KEY__NUM3,
  OS_KEY__NUM4,
  OS_KEY__NUM5,
  OS_KEY__NUM6,
  OS_KEY__NUM7,
  OS_KEY__NUM8,
  OS_KEY__NUM9,
  OS_KEY__LEFT_MOUSE_BUTTON,
  OS_KEY__MIDDLE_MOUSE_BUTTON,
  OS_KEY__RIGHT_MOUSE_BUTTON,
  OS_KEY_COUNT,
};

#define OS_MOUSE_BUTTON__LEFT   OS_KEY__LEFT_MOUSE_BUTTON
#define OS_MOUSE_BUTTON__MIDDLE OS_KEY__MIDDLE_MOUSE_BUTTON
#define OS_MOUSE_BUTTON__RIGHT  OS_KEY__RIGHT_MOUSE_BUTTON
#define OS_MOUSE_BUTTON_COUNT 3

typedef struct OS_GFX_State OS_GFX_State;
struct OS_GFX_State {
	Arena *arena;

	OS_Window *hovered_window;
	OS_Window *focused_window;
	OS_Window *first_window;

	struct wl_display *display;

	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct xdg_wm_base *xdg_wm_base;

	struct wl_seat *seat;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;

	struct wl_cursor_theme *cursor_theme;
	struct wl_cursor *default_cursor;
	struct wl_surface *cursor_surface;

	struct xdg_wm_base_listener xdg_wm_base_listener;
	struct wl_seat_listener seat_listener;
	struct wl_pointer_listener pointer_listener;
	struct wl_keyboard_listener keyboard_listener;

	Arena *event_arena;
	OS_Event_List events;

	B1 key_states[OS_KEY_COUNT];
	D1 mouse_x, mouse_y;
};

#endif

#if (ROM_)

Global OS_GFX_State *os_gfx_state = 0;

Internal String8 os_read_entire_file(Arena *arena, String8 filename) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);

  I1 file = open((CString)cstr_filename.str, O_RDONLY);
  if (LtSI1(file, 0)) {
    return (String8){0};
  }

  scratch_end(scratch);

  L1 size = lseek(file, 0, SEEK_END);
  lseek(file, 0, SEEK_SET);

  B1 *buffer = push_array(arena, B1, size);
  L1 bytes_read = read(file, buffer, size);
  Assert(size == bytes_read);

  close(file);

  String8 result = {
    .str = buffer,
    .len = size,
  };

  return result;
}

Internal L1 os_write_entire_file(String8 filename, void *data, L1 data_size) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);

  I1 file = open((CString)cstr_filename.str, O_CREAT | O_WRONLY, 0666);
  if (LtSI1(file, 0)) {
    return 0;
  }
  
  scratch_end(scratch);

  L1 bytes_written = write(file, data, data_size);
  close(file);

  return bytes_written;
}

Internal void *os_reserve(L1 size) {
  // TODO: Optinally do large pages: MAP_HUGETBL
  void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (result == MAP_FAILED) {
    result = 0;
  }
  return result;
}

Internal void os_commit(void *ptr, L1 size) {
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

Internal void os_decommit(void *ptr, L1 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

Internal void os_release(void *ptr, L1 size) { munmap(ptr, size); }

Internal I1 os_num_cores(void) { return get_nprocs(); }

Internal OS_Thread os_thread_launch(ThreadFunc *func, void *ptr) {
  OS_Thread result = {0};

  pthread_create(&result.handle, 0, func, ptr);

  return result;
}

Internal void os_thread_join(OS_Thread thread) { pthread_join(thread.handle, 0); }

Internal OS_Barrier os_barrier_alloc(I1 count) {
  OS_Barrier result = {0};
  pthread_barrier_init(&result.handle, 0, count);
  return result;
}

Internal void os_barrier_release(OS_Barrier *barrier) {
  pthread_barrier_destroy(&barrier->handle);
}

Internal void os_barrier_wait(OS_Barrier *barrier) {
  pthread_barrier_wait(&barrier->handle);
}

Internal L1 os_clock(void) {
  struct timespec ts = {0};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  L1 result = ts.tv_sec * 1000000000 + ts.tv_nsec;
  return result;
}

Internal void os_sleep(L1 time) {
  struct timespec ts = {
    .tv_sec = time / 1000000000L,
    .tv_nsec = time % 1000000000L,
  };
  nanosleep(&ts, 0);
}

Internal void *os_library_open(String8 filename) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);

  void *handle = dlopen((CString)cstr_filename.str, RTLD_LAZY|RTLD_LOCAL);

  scratch_end(scratch);

  return handle;
}

Internal VoidProc *os_library_load_proc(void *lib, String8 name) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_name = push_str8_copy(scratch.arena, name);

  VoidProc *proc = dlsym(lib, (CString)cstr_name.str);

  scratch_end(scratch);

  return proc;
}

Internal void os_library_close(void *handle) {
  dlclose(handle);
}

////////////////////////////////
//~ kti: Wayland

Internal I1 os_key_from_wl_key(I1 wl_key) {
  I1 result = OS_KEY__NULL;
  LocalPersist I1 initialized = 0;
  LocalPersist I1 key_table[256] = {0};
  if (initialized == 0) {
    initialized = 1;
    key_table[1] = OS_KEY__ESC;
    key_table[2] = OS_KEY__1;
    key_table[3] = OS_KEY__2;
    key_table[4] = OS_KEY__3;
    key_table[5] = OS_KEY__4;
    key_table[6] = OS_KEY__5;
    key_table[7] = OS_KEY__6;
    key_table[8] = OS_KEY__7;
    key_table[9] = OS_KEY__8;
    key_table[10] = OS_KEY__9;
    key_table[11] = OS_KEY__0;
    key_table[12] = OS_KEY__MINUS;
    key_table[13] = OS_KEY__EQUAL;
    key_table[14] = OS_KEY__BACKSPACE;
    key_table[15] = OS_KEY__TAB;
    key_table[16] = OS_KEY__Q;
    key_table[17] = OS_KEY__W;
    key_table[18] = OS_KEY__E;
    key_table[19] = OS_KEY__R;
    key_table[20] = OS_KEY__T;
    key_table[21] = OS_KEY__Y;
    key_table[22] = OS_KEY__U;
    key_table[23] = OS_KEY__I;
    key_table[24] = OS_KEY__O;
    key_table[25] = OS_KEY__P;
    key_table[26] = OS_KEY__LEFT_BRACKET;
    key_table[27] = OS_KEY__RIGHT_BRACKET;
    key_table[28] = OS_KEY__RETURN;
    key_table[29] = OS_KEY__CTRL;
    key_table[30] = OS_KEY__A;
    key_table[31] = OS_KEY__S;
    key_table[32] = OS_KEY__D;
    key_table[33] = OS_KEY__F;
    key_table[34] = OS_KEY__G;
    key_table[35] = OS_KEY__H;
    key_table[36] = OS_KEY__J;
    key_table[37] = OS_KEY__K;
    key_table[38] = OS_KEY__L;
    key_table[39] = OS_KEY__SEMICOLON;
    key_table[40] = OS_KEY__QUOTE;
    key_table[41] = OS_KEY__TICK;
    key_table[42] = OS_KEY__SHIFT;
    key_table[43] = OS_KEY__BACK_SLASH;
    key_table[44] = OS_KEY__Z;
    key_table[45] = OS_KEY__X;
    key_table[46] = OS_KEY__C;
    key_table[47] = OS_KEY__V;
    key_table[48] = OS_KEY__B;
    key_table[49] = OS_KEY__N;
    key_table[50] = OS_KEY__M;
    key_table[51] = OS_KEY__COMMA;
    key_table[52] = OS_KEY__PERIOD;
    key_table[53] = OS_KEY__SLASH;
    key_table[54] = OS_KEY__SHIFT;
    key_table[55] = OS_KEY__NUM_STAR;
    key_table[56] = OS_KEY__ALT;
    key_table[57] = OS_KEY__SPACE;
    key_table[58] = OS_KEY__CAPS_LOCK;
    key_table[59] = OS_KEY__F1;
    key_table[60] = OS_KEY__F2;
    key_table[61] = OS_KEY__F3;
    key_table[62] = OS_KEY__F4;
    key_table[63] = OS_KEY__F5;
    key_table[64] = OS_KEY__F6;
    key_table[65] = OS_KEY__F7;
    key_table[66] = OS_KEY__F8;
    key_table[67] = OS_KEY__F9;
    key_table[68] = OS_KEY__F10;
    key_table[69] = OS_KEY__NUM_LOCK;
    key_table[70] = OS_KEY__SCROLL_LOCK;
    key_table[71] = OS_KEY__NUM7;
    key_table[72] = OS_KEY__NUM8;
    key_table[73] = OS_KEY__NUM9;
    key_table[74] = OS_KEY__NUM_MINUS;
    key_table[75] = OS_KEY__NUM4;
    key_table[76] = OS_KEY__NUM5;
    key_table[77] = OS_KEY__NUM6;
    key_table[78] = OS_KEY__NUM_PLUS;
    key_table[79] = OS_KEY__NUM1;
    key_table[80] = OS_KEY__NUM2;
    key_table[81] = OS_KEY__NUM3;
    key_table[82] = OS_KEY__NUM0;
    key_table[83] = OS_KEY__NUM_PERIOD;
    key_table[87] = OS_KEY__F11;
    key_table[88] = OS_KEY__F12;
    key_table[97] = OS_KEY__CTRL;
    key_table[98] = OS_KEY__NUM_SLASH;
    key_table[100] = OS_KEY__ALT;
    key_table[102] = OS_KEY__HOME;
    key_table[103] = OS_KEY__UP;
    key_table[104] = OS_KEY__PAGE_UP;
    key_table[105] = OS_KEY__LEFT;
    key_table[106] = OS_KEY__RIGHT;
    key_table[107] = OS_KEY__END;
    key_table[108] = OS_KEY__DOWN;
    key_table[109] = OS_KEY__PAGE_DOWN;
    key_table[110] = OS_KEY__INSERT;
    key_table[111] = OS_KEY__DELETE;
    key_table[119] = OS_KEY__PAUSE;
    key_table[139] = OS_KEY__MENU;
    key_table[183] = OS_KEY__F13;
    key_table[184] = OS_KEY__F14;
    key_table[185] = OS_KEY__F15;
    key_table[186] = OS_KEY__F16;
    key_table[187] = OS_KEY__F17;
    key_table[188] = OS_KEY__F18;
    key_table[189] = OS_KEY__F19;
    key_table[190] = OS_KEY__F20;
    key_table[191] = OS_KEY__F21;
    key_table[192] = OS_KEY__F22;
    key_table[193] = OS_KEY__F23;
    key_table[194] = OS_KEY__F24;
  }
  result = key_table[wl_key];
  return result;
}

Internal OS_Modifier_Flags os_get_modifiers(void) {
	OS_Modifier_Flags result = 0;

	if (os_gfx_state->key_states[OS_KEY__CTRL]) {
		result |= OS_MODIFIER_FLAG__CTRL;
	}
	if (os_gfx_state->key_states[OS_KEY__SHIFT]) {
		result |= OS_MODIFIER_FLAG__SHIFT;
	}
	if (os_gfx_state->key_states[OS_KEY__ALT]) {
		result |= OS_MODIFIER_FLAG__ALT;
	}
	
	return result;
}

Internal void os_push_event(OS_Event event) {
  OS_Event *new_event = push_array(os_gfx_state->event_arena, OS_Event, 1);
  new_event[0] = event;
	new_event->modifiers = os_get_modifiers();

  DLLPushBack(os_gfx_state->events.first, os_gfx_state->events.last, new_event);
	os_gfx_state->events.count += 1;
}

Internal void registry_global_handler(void *data, struct wl_registry *registry,
                                      I1 name, CString interface, I1 version) {
  if (cstr_compare(interface, wl_compositor_interface.name)) {
    os_gfx_state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
  } else if (cstr_compare(interface, wl_shm_interface.name)) {
    os_gfx_state->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
  } else if (cstr_compare(interface, xdg_wm_base_interface.name)) {
    os_gfx_state->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
  } else if (cstr_compare(interface, wl_seat_interface.name)) {
    os_gfx_state->seat = wl_registry_bind(registry, name, &wl_seat_interface, 5);
  }
}

Internal void registry_global_remove_handler(void *data, struct wl_registry *registry, I1 name) {}

Internal void xdg_wm_base_ping_handler(void *data, struct xdg_wm_base *xdg_wm_base, I1 serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

Internal void xdg_surface_configure_handler(void *data, struct xdg_surface *xdg_surface, I1 serial) {
  OS_Window *window = (OS_Window *)data;
  xdg_surface_ack_configure(xdg_surface, serial);
  window->configured = 1;
}

Internal void xdg_toplevel_configure_handler(void *data, struct xdg_toplevel *xdg_toplevel, SI1 width, SI1 height, struct wl_array *states) {
  if (width > 0 && height > 0) {
    OS_Window *window = (OS_Window *)data;
    window->width = width;
    window->height = height;
  }
}

Internal void xdg_toplevel_close_handler(void *data, struct xdg_toplevel *xdg_toplevel) {
  OS_Window *window = (OS_Window *)data;

  OS_Event event = {
    .timestamp_ns = 0,
    .type = OS_EVENT_TYPE__WINDOW_CLOSE,
    .window = window,
  };
  os_push_event(event);
}

////////////////////////////////
//~ kti: Seat event handlers

Internal void seat_capabilities_handler(void *data, struct wl_seat *seat, I1 capabilities) {
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    os_gfx_state->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(os_gfx_state->pointer, &os_gfx_state->pointer_listener, 0);
  }
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    os_gfx_state->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(os_gfx_state->keyboard, &os_gfx_state->keyboard_listener, 0);
  }
}

Internal void seat_name_handler(void *data, struct wl_seat *seat, CString name) {}

// Pointer event handlers
Internal void pointer_enter_handler(void *data, struct wl_pointer *pointer, I1 serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
  for (OS_Window *it = os_gfx_state->first_window; it != 0; it = it->next) {
    if (surface == it->surface) {
			os_gfx_state->hovered_window = it;
      break;
    }
  }

  // Set the cursor to default pointer
  if (os_gfx_state->default_cursor && os_gfx_state->cursor_surface) {
    struct wl_cursor_image *image = os_gfx_state->default_cursor->images[0];
    struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    wl_pointer_set_cursor(pointer, serial, os_gfx_state->cursor_surface, image->hotspot_x, image->hotspot_y);
    wl_surface_attach(os_gfx_state->cursor_surface, buffer, 0, 0);
    wl_surface_damage(os_gfx_state->cursor_surface, 0, 0, image->width, image->height);
    wl_surface_commit(os_gfx_state->cursor_surface);
  }
}

Internal void pointer_leave_handler(void *data, struct wl_pointer *pointer, I1 serial, struct wl_surface *surface) {
  os_gfx_state->hovered_window = 0;
}

Internal void pointer_motion_handler(void *data, struct wl_pointer *pointer, I1 time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
  OS_Window *window = os_gfx_state->hovered_window;
	D1 x = wl_fixed_to_double(surface_x);
	D1 y = wl_fixed_to_double(surface_y);

  OS_Event event = {
		.window = window,
    .timestamp_ns = (L1)time * 1000000LLU,
    .type = OS_EVENT_TYPE__MOUSE_MOVE,
    .x = x,
    .y = y,
  };
  os_push_event(event);

	os_gfx_state->mouse_x = x;
	os_gfx_state->mouse_y = y;
}

Internal void pointer_button_handler(void *data, struct wl_pointer *pointer, I1 serial, I1 time, I1 button, I1 state) {
  OS_Window *window = os_gfx_state->hovered_window;

  I1 key = OS_KEY__NULL;
  switch (button) {
    case 0x110: {
      key = OS_KEY__LEFT_MOUSE_BUTTON;
    } break;
    case 0x111: {
      key = OS_KEY__RIGHT_MOUSE_BUTTON;
    } break;
    case 0x112: {
      key = OS_KEY__MIDDLE_MOUSE_BUTTON;
    } break;
  }

  if (key != OS_KEY__NULL) {
		L1 type = (state == WL_POINTER_BUTTON_STATE_PRESSED) ? OS_EVENT_TYPE__PRESS : OS_EVENT_TYPE__RELEASE;
    OS_Event event = {
			.window = window,
      .timestamp_ns = (L1)time * 1000000LLU,
      .type = type,
      .key = key,
			.x = os_gfx_state->mouse_x,
			.y = os_gfx_state->mouse_y,
    };
    os_push_event(event);

		os_gfx_state->key_states[key] = (type == OS_EVENT_TYPE__PRESS);
  }
}

Internal void pointer_axis_handler(void *data, struct wl_pointer *pointer, I1 time, I1 axis, wl_fixed_t value) {
  OS_Window *window = os_gfx_state->hovered_window;

  OS_Event event = {
		.window = window,
    .timestamp_ns = (L1)time * 1000000LLU,
    .type = OS_EVENT_TYPE__SCROLL,
		.x = os_gfx_state->mouse_x,
		.y = os_gfx_state->mouse_y,
  };
  if (axis == 0) {
    event.delta_y = wl_fixed_to_double(value);
  } else if (axis == 1) {
    event.delta_x = wl_fixed_to_double(value);
  }

  os_push_event(event);
}

Internal void pointer_frame_handler(void *data, struct wl_pointer *pointer) {}

Internal void pointer_axis_source_handler(void *data, struct wl_pointer *pointer,
                                          I1 axis_source) {}

Internal void pointer_axis_stop_handler(void *data, struct wl_pointer *pointer, I1 time, I1 axis) {}

Internal void pointer_axis_discrete_handler(void *data, struct wl_pointer *pointer, I1 axis, SI1 discrete) {}

////////////////////////////////
//~ kti: Keyboard Event Handlers

Internal void keyboard_keymap_handler(void *data, struct wl_keyboard *keyboard, I1 format, SI1 fd, I1 size) {
  close(fd);
}

Internal void keyboard_enter_handler(void *data, struct wl_keyboard *keyboard, I1 serial, struct wl_surface *surface, struct wl_array *keys) {
  for (OS_Window *it = os_gfx_state->first_window; it != 0; it = it->next) {
    if (surface == it->surface) {
      os_gfx_state->focused_window = it;
      break;
    }
  }
}

Internal void keyboard_leave_handler(void *data, struct wl_keyboard *keyboard, I1 serial, struct wl_surface *surface) {
  os_gfx_state->focused_window = 0;
}

Internal void keyboard_key_handler(void *data, struct wl_keyboard *keyboard, I1 serial, I1 time, I1 key, I1 state) {
  OS_Window *window = os_gfx_state->focused_window;
	
  I1 os_key = os_key_from_wl_key(key);
  if (os_key != OS_KEY__NULL) {
		L1 type = (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? OS_EVENT_TYPE__PRESS : OS_EVENT_TYPE__RELEASE;
    OS_Event event = {
			.window = window,
      .timestamp_ns = (L1)time * 1000000LLU,
      .type = type,
      .key = os_key,
    };
    os_push_event(event);

		os_gfx_state->key_states[os_key] = (type == OS_EVENT_TYPE__PRESS) ? 1 : 0;
  }
}

Internal void keyboard_modifiers_handler(void *data, struct wl_keyboard *keyboard, I1 serial, I1 mods_depressed, I1 mods_latched, I1 mods_locked, I1 group) {}

Internal void keyboard_repeat_info_handler(void *data, struct wl_keyboard *keyboard, SI1 rate, SI1 delay) {}

Internal OS_Window *os_window_open(String8 title, I1 width, I1 height) {
  if (os_gfx_state == 0) {
		Arena *arena = arena_create(MiB(64));
		os_gfx_state = push_array(arena, OS_GFX_State, 1);
		os_gfx_state->arena = arena;

    os_gfx_state->display = wl_display_connect(0);
    Assert(os_gfx_state->display != 0);

    struct wl_registry *registry = wl_display_get_registry(os_gfx_state->display);
    Assert(registry != 0);

    struct wl_registry_listener registry_listener = {
        .global = registry_global_handler,
        .global_remove = registry_global_remove_handler,
    };
    wl_registry_add_listener(registry, &registry_listener, 0);
    wl_display_roundtrip(os_gfx_state->display);
    Assert(os_gfx_state->compositor != 0);
    Assert(os_gfx_state->xdg_wm_base != 0);
    Assert(os_gfx_state->seat != 0);

    os_gfx_state->xdg_wm_base_listener.ping = xdg_wm_base_ping_handler;
    xdg_wm_base_add_listener(os_gfx_state->xdg_wm_base, &os_gfx_state->xdg_wm_base_listener, 0);

    os_gfx_state->seat_listener.capabilities = seat_capabilities_handler;
    os_gfx_state->seat_listener.name = seat_name_handler;
    wl_seat_add_listener(os_gfx_state->seat, &os_gfx_state->seat_listener, 0);

    os_gfx_state->pointer_listener.enter = pointer_enter_handler;
    os_gfx_state->pointer_listener.leave = pointer_leave_handler;
    os_gfx_state->pointer_listener.motion = pointer_motion_handler;
    os_gfx_state->pointer_listener.button = pointer_button_handler;
    os_gfx_state->pointer_listener.axis = pointer_axis_handler;
    os_gfx_state->pointer_listener.frame = pointer_frame_handler;
    os_gfx_state->pointer_listener.axis_source = pointer_axis_source_handler;
    os_gfx_state->pointer_listener.axis_stop = pointer_axis_stop_handler;
    os_gfx_state->pointer_listener.axis_discrete = pointer_axis_discrete_handler;

    os_gfx_state->keyboard_listener.keymap = keyboard_keymap_handler;
    os_gfx_state->keyboard_listener.enter = keyboard_enter_handler;
    os_gfx_state->keyboard_listener.leave = keyboard_leave_handler;
    os_gfx_state->keyboard_listener.key = keyboard_key_handler;
    os_gfx_state->keyboard_listener.modifiers = keyboard_modifiers_handler;
    os_gfx_state->keyboard_listener.repeat_info = keyboard_repeat_info_handler;

    wl_display_roundtrip(os_gfx_state->display);

    // Initialize cursor theme and default cursor
    os_gfx_state->cursor_theme = wl_cursor_theme_load(0, 24, os_gfx_state->shm);
    if (os_gfx_state->cursor_theme) {
      os_gfx_state->default_cursor = wl_cursor_theme_get_cursor(os_gfx_state->cursor_theme, "default");
      if (!os_gfx_state->default_cursor) {
        os_gfx_state->default_cursor = wl_cursor_theme_get_cursor(os_gfx_state->cursor_theme, "left_ptr");
      }
      os_gfx_state->cursor_surface = wl_compositor_create_surface(os_gfx_state->compositor);
    }
  }

  OS_Window *result = push_array(os_gfx_state->arena, OS_Window, 1);

  result->surface = wl_compositor_create_surface(os_gfx_state->compositor);
  Assert(result->surface != 0);

  result->xdg_surface = xdg_wm_base_get_xdg_surface(os_gfx_state->xdg_wm_base, result->surface);
  Assert(result->xdg_surface);

  result->xdg_surface_listener.configure = xdg_surface_configure_handler;
  xdg_surface_add_listener(result->xdg_surface, &result->xdg_surface_listener, (void *)result);

  result->xdg_toplevel = xdg_surface_get_toplevel(result->xdg_surface);
  Assert(result->xdg_toplevel);

  result->xdg_toplevel_listener.configure = xdg_toplevel_configure_handler;
  result->xdg_toplevel_listener.close = xdg_toplevel_close_handler;
  xdg_toplevel_add_listener(result->xdg_toplevel, &result->xdg_toplevel_listener, (void *)result);

  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_title = push_str8_copy(scratch.arena, title);
  xdg_toplevel_set_title(result->xdg_toplevel, (CString)cstr_title.str);
  scratch_end(scratch);

  wl_surface_commit(result->surface);

  while (!result->configured) {
    wl_display_dispatch(os_gfx_state->display);
  }

  result->width = width;
  result->height = height;

  result->next = os_gfx_state->first_window;
  if (os_gfx_state->first_window != 0) {
    os_gfx_state->first_window->prev = result;
  }
  os_gfx_state->first_window = result;

  return result;
}

Internal OS_Event_List os_poll_events(Arena *arena) {
  Assert(os_gfx_state->first_window != 0);

  os_gfx_state->event_arena = arena;
  os_gfx_state->events.first = 0;
  os_gfx_state->events.last = 0;
	os_gfx_state->events.count = 0;

  while (wl_display_prepare_read(os_gfx_state->display) != 0) {
    wl_display_dispatch_pending(os_gfx_state->display);
  }
  wl_display_flush(os_gfx_state->display);
  
  struct pollfd pfd = {
    .fd = wl_display_get_fd(os_gfx_state->display),
    .events = POLLIN,
  };

  if (poll(&pfd, 1, 0) > 0) {
    wl_display_read_events(os_gfx_state->display);
  } else {
    wl_display_cancel_read(os_gfx_state->display);
  }
  
  wl_display_dispatch_pending(os_gfx_state->display);

  return os_gfx_state->events;
}

Internal void os_window_close(OS_Window *window) {
  xdg_toplevel_destroy(window->xdg_toplevel);
  xdg_surface_destroy(window->xdg_surface);
  wl_surface_destroy(window->surface);

  if (os_gfx_state->first_window == window && window->next == 0) {
    if (os_gfx_state->cursor_surface)
      wl_surface_destroy(os_gfx_state->cursor_surface);
    if (os_gfx_state->cursor_theme)
      wl_cursor_theme_destroy(os_gfx_state->cursor_theme);
    if (os_gfx_state->pointer)
      wl_pointer_destroy(os_gfx_state->pointer);
    if (os_gfx_state->keyboard)
      wl_keyboard_destroy(os_gfx_state->keyboard);
    if (os_gfx_state->seat)
      wl_seat_destroy(os_gfx_state->seat);
    if (os_gfx_state->shm)
      wl_shm_destroy(os_gfx_state->shm);

    xdg_wm_base_destroy(os_gfx_state->xdg_wm_base);
    wl_display_disconnect(os_gfx_state->display);
  }

  if (window->prev != 0) {
    window->prev->next = window->next;
  }
  if (window->next != 0) {
    window->next->prev = window->prev;
  }
  if (window == os_gfx_state->first_window) {
    os_gfx_state->first_window = window->next;
  }
}

Internal OS_Window *os_hovered_window(void) {
	return os_gfx_state->hovered_window;
}

Internal I1 os_key_is_down(L1 key) {
	I1 result = 0;

	if (key > OS_KEY__NULL && key < OS_KEY_COUNT) {
		result = os_gfx_state->key_states[key];
	}

	return result;
}

Internal D1 os_mouse_x(void) {
	D1 result = os_gfx_state->mouse_x;
	return result;
}

Internal D1 os_mouse_y(void) {
	D1 result = os_gfx_state->mouse_y;
	return result;
}

Internal F2 os_mouse_pos(void) {
	F2 result = {os_gfx_state->mouse_x, os_gfx_state->mouse_y};
	return result;
}

#endif
