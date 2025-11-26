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
  I1 x, y;
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

// Wayland

#define WL_KEY__NULL 0
#define WL_KEY__ESC 1
#define WL_KEY__1 2
#define WL_KEY__2 3
#define WL_KEY__3 4
#define WL_KEY__4 5
#define WL_KEY__5 6
#define WL_KEY__6 7
#define WL_KEY__7 8
#define WL_KEY__8 9
#define WL_KEY__9 10
#define WL_KEY__0 11
#define WL_KEY__MINUS 12
#define WL_KEY__EQUAL 13
#define WL_KEY__BACKSPACE 14
#define WL_KEY__TAB 15
#define WL_KEY__Q 16
#define WL_KEY__W 17
#define WL_KEY__E 18
#define WL_KEY__R 19
#define WL_KEY__T 20
#define WL_KEY__Y 21
#define WL_KEY__U 22
#define WL_KEY__I 23
#define WL_KEY__O 24
#define WL_KEY__P 25
#define WL_KEY__LEFTBRACE 26
#define WL_KEY__RIGHTBRACE 27
#define WL_KEY__ENTER 28
#define WL_KEY__LEFTCTRL 29
#define WL_KEY__A 30
#define WL_KEY__S 31
#define WL_KEY__D 32
#define WL_KEY__F 33
#define WL_KEY__G 34
#define WL_KEY__H 35
#define WL_KEY__J 36
#define WL_KEY__K 37
#define WL_KEY__L 38
#define WL_KEY__SEMICOLON 39
#define WL_KEY__APOSTROPHE 40
#define WL_KEY__GRAVE 41
#define WL_KEY__LEFTSHIFT 42
#define WL_KEY__BACKSLASH 43
#define WL_KEY__Z 44
#define WL_KEY__X 45
#define WL_KEY__C 46
#define WL_KEY__V 47
#define WL_KEY__B 48
#define WL_KEY__N 49
#define WL_KEY__M 50
#define WL_KEY__COMMA 51
#define WL_KEY__DOT 52
#define WL_KEY__SLASH 53
#define WL_KEY__RIGHTSHIFT 54
#define WL_KEY__KPASTERISK 55
#define WL_KEY__LEFTALT 56
#define WL_KEY__SPACE 57
#define WL_KEY__CAPSLOCK 58
#define WL_KEY__F1 59
#define WL_KEY__F2 60
#define WL_KEY__F3 61
#define WL_KEY__F4 62
#define WL_KEY__F5 63
#define WL_KEY__F6 64
#define WL_KEY__F7 65
#define WL_KEY__F8 66
#define WL_KEY__F9 67
#define WL_KEY__F10 68
#define WL_KEY__NUMLOCK 69
#define WL_KEY__SCROLLLOCK 70
#define WL_KEY__KP7 71
#define WL_KEY__KP8 72
#define WL_KEY__KP9 73
#define WL_KEY__KPMINUS 74
#define WL_KEY__KP4 75
#define WL_KEY__KP5 76
#define WL_KEY__KP6 77
#define WL_KEY__KPPLUS 78
#define WL_KEY__KP1 79
#define WL_KEY__KP2 80
#define WL_KEY__KP3 81
#define WL_KEY__KP0 82
#define WL_KEY__KPDOT 83
#define WL_KEY__ZENKAKUHANKAKU 85
#define WL_KEY__102ND 86
#define WL_KEY__F11 87
#define WL_KEY__F12 88
#define WL_KEY__RO 89
#define WL_KEY__KATAKANA 90
#define WL_KEY__HIRAGANA 91
#define WL_KEY__HENKAN 92
#define WL_KEY__KATAKANAHIRAGANA 93
#define WL_KEY__MUHENKAN 94
#define WL_KEY__KPJPCOMMA 95
#define WL_KEY__KPENTER 96
#define WL_KEY__RIGHTCTRL 97
#define WL_KEY__KPSLASH 98
#define WL_KEY__SYSRQ 99
#define WL_KEY__RIGHTALT 100
#define WL_KEY__LINEFEED 101
#define WL_KEY__HOME 102
#define WL_KEY__UP 103
#define WL_KEY__PAGEUP 104
#define WL_KEY__LEFT 105
#define WL_KEY__RIGHT 106
#define WL_KEY__END 107
#define WL_KEY__DOWN 108
#define WL_KEY__PAGEDOWN 109
#define WL_KEY__INSERT 110
#define WL_KEY__DELETE 111
#define WL_KEY__MACRO 112
#define WL_KEY__MUTE 113
#define WL_KEY__VOLUMEDOWN 114
#define WL_KEY__VOLUMEUP 115
#define WL_KEY__POWER 116 /* SC System Power Down */
#define WL_KEY__KPEQUAL 117
#define WL_KEY__KPPLUSMINUS 118
#define WL_KEY__PAUSE 119
#define WL_KEY__SCALE 120 /* AL Compiz Scale (Expose) */
#define WL_KEY__KPCOMMA 121
#define WL_KEY__HANGEUL 122
#define WL_KEY__HANGUEL KEY_HANGEUL
#define WL_KEY__HANJA 123
#define WL_KEY__YEN 124
#define WL_KEY__LEFTMETA 125
#define WL_KEY__RIGHTMETA 126
#define WL_KEY__COMPOSE 127
#define WL_KEY__STOP 128 /* AC Stop */
#define WL_KEY__AGAIN 129
#define WL_KEY__PROPS 130 /* AC Properties */
#define WL_KEY__UNDO 131  /* AC Undo */
#define WL_KEY__FRONT 132
#define WL_KEY__COPY 133  /* AC Copy */
#define WL_KEY__OPEN 134  /* AC Open */
#define WL_KEY__PASTE 135 /* AC Paste */
#define WL_KEY__FIND 136  /* AC Search */
#define WL_KEY__CUT 137   /* AC Cut */
#define WL_KEY__HELP 138  /* AL Integrated Help Center */
#define WL_KEY__MENU 139  /* Menu (show menu) */
#define WL_KEY__CALC 140  /* AL Calculator */
#define WL_KEY__SETUP 141
#define WL_KEY__SLEEP 142  /* SC System Sleep */
#define WL_KEY__WAKEUP 143 /* System Wake Up */
#define WL_KEY__FILE 144   /* AL Local Machine Browser */
#define WL_KEY__SENDFILE 145
#define WL_KEY__DELETEFILE 146
#define WL_KEY__XFER 147
#define WL_KEY__PROG1 148
#define WL_KEY__PROG2 149
#define WL_KEY__WWW 150 /* AL Internet Browser */
#define WL_KEY__MSDOS 151
#define WL_KEY__COFFEE 152 /* AL Terminal Lock/Screensaver */
#define WL_KEY__SCREENLOCK KEY_COFFEE
#define WL_KEY__ROTATE_DISPLAY 153 /* Display orientation for e.g. tablets */
#define WL_KEY__DIRECTION KEY_ROTATE_DISPLAY
#define WL_KEY__CYCLEWINDOWS 154
#define WL_KEY__MAIL 155
#define WL_KEY__BOOKMARKS 156 /* AC Bookmarks */
#define WL_KEY__COMPUTER 157
#define WL_KEY__BACK 158    /* AC Back */
#define WL_KEY__FORWARD 159 /* AC Forward */
#define WL_KEY__CLOSECD 160
#define WL_KEY__EJECTCD 161
#define WL_KEY__EJECTCLOSECD 162
#define WL_KEY__NEXTSONG 163
#define WL_KEY__PLAYPAUSE 164
#define WL_KEY__PREVIOUSSONG 165
#define WL_KEY__STOPCD 166
#define WL_KEY__RECORD 167
#define WL_KEY__REWIND 168
#define WL_KEY__PHONE 169 /* Media Select Telephone */
#define WL_KEY__ISO 170
#define WL_KEY__CONFIG 171   /* AL Consumer Control Configuration */
#define WL_KEY__HOMEPAGE 172 /* AC Home */
#define WL_KEY__REFRESH 173  /* AC Refresh */
#define WL_KEY__EXIT 174     /* AC Exit */
#define WL_KEY__MOVE 175
#define WL_KEY__EDIT 176
#define WL_KEY__SCROLLUP 177
#define WL_KEY__SCROLLDOWN 178
#define WL_KEY__KPLEFTPAREN 179
#define WL_KEY__KPRIGHTPAREN 180
#define WL_KEY__NEW 181  /* AC New */
#define WL_KEY__REDO 182 /* AC Redo/Repeat */
#define WL_KEY__F13 183
#define WL_KEY__F14 184
#define WL_KEY__F15 185
#define WL_KEY__F16 186
#define WL_KEY__F17 187
#define WL_KEY__F18 188
#define WL_KEY__F19 189
#define WL_KEY__F20 190
#define WL_KEY__F21 191
#define WL_KEY__F22 192
#define WL_KEY__F23 193
#define WL_KEY__F24 194
#define WL_KEY__PLAYCD 200
#define WL_KEY__PAUSECD 201
#define WL_KEY__PROG3 202
#define WL_KEY__PROG4 203
#define WL_KEY__DASHBOARD 204 /* AL Dashboard */
#define WL_KEY__SUSPEND 205
#define WL_KEY__CLOSE 206 /* AC Close */
#define WL_KEY__PLAY 207
#define WL_KEY__FASTFORWARD 208
#define WL_KEY__BASSBOOST 209
#define WL_KEY__PRINT 210 /* AC Print */
#define WL_KEY__HP 211
#define WL_KEY__CAMERA 212
#define WL_KEY__SOUND 213
#define WL_KEY__QUESTION 214
#define WL_KEY__EMAIL 215
#define WL_KEY__CHAT 216
#define WL_KEY__SEARCH 217
#define WL_KEY__CONNECT 218
#define WL_KEY__FINANCE 219 /* AL Checkbook/Finance */
#define WL_KEY__SPORT 220
#define WL_KEY__SHOP 221
#define WL_KEY__ALTERASE 222
#define WL_KEY__CANCEL 223 /* AC Cancel */
#define WL_KEY__BRIGHTNESSDOWN 224
#define WL_KEY__BRIGHTNESSUP 225
#define WL_KEY__MEDIA 226
#define WL_KEY__SWITCHVIDEOMODE 227 /* Cycle between available video outputs (Monitor/LCD/TV-out/etc) */
#define WL_KEY__KBDILLUMTOGGLE 228
#define WL_KEY__KBDILLUMDOWN 229
#define WL_KEY__KBDILLUMUP 230
#define WL_KEY__SEND 231        /* AC Send */
#define WL_KEY__REPLY 232       /* AC Reply */
#define WL_KEY__FORWARDMAIL 233 /* AC Forward Msg */
#define WL_KEY__SAVE 234        /* AC Save */
#define WL_KEY__DOCUMENTS 235
#define WL_KEY__BATTERY 236
#define WL_KEY__BLUETOOTH 237
#define WL_KEY__WLAN 238
#define WL_KEY__UWB 239
#define WL_KEY__UNKNOWN 240
#define WL_KEY__VIDEO_NEXT 241       /* drive next video source */
#define WL_KEY__VIDEO_PREV 242       /* drive previous video source */
#define WL_KEY__BRIGHTNESS_CYCLE 243 /* brightness up, after max is min */
#define WL_KEY__BRIGHTNESS_AUTO 244 /* Set Auto Brightness: manual brightness control is off, rely on ambient */
#define WL_KEY__BRIGHTNESS_ZERO KEY_BRIGHTNESS_AUTO
#define WL_KEY__DISPLAY_OFF 245 /* display device to off state */
#define WL_KEY__WWAN 246        /* Wireless WAN (LTE, UMTS, GSM, etc.) */
#define WL_KEY__WIMAX KEY_WWAN
#define WL_KEY__RFKILL 247  /* Key that controls all radios */
#define WL_KEY__MICMUTE 248 /* Mute / unmute the microphone */

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
    .x = surface_x,
    .y = surface_y,
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
    event.y = value;
  } else if (axis == 1) {
    event.x = value;
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
  if (key < 256) {
    OS_Event event = {
      .timestamp_ns = time * 1000000,
      .type = (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? OS_EVENT_TYPE__PRESS : OS_EVENT_TYPE__RELEASE,
      .key = key,
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