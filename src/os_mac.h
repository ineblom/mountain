#ifndef OS_MAC_H
#define OS_MAC_H

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

struct OS_Window {
  OS_Window *prev;
  OS_Window *next;

  SI1 width;
  SI1 height;
  D1 pixel_ratio;

  NSWindow *window;
  NSView *view;
  id delegate;
  CAMetalLayer *metal_layer;
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

  OS_Modifier_Flags modifiers;
  I1 initialized;
};

#endif
