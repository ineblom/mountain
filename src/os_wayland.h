#ifndef OS_WAYLAND_H
#define OS_WAYLAND_H

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <xdg-shell-protocol.h>
#include <poll.h>

typedef struct OS_Wayland_Output OS_Wayland_Output;
struct OS_Wayland_Output {
  OS_Wayland_Output *next;
  I1 registry_name;
  SI1 scale;
  struct wl_output *output;
  struct wl_output_listener listener;
};

typedef struct OS_Wayland_Window_Output OS_Wayland_Window_Output;
struct OS_Wayland_Window_Output {
  OS_Wayland_Window_Output *next;
  OS_Wayland_Output *output;
};

struct OS_Window {
  OS_Window *prev;
  OS_Window *next;

  SI1 width;
  SI1 height;
  D1 pixel_ratio;

  OS_Wayland_Window_Output *first_output;
  OS_Wayland_Window_Output *first_free_output;
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_surface_listener surface_listener;
  struct xdg_surface_listener xdg_surface_listener;
  struct xdg_toplevel_listener xdg_toplevel_listener;
  I1 has_preferred_buffer_scale;
  I1 configured;
};

typedef struct OS_GFX_State OS_GFX_State;
struct OS_GFX_State {
  Arena *arena;
  OS_Window *hovered_window;
  OS_Window *focused_window;
  OS_Window *first_window;
  Arena *event_arena;
  OS_Event_List events;
  B1 key_states[OS_KEY_COUNT];
  D1 mouse_x;
  D1 mouse_y;

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
  OS_Wayland_Output *first_output;
  SI1 cursor_scale;
  I1 cursor_enter_serial;
  struct wl_registry_listener registry_listener;
  struct xdg_wm_base_listener xdg_wm_base_listener;
  struct wl_seat_listener seat_listener;
  struct wl_pointer_listener pointer_listener;
  struct wl_keyboard_listener keyboard_listener;
  struct xkb_context *xkb_context;
  struct xkb_keymap *xkb_keymap;
  struct xkb_state *xkb_state;
  SI1 key_repeat_rate;
  SI1 key_repeat_delay_ms;
  OS_Key repeat_key;
  I1 repeat_wl_key;
  OS_Window *repeat_window;
  L1 next_repeat_ns;
};

#endif
