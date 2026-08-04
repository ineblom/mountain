////////////////////////////////
//~ kti: TODO

//- kti: Simplify lister building code.

#if (HEADER)

typedef struct Camera Camera;
struct Camera {
  F4 pos;
  F1 yaw;
  F1 pitch;
  F1 fov;
  F1 near_z;
  F1 far_z;
};

////////////////////////////////
//~ kti: UI

typedef I1 View_Kind;
enum {
  VIEW_KIND__LISTER = 0,
  VIEW_KIND__VIEWPORT,

  VIEW_KIND_COUNT,
};

Global String8 view_kind_names[VIEW_KIND_COUNT] = {
  [VIEW_KIND__LISTER] = str8("Lister"),
  [VIEW_KIND__VIEWPORT] = str8("Viewport"),
};

typedef struct View View;
struct View {
  View_Kind kind;
  String8 title;
  L1 name_len;
  B1 name[512];

  //- kti: Viewport
  UI_Box *viewport_box;
  Camera camera;
  Camera target_camera;
  F4 camera_drag_start_pos;
  F1 camera_drag_start_yaw;
  F1 camera_drag_start_pitch;

  //- kti: Translation gizmo.
  Axis gizmo_hot_axis;
  Axis gizmo_active_axis;
  F1 gizmo_drag_applied_delta;
  F2 gizmo_drag_axis_screen;
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

///////////////////////////////
//~ kti: Entity

enum {
  ENTITY_FLAG__SHAPE  = 1 << 0,
  ENTITY_FLAG__CAMERA = 1 << 1,
  ENTITY_FLAG__SELECTED = 1 << 2,
};

typedef struct Entity Entity;
struct Entity {
  Entity *next;
  Entity *prev;
  L1 gen;

  L1 flags;
  B1 name[128];
  L1 name_len;
  F4 pos;
  F4 size;
  F4 plane_normal;
  F1 sphere_diameter;
  Shape_Kind shape_kind;
  RT_Material material;
  F4 camera_forward;
  F1 camera_vertical_fov;
  F1 camera_aperture_radius;
  F1 camera_focal_distance;
};

typedef struct Entity_Handle Entity_Handle;
struct Entity_Handle {
  Entity *ptr;
  L1 gen;
};

typedef struct Mesh Mesh;
struct Mesh {
  GFX_Buffer *vertex_buffer;
  GFX_Buffer *index_buffer;
  L1 vertex_count;
  L1 index_count;
};

////////////////////////////////
//~ kti: Cmds

typedef I1 Cmd_Kind;
enum {
  CMD_KIND__NONE = 0,

  CMD_KIND__OPEN_PANEL,
  CMD_KIND__CLOSE_PANEL,
  CMD_KIND__FOCUS_PANEL,

  CMD_KIND__SELECT_ENTITY,
  CMD_KIND__CREATE_ENTITY,
  CMD_KIND__CREATE_CAMERA,
  CMD_KIND__DELETE_SELECTED_ENTITIES,

  CMD_KIND__RENDER,

  CMD_KIND_COUNT,
};

typedef struct Cmd Cmd;
struct Cmd {
  Cmd_Kind kind;

  Window *window;
  Panel *panel;
  Entity_Handle entity;

  Dir dir;
};

////////////////////////////////
//~ kti: Lister.

typedef enum Lister_Entry_Kind {
  LISTER_ENTRY_KIND__HEADER,
  LISTER_ENTRY_KIND__TEXTEDIT,
  LISTER_ENTRY_KIND__F1,
  LISTER_ENTRY_KIND__XYZ,
  LISTER_ENTRY_KIND__COLOR,
  LISTER_ENTRY_KIND__ENUM,
  LISTER_ENTRY_KIND__CMD,
} Lister_Entry_Kind;

typedef I1 Lister_Apply;
enum {
  LISTER_APPLY__DELTA,
  LISTER_APPLY__SET,
};

typedef L1 Lister_Entry_Flags;
enum {
  LISTER_ENTRY_FLAG__NORMALIZE_F4 = 1 << 0,
};

typedef struct Lister_Number_Options Lister_Number_Options;
struct Lister_Number_Options {
  Lister_Apply apply;
  F1 default_f1;
  F1 pixels_per_unit;
  F1 min;
  F1 max;
  Lister_Entry_Flags flags;
};

typedef struct Lister_Enum_Options Lister_Enum_Options;
struct Lister_Enum_Options {
  String8 *names;
  L1 count;
};

typedef struct Lister_Value Lister_Value;
struct Lister_Value {
  Lister_Value *next;
  String8 name;
  void *data;
  L1 *text_len;
  L1 text_capacity;
};

typedef struct Lister_Entry Lister_Entry;
struct Lister_Entry {
  Lister_Entry *hash_next;
  Lister_Entry_Kind kind;
  String8 str;
  Lister_Value *first_value;
  Lister_Value *last_value;
  L1 value_count;

  union {
    Lister_Number_Options number;
    Lister_Enum_Options enum_options;
    Cmd cmd;
  } data;
};

////////////////////////////////
//~ kti: Render

typedef struct Render_Settings Render_Settings;
struct Render_Settings {
  L1 width;
  L1 height;
  L1 rays_per_pixel;
  L1 max_num_bounces;

  Image_Bloom_Params bloom;
  String8 output_filename;
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
  Entity *first_entity;
  Entity *last_entity;
  Entity *first_free_entity;
  Entity nil_entity;

  //- kti: Lister.
  Arena *lister_arena;
  L1 lister_entry_count;
  Lister_Entry lister_entries[512];
  Lister_Entry *lister_entry_hash_table[128];

  //- kti: Graphics.
  Mesh meshes[SHAPE_KIND_COUNT];

  //- kti: Render.
  Render_Settings render_settings;
  Lane_Group *render_lanes;
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
#define BV(px, py, pz, nx, ny, nz) \
  {{px, py, pz, 1.0f}, {nx, ny, nz, 0.0f}}
  GFX_Mesh_Vertex vertices[] = {
    BV(-.5f,-.5f, .5f, 0, 0, 1), BV( .5f,-.5f, .5f, 0, 0, 1), BV( .5f, .5f, .5f, 0, 0, 1), BV(-.5f, .5f, .5f, 0, 0, 1),
    BV( .5f,-.5f,-.5f, 0, 0,-1), BV(-.5f,-.5f,-.5f, 0, 0,-1), BV(-.5f, .5f,-.5f, 0, 0,-1), BV( .5f, .5f,-.5f, 0, 0,-1),
    BV(-.5f,-.5f,-.5f,-1, 0, 0), BV(-.5f,-.5f, .5f,-1, 0, 0), BV(-.5f, .5f, .5f,-1, 0, 0), BV(-.5f, .5f,-.5f,-1, 0, 0),
    BV( .5f,-.5f, .5f, 1, 0, 0), BV( .5f,-.5f,-.5f, 1, 0, 0), BV( .5f, .5f,-.5f, 1, 0, 0), BV( .5f, .5f, .5f, 1, 0, 0),
    BV(-.5f, .5f, .5f, 0, 1, 0), BV( .5f, .5f, .5f, 0, 1, 0), BV( .5f, .5f,-.5f, 0, 1, 0), BV(-.5f, .5f,-.5f, 0, 1, 0),
    BV(-.5f,-.5f,-.5f, 0,-1, 0), BV( .5f,-.5f,-.5f, 0,-1, 0), BV( .5f,-.5f, .5f, 0,-1, 0), BV(-.5f,-.5f, .5f, 0,-1, 0),
  };
#undef BV
  I1 indices[] = {
    0, 1, 2,  0, 2, 3,  4, 5, 6,  4, 6, 7,
    8, 9,10,  8,10,11, 12,13,14, 12,14,15,
    16,17,18, 16,18,19, 20,21,22, 20,22,23,
  };
  return mesh_alloc_from_vertices_indices(vertices, ArrayCount(vertices), indices, ArrayCount(indices));
}

Internal Mesh mesh_alloc_plane(void) {
  GFX_Mesh_Vertex vertices[] = {
    {{-0.5f, 0.0f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
    {{ 0.5f, 0.0f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
    {{ 0.5f, 0.0f,  0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.0f,  0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
  };
  I1 indices[] = {
    0, 2, 1,
    0, 3, 2,
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
  Axis split_axis = (dir == DIR__RIGHT || dir == DIR__LEFT) ? AXIS__X : AXIS__Y;
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
      .pos = (F4){0.0f, 1.0f, -7.0f},
      .fov = 70.0f * PI/180.0f,
      .near_z = 0.1f,
      .far_z = 100.0f,
    };
    view->target_camera = view->camera;
    view->gizmo_hot_axis = AXIS__INVALID;
    view->gizmo_active_axis = AXIS__INVALID;
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
  L1 idx = atomic_add_L1(&state->cmd_count, 1);
  if (idx < ArrayCount(state->cmds)) {
    state->cmds[idx] = cmd;
  }
}

Internal UI_Signal focus_on_press(Panel *panel, UI_Signal signal) {
  if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
    cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
  }
  return signal;
}

////////////////////////////////
//~ kti: Entities

Internal Entity_Handle entity_handle_zero() {
  Entity_Handle result = {0};
  return result;
}

Internal I1 entity_handle_match(Entity_Handle a, Entity_Handle b) {
  I1 result = (a.gen == b.gen && a.ptr == b.ptr);
  return result;
}

Internal I1 entity_is_nil(Entity *entity) {
  I1 result = (entity == 0 || entity == &state->nil_entity);
  return result;
}

Internal Entity *entity_from_handle(Entity_Handle handle) {
  Entity *result = &state->nil_entity;

  if (!entity_is_nil(handle.ptr) && handle.gen == handle.ptr->gen) {
    result = handle.ptr;
  }

  return result;
}

Internal Entity_Handle entity_handle(Entity *entity) {
  Entity_Handle result = {0};
  if (!entity_is_nil(entity)) {
    result.ptr = entity;
    result.gen = entity->gen;
  }
  return result;
}

Internal void entity_selection_clear(void) {
  for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
    entity->flags &= ~ENTITY_FLAG__SELECTED;
  }
}

Internal void entity_select(Entity_Handle handle, I1 additive) {
  Entity *entity = entity_from_handle(handle);
  if (!additive) {
    entity_selection_clear();
  }
  if (!entity_is_nil(entity)) {
    if (additive) {
      entity->flags ^= ENTITY_FLAG__SELECTED;
    } else {
      entity->flags |= ENTITY_FLAG__SELECTED;
    }
  }
}

Internal I1 entity_selection_average_pos(F4 *average_out) {
  F4 sum = {0};
  L1 count = 0;
  for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
    if (entity->flags & ENTITY_FLAG__SELECTED) {
      sum += entity->pos;
      count += 1;
    }
  }
  if (count != 0) {
    *average_out = sum / (F1)count;
  }
  return count != 0;
}

Internal Entity *entity_create(L1 flags, String8 name) {
  Entity *entity = state->first_free_entity;
  if (entity == 0) {
    entity = push_array(state->arena, Entity, 1);
    entity->gen = 1;
  } else {
    SLLStackPop(state->first_free_entity);
    L1 gen = entity->gen;
    MemoryZeroStruct(entity);
    entity->gen = gen;
  }

  DLLPushBack(state->first_entity, state->last_entity, entity);

  entity->flags = flags;
  entity->size = (F4){1.0f, 1.0f, 1.0f};
  entity->plane_normal = (F4){0.0f, 1.0f, 0.0f, 0.0f};
  entity->sphere_diameter = 1.0f;
  entity->name_len = Min(name.len, sizeof(entity->name));
  entity->material.base_color = (F4){0.9f, 0.9f, 0.9f, 1.0f};
  entity->material.emissive = (F4){0.0f, 0.0f, 0.0f, 1.0f};
  memmove(entity->name, name.str, entity->name_len);

  state->entity_count += 1;

  return entity;
}

Internal void entity_delete_selected(void) {
  for (Entity *entity = state->first_entity, *next = 0;
       !entity_is_nil(entity);
       entity = next) {
    next = entity->next;
    if (entity->flags & ENTITY_FLAG__SELECTED) {
      entity->gen += 1;
      DLLRemove(state->first_entity, state->last_entity, entity);
      SLLStackPush(state->first_free_entity, entity);
      state->entity_count -= 1;
    }
  }
}

Internal Shape shape_from_entity(Entity *entity) {
  Shape result = {.kind = entity->shape_kind};

  switch (result.kind) {
  case SHAPE_KIND__SPHERE:
    result.sphere.pos = V3_from_F4(entity->pos);
    result.sphere.radius = entity->sphere_diameter*0.5f;
    break;
  case SHAPE_KIND__BOX:
    result.box.min = V3_from_F4(entity->pos - entity->size*0.5f);
    result.box.max = V3_from_F4(entity->pos + entity->size*0.5f);
    break;
  case SHAPE_KIND__PLANE: {
    F4 normal = normalize_F4(entity->plane_normal);
    result.plane.normal = V3_from_F4(normal);
    result.plane.d = -dot_F4(normal, entity->pos);
  } break;
  default: break;
  }

  return result;
}

Internal F4 entity_mesh_size(Entity *entity) {
  F4 result = entity->size;
  if (entity->shape_kind == SHAPE_KIND__SPHERE) {
    result = (F4){
      entity->sphere_diameter,
      entity->sphere_diameter,
      entity->sphere_diameter,
      1.0f,
    };
  }
  return result;
}

////////////////////////////////
//~ kti: Lister

Internal void lister_begin(Arena *arena) {
  state->lister_arena = arena;
  state->lister_entry_count = 0;
  MemoryZeroArray(state->lister_entry_hash_table);
}

Internal Lister_Entry *lister_entry_push(Lister_Entry_Kind kind) {
  Lister_Entry *entry = 0;

  if (state->lister_entry_count < ArrayCount(state->lister_entries)) {
    entry = &state->lister_entries[state->lister_entry_count];
    state->lister_entry_count += 1;
    MemoryZeroStruct(entry);
    entry->kind = kind;
  }

  return entry;
}

Internal Lister_Entry *lister_entry_get_or_push(Lister_Entry_Kind kind, String8 str) {
  L1 hash = str8_hash(str) ^ (5381*kind);
  Lister_Entry **slot = &state->lister_entry_hash_table[hash%ArrayCount(state->lister_entry_hash_table)];
  Lister_Entry *entry = 0;

  for (Lister_Entry *it = slot[0]; it != 0; it = it->hash_next) {
    if (it->kind == kind && str8_match(it->str, str)) {
      entry = it;
      break;
    }
  }

  if (entry == 0) {
    entry = lister_entry_push(kind);
    if (entry) {
      entry->str = str;
      entry->hash_next = slot[0];
      slot[0] = entry;
    }
  }
  return entry;
}

Internal void lister_header(String8 str) {
  lister_entry_get_or_push(LISTER_ENTRY_KIND__HEADER, str);
}

Internal Lister_Value *lister_value_push(Lister_Entry *entry, String8 name, void *data) {
  Lister_Value *value = 0;
  if (entry && data && state->lister_arena) {
    value = push_array(state->lister_arena, Lister_Value, 1);
    value->name = name;
    value->data = data;
    SLLQueuePush(entry->first_value, entry->last_value, value);
    entry->value_count += 1;
  }
  return value;
}

Internal void lister_textedit(String8 str, String8 name, B1 *data, L1 *len, L1 capacity) {
  Lister_Value *value = lister_value_push(lister_entry_get_or_push(LISTER_ENTRY_KIND__TEXTEDIT, str), name, data);
  if (value) {
    value->text_len = len;
    value->text_capacity = capacity;
  }
}

Internal void lister_number(Lister_Entry_Kind kind, String8 str, String8 name, void *data, Lister_Number_Options options) {
  Lister_Entry *entry = lister_entry_get_or_push(kind, str);
  if (entry) {
    entry->data.number = options;
    lister_value_push(entry, name, data);
  }
}

#define lister_F1(str, name, data, ...) lister_number(LISTER_ENTRY_KIND__F1, (str), (name), (data), (Lister_Number_Options){__VA_ARGS__})
#define lister_xyz(str, name, data, ...) lister_number(LISTER_ENTRY_KIND__XYZ, (str), (name), (data), (Lister_Number_Options){__VA_ARGS__})
#define lister_color(str, name, data, apply_mode) lister_number(LISTER_ENTRY_KIND__COLOR, (str), (name), (data), (Lister_Number_Options){.apply = (apply_mode), .max = 1.0f})
#define lister_value(value, T) ((T *)(value)->data)

Internal void lister_apply_F1s(Lister_Entry *entry, F1 before, F1 after) {
  if (before != after) {
    F1 value = entry->data.number.apply == LISTER_APPLY__DELTA ? after - before : after;
    for (Lister_Value *it = entry->first_value; it != 0; it = it->next) {
      if (entry->data.number.apply == LISTER_APPLY__DELTA) {
        lister_value(it, F1)[0] += value;
      } else {
        lister_value(it, F1)[0] = value;
      }
    }
  }
}

Internal void lister_apply_F4s(Lister_Entry *entry, F4 before, F4 after) {
  F4 delta = after - before;
  for (Lister_Value *it = entry->first_value; it != 0; it = it->next) {
    F4 old_value = lister_value(it, F4)[0];
    if (entry->data.number.apply == LISTER_APPLY__DELTA) {
      lister_value(it, F4)[0] += delta;
    } else {
      // Preserve components that were not edited when setting multiple values.
      for (L1 component = 0; component < 4; component += 1) {
        if (delta[component] != 0.0f) {
          lister_value(it, F4)[0][component] = after[component];
        }
      }
    }

    if (entry->data.number.flags & LISTER_ENTRY_FLAG__NORMALIZE_F4) {
      F4 value = F4_with_w(lister_value(it, F4)[0], 0.0f);
      F1 length_sq = length_sq_F4(value);
      if (length_sq > Square(0.00001f)) {
        lister_value(it, F4)[0] = value * (1.0f / sqrt_F1(length_sq));
      } else {
        old_value = F4_with_w(old_value, 0.0f);
        F1 old_length_sq = length_sq_F4(old_value);
        lister_value(it, F4)[0] = old_length_sq > Square(0.00001f) ? old_value * (1.0f / sqrt_F1(old_length_sq)) : (F4){0.0f, 0.0f, 1.0f, 0.0f};
      }
    }
  }
}

Internal void lister_value_tooltip(UI_Box *overlay, Lister_Entry *entry, UI_Signal signal, L1 component) {
  String8 tooltip_id = str8f(ui_build_arena(), "##lister_value_tooltip_%p_%llu", entry, component);
  UI_Key tooltip_key = ui_key_from_string(overlay->key, tooltip_id);
  UI_Box *previous_tooltip = ui_box_from_key(tooltip_key);
  I1 tooltip_hovered = !ui_box_is_nil(previous_tooltip) && rect_contains(previous_tooltip->rect, ui_mouse());
  I1 tooltip_dragging = 0;
  if (!ui_box_is_nil(previous_tooltip)) {
    UI_Box *active_box = ui_box_from_key(ui_active_key(OS_MOUSE_BUTTON__LEFT));
    for (UI_Box *box = active_box; !ui_box_is_nil(box); box = box->parent) {
      if (box == previous_tooltip) {
        tooltip_dragging = 1;
        break;
      }
    }
  }

  if (entry->value_count > 1 &&
      (signal.flags & (UI_SIGNAL_FLAG__HOVERING | UI_SIGNAL_FLAG__DRAGGING) ||
       tooltip_hovered || tooltip_dragging)) {
    UI_Parent(overlay) {
      ui_set_next_fixed_x(signal.box->rect[0]);
      ui_set_next_fixed_y(signal.box->rect[1] + signal.box->rect[3]);
      ui_set_next_pref_width(ui_children_sum(1.0f));
      ui_set_next_pref_height(ui_children_sum(1.0f));
      ui_set_next_child_layout_axis(AXIS__Y);
      ui_set_next_background_color(oklch(0.15f, 0.0f, 0.0f, 1.0f));
      ui_set_next_corner_radius(0.0f);
      UI_Box *tooltip = ui_build_box_from_stringf(
          UI_BOX_FLAG__MOUSE_CLICKABLE |
          UI_BOX_FLAG__DRAW_BACKGROUND |
          UI_BOX_FLAG__DRAW_BORDER |
          UI_BOX_FLAG__DRAW_DROP_SHADOW,
          "%.*s", (int)tooltip_id.len, tooltip_id.str);

      UI_Parent(tooltip) {
        F1 name_width = 0.0f;
        F1 value_width = 0.0f;
        F1 row_height = 0.0f;
        for (Lister_Value *it = entry->first_value; it != 0; it = it->next) {
          F1 value = entry->kind == LISTER_ENTRY_KIND__F1
              ? lister_value(it, F1)[0]
              : lister_value(it, F4)[0][component];
          String8 value_string = str8f(ui_build_arena(), "%.2f", value);
          F2 name_dim = fc_dim_from_tag_size_string(
              ui_top_font(), ui_top_font_size(), 0, ui_top_tab_size(), it->name);
          F2 value_dim = fc_dim_from_tag_size_string(
              ui_top_font(), ui_top_font_size(), 0, ui_top_tab_size(), value_string);
          name_width = Max(name_width, name_dim[0]);
          value_width = Max(value_width, value_dim[0]);
          row_height = Max(row_height, Max(name_dim[1], value_dim[1]));
        }
        name_width += 12.0f;
        value_width += 12.0f;
        row_height += 12.0f;

        for (Lister_Value *it = entry->first_value; it != 0; it = it->next) {
          F1 default_value = entry->kind == LISTER_ENTRY_KIND__COLOR ? 0.0f : entry->data.number.default_f1;
          F1 pixels_per_unit = entry->kind == LISTER_ENTRY_KIND__COLOR ? 0.0f : entry->data.number.pixels_per_unit;
          F1 *value = 0;
          if (entry->kind == LISTER_ENTRY_KIND__F1) {
            value = lister_value(it, F1);
          } else {
            value = lister_value(it, F1) + component;
          }

          ui_set_next_pref_width(ui_children_sum(1.0f));
          ui_set_next_pref_height(ui_px(row_height, 1.0f));
          ui_set_next_child_layout_axis(AXIS__X);
          UI_Signal drag_signal = ui_drag_F1(value, default_value, pixels_per_unit, entry->data.number.min, entry->data.number.max);
          UI_Parent(drag_signal.box)
          UI_Text_Padding(6.0f) {
            ui_set_next_pref_width(ui_px(name_width, 1.0f));
            ui_set_next_pref_height(ui_px(row_height, 1.0f));
            ui_label(it->name);

            ui_set_next_pref_width(ui_px(value_width, 1.0f));
            ui_set_next_pref_height(ui_px(row_height, 1.0f));
            ui_set_next_text_align(UI_TEXT_ALIGN__CENTER);
            ui_label(str8f(ui_build_arena(), "%.2f", value[0]));
          }
        }
      }
      ui_signal_from_box(tooltip);
    }
  }
}

Internal void lister_enum(String8 str, String8 name, I1 *data, String8 *names, L1 enum_count) {
  Lister_Entry *entry = lister_entry_get_or_push(LISTER_ENTRY_KIND__ENUM, str);
  if (entry) {
    entry->data.enum_options.names = names;
    entry->data.enum_options.count = enum_count;
    lister_value_push(entry, name, data);
  }
}

Internal Lister_Entry *lister_cmd(String8 str, Cmd cmd) {
  Lister_Entry *entry = lister_entry_push(LISTER_ENTRY_KIND__CMD);
  if (entry) {
    entry->str = str;
    entry->data.cmd = cmd;
  }

  return entry;
}

////////////////////////////////
//~ kti: Camera

Internal M4F camera_view_projection(Camera camera, F1 width, F1 height) {
  F1 aspect = width / height;
  M4F projection = perspective_fov_M4F(camera.fov, aspect, camera.near_z, camera.far_z);
  M4F view = translate_M4F(-camera.pos);
  view = mul_M4F(view, rotate_y_M4F(-camera.yaw));
  view = mul_M4F(view, rotate_x_M4F(-camera.pitch));
  M4F view_projection = mul_M4F(view, projection);
  return view_projection;
}

Internal M4F line_transform_M4F(F4 begin, F4 direction, F1 thickness) {
  F4 line_axis = F4_with_w(direction, 0.0f);
  F4 reference_axis = abs_F1(line_axis[1]) < 0.99f
    ? (F4){0.0f, 1.0f, 0.0f, 0.0f}
    : (F4){1.0f, 0.0f, 0.0f, 0.0f};
  F4 side_axis = normalize_F4(cross_F4(line_axis, reference_axis));
  F4 up_axis = normalize_F4(cross_F4(side_axis, line_axis));

  M4F result = identity_M4F();
  result.r[0] = line_axis;
  result.r[1] = thickness*side_axis;
  result.r[2] = thickness*up_axis;
  result.r[3] = F4_with_w(begin + 0.5f*line_axis, 1.0f);
  return result;
}

Internal void plane_axes_from_normal(F4 normal, F4 *tangent_out, F4 *bitangent_out) {
  F4 reference_axis = abs_F1(normal[1]) < 0.99f
    ? (F4){0.0f, 1.0f, 0.0f, 0.0f}
    : (F4){0.0f, 0.0f, 1.0f, 0.0f};
  F4 tangent = normalize_F4(cross_F4(normal, reference_axis));
  F4 bitangent = cross_F4(tangent, normal);

  *tangent_out = tangent;
  *bitangent_out = bitangent;
}

Internal M4F plane_transform_M4F(Entity *entity, Camera camera) {
  F4 normal = normalize_F4(entity->plane_normal);
  F4 tangent;
  F4 bitangent;
  plane_axes_from_normal(normal, &tangent, &bitangent);

  // Keep the editor proxy centered under the viewport camera and large enough
  // to cover the full visible range. The ray-traced plane itself is infinite.
  F4 camera_to_plane = camera.pos - entity->pos;
  F4 preview_center = camera.pos - dot_F4(normal, camera_to_plane)*normal;
  F1 preview_size = 2.0f*camera.far_z;

  M4F result = identity_M4F();
  result.r[0] = preview_size*tangent;
  result.r[1] = normal;
  result.r[2] = preview_size*bitangent;
  result.r[3] = F4_with_w(preview_center, 1.0f);
  return result;
}

////////////////////////////////
//~ kti: Render

Internal void render_lane(void *user_data) {
  Arena *arena = lane_arena();

  RT_Scene *scene = 0;
  Image *hdr = 0;

  if (lane_idx() == 0) {
    Entity *camera_entity = &state->nil_entity; 

    L1 shape_count = 0;
    for (Entity *it = state->first_entity; !entity_is_nil(it); it = it->next) {
      if (it->flags & ENTITY_FLAG__CAMERA) {
        camera_entity = it;
      }
      if (it->flags & ENTITY_FLAG__SHAPE) {
        shape_count += 1;
      }
    }

    L1 shape_idx = 0;
    Shape *shapes = push_array(arena, Shape, shape_count);
    RT_Material *materials = push_array(arena, RT_Material, shape_count);

    for (Entity *it = state->first_entity; !entity_is_nil(it); it = it->next) {
      if (it->flags & ENTITY_FLAG__SHAPE) {
        shapes[shape_idx] = shape_from_entity(it);
        materials[shape_idx] = it->material;
        shape_idx += 1;
      }
    }

    scene = push_array(arena, RT_Scene, 1);
    scene[0] = (RT_Scene){
      .rays_per_pixel = state->render_settings.rays_per_pixel,
      .max_num_bounces = state->render_settings.max_num_bounces,

      .camera = {
        .pos = camera_entity->pos,  
        .forward = camera_entity->camera_forward,
        .vertical_fov = camera_entity->camera_vertical_fov,
        .aperture_radius = camera_entity->camera_aperture_radius,
        .focal_distance = camera_entity->camera_focal_distance,
      },

      .shape_count = shape_count,
      .shapes = shapes,
      .materials = materials,
    };

    hdr = push_array(arena, Image, 1);
    hdr[0] = image_alloc(arena, state->render_settings.width, state->render_settings.height, IMAGE_FORMAT__RGBA32F_LINEAR);
  }

  lane_sync_L1((L1 *)&scene, 0);
  lane_sync_L1((L1 *)&hdr, 0);

  rt_trace_scene(scene, hdr[0], lane_range(hdr->width*hdr->height));

  lane_sync();

  //- kti: Postprocess and write image to file.
  if (lane_idx() == 0) {
    Image bloomed = image_apply_bloom(arena, hdr[0], state->render_settings.bloom);
    Image postprocessed = image_I1_from_F4_tonemap(arena, bloomed, TONEMAP_KIND__LOTTES); 
    image_write_to_file(postprocessed, state->render_settings.output_filename);
  }
}

////////////////////////////////
//~ kti: Main

Internal void lane(void *user_data) {
  Arena *arena = lane_arena();

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

    state->meshes[SHAPE_KIND__SPHERE] = mesh_alloc_sphere(16, 32);
    state->meshes[SHAPE_KIND__BOX] = mesh_alloc_box();
    state->meshes[SHAPE_KIND__PLANE] = mesh_alloc_plane();

    Window *window = window_open();

    //- kti: Create initial state.

    Panel *lister_panel = panel_alloc();
    panel_push_view(lister_panel, VIEW_KIND__LISTER);
    panel_insert(lister_panel, &window->root_panel, 0);

    Panel *viewport_panel = panel_alloc();
    panel_push_view(viewport_panel, VIEW_KIND__VIEWPORT);
    panel_insert(viewport_panel, lister_panel, DIR__RIGHT);

    lister_panel->pct_of_parent = 0.3f;
    viewport_panel->pct_of_parent = 0.7f;

    Entity *starting_entity = entity_create(ENTITY_FLAG__SHAPE, str8("Starting Entity"));
    entity_select(entity_handle(starting_entity), 0);

    state->render_settings.width = 1280;
    state->render_settings.height = 720;
    state->render_settings.rays_per_pixel = 64;
    state->render_settings.max_num_bounces = 8;

    state->render_settings.bloom.pass_count = 8;
    state->render_settings.bloom.threshold = 0.5f;
    state->render_settings.bloom.strength = 0.4f;
    state->render_settings.bloom.knee = 0.5f;

    state->render_settings.output_filename = str8("output.bmp");
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
      //- kti: Build lister.
      lister_begin(scratch.arena);

      {
        String8 *shape_names = push_array(scratch.arena, String8, SHAPE_KIND_COUNT);
        for (L1 i = 0; i < SHAPE_KIND_COUNT; i += 1) {
          shape_names[i] = shape_kind_name(i);
        }

        I1 has_camera = 0;
        for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
          has_camera |= !!(entity->flags & ENTITY_FLAG__CAMERA);
          if (entity->flags & ENTITY_FLAG__SELECTED) {
            String8 entity_name = (String8){entity->name, entity->name_len};
            I1 is_shape = !!(entity->flags & ENTITY_FLAG__SHAPE);
            I1 is_camera = !!(entity->flags & ENTITY_FLAG__CAMERA);

            //- kti: Common entries.

            lister_textedit(str8("Name"), entity_name, entity->name, &entity->name_len, sizeof(entity->name));

            lister_xyz(str8("Pos"), entity_name,
              &entity->pos,
              .apply = LISTER_APPLY__DELTA,
              .pixels_per_unit = 50.0f);

            lister_enum(str8("Shape"), entity_name,
              is_shape ? &entity->shape_kind : 0,
              shape_names, SHAPE_KIND_COUNT);

            //- kti: Shape specific entries.

            lister_xyz(str8("Size"), entity_name,
              (is_shape && entity->shape_kind == SHAPE_KIND__BOX) ? &entity->size : 0,
              .apply = LISTER_APPLY__DELTA,
              .default_f1 = 1.0f,
              .max = F1_MAX);

            lister_F1(str8("Diameter"), entity_name, (is_shape && entity->shape_kind == SHAPE_KIND__SPHERE) ? &entity->sphere_diameter : 0,
              .apply = LISTER_APPLY__DELTA,
              .default_f1 = 1.0f,
              .max = F1_MAX);

            lister_xyz(str8("Normal"), entity_name, (is_shape && entity->shape_kind == SHAPE_KIND__PLANE) ? &entity->plane_normal : 0,
              .apply = LISTER_APPLY__SET,
              .pixels_per_unit = 50.0f,
              .min = -1.0f,
              .max = 1.0f,
              .flags = LISTER_ENTRY_FLAG__NORMALIZE_F4);

            //- kti: For all shapes.

            if (is_shape) {
              lister_header(str8("Material"));
              lister_color(str8("Base"), entity_name, &entity->material.base_color, LISTER_APPLY__SET);
              lister_F1(str8("Metallic"), entity_name, &entity->material.metallic,
                .apply = LISTER_APPLY__DELTA,
                .default_f1 = 0.3f,
                .max = 1.0f);
              lister_F1(str8("Roughness"), entity_name, &entity->material.roughness,
                .apply = LISTER_APPLY__DELTA,
                .default_f1 = 0.3f,
                .max = 1.0f);
              lister_color(str8("Emissive"), entity_name, &entity->material.emissive, LISTER_APPLY__SET);
            }

            //- kti: Camera entries.

            if (is_camera) {
              lister_header(str8("Camera"));
              lister_xyz(str8("Forward"), entity_name,
                &entity->camera_forward,
                .apply = LISTER_APPLY__SET,
                .pixels_per_unit = 50.0f,
                .min = -1.0f,
                .max = 1.0f,
                .flags = LISTER_ENTRY_FLAG__NORMALIZE_F4);
              lister_F1(str8("Vertical FOV"), entity_name,
                &entity->camera_vertical_fov,
                .apply = LISTER_APPLY__SET,
                .default_f1 = 70.0f*PI/180.0f,
                .min = PI/180.0f,
                .max = 179.0f*PI/180.0f);
              lister_F1(str8("Aperture Radius"), entity_name,
                &entity->camera_aperture_radius,
                .apply = LISTER_APPLY__SET,
                .max = F1_MAX);
              lister_F1(str8("Focal Distance"), entity_name,
                &entity->camera_focal_distance,
                .apply = LISTER_APPLY__SET,
                .default_f1 = 5.0f,
                .min = 0.001f,
                .max = F1_MAX);
            }
          }
        }

        if (state->lister_entry_count != 0) {
          lister_cmd(str8("Delete"), (Cmd){
            .kind = CMD_KIND__DELETE_SELECTED_ENTITIES,
          });
        } else {
          lister_header(str8("Entities"));
          for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
            lister_cmd((String8){
              .str = entity->name,
              .len = entity->name_len,
            }, (Cmd){
              .kind = CMD_KIND__SELECT_ENTITY,
              .entity = entity_handle(entity),
            });
          }
        }

        lister_header(str8("Actions"));
        lister_cmd(str8("Create Entity"), (Cmd){
          .kind = CMD_KIND__CREATE_ENTITY,
        });
        if (!has_camera) {
          lister_cmd(str8("Create Camera"), (Cmd){
            .kind = CMD_KIND__CREATE_CAMERA,
          });
        } else if (state->entity_count >= 2) {
          lister_cmd(str8("Render"), (Cmd){
            .kind = CMD_KIND__RENDER,
          });
        }
      }

      ////////////////////////////////
      //~ UI

      ProfBegin("UI");

      fc_frame();

#if defined(__APPLE__)
      CString user_home = getenv("HOME");
      String8 prop_fnt_path = str8f(scratch.arena, "%s/Library/Fonts/Bloomberg-PropU_N.ttf", user_home);
      String8 fixed_fnt_path = str8f(scratch.arena, "%s/Library/Fonts/Bloomberg-FixedU_N.ttf", user_home);
      FC_Tag prop_fnt = fc_tag_from_path(prop_fnt_path);
      FC_Tag fixed_fnt = fc_tag_from_path(fixed_fnt_path);
#else
      FC_Tag prop_fnt = fc_tag_from_path(str8("/usr/share/fonts/bloomberg/" "Bloomberg-PropU_N.ttf"));
      FC_Tag fixed_fnt = fc_tag_from_path(str8("/usr/share/fonts/bloomberg/" "Bloomberg-FixedU_N.ttf"));
#endif

      for (Window *w = state->first_window; w != 0; w = w->next) {
        ui_state_equip(w->ui);
        ui_begin_build(w->os, events, ui_cmds);

        ui_push_font(prop_fnt);
        ui_push_background_color((F4){0.0f, 0.0f, 0.0f, 1.0f});
        ui_push_border_color((F4){0.5f, 0.0f, 0.0f, 1.0f});

        F4 root_plane_rect = {0, 0, w->os->width, w->os->height};
        ui_set_next_fixed_rect(root_plane_rect);
        UI_Box *overlay = ui_build_box_from_stringf(
            UI_BOX_FLAG__FLOATING |
            UI_BOX_FLAG__ALLOW_OVERFLOW_X |
            UI_BOX_FLAG__ALLOW_OVERFLOW_Y,
            "##overlay_%p", w);

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
                  //- kti: Lister view.
                  case VIEW_KIND__LISTER: {
                    ui_set_next_child_layout_axis(AXIS__Y);
                    UI_Box *lister = ui_build_box_from_stringf(0, "lister%p", view);
                    UI_Parent(lister) {
                      L1 visible_entry_idx = 0;
                      for (L1 i = 0; i < state->lister_entry_count; i += 1) {
                        Lister_Entry *entry = &state->lister_entries[i];

                        //- kti: Skip entries without values.
                        if (entry->kind != LISTER_ENTRY_KIND__HEADER &&
                            entry->kind != LISTER_ENTRY_KIND__CMD &&
                            entry->value_count == 0) {
                          continue;
                        }

                        UI_Box_Flags top_side = (visible_entry_idx == 0)*UI_BOX_FLAG__DRAW_SIDE_TOP;
                        visible_entry_idx += 1;
                        CString drag_label_format = entry->value_count == 1 ? "%.*s %.2f" : "%.*s";

                        switch (entry->kind) {
                          case LISTER_ENTRY_KIND__HEADER: {
                            ui_set_next_text_padding(10.0f);
                            ui_set_next_background_color(oklch(0.192f, 0.0f, 0.0f, 1.0f));
                            ui_set_next_text_color(oklch(0.507f, 0.208f, 29.2f, 1.0f));
                            ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
                                                     UI_BOX_FLAG__DRAW_BACKGROUND|
                                                     UI_BOX_FLAG__DRAW_SIDE_LEFT|
                                                     UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                                     UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                                     top_side , entry->str);
                          } break;
                          case LISTER_ENTRY_KIND__TEXTEDIT: {
                            Lister_Value *first_value = entry->first_value;
                            ui_set_next_pref_width(ui_pct(1.0f, 1.0f));
                            ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_LEFT|
                                              UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                              UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                              top_side);
                            ui_set_next_omit_flags(UI_BOX_FLAG__DRAW_BORDER);
                            UI_Text_Padding(10.0f) {
                              UI_Signal signal = ui_textedit(&state->name_cursor,
                                                             &state->name_mark,
                                                             state->name_edit_buffer,
                                                             sizeof(state->name_edit_buffer),
                                                             &state->name_edit_buffer_len,
                                                             (String8){first_value->data, first_value->text_len[0]},
                                                             str8f(ui_build_arena(), "###entity_name_%p", first_value->data));
                              if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
                                cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                              }
                              if (signal.flags & UI_SIGNAL_FLAG__COMMIT) {
                                for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
                                  value->text_len[0] = Min(state->name_edit_buffer_len, value->text_capacity);
                                  memmove(value->data, state->name_edit_buffer, value->text_len[0]);
                                }
                              }
                            }
                          } break;
                          case LISTER_ENTRY_KIND__F1: {
                            UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_f1%p", entry);
                            UI_Parent(drag_box) {
                              UI_Text_Align(UI_TEXT_ALIGN__CENTER)
                              UI_Text_Padding(10.0f) {
                                ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_LEFT|
                                                  UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                                  UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                                  top_side);
                                F1 before = lister_value(entry->first_value, F1)[0];
                                F1 after = before;
                                UI_Signal signal = ui_drag_F1_label(entry->str,
                                                                    drag_label_format,
                                                                    &after, entry->data.number.default_f1, entry->data.number.pixels_per_unit,
                                                                    entry->data.number.min, entry->data.number.max);
                                focus_on_press(panel, signal);
                                lister_apply_F1s(entry, before, after);
                                lister_value_tooltip(overlay, entry, signal, 0);
                              }
                            }
                          } break;
                          case LISTER_ENTRY_KIND__XYZ: {
                            UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_xyz%p", entry);
                            UI_Parent(drag_box) {
                              ui_set_next_text_padding(10.0f);
                              ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
                                                       UI_BOX_FLAG__DRAW_SIDE_LEFT|
                                                       UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                                       UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                                       top_side,
                                                       entry->str);
                              UI_Text_Align(UI_TEXT_ALIGN__CENTER) {
                                F4 before = lister_value(entry->first_value, F4)[0];
                                F4 after = before;
                                F1 after_components[3] = {after[0], after[1], after[2]};
                                UI_Signal signals[3] = {0};
                                ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_RIGHT|UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
                                signals[0] = ui_drag_F1_label(str8("X"), drag_label_format,
                                                              &after_components[0], entry->data.number.default_f1, entry->data.number.pixels_per_unit, entry->data.number.min, entry->data.number.max);
                                focus_on_press(panel, signals[0]);
                                ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_RIGHT|UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
                                signals[1] = ui_drag_F1_label(str8("Y"), drag_label_format,
                                                              &after_components[1], entry->data.number.default_f1, entry->data.number.pixels_per_unit, entry->data.number.min, entry->data.number.max);
                                focus_on_press(panel, signals[1]);
                                ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_RIGHT|UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
                                signals[2] = ui_drag_F1_label(str8("Z"), drag_label_format,
                                                              &after_components[2], entry->data.number.default_f1, entry->data.number.pixels_per_unit, entry->data.number.min, entry->data.number.max);
                                focus_on_press(panel, signals[2]);
                                after[0] = after_components[0];
                                after[1] = after_components[1];
                                after[2] = after_components[2];
                                lister_apply_F4s(entry, before, after);
                                for (L1 component = 0; component < 3; component += 1) {
                                  lister_value_tooltip(overlay, entry, signals[component], component);
                                }
                              }
                            }
                          } break;
                          case LISTER_ENTRY_KIND__COLOR: {
                            UI_Box *drag_box = ui_build_box_from_stringf(0, "drag_xyz%p", entry);
                            UI_Parent(drag_box) {
                              ui_set_next_text_padding(10.0f);
                              ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
                                                       UI_BOX_FLAG__DRAW_SIDE_LEFT|
                                                       UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                                       UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                                       top_side,
                                                       entry->str);

                              F4 before = lister_value(entry->first_value, F4)[0];
                              F4 after = before;
                              F1 after_components[3] = {after[0], after[1], after[2]};
                              ui_set_next_background_color(oklch_from_linear_rgb(after));
                              ui_set_next_pref_width(ui_px(30.0f, 1.0f));
                              ui_build_box_from_string(UI_BOX_FLAG__DRAW_BACKGROUND|
                                                       UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                                       UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                                       top_side, str8("color_preview"));

                              UI_Text_Align(UI_TEXT_ALIGN__CENTER)
                              UI_Pref_Width(ui_pct(0.75f/3.0f, 1.0f)) {
                                F1 pixels_per_unit = 0.0f;
                                UI_Signal signals[3] = {0};
                                ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_RIGHT|UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
                                ui_set_next_background_color(oklch(0.27f, 0.1f, 27.0f, 1.0f));
                                signals[0] = ui_drag_F1_label(str8("R"), drag_label_format,
                                                              &after_components[0], 0.0f, pixels_per_unit, 0.0f, 1.0f);
                                focus_on_press(panel, signals[0]);
                                ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_RIGHT|UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
                                ui_set_next_background_color(oklch(0.27f, 0.09f, 143.0f, 1.0f));
                                signals[1] = ui_drag_F1_label(str8("G"), drag_label_format,
                                                              &after_components[1], 0.0f, pixels_per_unit, 0.0f, 1.0f);
                                focus_on_press(panel, signals[1]);
                                ui_set_next_flags(UI_BOX_FLAG__DRAW_SIDE_RIGHT|UI_BOX_FLAG__DRAW_SIDE_BOTTOM|top_side);
                                ui_set_next_background_color(oklch(0.27f, 0.09f, 256.0f, 1.0f));
                                signals[2] = ui_drag_F1_label(str8("B"), drag_label_format,
                                                              &after_components[2], 0.0f, pixels_per_unit, 0.0f, 1.0f);
                                focus_on_press(panel, signals[2]);
                                after[0] = after_components[0];
                                after[1] = after_components[1];
                                after[2] = after_components[2];
                                lister_apply_F4s(entry, before, after);
                                for (L1 component = 0; component < 3; component += 1) {
                                  lister_value_tooltip(overlay, entry, signals[component], component);
                                }
                              }
                            }
                          } break;
                          case LISTER_ENTRY_KIND__ENUM: {
                            ui_set_next_child_layout_axis(AXIS__X);
                            UI_Box *enum_box = ui_build_box_from_stringf(0, "enum_%p", entry);
                            UI_Parent(enum_box) {
                              ui_set_next_text_padding(10.0f);
                              ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT|
                                                       UI_BOX_FLAG__DRAW_SIDE_LEFT|
                                                       UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                                       top_side,
                                                       entry->str);

                              UI_Text_Align(UI_TEXT_ALIGN__CENTER)
                              UI_Pref_Width(ui_pct(1.0f/(F1)entry->data.enum_options.count, 1.0f)) {
                                I1 enum_value = lister_value(entry->first_value, I1)[0];
                                I1 enum_is_mixed = 0;
                                for (Lister_Value *value = entry->first_value->next; value != 0; value = value->next) {
                                  if (lister_value(value, I1)[0] != enum_value) {
                                    enum_is_mixed = 1;
                                    break;
                                  }
                                }
                                for (L1 option = 0; option < entry->data.enum_options.count; option += 1) {
                                  if (!enum_is_mixed && enum_value == option) {
                                    ui_set_next_background_color(oklch(0.35f, 0.08f, 252.0f, 1.0f));
                                  }

                                  String8 name = entry->data.enum_options.names[option];
                                  UI_Box *option_box = ui_build_box_from_stringf(
                                      UI_BOX_FLAG__CLICKABLE|
                                      UI_BOX_FLAG__DRAW_TEXT|
                                      UI_BOX_FLAG__DRAW_BACKGROUND|
                                      UI_BOX_FLAG__DRAW_HOT_EFFECTS|
                                      UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS|
                                      UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                      UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                      top_side,
                                      "%.*s##enum_%p_%llu",
                                      (int)name.len, name.str,
                                      entry, option);
                                  UI_Signal signal = ui_signal_from_box(option_box);
                                  if (signal.flags & UI_SIGNAL_FLAG__PRESSED) {
                                    for (Lister_Value *value = entry->first_value; value != 0; value = value->next) {
                                      lister_value(value, I1)[0] = option;
                                    }
                                  }
                                }
                              }
                            }
                          } break;
                          case LISTER_ENTRY_KIND__CMD: {
                            ui_set_next_background_color(oklch(0.2f, 0.1, 27.0f, 1.0f));
                            ui_set_next_text_align(UI_TEXT_ALIGN__CENTER);
                            UI_Box *cmd_box = ui_build_box_from_stringf(
                                UI_BOX_FLAG__CLICKABLE|
                                UI_BOX_FLAG__DRAW_TEXT|
                                UI_BOX_FLAG__DRAW_BACKGROUND|
                                UI_BOX_FLAG__DRAW_HOT_EFFECTS|
                                UI_BOX_FLAG__DRAW_ACTIVE_EFFECTS|
                                UI_BOX_FLAG__DRAW_SIDE_LEFT|
                                UI_BOX_FLAG__DRAW_SIDE_RIGHT|
                                UI_BOX_FLAG__DRAW_SIDE_BOTTOM|
                                top_side,
                                "%.*s##lister_cmd_%p",
                                (int)entry->str.len, entry->str.str,
                                (void *)entry);
                            UI_Signal signal = ui_signal_from_box(cmd_box);
                            if (signal.flags & UI_SIGNAL_FLAG__PRESSED) {
                              cmd_push(entry->data.cmd);
                            }
                          } break;
                        }
                      }
                    }
                  } break;

                  //- kti: 3D viewport.
                  case VIEW_KIND__VIEWPORT: {
                    //- kti: Animate camera.
                    view->camera.pos = lerp_snap_F4(view->camera.pos, 0.2f, view->target_camera.pos, 0.001f);
                    view->camera.pitch = lerp_snap_F1(view->camera.pitch, 0.3f, view->target_camera.pitch, 0.001f);
                    view->camera.yaw = lerp_snap_F1(view->camera.yaw, 0.3f, view->target_camera.yaw, 0.001f);
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

                    //- kto: Gizmo
                    F4 gizmo_pos = {0};
                    I1 has_selection = entity_selection_average_pos(&gizmo_pos);
                    if (view->gizmo_active_axis == AXIS__INVALID) {
                      // - kti: Find axis closest to mouse.
                      view->gizmo_hot_axis = AXIS__INVALID;
                      F1 closest_dist = 8.0f;
                      F4 rect = view->viewport_box->rect;
                      F2 mouse_local = ui_mouse() - (F2){rect[0], rect[1]};

                      if (has_selection && rect[2] > 0.0f && rect[3] > 0.0f) {
                        M4F view_projection = camera_view_projection(view->camera, rect[2], rect[3]);

                        for (L1 axis = 0; axis < AXIS3_COUNT; axis += 1) {
                          F4 begin_world = F4_with_w(gizmo_pos, 1.0f);
                          F4 end_world = begin_world;
                          end_world[axis] += 1.0f;
                          F4 begin_clip = mul_M4F_F4(view_projection, begin_world);
                          F4 end_clip = mul_M4F_F4(view_projection, end_world);

                          if (begin_clip[3] > 0.0f && end_clip[3] > 0.0f) {
                            F2 begin_ndc = F2_from_F4(begin_clip / begin_clip[3]);
                            F2 end_ndc = F2_from_F4(end_clip / end_clip[3]);
                            F2 begin_local = {
                              (begin_ndc[0]*0.5f + 0.5f)*rect[2],
                              (0.5f - begin_ndc[1]*0.5f)*rect[3],
                            };
                            F2 end_local = {
                              (end_ndc[0]*0.5f + 0.5f)*rect[2],
                              (0.5f - end_ndc[1]*0.5f)*rect[3],
                            };
                            F1 dist = distance_to_segment_F2(mouse_local, begin_local, end_local);
                            if (dist < closest_dist) {
                              closest_dist = dist;
                              view->gizmo_hot_axis = axis;
                              view->gizmo_drag_axis_screen = end_local - begin_local;
                            }
                          }
                        }
                      }
                    } else {
                      view->gizmo_hot_axis = view->gizmo_active_axis;
                    }

                    //- kti: Capture press on axis.
                    I1 mouse_captured = view->gizmo_active_axis != AXIS__INVALID;
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED &&
                        view->gizmo_hot_axis != AXIS__INVALID &&
                        has_selection) {
                      view->gizmo_active_axis = view->gizmo_hot_axis;
                      view->gizmo_drag_applied_delta = 0.0f;
                      mouse_captured = 1;
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                    }

                    //- kti: Move along axis when dragging.
                    if (mouse_captured &&
                        viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING &&
                        has_selection) {
                      F1 axis_len_sq = dot_F2(view->gizmo_drag_axis_screen, view->gizmo_drag_axis_screen);
                      if (axis_len_sq > 1.0f) {
                        F1 world_delta = dot_F2(drag_delta, view->gizmo_drag_axis_screen) / axis_len_sq;
                        F1 delta = world_delta - view->gizmo_drag_applied_delta;
                        for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
                          if (entity->flags & ENTITY_FLAG__SELECTED) {
                            entity->pos[view->gizmo_active_axis] += delta;
                          }
                        }
                        view->gizmo_drag_applied_delta = world_delta;
                      }
                    }

                    //- kti: Dolly.
                    if (viewport_signal.scroll[1] != 0.0f) {
                      M4F camera_rotation = mul_M4F(rotate_x_M4F(view->camera.pitch), rotate_y_M4F(view->camera.yaw));
                      F4 camera_forward = mul_M4F_F4(camera_rotation, (F4){0.0f, 0.0f, 1.0f, 0.0f});
                      F1 dolly_speed = 0.025f;
                      view->target_camera.pos -= viewport_signal.scroll[1]*dolly_speed*camera_forward;
                    }

                    I1 is_click = dot_F2(drag_delta, drag_delta) <= Square(4.0f);

                    //- kti: Panning
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED && !mouse_captured) {
                      view->camera_drag_start_pos = view->camera.pos;
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                    }
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING && !is_click && !mouse_captured) {
                      M4F camera_rotation = mul_M4F(rotate_x_M4F(view->camera.pitch), rotate_y_M4F(view->camera.yaw));
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
                      F1 rotate_speed = 0.003f;
                      view->target_camera.yaw = view->camera_drag_start_yaw + drag_delta[0]*rotate_speed;
                      view->target_camera.pitch = Clamp(-0.49f*PI,
                          view->camera_drag_start_pitch + drag_delta[1]*rotate_speed,
                          0.49f*PI);
                    }

                    //- kti: Entity Selecting / Selection
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_CLICKED && is_click && !mouse_captured) {
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
                      Ray ray = {
                        .pos = view->camera.pos,
                        .dir = mul_M4F_F4(camera_rotation, ray_dir_camera),
                      };
                      ray.inv_dir = 1.0f/ray.dir;

                      F1 min_hit_distance = 0.001f;
                      F1 closest_t = F1_MAX;
                      Entity_Handle picked_entity = entity_handle_zero();
                      for (Entity *e = state->first_entity; !entity_is_nil(e); e = e->next) {
                        if (!(e->flags & ENTITY_FLAG__SHAPE)) {
                          continue;
                        }

                        Shape shape = shape_from_entity(e);
                        F1 t = ray_shape_intersect(ray, shape);

                        if (t > min_hit_distance && t < closest_t) {
                          closest_t = t;
                          picked_entity = entity_handle(e);
                        }
                      }

                      I1 additive = !!(viewport_signal.modifiers & OS_MODIFIER_FLAG__SHIFT);
                      if (!additive || !entity_is_nil(entity_from_handle(picked_entity))) {
                        entity_select(picked_entity, additive);
                      }
                    }

                    //- kti: Reset active axis on mouse release.
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_RELEASED) {
                      view->gizmo_active_axis = AXIS__INVALID;
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
                M4F view_projection = camera_view_projection(view->camera, viewport_rect[2], viewport_rect[3]);
                dr_mesh_view_projection(view_projection);

                //- kti: Draw scene.
                for (Entity *e = state->first_entity; !entity_is_nil(e); e = e->next) {
                  if (e->flags & ENTITY_FLAG__SHAPE) {
                    Mesh *mesh = &state->meshes[e->shape_kind];
                    M4F transform = e->shape_kind == SHAPE_KIND__PLANE
                      ? plane_transform_M4F(e, view->camera)
                      : mul_M4F(scale_M4F(entity_mesh_size(e)), translate_M4F(e->pos));
                    F4 color = e->material.base_color;
                    dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count, mesh->index_buffer, 0, mesh->index_count, transform, color, GFX_MESH_FEATURE__NONE);
                  }
                }

                //- kti: Draw extra stuff for selected entities.
                for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
                  if (entity->flags & ENTITY_FLAG__SELECTED && entity->flags & ENTITY_FLAG__SHAPE) {
                    Mesh *mesh = &state->meshes[entity->shape_kind];
                    M4F transform = entity->shape_kind == SHAPE_KIND__PLANE
                      ? plane_transform_M4F(entity, view->camera)
                      : mul_M4F(scale_M4F(entity_mesh_size(entity)), translate_M4F(entity->pos));
                    F4 color = {0.9f, 0.0f, 0.9f, 1.0f};
                    dr_mesh_outline(mesh->vertex_buffer, 0, mesh->vertex_count,
                                    mesh->index_buffer, 0, mesh->index_count,
                                    transform, color, 3.0f);
                  }
                }

                //- kti: Camera forward markers.
                dr_clear_depth();
                for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
                  if (entity->flags & ENTITY_FLAG__CAMERA) {
                    Mesh *mesh = &state->meshes[SHAPE_KIND__BOX];
                    F1 thickness = 0.025f;
                    M4F transform = line_transform_M4F(entity->pos, entity->camera_forward, thickness);
                    F4 color = {0.9f, 0.75f, 0.15f, 1.0f};
                    dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count,
                            mesh->index_buffer, 0, mesh->index_count,
                            transform, color, GFX_MESH_FEATURE__UNLIT);
                  }
                }

                //- kti: Gizmo.
                F4 gizmo_pos = {0};
                if (entity_selection_average_pos(&gizmo_pos)) {
                  dr_clear_depth();
                  Mesh *mesh = &state->meshes[SHAPE_KIND__BOX];
                  F1 thickness = 0.025f; 

                  for (L1 axis = 0; axis < AXIS3_COUNT; axis += 1) {
                    I1 is_hot = view->gizmo_hot_axis == axis;
                    I1 is_active = view->gizmo_active_axis == axis;
                    F1 axis_thickness = thickness * (is_hot ? 1.45f : 1.0f);
                    F1 offset = (axis == 0) ? -axis_thickness*0.5f : axis_thickness*0.5f;
                    F4 world = gizmo_pos;
                    world[axis] += 0.5f+offset;

                    F4 scale = (F4){axis_thickness, axis_thickness, axis_thickness, 1.0f};
                    scale[axis] = 1.0f;

                    M4F transform = mul_M4F(scale_M4F(scale), translate_M4F(world));

                    F1 base_brightness = is_active ? 0.35f : 0.0f;
                    F4 color = (F4){base_brightness, base_brightness, base_brightness, 1.0f};
                    color[axis] = is_active ? 1.0f : 0.8f;

                    dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count,
                            mesh->index_buffer, 0, mesh->index_count,
                            transform, color, GFX_MESH_FEATURE__UNLIT);

                  }
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

      ////////////////////////////////
      //~ kti: Execute Cmds

      for (L1 i = 0; i < state->cmd_count && i < ArrayCount(state->cmds); i += 1) {
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

          case CMD_KIND__SELECT_ENTITY: {
            if (!entity_is_nil(entity_from_handle(cmd.entity))) {
              entity_select(cmd.entity, 0);
            }
          } break;
          case CMD_KIND__CREATE_ENTITY: {
            Entity *new = entity_create(ENTITY_FLAG__SHAPE, str8("New Entity"));
            entity_select(entity_handle(new), 0);
          } break;
          case CMD_KIND__CREATE_CAMERA: {
            Entity *new = entity_create(ENTITY_FLAG__CAMERA, str8("Camera"));
            new->pos = (F4){0.0f, 0.5f, -5.0f};
            new->camera_forward = normalize_F4((F4){0.0f, -0.5f, 5.0f});
            new->camera_vertical_fov = 70.0f*PI/180.0f;
            new->camera_aperture_radius = 0.0f;
            new->camera_focal_distance = 5.0f;
            entity_select(entity_handle(new), 0);
          } break;
          case CMD_KIND__DELETE_SELECTED_ENTITIES: {
            entity_delete_selected();
          } break;
          case CMD_KIND__RENDER: {
            Lane_Group_Params lane_params = {
              .count = Max(1, os_core_count()/2),

              .proc = render_lane,

              .arena_size = GiB(1),
              .scratch_size = MiB(64),
            };
            state->render_lanes = lane_group_start(lane_params);
          } break;
        }
      }
      state->cmd_count = 0;

      if (state->render_lanes && lane_group_completed(state->render_lanes)) {
        lane_group_stop(state->render_lanes);
        state->render_lanes = 0;
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

SI1 main(void) {
  Lane_Group_Params params = {
    .count = os_core_count(),
    .proc = lane,

    .arena_size = GiB(1),
    .scratch_size = MiB(64),

    .lane_zero_on_caller = 1,
  };
  Lane_Group *group = lane_group_start(params);
  lane_group_stop(group);

  return 0;
}

#endif
