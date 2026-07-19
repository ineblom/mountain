#if (HEADER)

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

struct OS_Window {
  NSWindow *window;
  NSView *view;
  id delegate;
  CAMetalLayer *metal_layer;
  OS_WINDOW_COMMON_MEMBERS;
};

typedef struct OS_GFX_State OS_GFX_State;
struct OS_GFX_State {
  OS_GFX_STATE_COMMON_MEMBERS;
  OS_Modifier_Flags modifiers;
  I1 initialized;
};

#endif

#if (SOURCE)

Internal OS_Modifier_Flags os_get_modifiers(void) {
  return os_gfx_state != 0 ? os_gfx_state->modifiers : 0;
}

Internal void os_event_list_push(Arena *arena, OS_Event_List *list, OS_Event event) {
  OS_Event *new_event = push_array(arena, OS_Event, 1);
  *new_event = event;
  new_event->modifiers = os_get_modifiers();
  DLLPushBack(list->first, list->last, new_event);
  list->count += 1;
}

Internal void os_push_event(OS_Event event) {
  if (os_gfx_state != 0 && os_gfx_state->event_arena != 0) {
    os_event_list_push(os_gfx_state->event_arena, &os_gfx_state->events, event);
  }
}

Internal L1 os_timestamp_from_mac_event(NSEvent *event) {
  return (L1)(event.timestamp * 1000000000.0);
}

Internal OS_Modifier_Flags os_modifiers_from_mac_flags(NSEventModifierFlags flags) {
  OS_Modifier_Flags result = 0;
  if (flags & (NSEventModifierFlagControl | NSEventModifierFlagCommand)) {
    result |= OS_MODIFIER_FLAG__CTRL;
  }
  if (flags & NSEventModifierFlagShift) {
    result |= OS_MODIFIER_FLAG__SHIFT;
  }
  if (flags & NSEventModifierFlagOption) {
    result |= OS_MODIFIER_FLAG__ALT;
  }
  return result;
}

Internal OS_Key os_key_from_mac_keycode(I1 keycode) {
  LocalPersist OS_Key table[128];
  LocalPersist I1 initialized;
  if (!initialized) {
    initialized = 1;
    table[0] = OS_KEY__A; table[1] = OS_KEY__S; table[2] = OS_KEY__D;
    table[3] = OS_KEY__F; table[4] = OS_KEY__H; table[5] = OS_KEY__G;
    table[6] = OS_KEY__Z; table[7] = OS_KEY__X; table[8] = OS_KEY__C;
    table[9] = OS_KEY__V; table[11] = OS_KEY__B; table[12] = OS_KEY__Q;
    table[13] = OS_KEY__W; table[14] = OS_KEY__E; table[15] = OS_KEY__R;
    table[16] = OS_KEY__Y; table[17] = OS_KEY__T; table[18] = OS_KEY__1;
    table[19] = OS_KEY__2; table[20] = OS_KEY__3; table[21] = OS_KEY__4;
    table[22] = OS_KEY__6; table[23] = OS_KEY__5; table[24] = OS_KEY__EQUAL;
    table[25] = OS_KEY__9; table[26] = OS_KEY__7; table[27] = OS_KEY__MINUS;
    table[28] = OS_KEY__8; table[29] = OS_KEY__0; table[30] = OS_KEY__RIGHT_BRACKET;
    table[31] = OS_KEY__O; table[32] = OS_KEY__U; table[33] = OS_KEY__LEFT_BRACKET;
    table[34] = OS_KEY__I; table[35] = OS_KEY__P; table[36] = OS_KEY__ENTER;
    table[37] = OS_KEY__L; table[38] = OS_KEY__J; table[39] = OS_KEY__QUOTE;
    table[40] = OS_KEY__K; table[41] = OS_KEY__SEMICOLON; table[42] = OS_KEY__BACK_SLASH;
    table[43] = OS_KEY__COMMA; table[44] = OS_KEY__SLASH; table[45] = OS_KEY__N;
    table[46] = OS_KEY__M; table[47] = OS_KEY__PERIOD; table[48] = OS_KEY__TAB;
    table[49] = OS_KEY__SPACE; table[50] = OS_KEY__TICK; table[51] = OS_KEY__BACKSPACE;
    table[53] = OS_KEY__ESC; table[54] = OS_KEY__CTRL; table[55] = OS_KEY__CTRL;
    table[56] = OS_KEY__SHIFT; table[57] = OS_KEY__CAPS_LOCK; table[58] = OS_KEY__ALT;
    table[59] = OS_KEY__CTRL; table[60] = OS_KEY__SHIFT; table[61] = OS_KEY__ALT;
    table[64] = OS_KEY__F17; table[65] = OS_KEY__NUM_PERIOD; table[67] = OS_KEY__NUM_STAR;
    table[69] = OS_KEY__NUM_PLUS; table[71] = OS_KEY__NUM_LOCK; table[75] = OS_KEY__NUM_SLASH;
    table[76] = OS_KEY__ENTER; table[78] = OS_KEY__NUM_MINUS; table[79] = OS_KEY__F18;
    table[80] = OS_KEY__F19; table[81] = OS_KEY__EQUAL; table[82] = OS_KEY__NUM0;
    table[83] = OS_KEY__NUM1; table[84] = OS_KEY__NUM2; table[85] = OS_KEY__NUM3;
    table[86] = OS_KEY__NUM4; table[87] = OS_KEY__NUM5; table[88] = OS_KEY__NUM6;
    table[89] = OS_KEY__NUM7; table[91] = OS_KEY__NUM8; table[92] = OS_KEY__NUM9;
    table[96] = OS_KEY__F5; table[97] = OS_KEY__F6; table[98] = OS_KEY__F7;
    table[99] = OS_KEY__F3; table[100] = OS_KEY__F8; table[101] = OS_KEY__F9;
    table[103] = OS_KEY__F11; table[105] = OS_KEY__F13; table[106] = OS_KEY__F16;
    table[107] = OS_KEY__F14; table[109] = OS_KEY__F10; table[111] = OS_KEY__F12;
    table[113] = OS_KEY__F15; table[114] = OS_KEY__INSERT; table[115] = OS_KEY__HOME;
    table[116] = OS_KEY__PAGE_UP; table[117] = OS_KEY__DELETE; table[118] = OS_KEY__F4;
    table[119] = OS_KEY__END; table[120] = OS_KEY__F2; table[121] = OS_KEY__PAGE_DOWN;
    table[122] = OS_KEY__F1; table[123] = OS_KEY__LEFT; table[124] = OS_KEY__RIGHT;
    table[125] = OS_KEY__DOWN; table[126] = OS_KEY__UP;
  }
  return keycode < ArrayCount(table) ? table[keycode] : OS_KEY__NULL;
}

Internal void os_mac_update_mouse(OS_Window *window, NSEvent *event) {
  NSPoint point = [window->view convertPoint:event.locationInWindow fromView:nil];
  os_gfx_state->mouse_x = point.x;
  os_gfx_state->mouse_y = point.y;
  os_gfx_state->hovered_window = window;
}

Internal void os_mac_push_mouse_move(OS_Window *window, NSEvent *event) {
  os_mac_update_mouse(window, event);
  os_push_event((OS_Event){
    .window = window,
    .timestamp_ns = os_timestamp_from_mac_event(event),
    .kind = OS_EVENT_KIND__MOUSE_MOVE,
    .x = os_gfx_state->mouse_x,
    .y = os_gfx_state->mouse_y,
  });
}

Internal OS_Key os_mac_mouse_key(NSEvent *event) {
  switch (event.buttonNumber) {
    case 0: return OS_MOUSE_BUTTON__LEFT;
    case 1: return OS_MOUSE_BUTTON__RIGHT;
    case 2: return OS_MOUSE_BUTTON__MIDDLE;
  }
  return OS_KEY__NULL;
}

Internal void os_mac_push_mouse_button(OS_Window *window, NSEvent *event, I1 kind) {
  OS_Key key = os_mac_mouse_key(event);
  if (key != OS_KEY__NULL) {
    os_mac_update_mouse(window, event);
    os_gfx_state->key_states[key] = kind == OS_EVENT_KIND__PRESS;
    os_push_event((OS_Event){
      .window = window,
      .timestamp_ns = os_timestamp_from_mac_event(event),
      .kind = kind,
      .key = key,
      .x = os_gfx_state->mouse_x,
      .y = os_gfx_state->mouse_y,
    });
  }
}

@interface MountainView : NSView <CALayerDelegate> {
@public
  OS_Window *os_window;
  NSTrackingArea *tracking_area;
}
@end

@interface MountainWindowDelegate : NSObject <NSWindowDelegate> {
@public
  OS_Window *os_window;
}
@end

@implementation MountainView

- (BOOL)isFlipped { return YES; }
- (BOOL)acceptsFirstResponder { return YES; }

- (void)updateTrackingAreas {
  if (tracking_area != nil) {
    [self removeTrackingArea:tracking_area];
    [tracking_area release];
  }
  tracking_area = [[NSTrackingArea alloc] initWithRect:self.bounds
    options:(NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways)
    owner:self userInfo:nil];
  [self addTrackingArea:tracking_area];
  [super updateTrackingAreas];
}

- (void)dealloc {
  if (tracking_area != nil) {
    [self removeTrackingArea:tracking_area];
    [tracking_area release];
  }
  [super dealloc];
}

- (void)mouseEntered:(NSEvent *)event { os_mac_push_mouse_move(os_window, event); }
- (void)mouseExited:(NSEvent *)event {
  if (os_gfx_state->hovered_window == os_window) os_gfx_state->hovered_window = 0;
}
- (void)mouseMoved:(NSEvent *)event { os_mac_push_mouse_move(os_window, event); }
- (void)mouseDragged:(NSEvent *)event { os_mac_push_mouse_move(os_window, event); }
- (void)rightMouseDragged:(NSEvent *)event { os_mac_push_mouse_move(os_window, event); }
- (void)otherMouseDragged:(NSEvent *)event { os_mac_push_mouse_move(os_window, event); }
- (void)mouseDown:(NSEvent *)event { os_mac_push_mouse_button(os_window, event, OS_EVENT_KIND__PRESS); }
- (void)mouseUp:(NSEvent *)event { os_mac_push_mouse_button(os_window, event, OS_EVENT_KIND__RELEASE); }
- (void)rightMouseDown:(NSEvent *)event { os_mac_push_mouse_button(os_window, event, OS_EVENT_KIND__PRESS); }
- (void)rightMouseUp:(NSEvent *)event { os_mac_push_mouse_button(os_window, event, OS_EVENT_KIND__RELEASE); }
- (void)otherMouseDown:(NSEvent *)event { os_mac_push_mouse_button(os_window, event, OS_EVENT_KIND__PRESS); }
- (void)otherMouseUp:(NSEvent *)event { os_mac_push_mouse_button(os_window, event, OS_EVENT_KIND__RELEASE); }

- (void)scrollWheel:(NSEvent *)event {
  os_mac_update_mouse(os_window, event);
  os_push_event((OS_Event){
    .window = os_window,
    .timestamp_ns = os_timestamp_from_mac_event(event),
    .kind = OS_EVENT_KIND__SCROLL,
    .x = os_gfx_state->mouse_x,
    .y = os_gfx_state->mouse_y,
    .delta_x = event.scrollingDeltaX,
    .delta_y = -event.scrollingDeltaY,
  });
}

- (void)keyDown:(NSEvent *)event {
  os_gfx_state->modifiers = os_modifiers_from_mac_flags(event.modifierFlags);
  OS_Key key = os_key_from_mac_keycode(event.keyCode);
  if (key != OS_KEY__NULL) {
    os_gfx_state->key_states[key] = 1;
    os_push_event((OS_Event){
      .window = os_window,
      .timestamp_ns = os_timestamp_from_mac_event(event),
      .kind = OS_EVENT_KIND__PRESS,
      .key = key,
      .is_repeat = event.isARepeat,
    });
  }

  NSData *utf8 = [event.characters dataUsingEncoding:NSUTF8StringEncoding];
  B1 const *bytes = utf8.bytes;
  L1 byte_count = utf8.length;
  if (byte_count > 0 && bytes[0] >= 0x20 && bytes[0] != 0x7f) {
    L1 scalar_size = bytes[0] < 0x80 ? 1 : bytes[0] < 0xe0 ? 2 : bytes[0] < 0xf0 ? 3 : 4;
    scalar_size = Min(scalar_size, Min(byte_count, 4));
    OS_Event text_event = {
      .window = os_window,
      .timestamp_ns = os_timestamp_from_mac_event(event),
      .kind = OS_EVENT_KIND__TEXT,
      .is_repeat = event.isARepeat,
      .text_len = scalar_size,
    };
    memmove(text_event.text, bytes, scalar_size);
    os_push_event(text_event);
  }
}

- (void)keyUp:(NSEvent *)event {
  os_gfx_state->modifiers = os_modifiers_from_mac_flags(event.modifierFlags);
  OS_Key key = os_key_from_mac_keycode(event.keyCode);
  if (key != OS_KEY__NULL) {
    os_gfx_state->key_states[key] = 0;
    os_push_event((OS_Event){
      .window = os_window,
      .timestamp_ns = os_timestamp_from_mac_event(event),
      .kind = OS_EVENT_KIND__RELEASE,
      .key = key,
    });
  }
}

- (void)flagsChanged:(NSEvent *)event {
  OS_Modifier_Flags old_modifiers = os_gfx_state->modifiers;
  os_gfx_state->modifiers = os_modifiers_from_mac_flags(event.modifierFlags);
  OS_Key key = os_key_from_mac_keycode(event.keyCode);
  if (key == OS_KEY__CTRL || key == OS_KEY__SHIFT || key == OS_KEY__ALT) {
    OS_Modifier_Flags flag = key == OS_KEY__CTRL ? OS_MODIFIER_FLAG__CTRL :
      key == OS_KEY__SHIFT ? OS_MODIFIER_FLAG__SHIFT : OS_MODIFIER_FLAG__ALT;
    I1 pressed = (os_gfx_state->modifiers & flag) != 0;
    if (((old_modifiers & flag) != 0) != pressed) {
      os_gfx_state->key_states[key] = pressed;
      os_push_event((OS_Event){
        .window = os_window,
        .timestamp_ns = os_timestamp_from_mac_event(event),
        .kind = pressed ? OS_EVENT_KIND__PRESS : OS_EVENT_KIND__RELEASE,
        .key = key,
      });
    }
  }
}

@end


@implementation MountainWindowDelegate

Internal void os_mac_update_window_dimensions(OS_Window *window) {
  NSSize logical_size = window->view.bounds.size;
  NSRect backing_rect = [window->view convertRectToBacking:window->view.bounds];
  D1 pixel_ratio = logical_size.width > 0 ? backing_rect.size.width / logical_size.width : 1.0;
  if (pixel_ratio <= 0) pixel_ratio = 1.0;
  window->width = (SI1)logical_size.width;
  window->height = (SI1)logical_size.height;
  window->pixel_ratio = pixel_ratio;
  window->metal_layer.contentsScale = pixel_ratio;
  window->metal_layer.drawableSize = backing_rect.size;
}

- (BOOL)windowShouldClose:(id)sender {
  os_push_event((OS_Event){.window = os_window, .kind = OS_EVENT_KIND__WINDOW_CLOSE});
  return NO;
}

- (void)windowDidResize:(NSNotification *)notification {
  os_mac_update_window_dimensions(os_window);
}

- (void)windowDidChangeBackingProperties:(NSNotification *)notification {
  os_mac_update_window_dimensions(os_window);
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
  os_gfx_state->focused_window = os_window;
}

- (void)windowDidResignKey:(NSNotification *)notification {
  if (os_gfx_state->focused_window == os_window) os_gfx_state->focused_window = 0;
  MemoryZeroArray(os_gfx_state->key_states);
  os_gfx_state->modifiers = 0;
}

@end


Internal void os_mac_initialize(void) {
  if (os_gfx_state == 0) {
    Arena *arena = arena_alloc(MiB(64));
    os_gfx_state = push_array(arena, OS_GFX_State, 1);
    os_gfx_state->arena = arena;
  }
  if (!os_gfx_state->initialized) {
    os_gfx_state->initialized = 1;
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
  }
}

Internal OS_Window *os_window_open(String8 title, I1 width, I1 height) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  os_mac_initialize();

  OS_Window *result = push_array(os_gfx_state->arena, OS_Window, 1);
  NSRect content_rect = NSMakeRect(0, 0, width, height);
  NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
    NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
  result->window = [[NSWindow alloc] initWithContentRect:content_rect styleMask:style
    backing:NSBackingStoreBuffered defer:NO];
  result->window.releasedWhenClosed = NO;

  MountainView *view = [[MountainView alloc] initWithFrame:content_rect];
  view->os_window = result;
  view.wantsLayer = YES;
  CAMetalLayer *layer = [CAMetalLayer layer];
  layer.delegate = view;
  view.layer = layer;
  result->view = view;
  result->metal_layer = layer;

  MountainWindowDelegate *delegate = [[MountainWindowDelegate alloc] init];
  delegate->os_window = result;
  result->delegate = delegate;
  result->window.delegate = delegate;
  result->window.contentView = view;
  result->window.acceptsMouseMovedEvents = YES;

  NSString *window_title = [[NSString alloc] initWithBytes:title.str length:title.len encoding:NSUTF8StringEncoding];
  result->window.title = window_title != nil ? window_title : @"Mountain";
  [window_title release];
  [result->window center];
  [result->window makeKeyAndOrderFront:nil];
  [result->window makeFirstResponder:view];
  [NSApp activateIgnoringOtherApps:YES];

  os_mac_update_window_dimensions(result);
  result->next = os_gfx_state->first_window;
  if (result->next != 0) result->next->prev = result;
  os_gfx_state->first_window = result;
  os_gfx_state->focused_window = result;

  [pool drain];
  return result;
}

Internal OS_Event_List os_poll_events(Arena *arena) {
  Assert(os_gfx_state != 0 && os_gfx_state->first_window != 0);
  os_gfx_state->event_arena = arena;
  os_gfx_state->events = (OS_Event_List){0};

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  for (;;) {
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast]
      inMode:NSDefaultRunLoopMode dequeue:YES];
    if (event == nil) break;
    os_gfx_state->modifiers = os_modifiers_from_mac_flags(event.modifierFlags);
    [NSApp sendEvent:event];
  }
  [NSApp updateWindows];
  [pool drain];
  return os_gfx_state->events;
}

Internal void os_window_close(OS_Window *window) {
  if (window == 0) return;
  if (window->prev != 0) window->prev->next = window->next;
  if (window->next != 0) window->next->prev = window->prev;
  if (os_gfx_state->first_window == window) os_gfx_state->first_window = window->next;
  if (os_gfx_state->hovered_window == window) os_gfx_state->hovered_window = 0;
  if (os_gfx_state->focused_window == window) os_gfx_state->focused_window = 0;

  window->window.delegate = nil;
  [window->window orderOut:nil];
  [window->window close];
  [window->view release];
  [window->delegate release];
  [window->window release];
  window->window = nil;
  window->view = nil;
  window->delegate = nil;
  window->metal_layer = nil;
}

Internal OS_Window *os_hovered_window(void) {
  return os_gfx_state != 0 ? os_gfx_state->hovered_window : 0;
}

Internal OS_Window *os_focused_window(void) {
  return os_gfx_state != 0 ? os_gfx_state->focused_window : 0;
}

Internal I1 os_key_is_down(L1 key) {
  return os_gfx_state != 0 && key > OS_KEY__NULL && key < OS_KEY_COUNT ? os_gfx_state->key_states[key] : 0;
}

Internal D1 os_mouse_x(void) { return os_gfx_state != 0 ? os_gfx_state->mouse_x : 0; }
Internal D1 os_mouse_y(void) { return os_gfx_state != 0 ? os_gfx_state->mouse_y : 0; }
Internal F2 os_mouse_pos(void) { return (F2){os_mouse_x(), os_mouse_y()}; }

#endif
