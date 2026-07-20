#if (HEADER)

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
#include "os_mac.c"
#else
#include "os_wayland.c"
#endif

#endif

#if (SOURCE)

Global OS_GFX_State *os_gfx_state = 0;

Internal String8 os_read_entire_file(Arena *arena, String8 filename) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);
  I1 file = open((CString)cstr_filename.str, O_RDONLY);
  if (LtSI1(file, 0)) {
    scratch_end(scratch);
    return (String8){0};
  }

  L1 size = lseek(file, 0, SEEK_END);
  lseek(file, 0, SEEK_SET);
  B1 *buffer = push_array(arena, B1, size);
  L1 bytes_read = read(file, buffer, size);
  close(file);
  scratch_end(scratch);
  Assert(size == bytes_read);

  return (String8){.str = buffer, .len = size};
}

Internal L1 os_write_entire_file(String8 filename, void *data, L1 data_size) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);
  I1 file = open((CString)cstr_filename.str, O_CREAT | O_WRONLY | O_TRUNC, 0666);
  if (LtSI1(file, 0)) {
    scratch_end(scratch);
    return 0;
  }

  L1 bytes_written = write(file, data, data_size);
  close(file);
  scratch_end(scratch);
  return bytes_written;
}

Internal void *os_reserve(L1 size) {
  void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
  return result == MAP_FAILED ? 0 : result;
}

Internal void os_commit(void *ptr, L1 size) {
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

Internal void os_decommit(void *ptr, L1 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

Internal void os_memory_release(void *ptr, L1 size) {
  munmap(ptr, size);
}

Internal I1 os_num_cores(void) {
  long count = sysconf(_SC_NPROCESSORS_ONLN);
  return count > 0 ? (I1)count : 1;
}

Internal OS_Thread os_thread_launch(ThreadFunc *func, void *ptr) {
  OS_Thread result = {0};
  pthread_create(&result.handle, 0, func, ptr);
  return result;
}

Internal void os_thread_join(OS_Thread thread) {
  pthread_join(thread.handle, 0);
}

Internal OS_Barrier os_barrier_alloc(I1 count) {
  OS_Barrier result = {.threshold = count, .count = count};
  pthread_mutex_init(&result.mutex, 0);
  pthread_cond_init(&result.condition, 0);
  return result;
}

Internal void os_barrier_release(OS_Barrier *barrier) {
  pthread_cond_destroy(&barrier->condition);
  pthread_mutex_destroy(&barrier->mutex);
}

Internal void os_barrier_wait(OS_Barrier *barrier) {
  pthread_mutex_lock(&barrier->mutex);
  I1 generation = barrier->generation;
  barrier->count -= 1;
  if (barrier->count == 0) {
    barrier->generation += 1;
    barrier->count = barrier->threshold;
    pthread_cond_broadcast(&barrier->condition);
  } else {
    while (generation == barrier->generation) {
      pthread_cond_wait(&barrier->condition, &barrier->mutex);
    }
  }
  pthread_mutex_unlock(&barrier->mutex);
}

Internal L1 os_clock(void) {
  struct timespec ts = {0};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (L1)ts.tv_sec * 1000000000LLU + (L1)ts.tv_nsec;
}

Internal void os_sleep(L1 time) {
  struct timespec ts = {
    .tv_sec = time / 1000000000LLU,
    .tv_nsec = time % 1000000000LLU,
  };
  nanosleep(&ts, 0);
}

Internal void *os_library_open(String8 filename) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);
  void *handle = dlopen((CString)cstr_filename.str, RTLD_LAZY | RTLD_LOCAL);
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
  if (handle != 0) {
    dlclose(handle);
  }
}

#if defined(__APPLE__)
#include "os_mac.c"
#else
#include "os_wayland.c"
#endif

#endif
