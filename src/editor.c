#if (HEADER)

////////////////////////////////
//~ kti: UI

typedef I1 View_Kind;
enum {
  VIEW_KIND__ENTITIES = 0,
  VIEW_KIND__ENTITY,
  VIEW_KIND__VIEWPORT,

  VIEW_KIND_COUNT,
};

Global String8 view_kind_names[VIEW_KIND_COUNT] = {
  [VIEW_KIND__ENTITIES] = str8("Entities"),
  [VIEW_KIND__ENTITY] = str8("Entity"),
  [VIEW_KIND__VIEWPORT] = str8("Viewport"),
};

typedef struct Camera Camera;
struct Camera {
  F4 pos;
  F1 yaw;
  F1 pitch;
  F1 fov;
  F1 near_z;
  F1 far_z;
};

typedef struct View View;
struct View {
  View_Kind kind;
  String8 title;
  L1 name_len;
  B1 name[512];

  UI_Box *viewport_box;

  Camera camera;
  Camera target_camera;

  F4 camera_drag_start_pos;
  F1 camera_drag_start_yaw;
  F1 camera_drag_start_pitch;
};

typedef struct Panel Panel;
struct Panel {
  Panel *first;
  Panel *last;
  Panel *next;
  Panel *prev;
  Panel *parent;
  F1 pct_of_parent;
  Axis split_axis;

  View views[64];
  L1 view_count;
  L1 selected_view_idx;
};

typedef struct Panel_Rec Panel_Rec;
struct Panel_Rec {
  Panel *next;
  I1 push_count;
  I1 pop_count;
};

typedef struct Window Window;
struct Window {
  Window *next;
  Window *prev;

  Arena *arena;

  OS_Window *os;
  GFX_Window *gfx;

  UI_State *ui;
  Panel root_panel;
};

////////////////////////////////
//~ kti: Cmds

typedef I1 Cmd_Kind;
enum {
  CMD_KIND__NONE = 0,

  CMD_KIND__OPEN_PANEL,
  CMD_KIND__CLOSE_PANEL,
  CMD_KIND__FOCUS_PANEL,

  CMD_KIND__CREATE_ENTITY,

  CMD_KIND_COUNT,
};

typedef struct Cmd Cmd;
struct Cmd {
  Cmd_Kind kind;

  Window *window;
  Panel *panel;

  Dir dir;
};

////////////////////////////////
//~ kti: Entity

typedef struct Entity_Handle Entity_Handle;
struct Entity_Handle {
  L1 idx;
  L1 id;
};

enum {
  ENTITY_FLAG__SHAPE  = 1 << 0,
  ENTITY_FLAG__CAMERA = 1 << 1,
};

typedef I1 Shape;
enum {
  SHAPE__BOX = 0,
  SHAPE__SPHERE,
  SHAPE_COUNT,
};

Global String8 shape_names[SHAPE_COUNT] = {
  [SHAPE__BOX] = str8("Box"),
  [SHAPE__SPHERE] = str8("Sphere"),
};

typedef struct Entity Entity;
struct Entity {
  Entity_Handle handle;

  L1 flags;
  B1 name[128];
  L1 name_len;
  F4 pos;
  F4 size;
  Shape shape;
  RT_Material material;
};

typedef struct Mesh Mesh;
struct Mesh {
  GFX_Buffer *vertex_buffer;
  GFX_Buffer *index_buffer;
  L1 vertex_count;
  L1 index_count;
};

////////////////////////////////
//~ kti: State

typedef struct State State;
struct State {
  Arena *arena;
  Window *first_window;
  Window *last_window;
  Window *free_window;
  Panel *free_panel;

  Panel *focused_panel;

  Cmd cmds[512];
  L1 cmd_count;

  L1 name_edit_buffer_len;
  B1 name_edit_buffer[512];
  Txt_Pt name_cursor;
  Txt_Pt name_mark;

  //- kti: Entities.
  L1 entity_count;
  L1 last_entity_id;
  Entity entities[128];
  Entity nil_entity;

  Entity_Handle selected_entity;

  Mesh meshes[SHAPE_COUNT];
};

#endif

#if (SOURCE)

Global State *state = 0;

////////////////////////////////
//~ kti: Meshes

Internal Mesh mesh_alloc_from_vertices_indices(GFX_Mesh_Vertex *vertices, L1 vertex_count, I1 *indices, L1 index_count) {
  Mesh result = {0};
  result.vertex_buffer = gfx_buffer_alloc(
      GFX_BUFFER_USAGE__STATIC, GFX_BUFFER_KIND__VERTEX,
      vertex_count*sizeof(vertices[0]), vertices);
  result.index_buffer = gfx_buffer_alloc(
      GFX_BUFFER_USAGE__STATIC, GFX_BUFFER_KIND__INDEX,
      index_count*sizeof(indices[0]), indices);
  result.vertex_count = vertex_count;
  result.index_count = index_count;
  return result;
}

Internal Mesh mesh_alloc_sphere(I1 latitude_segments, I1 longitude_segments) {
  Temp_Arena scratch = scratch_begin(0, 0);

  latitude_segments = Max(latitude_segments, 2);
  longitude_segments = Max(longitude_segments, 3);

  L1 vertex_count = (L1)(latitude_segments + 1)*(longitude_segments + 1);
  L1 index_count = (L1)latitude_segments*longitude_segments*6;
  GFX_Mesh_Vertex *vertices = push_array_no_zero(scratch.arena, GFX_Mesh_Vertex, vertex_count);
  I1 *indices = push_array_no_zero(scratch.arena, I1, index_count);

  L1 vertex_idx = 0;
  for (I1 lat = 0; lat <= latitude_segments; lat += 1) {
    F1 v = (F1)lat/(F1)latitude_segments;
    F1 latitude = PI*(v - 0.5f);
    F1 y = sinf(latitude);
    F1 ring_radius = cosf(latitude);
    for (I1 lon = 0; lon <= longitude_segments; lon += 1) {
      F1 u = (F1)lon/(F1)longitude_segments;
      F1 longitude = 2.0f*PI*u;
      F1 x = ring_radius*cosf(longitude);
      F1 z = ring_radius*sinf(longitude);
      vertices[vertex_idx++] = (GFX_Mesh_Vertex){
        .pos = {0.5f*x, 0.5f*y, 0.5f*z, 1.0f},
        .normal = {x, y, z, 0.0f},
        .uv = {u, 1.0f-v},
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
      };
    }
  }

  L1 index_idx = 0;
  I1 row_size = longitude_segments + 1;
  for (I1 lat = 0; lat < latitude_segments; lat += 1) {
    for (I1 lon = 0; lon < longitude_segments; lon += 1) {
      I1 a = lat*row_size + lon;
      I1 b = a + row_size;
      indices[index_idx++] = a;
      indices[index_idx++] = b;
      indices[index_idx++] = a + 1;
      indices[index_idx++] = a + 1;
      indices[index_idx++] = b;
      indices[index_idx++] = b + 1;
    }
  }

  Mesh mesh = mesh_alloc_from_vertices_indices(vertices, vertex_count, indices, index_count);
  scratch_end(scratch);

  return mesh;
}

Internal Mesh mesh_alloc_box(void) {
#define BV(px, py, pz, nx, ny, nz, u, v) \
  {{px, py, pz, 1.0f}, {nx, ny, nz, 0.0f}, {u, v}, {1.0f, 1.0f, 1.0f, 1.0f}}
  GFX_Mesh_Vertex vertices[] = {
    BV(-.5f,-.5f, .5f, 0, 0, 1, 0, 0), BV( .5f,-.5f, .5f, 0, 0, 1, 1, 0), BV( .5f, .5f, .5f, 0, 0, 1, 1, 1), BV(-.5f, .5f, .5f, 0, 0, 1, 0, 1),
    BV( .5f,-.5f,-.5f, 0, 0,-1, 0, 0), BV(-.5f,-.5f,-.5f, 0, 0,-1, 1, 0), BV(-.5f, .5f,-.5f, 0, 0,-1, 1, 1), BV( .5f, .5f,-.5f, 0, 0,-1, 0, 1),
    BV(-.5f,-.5f,-.5f,-1, 0, 0, 0, 0), BV(-.5f,-.5f, .5f,-1, 0, 0, 1, 0), BV(-.5f, .5f, .5f,-1, 0, 0, 1, 1), BV(-.5f, .5f,-.5f,-1, 0, 0, 0, 1),
    BV( .5f,-.5f, .5f, 1, 0, 0, 0, 0), BV( .5f,-.5f,-.5f, 1, 0, 0, 1, 0), BV( .5f, .5f,-.5f, 1, 0, 0, 1, 1), BV( .5f, .5f, .5f, 1, 0, 0, 0, 1),
    BV(-.5f, .5f, .5f, 0, 1, 0, 0, 0), BV( .5f, .5f, .5f, 0, 1, 0, 1, 0), BV( .5f, .5f,-.5f, 0, 1, 0, 1, 1), BV(-.5f, .5f,-.5f, 0, 1, 0, 0, 1),
    BV(-.5f,-.5f,-.5f, 0,-1, 0, 0, 0), BV( .5f,-.5f,-.5f, 0,-1, 0, 1, 0), BV( .5f,-.5f, .5f, 0,-1, 0, 1, 1), BV(-.5f,-.5f, .5f, 0,-1, 0, 0, 1),
  };
#undef BV
  I1 indices[] = {
    0, 1, 2,  0, 2, 3,  4, 5, 6,  4, 6, 7,
    8, 9,10,  8,10,11, 12,13,14, 12,14,15,
    16,17,18, 16,18,19, 20,21,22, 20,22,23,
  };
  return mesh_alloc_from_vertices_indices(vertices, ArrayCount(vertices), indices, ArrayCount(indices));
}

////////////////////////////////
//~ kti: Panel

Internal Panel_Rec panel_rec_depth_first_pre_order(Panel *panel) {
  Panel_Rec rec = {0};

  if (panel->first) {
    rec.next = panel->first;
    rec.push_count = 1;
  } else
  for (Panel *p = panel; p != 0; p = p->parent) {
    if (p->next) {
      rec.next = p->next;
      break;
    }
    rec.pop_count += 1;
  }

  return rec;
}

Internal F4 panel_rect_from_parent_rect(Panel *child, F4 parent_rect) {
  F4 result = parent_rect;
  Panel *parent = child->parent;
  if (parent) {
    for (Panel *p = parent->first; p != child && p != 0; p = p->next) {
      result[parent->split_axis] += p->pct_of_parent * parent_rect[2 + parent->split_axis];
    }
    result[2 + parent->split_axis] = child->pct_of_parent * parent_rect[2 + parent->split_axis];
  }
  return result;
}

Internal F4 panel_rect_from_root_rect(Panel *panel, F4 root_rect) {
  Temp_Arena scratch = scratch_begin(0, 0);

  typedef struct Walk_Node Walk_Node;
  struct Walk_Node {
    Walk_Node *next;
    Panel *child;
  };
  Walk_Node *top_walk_node = 0;
  for (Panel *p = panel; p != 0 && p->parent != 0; p = p->parent) {
    Walk_Node *node = push_array(scratch.arena, Walk_Node, 1);
    node->child = p;
    SLLStackPush(top_walk_node, node);
  }

  F4 result = root_rect;
  for (Walk_Node *n = top_walk_node; n != 0; n = n->next) {
    result = panel_rect_from_parent_rect(n->child, result);
  }

  scratch_end(scratch);

  return result;
}

Internal Panel *panel_alloc() {
  Panel *result = state->free_panel;
  if (result != 0) {
    SLLStackPop(state->free_panel);
    MemoryZeroStruct(result);
  } else {
    result = push_array(state->arena, Panel, 1);
  }
  return result;
}

Internal void panel_insert(Panel *panel, Panel *at, Dir dir) {
  I1 split_axis = (dir == DIR__RIGHT || dir == DIR__LEFT) ? AXIS__X : AXIS__Y;
  Panel *parent = at->parent;
  if (parent == 0) {
    panel->parent = at;
    panel->pct_of_parent = 1.0f;
    DLLPushBack(at->first, at->last, panel);
  } else if (parent->split_axis == split_axis || parent->first == parent->last) {
    parent->split_axis = split_axis;
    panel->parent = parent;
    panel->pct_of_parent = at->pct_of_parent = at->pct_of_parent * 0.5f;
    DLLInsert(parent->first, parent->last, at, panel);
  } else {
    Panel *container = panel_alloc();
    container->split_axis = split_axis;
    container->parent = parent;
    container->pct_of_parent = at->pct_of_parent;

    DLLInsert(parent->first, parent->last, at, container);
    DLLRemove(parent->first, parent->last, at);

    Panel *first = at;
    Panel *second = panel;
    if (dir == DIR__LEFT || dir == DIR__UP) {
      first = panel;
      second = at;
    }
    DLLPushBack(container->first, container->last, first);
    DLLPushBack(container->first, container->last, second);

    at->parent = container;
    panel->parent = container;
    at->pct_of_parent = panel->pct_of_parent = 0.5f;
  }
}

Internal void panel_close(Panel *root, Panel *panel) {
  if (panel->first != 0) {
    return;
  }

  if (state->focused_panel == panel) {
    state->focused_panel = 0;
  }

  Panel *parent = panel->parent;

  if (panel->prev) {
    panel->prev->pct_of_parent += panel->pct_of_parent;
  } else if (panel->next) {
    panel->next->pct_of_parent += panel->pct_of_parent;
  }

  DLLRemove(parent->first, parent->last, panel);

  if (parent->first && parent->first == parent->last && parent != root) {
    Panel *grandparent = parent->parent;
    Panel *survivor = parent->first;
    survivor->parent = grandparent;
    survivor->pct_of_parent = parent->pct_of_parent;
    DLLInsert(grandparent->first, grandparent->last, parent, survivor);
    DLLRemove(grandparent->first, grandparent->last, parent);
    SLLStackPush(state->free_panel, parent);
  }

  SLLStackPush(state->free_panel, panel);
}

Internal void panel_push_view(Panel *panel, View_Kind kind) {
  View *view = &panel->views[panel->view_count];
  panel->view_count += 1;

  view->kind = kind;
  view->title = view_kind_names[kind];
  String8 default_name = str8("Theodor");
  view->name_len = Min(sizeof(view->name), default_name.len);
  memmove(view->name, default_name.str, view->name_len);

  if (kind == VIEW_KIND__VIEWPORT) {
    view->camera = (Camera){
      .pos = (F4){0.0f, 0.0f, -7.0f},
      .fov = 70.0f * PI/180.0f,
      .near_z = 0.1f,
      .far_z = 100.0f,
    };
    view->target_camera = view->camera;
  }
}

////////////////////////////////
//~ kti: Window

Internal Window *window_open(void) {
  Window *window = state->free_window;
  if (window != 0) {
    SLLStackPop(state->free_window);
    MemoryZeroStruct(window);
  } else {
    window = push_array(state->arena, Window, 1);
  }

  window->os = os_window_open(str8("Testing"), 1280, 720);
  window->gfx = gfx_window_equip(window->os);
  window->ui = ui_state_alloc();
  window->arena = arena_alloc(MiB(32));
  window->root_panel.split_axis = AXIS__X;

  DLLPushBack(state->first_window, state->last_window, window);

  return window;
}

Internal void window_close(Window *window) {
  if (window != 0) {
    DLLRemove(state->first_window, state->last_window, window);
    SLLStackPush(state->free_window, window);
    ui_state_release(window->ui);
    gfx_window_unequip(window->gfx);
    os_window_close(window->os);
  }
}

Internal Window *window_from_os_window(OS_Window *os) {
  Window *result = 0;

  for (Window *w = state->first_window; w != 0; w = w->next) {
    if (w->os == os) {
      result = w;
    }
  }

  return result;
}

////////////////////////////////
//~ kti: Cmd

Internal void cmd_push(Cmd cmd) {
  if (state->cmd_count < ArrayCount(state->cmds)) {
    L1 idx = atomic_add_L1(&state->cmd_count, 1);
    state->cmds[idx] = cmd;
  }
}

////////////////////////////////
//~ kti: Entities

Internal Entity_Handle entity_handle_zero() {
  Entity_Handle result = {0};
  return result;
}

Internal I1 entity_is_nil(Entity *entity) {
  I1 result = (entity == 0 || entity == &state->nil_entity);
  return result;
}

Internal I1 entity_handle_match(Entity_Handle a, Entity_Handle b) {
  I1 result = (a.idx == b.idx && a.id == b.id);
  return result;
}

Internal Entity *entity_from_handle(Entity_Handle handle) {
  Entity *result = &state->nil_entity;

  if (handle.idx < state->entity_count && handle.id != 0) {
    Entity *candidate = &state->entities[handle.idx];
    if (candidate->handle.id == handle.id) {
      result = candidate;
    }
  }

  return result;
}

Internal Entity *entity_create(L1 flags, String8 name) {
  Entity *entity = 0;

  L1 idx = state->entity_count;
  if (idx < ArrayCount(state->entities)) {
    state->entity_count += 1;
    state->last_entity_id += 1;
    L1 id = state->last_entity_id;

    entity = &state->entities[idx];
    MemoryZeroStruct(entity);
    entity->handle.idx = idx;
    entity->handle.id = id;
    entity->flags = flags;
    entity->size = (F4){1.0f, 1.0f, 1.0f};
    entity->name_len = Min(name.len, sizeof(entity->name));
    entity->material.base_color = (F4){0.9f, 0.9f, 0.9f, 1.0f};
    entity->material.emissive = (F4){0.0f, 0.0f, 0.0f, 1.0f};
    memmove(entity->name, name.str, entity->name_len);
  }
  return entity;
}

Internal void widget_rgb_edit(String8 label, F4 *rgb) {
  UI_Row()
  UI_Corner_Radius(ui_top_font_size()*0.4f) {
    UI_Pref_Width(ui_pct(0.15f, 1.0f))
    ui_label(label);
    ui_spacer(ui_px(10.0f, 1.0f));
    
    ui_push_text_align(UI_TEXT_ALIGN__CENTER);

    UI_Background_Color(oklch(0.3f, 0.15f, 35, 1.0f))
    UI_Border_Color(oklch(0.5f, 0.2f, 35, 1.0f)) {
      ui_drag_F1(str8("R"), &rgb[0][0], 200.0f, 1.0f);
    }
    ui_spacer(ui_px(5.0f, 1.0f));

    UI_Background_Color(oklch(0.272f, 0.076f, 145, 1.0f))
    UI_Border_Color(oklch(0.484f, 0.164f, 145, 1.0f)) {
      ui_drag_F1(str8("G"), &rgb[0][1], 200.0f, 1.0f);
    }
    ui_spacer(ui_px(5.0f, 1.0f));

    UI_Background_Color(oklch(0.277f, 0.077f, 252, 1.0f))
    UI_Border_Color(oklch(0.493f, 0.172f, 252, 1.0f)) {
      ui_drag_F1(str8("B"), &rgb[0][2], 200.0f, 1.0f);
    }
    ui_spacer(ui_px(5.0f, 1.0f));

    F4 color = oklch_from_linear_rgb(rgb[0]);
    UI_Background_Color(color)
    UI_Pref_Width(ui_px(50.0f, 1.0f))
    UI_Corner_Radius(0) {
      ui_build_box_from_string(UI_BOX_FLAG__DRAW_BACKGROUND, str8("color"));
    }

    ui_pop_text_align();

    rgb[0] = clamp01_F4(rgb[0]);
  }
}


////////////////////////////////
//~ kti: Main

Internal void lane(Arena *arena) {
  L1 frame_count = 0;
  L1 total_frame_time = 0;
  L1 min_frame_time = L1_MAX;
  L1 max_frame_time = 0;
  F1 fps = 0.0f;

  ////////////////////////////////
  //~ kti: Initialization.

  if (lane_idx() == 0) {
    //- kti: System init.
    ProfInit();
    gfx_init();
    fp_init();
    fc_init();

    state = push_array(arena, State, 1);
    state->arena = arena;

    state->meshes[SHAPE__BOX] = mesh_alloc_box();
    state->meshes[SHAPE__SPHERE] = mesh_alloc_sphere(16, 32);

    Window *window = window_open();

    //- kti: Create initial state.
    Panel *entities_panel = panel_alloc();
    panel_push_view(entities_panel, VIEW_KIND__ENTITIES);
    panel_insert(entities_panel, &window->root_panel, 0);

    Panel *viewport_panel = panel_alloc();
    panel_push_view(viewport_panel, VIEW_KIND__VIEWPORT);
    panel_insert(viewport_panel, entities_panel, DIR__RIGHT);

    Panel *entity_panel = panel_alloc();
    panel_push_view(entity_panel, VIEW_KIND__ENTITY);
    panel_insert(entity_panel, viewport_panel, DIR__RIGHT);

    entities_panel->pct_of_parent = 0.2f;
    viewport_panel->pct_of_parent = 0.5f;
    entity_panel->pct_of_parent = 0.3f;

    Entity *starting_entity = entity_create(0, str8("Starting Entity"));
    state->selected_entity = starting_entity->handle;
  }

  lane_sync();

  L1 running = 1;

  ////////////////////////////////
  //~ kti: Main loop

  while (running) {
    ProfBegin("Frame");

    L1 frame_begin_time = os_clock();
    F1 time = (F1)(frame_begin_time / 1000000ULL) / 1000.0f;

    Temp_Arena scratch = scratch_begin(0, 0);

    UI_Cmd_List ui_cmds = {0};
    OS_Event_List events = {0};
    if (lane_idx() == 0) {

      ////////////////////////////////
      //~ kti: Events
      events = os_poll_events(scratch.arena);
      for (OS_Event *e = events.first; e != 0; e = e->next) {
        if (e->kind == OS_EVENT_KIND__WINDOW_CLOSE) {
          Window *window = window_from_os_window(e->window);
          window_close(window);
        }
        if (e->kind == OS_EVENT_KIND__PRESS && e->key == OS_KEY__ESC) {
          ui_cmd_list_push(scratch.arena, &ui_cmds, (UI_Cmd){
            .kind = UI_CMD_KIND__CANCEL,
            .timestamp_ns = e->timestamp_ns
          });
        }
        if (e->kind == OS_EVENT_KIND__TEXT) {
          ui_cmd_list_push(scratch.arena, &ui_cmds, (UI_Cmd){
            .kind = UI_CMD_KIND__TEXT,
            .string = {e->text, e->text_len},
            .timestamp_ns = e->timestamp_ns
          });
        }
        if (e->kind == OS_EVENT_KIND__PRESS && (e->key == OS_KEY__LEFT || e->key == OS_KEY__RIGHT)) {
          UI_Cmd_Delta_Unit delta_unit = (e->modifiers&OS_MODIFIER_FLAG__CTRL) ? UI_CMD_DELTA_UNIT__WORD : UI_CMD_DELTA_UNIT__CHAR;
          UI_Cmd_Flags flags = UI_CMD_FLAG__CAP_AT_LINE;
          if (e->modifiers&OS_MODIFIER_FLAG__SHIFT) {
            flags |= UI_CMD_FLAG__KEEP_MARK;
          } else {
            flags |= UI_CMD_FLAG__PICK_SELECT_SIDE;
          }
          ui_cmd_list_push(scratch.arena, &ui_cmds, (UI_Cmd){
              .kind = UI_CMD_KIND__NAVIGATE,
              .delta_unit = delta_unit,
              .flags = flags,
              .delta_si2 = {(e->key == OS_KEY__LEFT) ? -1 : 1, 0},
              .timestamp_ns = e->timestamp_ns,
            });
        }
        if (e->kind == OS_EVENT_KIND__PRESS && (e->key == OS_KEY__HOME || e->key == OS_KEY__END)) {
          UI_Cmd_Flags flags = UI_CMD_FLAG__CAP_AT_LINE;
          if (e->modifiers&OS_MODIFIER_FLAG__SHIFT) {
            flags |= UI_CMD_FLAG__KEEP_MARK;
          }
          ui_cmd_list_push(scratch.arena, &ui_cmds, (UI_Cmd){
            .kind = UI_CMD_KIND__NAVIGATE,
            .delta_unit = UI_CMD_DELTA_UNIT__LINE,
            .flags = flags,
            .delta_si2 = {(e->key == OS_KEY__HOME) ? -1 : 1, 0},
            .timestamp_ns = e->timestamp_ns,
          });
        }
        if (e->kind == OS_EVENT_KIND__PRESS && (e->key == OS_KEY__BACKSPACE || e->key == OS_KEY__DELETE)) {
          UI_Cmd_Delta_Unit delta_unit = (e->modifiers&OS_MODIFIER_FLAG__CTRL) ? UI_CMD_DELTA_UNIT__WORD : UI_CMD_DELTA_UNIT__CHAR;
          ui_cmd_list_push(scratch.arena, &ui_cmds, (UI_Cmd){
            .kind = UI_CMD_KIND__EDIT,
            .delta_unit = delta_unit,
            .flags = UI_CMD_FLAG__CAP_AT_LINE | UI_CMD_FLAG__ZERO_DELTA_ON_SELECT | UI_CMD_FLAG__DELETE,
            .delta_si2 = {(e->key == OS_KEY__BACKSPACE) ? -1 : 1, 0},
            .timestamp_ns = e->timestamp_ns,
          });
        }
      }
    }

    lane_sync();

    if (state->first_window == 0) {
      running = 0;
    }

    lane_sync();

    if (lane_idx() == 0) {
      ////////////////////////////////
      //~ kti: Execute Cmds
      for (L1 i = 0; i < state->cmd_count; i += 1) {
        Cmd cmd = state->cmds[i];
        switch (cmd.kind) {
          case CMD_KIND__OPEN_PANEL: {
            panel_insert(panel_alloc(), cmd.panel, cmd.dir);
          } break;
          case CMD_KIND__CLOSE_PANEL: {
            panel_close(&cmd.window->root_panel, cmd.panel);
          } break;
          case CMD_KIND__FOCUS_PANEL: {
            state->focused_panel = cmd.panel;
          } break;

          case CMD_KIND__CREATE_ENTITY: {
            entity_create(ENTITY_FLAG__SHAPE, str8("New Entity"));
          } break;
        }
      }
      state->cmd_count = 0;

      ////////////////////////////////
      //~ UI

      ProfBegin("UI");

      fc_frame();

      FC_Tag prop_fnt = fc_tag_from_path(str8("/usr/share/fonts/bloomberg/" "Bloomberg-PropU_N.ttf"));
      FC_Tag fixed_fnt = fc_tag_from_path(str8("/usr/share/fonts/bloomberg/" "Bloomberg-FixedU_N.ttf"));

      for (Window *w = state->first_window; w != 0; w = w->next) {
        ui_state_equip(w->ui);
        ui_begin_build(w->os, events, ui_cmds);

        ui_push_font(prop_fnt);
        ui_push_background_color((F4){0.0f, 0.0f, 0.0f, 1.0f});
        ui_push_border_color((F4){0.5f, 0.0f, 0.0f, 1.0f});

        F4 root_plane_rect = {0, 0, w->os->width, w->os->height};

        //- kti: Non leaf panel ui
        F1 resize_box_w = 8;

        for (Panel *panel = &w->root_panel; panel != 0; panel = panel_rec_depth_first_pre_order(panel).next) {
          F4 panel_rect = panel_rect_from_root_rect(panel, root_plane_rect);

          for (Panel *child = panel->first; child != 0 && child->next != 0; child = child->next) {
            F4 child_rect = panel_rect_from_parent_rect(child, panel_rect);

            //- kti: Build separator box.
            F4 boundary_rect = child_rect;
            boundary_rect[panel->split_axis] += child_rect[2 + panel->split_axis] - resize_box_w * 0.5f;
            boundary_rect[2 + panel->split_axis] = resize_box_w;
            ui_set_next_fixed_rect(boundary_rect);
            UI_Box *boundary_box = ui_build_box_from_stringf(UI_BOX_FLAG__CLICKABLE | UI_BOX_FLAG__FLOATING, "##panel_boundary_%p", child);

            //- kti: Handle resize.
            UI_Signal sig = ui_signal_from_box(boundary_box);
            if (sig.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
              Panel *min_child = child;
              Panel *max_child = child->next;
              if (sig.flags & UI_SIGNAL_FLAG__PRESSED) {
                F2 drag_data = {min_child->pct_of_parent, max_child->pct_of_parent};
                ui_store_drag_struct(&drag_data);
              }
              F2 drag_data = ui_get_drag_struct(F2)[0];
              F2 drag_delta = ui_drag_delta();
              F1 min_child_pct__pre_drag = drag_data[0];
              F1 max_child_pct__pre_drag = drag_data[1];
              F1 min_child_px__pre_drag = min_child_pct__pre_drag * panel_rect[2 + panel->split_axis];
              F1 max_child_px__pre_drag = max_child_pct__pre_drag * panel_rect[2 + panel->split_axis];
              F1 min_child_px__post_drag = min_child_px__pre_drag + drag_delta[panel->split_axis];
              F1 max_child_px__post_drag = max_child_px__pre_drag - drag_delta[panel->split_axis];
              min_child->pct_of_parent = min_child_px__post_drag / panel_rect[2 + panel->split_axis];
              max_child->pct_of_parent = max_child_px__post_drag / panel_rect[2 + panel->split_axis];
            }
          }
        }

        //- kti: build all leaf panel ui
        for (Panel *panel = w->root_panel.first; panel != 0; panel = panel_rec_depth_first_pre_order(panel).next) {
          F4 panel_rect = panel_rect_from_root_rect(panel, root_plane_rect);
          I1 panel_is_focused = state->focused_panel == panel;

          //- kti: Build ui
          if (panel->first == 0)
          UI_Focus(panel_is_focused ? UI_FOCUS_KIND__NULL : UI_FOCUS_KIND__OFF) {
            UI_Box *panel_box = 0;

            UI_Focus(UI_FOCUS_KIND__ON) {
              ui_set_next_fixed_rect(rect_pad(panel_rect, -2.0f));
              ui_set_next_child_layout_axis(AXIS__Y);
              panel_box = ui_build_box_from_stringf(
                UI_BOX_FLAG__MOUSE_CLICKABLE |
                UI_BOX_FLAG__DISABLE_FOCUS_OVERLAY |
                UI_BOX_FLAG__DRAW_BACKGROUND |
                UI_BOX_FLAG__DRAW_BORDER |
                UI_BOX_FLAG__FLOATING |
                UI_BOX_FLAG__CLIP |
                UI_BOX_FLAG__DEFAULT_FOCUS_NAV |
                UI_BOX_FLAG__CLICK_TO_FOCUS,
                "##panel_box_%p", panel);
            }

            UI_Parent(panel_box)
            UI_Pref_Width(ui_pct(1.0f, 0.0f)) {
              UI_Child_Layout_Axis(AXIS__X);
              UI_Box *title_bar = ui_build_box_from_key(UI_BOX_FLAG__DRAW_BACKGROUND | UI_BOX_FLAG__DRAW_BORDER, ui_key_zero());

              UI_Parent((title_bar))
              UI_Font_Size(10.0f) {
                UI_Padding(ui_px(10.0f, 1.0f))
                UI_Pref_Width(ui_text_dim(0.0f, 1.0f))
                if (panel->view_count == 0) {
                  ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str8("<no view>"));
                } else for (L1 i = 0; i < panel->view_count; i += 1) {
                  ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, panel->views[i].title);
                }

                ui_spacer(ui_pct(1.0f, 0.0f));

                UI_Pref_Width(ui_text_dim(20.0f, 1.0f))
                UI_Pref_Height(ui_pct(1.0f, 1.0f))
                UI_Text_Align((UI_TEXT_ALIGN__CENTER))
                UI_Background_Color(((F4){0.2f, 0.0f, 0.0f, 1.0f})) {
                  if (ui_button(str8("Split X")).flags & UI_SIGNAL_FLAG__CLICKED) {
                    cmd_push((Cmd){
                      .kind = CMD_KIND__OPEN_PANEL,
                      .window = w,
                      .panel = panel,
                      .dir = DIR__RIGHT,
                    });
                  }
                  if (ui_button(str8("Split Y")).flags & UI_SIGNAL_FLAG__CLICKED) {
                    cmd_push((Cmd){
                      .kind = CMD_KIND__OPEN_PANEL,
                      .window = w,
                      .panel = panel,
                      .dir = DIR__DOWN,
                    });
                  }
                  if (ui_button(str8("Close")).flags & UI_SIGNAL_FLAG__CLICKED) {
                    cmd_push((Cmd){
                      .kind = CMD_KIND__CLOSE_PANEL,
                      .window = w,
                      .panel = panel,
                    });
                  }
                }
              }

              if (panel->view_count == 0) {
                UI_Row()
                UI_Padding(ui_px(10.0f, 1.0f)) {
                  UI_Column() {
                    UI_Text_Color(oklch(0.682f, 0.176f, 252, 1.0f))
                    ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str8("Choose view kind."));

                    UI_Row()
                    UI_Background_Color(((F4){0.2f, 0.0f, 0.0f, 1.0f}))
                    UI_Text_Align((UI_TEXT_ALIGN__CENTER))
                    UI_Pref_Width(ui_text_dim(10.0f, 1.0f))
                    UI_Pref_Height(ui_text_dim(5.0f, 1.0f))
                    for (L1 i = 0; i < VIEW_KIND_COUNT; i += 1) {
                      if (ui_button(view_kind_names [i]) .flags & UI_SIGNAL_FLAG__PRESSED) {
                        panel_push_view(panel, i);
                      }
                    }
                  }
                }
              } else {

                ////////////////////////////////
                //~ Views.

                View *view = &panel->views[panel->selected_view_idx];
                switch (view->kind) {
                  //- kti: Entities view.
                  case VIEW_KIND__ENTITIES: {
                    UI_Row()
                    UI_Padding(ui_px(10.0f, 1.0f))
                    UI_Column() {
                      ui_spacer(ui_px(10.0f, 1.0f));
                      //- kti: Build entities box.
                      ui_set_next_pref_height(ui_children_sum(1.0f));
                      ui_set_next_child_layout_axis(AXIS__Y);
                      ui_set_next_corner_radius(ui_top_font_size()*0.4f);
                      UI_Box *entities_box = ui_build_box_from_string(
                        UI_BOX_FLAG__DRAW_BORDER|
                        UI_BOX_FLAG__ROUND_CHILDREN_BY_PARENT,
                        str8("entities"));

                      //- kti: Fill entries
                      UI_Text_Padding(10.0f)
                      UI_Parent(entities_box) {
                        for (L1 i = 0; i < state->entity_count; i += 1) {
                          Entity *e = &state->entities[i];
                          String8 name = {e->name, e->name_len};
                          I1 selected = entity_handle_match(e->handle, state->selected_entity);

                          if (selected) {
                            ui_set_next_background_color((F4){0.2f, 0.0f, 0.0f, 1.0f});
                            ui_set_next_text_color(oklch(0.682f, 0.176f, 252, 1.0f));
                          }
                          UI_Box *row = ui_build_box_from_stringf(
                            UI_BOX_FLAG__DRAW_BACKGROUND|
                            UI_BOX_FLAG__DRAW_TEXT|
                            UI_BOX_FLAG__CLICKABLE|
                            UI_BOX_FLAG__CLICK_TO_FOCUS|
                            (selected*UI_BOX_FLAG__DISABLE_FOCUS_OVERLAY),
                            "entity_%llu", i);
                          ui_box_equip_display_string(row, name);

                          UI_Signal sig = ui_signal_from_box(row);
                          if (sig.flags & UI_SIGNAL_FLAG__PRESSED) {
                            state->selected_entity = e->handle;
                            cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                          }
                        }
                        if (state->entity_count == 0) {
                          ui_set_next_text_color((F4){0.4f, 0.0f, 0.0f, 1.0f});
                          ui_label(str8("No entities..."));
                        }
                      }

                      ui_spacer(ui_px(5.0f, 1.0f));

                      UI_Background_Color(((F4){0.2f, 0.0f, 0.0f, 1.0f}))
                      UI_Pref_Width(ui_text_dim(20.0f, 1.0f))
                      UI_Pref_Height(ui_text_dim(5.0f, 1.0f))
                      UI_Text_Align(UI_TEXT_ALIGN__CENTER) {
                        if (ui_button(str8("Create")).flags & UI_SIGNAL_FLAG__PRESSED) {
                          cmd_push((Cmd){.kind = CMD_KIND__CREATE_ENTITY});
                          cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                        }
                      }
                    }
                  } break;

                  //- kti: Entity view.
                  case VIEW_KIND__ENTITY: {
                    UI_Row()
                    UI_Padding(ui_px(10.0f, 1.0f))
                    UI_Column() {
                      ui_spacer(ui_px(10.0f, 1.0f));
                      Entity *entity = entity_from_handle(state->selected_entity);
                      if (entity_is_nil(entity)) {
                        ui_label(str8("Select an entity..."));
                      } else {
                        ui_set_next_child_layout_axis(AXIS__Y);
                        UI_Parent(ui_build_box_from_stringf(0, "entity_%p", entity)) {
                          //- kti: Name edit
                          ui_set_next_pref_height(ui_text_dim(5.0f, 1.0f));
                          ui_set_next_font_size(ui_top_font_size()*0.8f);
                          ui_set_next_text_color((F4){0.7f, 0.0f, 0.0f, 1.0f});
                          ui_label(str8("Name"));

                          String8 name = {.str = entity->name, .len = entity->name_len};
                          UI_Pref_Width(ui_px(500.0f, 1.0f))
                          UI_Pref_Height(ui_em(2.8f, 1.0f))
                          UI_Corner_Radius(ui_top_font_size()*0.4f)
                          UI_Text_Padding(floor_F1(ui_top_font_size()*0.5f)) {
                            UI_Signal signal = ui_textedit(&state->name_cursor,
                                                           &state->name_mark,
                                                           state->name_edit_buffer,
                                                           sizeof(entity->name),
                                                           &state->name_edit_buffer_len,
                                                           name,
                                                           str8("name_textedit"));
                            if (signal.flags & UI_SIGNAL_FLAG__COMMIT) {
                              entity->name_len = Min(sizeof(entity->name), state->name_edit_buffer_len);
                              memmove(entity->name, state->name_edit_buffer, Min(entity->name_len, sizeof(entity->name)));
                            }
                            if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
                              cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                            }
                          }

                          ui_spacer(ui_px(10.0f, 1.0f));

                          //- kti: Position
                          ui_set_next_pref_height(ui_text_dim(5.0f, 1.0f));
                          ui_set_next_font_size(ui_top_font_size()*0.8f);
                          ui_set_next_text_color((F4){0.7f, 0.0f, 0.0f, 1.0f});
                          ui_label(str8("Postiion"));

                          UI_Row()
                          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
                          UI_Corner_Radius(ui_top_font_size()*0.4f) {
                            ui_drag_F1(str8("X"), &entity->pos[0], 40.0f, 0.0f);
                            ui_spacer(ui_px(5.0f, 1.0f));
                            ui_drag_F1(str8("Y"), &entity->pos[1], 40.0f, 0.0f);
                            ui_spacer(ui_px(5.0f, 1.0f));
                            ui_drag_F1(str8("Z"), &entity->pos[2], 40.0f, 0.0f);
                          }

                          ui_spacer(ui_px(10.0f, 1.0f));

                          //- kti: Size
                          UI_Pref_Height(ui_text_dim(5.0f, 1.0f))
                          UI_Font_Size(ui_top_font_size()*0.8f)
                          UI_Text_Color(((F4){0.7f, 0.0f, 0.0f, 1.0f}))
                          ui_label(entity->shape == SHAPE__SPHERE ? str8("Radius") : str8("Size"));

                          UI_Row()
                          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
                          UI_Corner_Radius(ui_top_font_size()*0.4f) {
                            if (entity->shape == SHAPE__SPHERE) {
                              F1 radius = entity->size[0];
                              ui_drag_F1(str8("R"), &radius, 50.0f, 1.0f);
                              entity->size = (F4){radius, radius, radius};
                            } else {
                              ui_drag_F1(str8("X"), &entity->size[0], 50.0f, 1.0f);
                              ui_spacer(ui_px(5.0f, 1.0f));
                              ui_drag_F1(str8("Y"), &entity->size[1], 50.0f, 1.0f);
                              ui_spacer(ui_px(5.0f, 1.0f));
                              ui_drag_F1(str8("Z"), &entity->size[2], 50.0f, 1.0f);
                            }
                          }

                          ui_spacer(ui_px(10.0f, 1.0f));

                          //- kti: Shape
                          UI_Pref_Height(ui_text_dim(5.0f, 1.0f))
                          UI_Font_Size(ui_top_font_size()*0.8f)
                          UI_Text_Color(((F4){0.7f, 0.0f, 0.0f, 1.0f}))
                          ui_label(str8("Shape"));

                          ui_set_next_child_layout_axis(AXIS__X);
                          ui_set_next_pref_height(ui_children_sum(1.0f));
                          UI_Box *shape_selection = ui_build_box_from_string(0, str8("shape_selection"));
                          UI_Parent(shape_selection) 
                          UI_Text_Align(UI_TEXT_ALIGN__CENTER) {
                            for (L1 shape = 0; shape < SHAPE_COUNT; shape += 1) {
                              I1 selected = (shape == entity->shape);
                              String8 name = shape_names[shape];

                              if (selected) {
                                ui_set_next_background_color((F4){0.2f, 0.0f, 0.0f, 1.0f});
                                ui_set_next_text_color(oklch(0.682f, 0.176f, 252, 1.0f));
                              }
                              UI_Box *box = ui_build_box_from_string(
                                UI_BOX_FLAG__DRAW_BORDER|
                                UI_BOX_FLAG__DRAW_TEXT|
                                UI_BOX_FLAG__CLICKABLE|
                                UI_BOX_FLAG__DRAW_HOT_EFFECTS|
                                UI_BOX_FLAG__DRAW_BACKGROUND,
                                name);
                              UI_Signal signal = ui_signal_from_box(box);
                              if (signal.flags & UI_SIGNAL_FLAG__PRESSED) {
                                entity->shape = shape;
                                if (shape == SHAPE__SPHERE) {
                                  entity->size[1] = entity->size[0];
                                  entity->size[2] = entity->size[0];
                                }
                              }
                            }
                          }

                          ui_spacer(ui_px(15.0f, 1.0f));

                          //- kti: Material
                          F1 spacing = 20.0f;

                          UI_Text_Color(oklch(1.0f, 0.0f, 0.0f, 1.0f))
                          UI_Text_Align(UI_TEXT_ALIGN__CENTER)
                          ui_label(str8("Material"));
                          ui_spacer(ui_px(spacing*0.5f, 1.0f));

                          //- kti: Base Color
                          widget_rgb_edit(str8("Base Color"), &entity->material.base_color);
                          ui_spacer(ui_px(spacing, 1.0f));

                          //- kti: Metallic
                          UI_Row() {
                            UI_Pref_Width(ui_pct(0.15f, 1.0f))
                            ui_label(str8("Metallic"));
                            ui_spacer(ui_px(10.0f, 1.0f));
                            ui_slider_F1(&entity->material.metallic, 0.0f, 1.0f); 
                          }
                          ui_spacer(ui_px(spacing, 1.0f));

                          //- kti: Roughness
                          UI_Row() {
                            UI_Pref_Width(ui_pct(0.15f, 1.0f))
                            ui_label(str8("Roughness"));
                            ui_spacer(ui_px(10.0f, 1.0f));
                            ui_slider_F1(&entity->material.roughness, 0.0f, 1.0f); 
                          }
                          ui_spacer(ui_px(spacing, 1.0f));

                          //- kti: Emissive
                          widget_rgb_edit(str8("Emissive"), &entity->material.emissive);
                          ui_spacer(ui_px(spacing, 1.0f));
                        }
                      }
                    }
                  } break;

                  //- kti: 3D viewport.
                  case VIEW_KIND__VIEWPORT: {
                    //- kti: Animate camera.
                    view->camera.pos = lerp_snap_F4(view->camera.pos, 0.15f, view->target_camera.pos, 0.001f);
                    view->camera.pitch = lerp_snap_F1(view->camera.pitch, 0.2f, view->target_camera.pitch, 0.001f);
                    view->camera.yaw = lerp_snap_F1(view->camera.yaw, 0.2f, view->target_camera.yaw, 0.001f);
                    view->camera.fov = lerp_snap_F1(view->camera.fov, 0.15f, view->target_camera.fov, 0.001f);
                    view->camera.near_z = lerp_snap_F1(view->camera.near_z, 0.15f, view->target_camera.near_z, 0.001f);
                    view->camera.far_z = lerp_snap_F1(view->camera.far_z, 0.15f, view->target_camera.far_z, 0.001f);

                    //- kti: Build box.
                    
                    ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
                    ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
                    ui_set_next_background_color((F4){0.025f, 0.025f, 0.035f, 1.0f});
                    view->viewport_box = ui_build_box_from_stringf(UI_BOX_FLAG__DRAW_BACKGROUND | UI_BOX_FLAG__CLIP | UI_BOX_FLAG__CLICKABLE | UI_BOX_FLAG__SCROLL, "##viewport_%p", panel);

                    UI_Signal viewport_signal = ui_signal_from_box(view->viewport_box);
                    F2 drag_delta = ui_drag_delta();

                    //- kti: Dolly.
                    if (viewport_signal.scroll[1] != 0.0f) {
                      M4F camera_rotation = mul_M4F(rotate_x_M4F(view->camera.pitch), rotate_y_M4F(view->camera.yaw));
                      F4 camera_forward = mul_M4F_F4(camera_rotation, (F4){0.0f, 0.0f, 1.0f, 0.0f});
                      F1 dolly_speed = 0.05f;
                      view->target_camera.pos -= viewport_signal.scroll[1]*dolly_speed*camera_forward;
                    }

                    I1 is_click = dot_F2(drag_delta) <= Square(4.0f);

                    //- kti: Panning
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
                      view->camera_drag_start_pos = view->camera.pos;
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                    }
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING && !is_click) {
                      M4F camera_rotation = mul_M4F(
                          rotate_x_M4F(view->camera.pitch),
                          rotate_y_M4F(view->camera.yaw));
                      F4 camera_right = mul_M4F_F4(camera_rotation, (F4){1.0f, 0.0f, 0.0f, 0.0f});
                      F4 camera_up = mul_M4F_F4(camera_rotation, (F4){0.0f, 1.0f, 0.0f, 0.0f});
                      F1 pan_speed = 0.01f;
                      view->target_camera.pos = view->camera_drag_start_pos -
                        drag_delta[0]*pan_speed*camera_right +
                        drag_delta[1]*pan_speed*camera_up;
                    }

                    //- kti: Rotating
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__RIGHT_PRESSED) {
                      view->camera_drag_start_yaw = view->camera.yaw;
                      view->camera_drag_start_pitch = view->camera.pitch;
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                    }
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__RIGHT_DRAGGING) {
                      F1 rotate_speed = 0.005f;
                      view->target_camera.yaw = view->camera_drag_start_yaw + drag_delta[0]*rotate_speed;
                      view->target_camera.pitch = Clamp(-0.49f*PI,
                          view->camera_drag_start_pitch + drag_delta[1]*rotate_speed,
                          0.49f*PI);
                    }

                    //- kti: Entity Selecting / Selection
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_CLICKED && is_click) {
                      F2 mouse = ui_mouse();
                      F4 rect = view->viewport_box->rect;
                      F1 aspect = rect[2] / rect[3];
                      F1 u = (mouse[0] - rect[0]) / rect[2];
                      F1 v = (mouse[1] - rect[1]) / rect[3];

                      F1 tan_half_fov = tanf(0.5f * view->camera.fov);
                      F4 ray_dir_camera = normalize_F4((F4){
                        (2.0f*u-1) * aspect * tan_half_fov,
                        (1.0f - 2.0f*v) * tan_half_fov,
                        1.0f,
                        0.0f,
                      });

                      M4F camera_rotation = mul_M4F(rotate_x_M4F(view->camera.pitch), rotate_y_M4F(view->camera.yaw));

                      F4 ray_direction = mul_M4F_F4(camera_rotation, ray_dir_camera);

                      F1 min_hit_distance = 0.001f;
                      F1 closest_t = F1_MAX;
                      Entity_Handle selected_entity = entity_handle_zero();
                      for (L1 i = 0; i < state->entity_count; i += 1) {
                        Entity *e = &state->entities[i];

                        F1 t = 0.0f;
                        if (e->shape == SHAPE__BOX) {
                          F4 min = e->pos - e->size*0.5f;
                          F4 max = e->pos + e->size*0.5f;
                          t = ray_aabb_intersect(view->camera.pos, ray_direction, min, max);
                        } else if (e->shape == SHAPE__SPHERE) {
                          t = ray_sphere_intersect(view->camera.pos, ray_direction, e->pos, e->size[0]*0.5f);
                        }

                        if (t > min_hit_distance && t < closest_t) {
                          closest_t = t;
                          selected_entity = e->handle;
                        }
                      }

                      state->selected_entity = selected_entity;
                    }
                  } break;
                }
              }

              UI_Signal signal = ui_signal_from_box(panel_box);
              if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
                cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
              }
            }
          }
        }

        if (w->root_panel.first == 0) {
          UI_Text_Align((UI_TEXT_ALIGN__CENTER))
          UI_Pref_Width(ui_text_dim(20.0f, 1.0f)) {
            ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str8("Last panel closed."));
            if (ui_button(str8("Open Panel")).flags & UI_SIGNAL_FLAG__CLICKED) {
              panel_insert(panel_alloc(), &w->root_panel, 0);
            }
          }
        }

        ui_end_build();

        ProfEnd();

        ProfBegin("Draw");

        dr_begin_frame();
        gfx_window_begin_frame(w->os, w->gfx);
        DR_Bucket *bucket = dr_bucket_make();
        dr_push_bucket(bucket);

        ui_draw();

        ////////////////////////////////
        //~ 3D Draw

        ProfBegin("3D draw");
        for (Panel *panel = w->root_panel.first; panel != 0; panel = panel_rec_depth_first_pre_order(panel).next) {
          if (panel->first == 0 && panel->view_count != 0) {
            View *view = &panel->views[panel->selected_view_idx];
            if (view->kind == VIEW_KIND__VIEWPORT && view->viewport_box != 0) {
              F4 viewport_rect = view->viewport_box->rect;
              if (viewport_rect[2] > 0.0f && viewport_rect[3] > 0.0f) {
                dr_mesh_viewport(viewport_rect);

                //- kti: Projection
                F1 aspect = viewport_rect[2] / viewport_rect[3];
                M4F projection = perspective_fov_M4F(view->camera.fov, aspect, view->camera.near_z, view->camera.far_z);
                M4F view_matrix = translate_M4F(-view->camera.pos);
                view_matrix = mul_M4F(view_matrix, rotate_y_M4F(-view->camera.yaw));
                view_matrix = mul_M4F(view_matrix, rotate_x_M4F(-view->camera.pitch));
                dr_mesh_view_projection(mul_M4F(view_matrix, projection));

                //- kti: Draw scene.
                for (L1 i = 0; i < state->entity_count; i += 1) {
                  Entity *e = &state->entities[i];
                  Mesh *mesh = &state->meshes[e->shape];
                  M4F transform = mul_M4F(scale_M4F(e->size), translate_M4F(e->pos));
                  dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count,
                          mesh->index_buffer, 0, mesh->index_count,
                          transform, e->material.base_color);
                }
              }
            }
          }
        }
        ProfEnd();

        //- kti: Submit to render.
        dr_submit_bucket(w->os, w->gfx, bucket);
        dr_pop_bucket();
        gfx_window_end_frame(w->os, w->gfx);

        ProfEnd();
      }
    }

    scratch_end(scratch);

    ProfEnd();
    ProfFlush();

    //- kti: Calculate time spent and sleep until target frame time is met.
    L1 target_frame_time = 1000000000ULL / 60;
    L1 frame_end_time = os_clock();
    L1 frame_time = frame_end_time - frame_begin_time;

    if (lane_idx() == 0) {
      frame_count += 1;
      total_frame_time += frame_time;
      min_frame_time = Min(min_frame_time, frame_time);
      max_frame_time = Max(max_frame_time, frame_time);

      if (frame_count % 60 == 0) {
        F1 avg_ms = (total_frame_time / 60) / 1000000.0f;
        F1 min_ms = min_frame_time / 1000000.0f;
        F1 max_ms = max_frame_time / 1000000.0f;
        // fps = 1000.0f / avg_ms;
        // printf("Avg: %.2fms  Min: %.2fms  Max: %.2fms  (%.1f fps)\n", avg_ms, min_ms, max_ms, fps);
        total_frame_time = 0;
        min_frame_time = L1_MAX;
        max_frame_time = 0;
      }
    }

    //- kti: 0 lane sleeps if we haven't hit the target frame time. Others wait on the barrier.
    if (lane_idx() == 0 && frame_time < target_frame_time) {
      L1 remainder = target_frame_time - frame_time;
      if (remainder > 50000ULL) {
        os_sleep(remainder - 50000ULL);
      }
      while (os_clock() - frame_begin_time < target_frame_time) { }
    }

    lane_sync();
  }

  ////////////////////////////////
  //~ kti: Shutdown

  lane_sync();

  if (lane_idx() == 0) {
    while (state->first_window != 0) {
      window_close(state->first_window);
    }

    ProfShutdown();
  }
}

#endif
