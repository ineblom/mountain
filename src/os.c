#if (CPU_ && DEF_)

// TODO: What garbage does this bring in?
// Can we minimize or stop relying on it completely?
#include <wayland-client.h>
#include <wayland-egl.h>
#include "xdg-shell-protocol.c"
#include "xdg-shell-protocol.h"
#include <EGL/egl.h>
#include <GL/gl.h>

#define PROT_READ 0x1
#define PROT_WRITE 02
#define PROT_NONE 0x0
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_ANON 0x20
#define MAP_FAILED L1_MAX
#define MADV_DONTNEED 4

#define O_RDWR 2
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 100
#define O_EXCL 200
#define SEEK_SET 0
#define SEEK_END 2

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define CLOCK_MONOTONIC 1

#define MAX_FILENAME_LEN 256
#define MAX_EVENT_COUNT 512

typedef L1 pthread_t;
typedef struct {
  B1 __opaque[32];
} pthread_barrier_t;

#endif

#if (CPU_ && TYP_)

typedef L1 ThreadFunc(L1);

typedef struct OS_Thread OS_Thread;
struct OS_Thread {
  pthread_t handle;
};

typedef struct OS_Barrier OS_Barrier;
struct OS_Barrier {
  pthread_barrier_t handle;
};

enum {
  OS_EVENT_TYPE__NULL,
  OS_EVENT_TYPE__PRESS,
  OS_EVENT_TYPE__RELEASE,
  OS_EVENT_TYPE__MOUSE_MOVE,
  OS_EVENT_TYPE__SCROLL,
  OS_EVENT_TYPE_COUNT,
};

enum {
  OS_MODIFIER_FLAG__CTRL  = (1<<0),
  OS_MODIFIER_FLAG__SHIFT = (1<<1),
  OS_MODIFIER_FLAG__ALT   = (1<<2),
};

typedef struct OS_Event OS_Event;
struct OS_Event {
  L1 window;
  L1 timestamp_ns;
  I1 type;
  I1 key;
  I1 modifiers;
  D1 x, y;
};

typedef struct Posix_Time_Spec Posix_Time_Spec;
struct Posix_Time_Spec {
  L1 seconds;
  L1 nanoseconds;
};

extern L1 stdin;
extern L1 stdout;
extern L1 stderr;

Internal L1 os_reserve(L1);
Internal void os_commit(L1, L1);
Internal void os_release(L1, L1);

SI1 printf(CString, ...);
SI1 snprintf(L1, L1, CString, ...);
SI1 vsnprintf(L1, L1, CString, va_list);
SI1 fprintf(L1, CString, ...);
SI1 fflush(L1);

L1 memset(L1, SI1, L1);
L1 memmove(L1, L1, L1);

Internal L1 os_clock(void);

L1 mmap(L1, L1, I1, I1, I1, L1);
I1 munmap(L1, L1);
I1 mprotect(L1, L1, I1);
I1 madvise(L1, L1, I1);
SI1 shm_open(CString, I1, I1);
SI1 shm_unlink(CString);
SI1 ftruncate(I1, L1);

I1 open(CString, I1, ...);
L1 lseek(I1, L1, I1);
L1 read(I1, L1, L1);
L1 write(I1, L1, L1);
I1 close(I1);

SI1 clock_gettime(I1, L1);

I1 get_nprocs(void);
I1 pthread_create(L1, L1, L1, L1);
I1 pthread_join(pthread_t, L1);
I1 pthread_barrier_init(L1, L1, I1);
I1 pthread_barrier_destroy(L1);
I1 pthread_barrier_wait(L1);

typedef struct OS_Window OS_Window;
struct OS_Window {
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_egl_window *egl_window;
  EGLSurface egl_surface;
  struct xdg_surface_listener xdg_surface_listener;
  struct xdg_toplevel_listener xdg_toplevel_listener;

  I1 configured;
  I1 should_close;
  SI1 width;
  SI1 height;
  I1 resized;

  L1 prev;
  L1 next;
};

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

#endif

#if (CPU_ && RAM_)

L1 hovered_window;
L1 first_window;

struct wl_display *display;
EGLDisplay egl_display;

struct wl_compositor *compositor;
struct xdg_wm_base *xdg_wm_base;

struct wl_seat *seat;
struct wl_pointer *pointer;
struct wl_keyboard *keyboard;

struct xdg_wm_base_listener xdg_wm_base_listener;
struct wl_seat_listener seat_listener;
struct wl_pointer_listener pointer_listener;
struct wl_keyboard_listener keyboard_listener;

EGLConfig egl_config;
EGLContext egl_context;

L1 event_count;
OS_Event events[MAX_EVENT_COUNT];

#endif

#if (CPU_ && ROM_)

Internal String8 os_read_entire_file(L1 arena, String8 filename) {
  Assert(filename.len < MAX_FILENAME_LEN);
  char cstr_filename[MAX_FILENAME_LEN] = {0};
  memmove(L1_(cstr_filename), filename.str, filename.len);

  I1 file = open(cstr_filename, O_RDONLY);
  if (LtSI1(file, 0)) {
    return (String8){0};
  }

  L1 size = lseek(file, 0, SEEK_END);
  lseek(file, 0, SEEK_SET);

  L1 buffer = push_array(arena, B1, size);
  L1 bytes_read = read(file, buffer, size);
  Assert(size == bytes_read);

  close(file);

  String8 result = {
    .str = buffer,
    .len = size,
  };
  return result;
}

Internal L1 os_write_entire_file(String8 filename, L1 data, L1 data_size) {
  Assert(filename.len < MAX_FILENAME_LEN);
  char cstr_filename[MAX_FILENAME_LEN] = {0};
  memmove(L1_(cstr_filename), filename.str, filename.len);

  I1 file = open(cstr_filename, O_CREAT | O_WRONLY, 0666);
  if (LtSI1(file, 0)) {
    return 0;
  }

  L1 bytes_written = write(file, data, data_size);
  close(file);

  return bytes_written;
}

Internal L1 os_reserve(L1 size) {
  // TODO: Optinally do large pages: MAP_HUGETBL
  L1 result = L1_(mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0));
  if (result == MAP_FAILED) {
    result = 0;
  }
  return result;
}

Internal void os_commit(L1 ptr, L1 size) {
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

Internal void os_decommit(L1 ptr, L1 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

Internal void os_release(L1 ptr, L1 size) { munmap(ptr, size); }

Internal I1 os_num_cores(void) { return get_nprocs(); }

Internal OS_Thread os_thread_launch(ThreadFunc *func, L1 ptr) {
  OS_Thread result = {0};

  pthread_create(L1_(&result.handle), 0, L1_(func), ptr);

  return result;
}

Internal void os_thread_join(OS_Thread thread) { pthread_join(thread.handle, 0); }

Internal OS_Barrier os_barrier_alloc(I1 count) {
  OS_Barrier result = {0};
  pthread_barrier_init(L1_(&result.handle), 0, count);
  return result;
}

Internal void os_barrier_release(L1 barrier) {
  L1 handle_ptr = L1_(&TR_(OS_Barrier, barrier)->handle);
  pthread_barrier_destroy(handle_ptr);
}

Internal void os_barrier_wait(L1 barrier) {
  L1 handle_ptr = L1_(&TR_(OS_Barrier, barrier)->handle);
  pthread_barrier_wait(handle_ptr);
}

Internal L1 os_clock(void) {
  Posix_Time_Spec timespec = {0};
  clock_gettime(CLOCK_MONOTONIC, L1_(&timespec));
  L1 result = timespec.seconds * 1000000000 + timespec.nanoseconds;
  return result;
}

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

Internal void os_push_event(OS_Event event) {
  L1 idx = ramR->event_count;
  Assert(idx < MAX_EVENT_COUNT);

  ramR->events[idx] = event;
  ramR->event_count += 1;
}

Internal void registry_global_handler(void *data, struct wl_registry *registry,
                                      I1 name, CString interface, I1 version) {
  if (cstr_compare(interface, wl_compositor_interface.name)) {
    ramR->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, 4);
  } else if (cstr_compare(interface, xdg_wm_base_interface.name)) {
    ramR->xdg_wm_base =
        wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
  } else if (cstr_compare(interface, wl_seat_interface.name)) {
    ramR->seat = wl_registry_bind(registry, name, &wl_seat_interface, 5);
  }
}

Internal void registry_global_remove_handler(void *data,
                                             struct wl_registry *registry,
                                             I1 name) {}

Internal void xdg_wm_base_ping_handler(void *data,
                                       struct xdg_wm_base *xdg_wm_base,
                                       I1 serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

Internal void xdg_surface_configure_handler(void *data,
                                            struct xdg_surface *xdg_surface,
                                            I1 serial) {
  TR(OS_Window) window = TR_(OS_Window, data);
  xdg_surface_ack_configure(xdg_surface, serial);
  window->configured = 1;
}

Internal void xdg_toplevel_configure_handler(void *data,
                                             struct xdg_toplevel *xdg_toplevel,
                                             SI1 width, SI1 height,
                                             struct wl_array *states) {
  if (width > 0 && height > 0) {
    TR(OS_Window) window = TR_(OS_Window, data);
    window->width = width;
    window->height = height;
    window->resized = 1;
  }
}

Internal void xdg_toplevel_close_handler(void *data,
                                         struct xdg_toplevel *xdg_toplevel) {
  TR(OS_Window) window = TR_(OS_Window, data);
  window->should_close = 1;
}

////////////////////////////////
//~ kti: Seat event handlers

Internal void seat_capabilities_handler(void *data, struct wl_seat *seat,
                                        I1 capabilities) {
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    ramR->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(ramR->pointer, &ramR->pointer_listener, 0);
  }
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    ramR->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(ramR->keyboard, &ramR->keyboard_listener, 0);
  }
}

Internal void seat_name_handler(void *data, struct wl_seat *seat, CString name) {}

// Pointer event handlers
Internal void pointer_enter_handler(void *data, struct wl_pointer *pointer,
                                    I1 serial, struct wl_surface *surface,
                                    wl_fixed_t surface_x,
                                    wl_fixed_t surface_y) {
  TR(OS_Window) window;
  for (L1 it = ramR->first_window; it != 0; it = TR_(OS_Window, it)->next) {
    window = TR_(OS_Window, it);
    if (surface == window->surface) {
      break;
    }
  }

  ramR->hovered_window = L1_(window);
}

Internal void pointer_leave_handler(void *data, struct wl_pointer *pointer,
                                    I1 serial, struct wl_surface *surface) {
  ramR->hovered_window = 0;
}

Internal void pointer_motion_handler(void *data, struct wl_pointer *pointer,
                                     I1 time, wl_fixed_t surface_x,
                                     wl_fixed_t surface_y) {
  OS_Event event = {
    .timestamp_ns = time * 1000000,
    .type = OS_EVENT_TYPE__MOUSE_MOVE,
    .x = wl_fixed_to_double(surface_x),
    .y = wl_fixed_to_double(surface_y),
  };
  os_push_event(event);
}

Internal void pointer_button_handler(void *data, struct wl_pointer *pointer,
                                     I1 serial, I1 time, I1 button, I1 state) {
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
    OS_Event event = {
      .timestamp_ns = time * 1000000,
      .type = ((state == WL_POINTER_BUTTON_STATE_PRESSED) ? OS_EVENT_TYPE__PRESS : OS_EVENT_TYPE__RELEASE),
      .key = key,
    };
    os_push_event(event);
  }
}

Internal void pointer_axis_handler(void *data, struct wl_pointer *pointer,
                                   I1 time, I1 axis, wl_fixed_t value) {
  OS_Event event = {
    .timestamp_ns = time * 1000000,
    .type = OS_EVENT_TYPE__SCROLL,
  };
  if (axis == 0) {
    event.y = wl_fixed_to_double(value);
  } else if (axis == 1) {
    event.x = wl_fixed_to_double(value);
  }

  os_push_event(event);
}

Internal void pointer_frame_handler(void *data, struct wl_pointer *pointer) {}

Internal void pointer_axis_source_handler(void *data,
                                          struct wl_pointer *pointer,
                                          I1 axis_source) {}

Internal void pointer_axis_stop_handler(void *data, struct wl_pointer *pointer,
                                        I1 time, I1 axis) {}

Internal void pointer_axis_discrete_handler(void *data,
                                            struct wl_pointer *pointer, I1 axis,
                                            SI1 discrete) {}

////////////////////////////////
//~ kti: Keyboard Event Handlers

Internal void keyboard_keymap_handler(void *data, struct wl_keyboard *keyboard,
                                      I1 format, SI1 fd, I1 size) {
  close(fd);
}

Internal void keyboard_enter_handler(void *data, struct wl_keyboard *keyboard,
                                     I1 serial, struct wl_surface *surface,
                                     struct wl_array *keys) {}

Internal void keyboard_leave_handler(void *data, struct wl_keyboard *keyboard,
                                     I1 serial, struct wl_surface *surface) {}

Internal void keyboard_key_handler(void *data, struct wl_keyboard *keyboard,
                                   I1 serial, I1 time, I1 key, I1 state) {
  I1 os_key = os_key_from_wl_key(key);
  if (os_key != OS_KEY__NULL) {
    OS_Event event = {
      .timestamp_ns = time * 1000000,
      .type = (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? OS_EVENT_TYPE__PRESS : OS_EVENT_TYPE__RELEASE,
      .key = os_key,
    };
    os_push_event(event);
  }
}

Internal void keyboard_modifiers_handler(void *data,
                                         struct wl_keyboard *keyboard,
                                         I1 serial, I1 mods_depressed,
                                         I1 mods_latched, I1 mods_locked,
                                         I1 group) {}

Internal void keyboard_repeat_info_handler(void *data,
                                           struct wl_keyboard *keyboard,
                                           SI1 rate, SI1 delay) {}

Internal L1 os_window_open(L1 arena, String8 title, I1 width, I1 height) {
  if (ramR->first_window == 0) {
    ramR->display = wl_display_connect(0);
    Assert(ramR->display != 0);

    struct wl_registry *registry = wl_display_get_registry(ramR->display);
    Assert(registry != 0);

    struct wl_registry_listener registry_listener = {
        .global = registry_global_handler,
        .global_remove = registry_global_remove_handler,
    };
    wl_registry_add_listener(registry, &registry_listener, 0);
    wl_display_roundtrip(ramR->display);
    Assert(ramR->compositor != 0);
    Assert(ramR->xdg_wm_base != 0);
    Assert(ramR->seat != 0);

    ramR->xdg_wm_base_listener.ping = xdg_wm_base_ping_handler;
    xdg_wm_base_add_listener(ramR->xdg_wm_base, &ramR->xdg_wm_base_listener, 0);

    ramR->seat_listener.capabilities = seat_capabilities_handler;
    ramR->seat_listener.name = seat_name_handler;
    wl_seat_add_listener(ramR->seat, &ramR->seat_listener, 0);

    ramR->pointer_listener.enter = pointer_enter_handler;
    ramR->pointer_listener.leave = pointer_leave_handler;
    ramR->pointer_listener.motion = pointer_motion_handler;
    ramR->pointer_listener.button = pointer_button_handler;
    ramR->pointer_listener.axis = pointer_axis_handler;
    ramR->pointer_listener.frame = pointer_frame_handler;
    ramR->pointer_listener.axis_source = pointer_axis_source_handler;
    ramR->pointer_listener.axis_stop = pointer_axis_stop_handler;
    ramR->pointer_listener.axis_discrete = pointer_axis_discrete_handler;

    ramR->keyboard_listener.keymap = keyboard_keymap_handler;
    ramR->keyboard_listener.enter = keyboard_enter_handler;
    ramR->keyboard_listener.leave = keyboard_leave_handler;
    ramR->keyboard_listener.key = keyboard_key_handler;
    ramR->keyboard_listener.modifiers = keyboard_modifiers_handler;
    ramR->keyboard_listener.repeat_info = keyboard_repeat_info_handler;

    wl_display_roundtrip(ramR->display);

    ramR->egl_display = eglGetDisplay((EGLNativeDisplayType)ramR->display);
    Assert(ramR->egl_display != EGL_NO_DISPLAY);

    EGLint major, minor;
    I1 egl_initialized = eglInitialize(ramR->egl_display, &major, &minor);
    Assert(egl_initialized);

    EGLint config_attribs[] = {EGL_SURFACE_TYPE,
                               EGL_WINDOW_BIT,
                               EGL_RENDERABLE_TYPE,
                               EGL_OPENGL_BIT,
                               EGL_RED_SIZE,
                               8,
                               EGL_GREEN_SIZE,
                               8,
                               EGL_BLUE_SIZE,
                               8,
                               EGL_ALPHA_SIZE,
                               8,
                               EGL_DEPTH_SIZE,
                               24,
                               EGL_NONE};

    EGLint num_configs;
    I1 config_chosen = eglChooseConfig(ramR->egl_display, config_attribs, &ramR->egl_config, 1, &num_configs);
    Assert(config_chosen && num_configs > 0);

    eglBindAPI(EGL_OPENGL_API);

    EGLint context_attribs[] = {EGL_CONTEXT_MAJOR_VERSION,
                                3,
                                EGL_CONTEXT_MINOR_VERSION,
                                3,
                                EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
                                EGL_NONE};
    ramR->egl_context = eglCreateContext(ramR->egl_display, ramR->egl_config,
                                         EGL_NO_CONTEXT, context_attribs);
    Assert(ramR->egl_context != EGL_NO_CONTEXT);
  }

  TR(OS_Window) result = TR_(OS_Window, push_array(arena, OS_Window, 1));

  result->surface = wl_compositor_create_surface(ramR->compositor);
  Assert(result->surface != 0);

  result->xdg_surface = xdg_wm_base_get_xdg_surface(ramR->xdg_wm_base, result->surface);
  Assert(result->xdg_surface);

  result->xdg_surface_listener.configure = xdg_surface_configure_handler;
  xdg_surface_add_listener(result->xdg_surface, &result->xdg_surface_listener, (void *)result);

  result->xdg_toplevel = xdg_surface_get_toplevel(result->xdg_surface);
  Assert(result->xdg_toplevel);

  result->xdg_toplevel_listener.configure = xdg_toplevel_configure_handler;
  result->xdg_toplevel_listener.close = xdg_toplevel_close_handler;
  xdg_toplevel_add_listener(result->xdg_toplevel, &result->xdg_toplevel_listener, (void *)result);

  //- kti: Use max filename length as a reasonable title length limit.
  Assert(title.len < MAX_FILENAME_LEN);
  char cstr_title[MAX_FILENAME_LEN] = {0};
  memmove(L1_(cstr_title), title.str, title.len);
  xdg_toplevel_set_title(result->xdg_toplevel, cstr_title);

  wl_surface_commit(result->surface);

  while (!result->configured) {
    wl_display_dispatch(ramR->display);
  }

  result->width = width;
  result->height = height;
  result->egl_window =
      wl_egl_window_create(result->surface, result->width, result->height);
  Assert(result->egl_window != 0);

  result->egl_surface =
      eglCreateWindowSurface(ramR->egl_display, ramR->egl_config,
                             (EGLNativeWindowType)result->egl_window, 0);
  Assert(result->egl_surface != EGL_NO_SURFACE);

  result->next = ramR->first_window;
  if (ramR->first_window != 0) {
    TR_(OS_Window, ramR->first_window)->prev = L1_(result);
  }
  ramR->first_window = L1_(result);

  return L1_(result);
}

Internal void os_window_get_context(L1 window_ptr) {
  TR(OS_Window) window = TR_(OS_Window, window_ptr);
  if (window_ptr == 0) {
    eglMakeCurrent(ramR->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
  } else {
    I1 made_current = eglMakeCurrent(ramR->egl_display, window->egl_surface,
                                     window->egl_surface, ramR->egl_context);
    Assert(made_current);
  }
}

Internal void os_window_swap_buffers(L1 window_ptr) {
  TR(OS_Window) window = TR_(OS_Window, window_ptr);
  eglSwapBuffers(ramR->egl_display, window->egl_surface);
}

Internal void os_poll_events(void) {
  Assert(ramR->first_window != 0);

  ramR->event_count = 0;

  while (wl_display_prepare_read(ramR->display) != 0) {
    wl_display_dispatch_pending(ramR->display);
  }
  wl_display_flush(ramR->display);
  wl_display_read_events(ramR->display);
  wl_display_dispatch_pending(ramR->display);

  for (L1 it = ramR->first_window; it != 0; it = TR_(OS_Window, it)->next) {
    TR(OS_Window) window = TR_(OS_Window, it);

    if (window->resized) {
      window->resized = 0;
      wl_egl_window_resize(window->egl_window, window->width, window->height, 0, 0);
    }
  }
}

Internal void os_window_close(L1 window_ptr) {
  TR(OS_Window) window = TR_(OS_Window, window_ptr);

  eglDestroySurface(ramR->egl_display, window->egl_surface);
  wl_egl_window_destroy(window->egl_window);

  xdg_toplevel_destroy(window->xdg_toplevel);
  xdg_surface_destroy(window->xdg_surface);
  wl_surface_destroy(window->surface);

  if (ramR->first_window == window_ptr && window->next == 0) {
    os_window_get_context(0);

    if (ramR->pointer)
      wl_pointer_destroy(ramR->pointer);
    if (ramR->keyboard)
      wl_keyboard_destroy(ramR->keyboard);
    if (ramR->seat)
      wl_seat_destroy(ramR->seat);

    eglDestroyContext(ramR->egl_display, ramR->egl_context);
    eglTerminate(ramR->egl_display);

    xdg_wm_base_destroy(ramR->xdg_wm_base);
    wl_display_disconnect(ramR->display);
  }

  if (window->prev != 0) {
    TR_(OS_Window, window->prev)->next = window->next;
  }
  if (window->next != 0) {
    TR_(OS_Window, window->next)->prev = window->prev;
  }
  if (window_ptr == ramR->first_window) {
    ramR->first_window = window->next;
  }
}

#endif