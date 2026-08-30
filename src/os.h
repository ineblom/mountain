#ifndef OS_H
#define OS_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>

typedef void *ThreadFunc(void *);

typedef struct OS_Thread OS_Thread;
struct OS_Thread {
  pthread_t handle;
};

typedef struct OS_Barrier OS_Barrier;
struct OS_Barrier {
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  I1 threshold;
  I1 count;
  I1 generation;
};

typedef struct OS_Window OS_Window;

enum {
  OS_EVENT_KIND__NULL,
  OS_EVENT_KIND__PRESS,
  OS_EVENT_KIND__RELEASE,
  OS_EVENT_KIND__TEXT,
  OS_EVENT_KIND__MOUSE_MOVE,
  OS_EVENT_KIND__SCROLL,
  OS_EVENT_KIND__WINDOW_CLOSE,
  OS_EVENT_KIND_COUNT,
};

typedef I1 OS_Key;

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
  I1 kind;
  OS_Key key;
  I1 is_repeat;
  B1 text[4];
  L1 text_len;
  OS_Modifier_Flags modifiers;
  D1 x, y;
  D1 delta_x, delta_y;
};
typedef struct OS_Event_List OS_Event_List;
struct OS_Event_List {
  OS_Event *first;
  OS_Event *last;
  L1 count;
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
  OS_KEY__ENTER,
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

Internal void *os_reserve(L1);
Internal void os_commit(void *, L1);
Internal void os_memory_release(void *, L1);
Internal L1 os_clock(void);

#if defined(__APPLE__)
#include "os_mac.h"
#else
#include "os_wayland.h"
#endif

#endif
