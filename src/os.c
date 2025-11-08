#if (CPU_ && DEF_)

// TODO: What garbage does this bring in?
// Can we minimize or stop relying on it completely?
#include <wayland-client.h>
#include "xdg-shell-protocol.h"
#include "xdg-shell-protocol.c"
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GL/gl.h>

#define PROT_READ 0x1
#define PROT_WRITE 02
#define PROT_NONE 0x0
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_ANON 0x20
#define MAP_FAILED L1_(-1)
#define MADV_DONTNEED 4

#define O_RDWR   2
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT  100
#define O_EXCL   200
#define SEEK_SET 0
#define SEEK_END 2

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define CLOCK_MONOTONIC 1

#define MAX_FILENAME_LEN 256

typedef L1 pthread_t;
typedef struct {
  B1 __opaque[32];
} pthread_barrier_t;

#endif

#if (CPU_ && TYP_)

typedef L1 ThreadFunc(L1);

typedef struct Thread Thread;
struct Thread {
  pthread_t handle;
};

typedef struct Barrier Barrier;
struct Barrier {
  pthread_barrier_t handle;
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

  I1 configured;
  I1 should_close;
  SI1 width;
  SI1 height;
  I1 resized;

  struct xdg_surface_listener xdg_surface_listener;
  struct xdg_toplevel_listener xdg_toplevel_listener;

  L1 prev;
  L1 next;
};

#endif

#if (CPU_ && RAM_)

L1 focused_window;
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

F1 mouse_x;
F1 mouse_y;
F1 scroll_x;
F1 scroll_y;
I1 mouse_buttons[8];
I1 keys[256];

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
  L1 result = L1_(mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0));
  if (result == MAP_FAILED) {
    result = 0;
  }
  return result;
}

Internal void os_commit(L1 ptr, L1 size) {
  mprotect(ptr, size, PROT_READ|PROT_WRITE);
}

Internal void os_decommit(L1 ptr, L1 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

Internal void os_release(L1 ptr, L1 size) {
  munmap(ptr, size);
}

Internal I1 os_num_cores(void) {
  return get_nprocs();
}

Internal Thread os_thread_launch(ThreadFunc *func, L1 ptr) {
  Thread result = {0};

  pthread_create(L1_(&result.handle), 0, L1_(func), ptr);

  return result;
}

Internal void os_thread_join(Thread thread) {
  pthread_join(thread.handle, 0);
}

Internal Barrier os_barrier_alloc(I1 count) {
  Barrier result = {0};
  pthread_barrier_init(L1_(&result.handle), 0, count);
  return result;
}

Internal void os_barrier_release(L1 barrier) {
  L1 handle_ptr = L1_(&TR_(Barrier, barrier)->handle);
  pthread_barrier_destroy(handle_ptr);
}

Internal void os_barrier_wait(L1 barrier) {
  L1 handle_ptr = L1_(&TR_(Barrier, barrier)->handle);
  pthread_barrier_wait(handle_ptr);
}

Internal L1 os_clock(void) {
  Posix_Time_Spec timespec = {0};
  clock_gettime(CLOCK_MONOTONIC, L1_(&timespec));
  L1 result = timespec.seconds * 1000000000 + timespec.nanoseconds;
  return result;
}


Internal void registry_global_handler(void *data, struct wl_registry *registry, I1 name, CString interface, I1 version)
{
  if (cstr_compare(interface, wl_compositor_interface.name)) {
    ramR->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
  } else if (cstr_compare(interface, xdg_wm_base_interface.name)) {
    ramR->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
  } else if (cstr_compare(interface, wl_seat_interface.name)) {
    ramR->seat = wl_registry_bind(registry, name, &wl_seat_interface, 5);
  }
}

Internal void registry_global_remove_handler(
  void *data,
  struct wl_registry *registry,
  I1 name)
{}

Internal void xdg_wm_base_ping_handler(
  void *data,
  struct xdg_wm_base *xdg_wm_base,
  I1 serial)
{
  xdg_wm_base_pong(xdg_wm_base, serial);
}

Internal void xdg_surface_configure_handler(
  void *data,
  struct xdg_surface *xdg_surface,
  I1 serial)
{
  TR(OS_Window) window = TR_(OS_Window, data);
  xdg_surface_ack_configure(xdg_surface, serial);
  window->configured = 1;
}

Internal void xdg_toplevel_configure_handler(
  void *data,
  struct xdg_toplevel *xdg_toplevel,
  SI1 width,
  SI1 height,
  struct wl_array *states)
{
  if (width > 0 && height > 0) {
    TR(OS_Window) window = TR_(OS_Window, data);
    window->width = width;
    window->height = height;
    window->resized = 1;
  }
}

Internal void xdg_toplevel_close_handler(
  void *data,
  struct xdg_toplevel *xdg_toplevel)
{
  TR(OS_Window) window = TR_(OS_Window, data);
  window->should_close = 1;
}

// Seat event handlers
Internal void seat_capabilities_handler(
  void *data,
  struct wl_seat *seat,
  I1 capabilities)
{
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    ramR->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(ramR->pointer, &ramR->pointer_listener, 0);
  }
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    ramR->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(ramR->keyboard, &ramR->keyboard_listener, 0);
  }
}

Internal void seat_name_handler(
  void *data,
  struct wl_seat *seat,
  CString name)
{}

// Pointer event handlers
Internal void pointer_enter_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 serial,
  struct wl_surface *surface,
  wl_fixed_t surface_x,
  wl_fixed_t surface_y)
{
  TR(OS_Window) window;
  for (L1 it = ramR->first_window; it != 0; it = TR_(OS_Window, it)->next) {
    window = TR_(OS_Window, it);
    if (surface == window->surface) {
      break;
    }
  }

  ramR->focused_window = L1_(window);
}

Internal void pointer_leave_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 serial,
  struct wl_surface *surface)
{
  ramR->focused_window = 0;
}

Internal void pointer_motion_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 time,
  wl_fixed_t surface_x,
  wl_fixed_t surface_y)
{
  ramR->mouse_x = F1_(wl_fixed_to_double(surface_x));
  ramR->mouse_y = F1_(wl_fixed_to_double(surface_y));
}

Internal void pointer_button_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 serial,
  I1 time,
  I1 button,
  I1 state)
{
  // Map Linux button codes to indices (BTN_LEFT=0x110, BTN_RIGHT=0x111, BTN_MIDDLE=0x112)
  I1 button_index = button - 0x110;
  if (button_index >= 0 && button_index < 8) {
    ramR->mouse_buttons[button_index] = (state == WL_POINTER_BUTTON_STATE_PRESSED);
  }
}

Internal void pointer_axis_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 time,
  I1 axis,
  wl_fixed_t value)
{
  if (axis == 0) {
    ramR->scroll_y = wl_fixed_to_double(value);
  } else if (axis == 1) {
    ramR->scroll_x = wl_fixed_to_double(value);
  }
}

Internal void pointer_frame_handler(
  void *data,
  struct wl_pointer *pointer)
{}

Internal void pointer_axis_source_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 axis_source)
{}

Internal void pointer_axis_stop_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 time,
  I1 axis)
{}

Internal void pointer_axis_discrete_handler(
  void *data,
  struct wl_pointer *pointer,
  I1 axis,
  SI1 discrete)
{
}

// Keyboard event handlers
Internal void keyboard_keymap_handler(
  void *data,
  struct wl_keyboard *keyboard,
  I1 format,
  SI1 fd,
  I1 size)
{
  close(fd);
}

Internal void keyboard_enter_handler(
  void *data,
  struct wl_keyboard *keyboard,
  I1 serial,
  struct wl_surface *surface,
  struct wl_array *keys)
{}

Internal void keyboard_leave_handler(
  void *data,
  struct wl_keyboard *keyboard,
  I1 serial,
  struct wl_surface *surface)
{}

Internal void keyboard_key_handler(
  void *data,
  struct wl_keyboard *keyboard,
  I1 serial,
  I1 time,
  I1 key,
  I1 state)
{
  if (key < 256) {
    ramR->keys[key] = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
  }
}

Internal void keyboard_modifiers_handler(
  void *data,
  struct wl_keyboard *keyboard,
  I1 serial,
  I1 mods_depressed,
  I1 mods_latched,
  I1 mods_locked,
  I1 group)
{}

Internal void keyboard_repeat_info_handler(
  void *data,
  struct wl_keyboard *keyboard,
  SI1 rate,
  SI1 delay)
{}

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

    EGLint config_attribs[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 24,
      EGL_NONE
    };

    EGLint num_configs;
    I1 config_chosen = eglChooseConfig(ramR->egl_display, config_attribs, &ramR->egl_config, 1, &num_configs);
    Assert(config_chosen && num_configs > 0);

    eglBindAPI(EGL_OPENGL_API);

    EGLint context_attribs[] = {
      EGL_CONTEXT_MAJOR_VERSION, 3,
      EGL_CONTEXT_MINOR_VERSION, 3,
      EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
      EGL_NONE
    };
    ramR->egl_context = eglCreateContext(ramR->egl_display, ramR->egl_config, EGL_NO_CONTEXT, context_attribs);
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

  // Use max filename length as a reasonable title length limit.
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
  result->egl_window = wl_egl_window_create(result->surface, result->width, result->height);
  Assert(result->egl_window != 0);

  result->egl_surface = eglCreateWindowSurface(
    ramR->egl_display,
    ramR->egl_config,
    (EGLNativeWindowType)result->egl_window,
    0);
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
    eglMakeCurrent(ramR->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  } else {
    I1 made_current = eglMakeCurrent(
      ramR->egl_display,
      window->egl_surface,
      window->egl_surface,
      ramR->egl_context);
    Assert(made_current);
  }
}

Internal void os_window_swap_buffers(L1 window_ptr) {
  TR(OS_Window) window = TR_(OS_Window, window_ptr);
  eglSwapBuffers(ramR->egl_display, window->egl_surface);
}

Internal void os_poll_events(void) {
  Assert(ramR->first_window != 0);

  ramR->scroll_x = 0.0f;
  ramR->scroll_y = 0.0f;

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

    if (ramR->pointer) wl_pointer_destroy(ramR->pointer);
    if (ramR->keyboard) wl_keyboard_destroy(ramR->keyboard);
    if (ramR->seat) wl_seat_destroy(ramR->seat);

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

Internal I1 os_mouse_button(I1 button) {
  if (button >= 0 && button < 8) {
    return ramR->mouse_buttons[button];
  }
  return 0;
}

Internal I1 os_key(I1 keycode) {
  if (keycode >= 0 && keycode < 256) {
    return ramR->keys[keycode];
  }
  return 0;
}


#endif