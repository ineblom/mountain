////////////////////////////////
//~ kti: TODO

//- kti: Gizmo rotation for directions. 
//- kti: Camera icon and picking.
//- kti: Define scene by code.
//- kti: Light rays.
//- kti: Remote Rendering on GPU.
//- kti: BVH.
//- kti: Lister groups.
//- kti: Better rendering in the viewport.
//- kti: Panel focus when pressing clickable boxes inside panel.
//- kti: Clamp panel pct of parent.
//- kti: Multiple listers with different contents.
//- kti: Reivew color picker code.
//- kti: Snapping.

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
  VIEW_KIND__RENDER_RESULT,

  VIEW_KIND_COUNT,
};

Global String8 view_kind_names[VIEW_KIND_COUNT] = {
  [VIEW_KIND__LISTER] = str8("Lister"),
  [VIEW_KIND__VIEWPORT] = str8("Viewport"),
  [VIEW_KIND__RENDER_RESULT] = str8("Render Result"),
};

enum {
  GIZMO_AXIS_LENGTH_PX = 82,
  GIZMO_SHAFT_THICKNESS_PX = 4,
  GIZMO_SIZE_HANDLE_SIZE_PX = 11,
  GIZMO_ROTATION_RADIUS_PX = 62,
  GIZMO_ROTATION_SEGMENT_COUNT = 48,
};

typedef I1 Gizmo_Kind;
enum {
  GIZMO_KIND__NONE = 0,
  GIZMO_KIND__TRANSLATE,
  GIZMO_KIND__SCALE,
  GIZMO_KIND__ROTATE,
};

typedef struct Gizmo_Drag Gizmo_Drag;
struct Gizmo_Drag {
  F1 applied_amount;
  F2 axis_screen;
  F4 rotation_axis;
  F4 rotation_direction;
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
  F1 camera_drag_start_yaw;
  F1 camera_drag_start_pitch;

  //- kti: Transform gizmo.
  Gizmo_Kind gizmo_hot_kind;
  Gizmo_Kind gizmo_active_kind;
  Axis gizmo_hot_axis;
  Axis gizmo_active_axis;
  F4 gizmo_pos;
  F2 gizmo_screen_pos;
  F2 gizmo_axes_screen[AXIS3_COUNT];
  F2 gizmo_rotation_points_screen[AXIS3_COUNT][GIZMO_ROTATION_SEGMENT_COUNT];
  I1 gizmo_rotation_points_visible[AXIS3_COUNT][GIZMO_ROTATION_SEGMENT_COUNT];
  I1 gizmo_rotation_visible;
  F1 gizmo_world_per_pixel;
  I1 gizmo_visible;

  //- kti: Render result
  UI_Box *render_result_box;
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
  F4 direction;
  F1 sphere_diameter;
  Shape_Kind shape_kind;
  RT_Material material;
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
  CMD_KIND__CANCEL_RENDER,

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
//~ kti: Render

typedef struct Render_Settings Render_Settings;
struct Render_Settings {
  L1 width;
  L1 height;
  L1 rays_per_pixel;
  L1 max_num_bounces;
};

typedef struct Postprocessing_Settings Postprocessing_Settings;
struct Postprocessing_Settings {
  Image_Bloom_Params bloom;
};

typedef struct Render_Job Render_Job;
struct Render_Job {
  Arena *arena;

  Render_Settings settings;
  RT_Scene scene;
  Image hdr;

  L1 pixels_completed;
  L1 pixels_total;
  L1 next_pixel;

  I1 cancel_requested;

  I1 completed;
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

  //- kti: Entities.
  L1 entity_count;
  Entity *first_entity;
  Entity *last_entity;
  Entity *first_free_entity;
  Entity nil_entity;

  //- kti: Graphics.
  Mesh meshes[SHAPE_KIND_COUNT];

  //- kti: Render.
  Render_Settings render_settings;
  Postprocessing_Settings postprocessing_settings;
  Render_Job *active_render;
  Render_Job *last_render;
  Arena *display_arena;
  Image display_image;
  GFX_Texture *render_result_texture;
};

#endif

#if (SOURCE)

Global State *state = 0;

#define UI_THEME_COLOR(r, g, b, a, ...) \
  { \
    .tags = { \
      .v = (String8[]){__VA_ARGS__}, \
      .count = ArrayCount(((String8[]){__VA_ARGS__})), \
    }, \
    .linear = (F4){(r), (g), (b), (a)}, \
  }

Global UI_Theme_Pattern default_theme_patterns[] = {
  UI_THEME_COLOR(0.0f, 0.0f, 0.0f, 1.0f, str8("background")),
  UI_THEME_COLOR(0.125f, 0.125f, 0.125f, 1.0f, str8("border")),
  UI_THEME_COLOR(0.729f, 0.729f, 0.729f, 1.0f, str8("text")),
  UI_THEME_COLOR(0.0f, 0.0f, 0.0f, 0.25f, str8("drop_shadow")),
  UI_THEME_COLOR(1.058446053f, -0.025667136f, -0.017405831f, 0.02f, str8("focus"), str8("overlay")),
  UI_THEME_COLOR(0.321444194f, 0.645543671f, 1.175802262f, 1.0f, str8("focus"), str8("border")),
  UI_THEME_COLOR(0.521444194f, 0.845543671f, 1.0f, 1.0f, str8("cursor")),
  UI_THEME_COLOR(1.113309022f, -0.230441843f, 0.096778714f, 0.05f, str8("selection")),

  UI_THEME_COLOR(0.008f, 0.008f, 0.008f, 1.0f, str8("subtle"), str8("background")),
  UI_THEME_COLOR(0.035227284f, 0.329110300f, 1.016848732f, 1.0f, str8("accent"), str8("text")),
  UI_THEME_COLOR(0.526210363f, 0.000019104f, 0.000108155f, 1.0f, str8("header"), str8("text")),
  UI_THEME_COLOR(0.008101465f, 0.017500665f, 0.023206312f, 1.0f, str8("field"), str8("background")),
  UI_THEME_COLOR(0.008402845f, 0.044952845f, 0.123825366f, 1.0f, str8("selected"), str8("background")),
  UI_THEME_COLOR(0.038710978f, -0.002232542f, -0.000659834f, 1.0f, str8("command"), str8("background")),
  UI_THEME_COLOR(0.037265774f, 0.381785296f, 0.004780161f, 1.0f, str8("progress"), str8("background")),
  UI_THEME_COLOR(0.0f, 0.0f, 0.0f, 1.0f, str8("viewport"), str8("background")),

  UI_THEME_COLOR(0.036278413f, -0.002480615f, 0.001241720f, 1.0f, str8("checkbox"), str8("background")),
  UI_THEME_COLOR(0.125f, 0.125f, 0.125f, 1.0f, str8("checkbox"), str8("border")),
  UI_THEME_COLOR(0.343f, 0.343f, 0.343f, 1.0f, str8("checkbox"), str8("check"), str8("background")),
};

Global UI_Theme default_theme = {
  .patterns = default_theme_patterns,
  .pattern_count = ArrayCount(default_theme_patterns),
};

#undef UI_THEME_COLOR

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
  entity->direction = (F4){0.0f, 1.0f, 0.0f, 0.0f};
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
    F4 normal = normalize_F4(entity->direction);
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
  F4 line_direction = normalize_F4(line_axis);
  F4 reference_axis = abs_F1(line_direction[1]) < 0.99f
    ? (F4){0.0f, 1.0f, 0.0f, 0.0f}
    : (F4){1.0f, 0.0f, 0.0f, 0.0f};
  F4 side_axis = normalize_F4(cross_F4(line_direction, reference_axis));
  F4 up_axis = normalize_F4(cross_F4(side_axis, line_direction));

  M4F result = identity_M4F();
  result.r[0] = line_axis;
  result.r[1] = thickness*side_axis;
  result.r[2] = thickness*up_axis;
  result.r[3] = F4_with_w(begin + 0.5f*line_axis, 1.0f);
  return result;
}

Internal void plane_axes_from_normal(F4 normal, F4 *tangent_out, F4 *bitangent_out) {
  F4 reference_axis = abs_F1(normal[1]) < 0.99f ? (F4){0.0f, 1.0f, 0.0f, 0.0f} : (F4){0.0f, 0.0f, 1.0f, 0.0f};
  F4 tangent = normalize_F4(cross_F4(normal, reference_axis));
  F4 bitangent = cross_F4(tangent, normal);

  tangent_out[0] = tangent;
  bitangent_out[0] = bitangent;
}

Internal M4F plane_transform_M4F(Entity *entity, Camera camera) {
  F4 normal = normalize_F4(entity->direction);
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
  Render_Job *job = (Render_Job *)user_data;
  Render_Settings settings = job->settings;
  RT_Scene scene = job->scene;

  if (lane_idx() == 0) {
    //- kti: Allocate the HDR image in the job arena so it remains available
    // after the worker lanes have exited.
    job->hdr = image_alloc(job->arena, settings.width, settings.height, IMAGE_FORMAT__RGBA32F_LINEAR);

    //- kti: Initialize progress.
    L1 pixels_total = job->hdr.width * job->hdr.height;
    atomic_swap_L1(&job->next_pixel, 0);
    atomic_swap_L1(&job->pixels_completed, 0);
    atomic_swap_L1(&job->pixels_total, pixels_total);
  }

  lane_sync();

  //- kti: Trace
  L1 pixels_total = job->hdr.width * job->hdr.height;
  L1 pixels_per_chunk = 256;

  while (atomic_load_I1(&job->cancel_requested) == 0) {
    L1 first_pixel = atomic_add_L1(&job->next_pixel, pixels_per_chunk);
    if (first_pixel >= pixels_total) break;

    Range range = { first_pixel, Min(first_pixel+pixels_per_chunk, pixels_total) };
    rt_trace_scene(scene, job->hdr, range);

    atomic_add_L1(&job->pixels_completed, range.max-range.min);
  }

  lane_sync();
}

Internal void editor_apply_postprocessing(void) {
  if (state->last_render != 0 && !image_is_nil(state->last_render->hdr)) {
    arena_clear(state->display_arena);
    state->display_image = (Image){0};

    Image bloomed = image_apply_bloom( state->display_arena, state->last_render->hdr, state->postprocessing_settings.bloom);
    state->display_image = image_I1_from_F4_tonemap(state->display_arena, bloomed, TONEMAP_KIND__LOTTES);

    if (!image_is_nil(state->display_image)) {
      GFX_Texture *new_texture = gfx_tex2d_alloc(
        GFX_TEXTURE_USAGE__STATIC,
        state->display_image.width,
        state->display_image.height,
        state->display_image.pixels);

      if (state->render_result_texture != 0) {
        gfx_tex2d_free(state->render_result_texture);
      }
      state->render_result_texture = new_texture;
    }
  }
}

Internal I1 postprocessing_settings_match(Postprocessing_Settings a, Postprocessing_Settings b) {
  I1 result =
    a.bloom.pass_count == b.bloom.pass_count &&
    a.bloom.threshold == b.bloom.threshold &&
    a.bloom.strength == b.bloom.strength &&
    a.bloom.knee == b.bloom.knee;
  return result;
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

    state->postprocessing_settings.bloom.pass_count = 8;
    state->postprocessing_settings.bloom.threshold = 0.5f;
    state->postprocessing_settings.bloom.strength = 0.4f;
    state->postprocessing_settings.bloom.knee = 0.5f;

    state->display_arena = arena_alloc(GiB(1));
  }

  lane_sync();

  L1 running = 1;
  L1 last_frame_begin_time = 0;

  ////////////////////////////////
  //~ kti: Main loop

  while (running) {
    ProfBegin("Frame");

    L1 frame_begin_time = os_clock();
    F1 time = (F1)(frame_begin_time / 1000000ULL) / 1000.0f;
    F1 animation_dt = 1.0f/60.0f;
    if (last_frame_begin_time != 0) {
      animation_dt = (F1)(frame_begin_time - last_frame_begin_time)/1000000000.0f;
    }
    last_frame_begin_time = frame_begin_time;

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
      Postprocessing_Settings postprocessing_settings_before_ui = state->postprocessing_settings;
      I1 postprocessing_dirty = 0;

      //- kti: Build lister.
      lister_reset(scratch.arena);

      {
        String8 *shape_names = push_array(scratch.arena, String8, SHAPE_KIND_COUNT);
        for (L1 i = 0; i < SHAPE_KIND_COUNT; i += 1) {
          shape_names[i] = shape_kind_name(i);
        }

        I1 has_camera = 0;
        L1 selected_count = 0;
        for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
          has_camera |= !!(entity->flags & ENTITY_FLAG__CAMERA);
          if (entity->flags & ENTITY_FLAG__SELECTED) {
            selected_count += 1;
            
            String8 entity_name = (String8){entity->name, entity->name_len};
            I1 is_shape = !!(entity->flags & ENTITY_FLAG__SHAPE);
            I1 is_camera = !!(entity->flags & ENTITY_FLAG__CAMERA);

            //- kti: Common entries.

            lister_textedit(str8("Name"), entity->name, &entity->name_len, sizeof(entity->name));

            lister_xyz( str8("Pos"), &entity->pos,.pixels_per_unit = 50.0f);

            lister_enum(str8("Shape"), is_shape ? &entity->shape_kind : 0, shape_names, SHAPE_KIND_COUNT);

            //- kti: Shape specific entries.

            lister_xyz(str8("Size"),
              (is_shape && entity->shape_kind == SHAPE_KIND__BOX) ? &entity->size : 0,
              .default_value = 1.0f,
              .min = 0.01f);

            lister_F1(str8("Diameter"),
              (is_shape && entity->shape_kind == SHAPE_KIND__SPHERE) ? &entity->sphere_diameter : 0,
              .default_value = 1.0f,
              .min = 0.01f);

            lister_xyz(str8("Normal"), 
              (is_shape && entity->shape_kind == SHAPE_KIND__PLANE) ? &entity->direction : 0,
              .pixels_per_unit = 50.0f,
              .min = -1.0f,
              .max = 1.0f,
              .flags = LISTER_ENTRY_FLAG__NORMALIZE_F4);

            //- kti: For all shapes.

            if (is_shape) {
              lister_header(str8("Material"));
              lister_color(str8("Base"), &entity->material.base_color,
                .max = 1.0f);
              lister_F1(str8("Metallic"), &entity->material.metallic,
                .default_value = 0.3f,
                .max = 1.0f);
              lister_F1(str8("Roughness"), &entity->material.roughness,
                .default_value = 0.3f,
                .max = 1.0f);
              lister_color(str8("Emissive"), &entity->material.emissive);
            }

            //- kti: Camera entries.

            if (is_camera) {
              lister_header(str8("Camera"));
              lister_xyz(str8("Forward"),
                &entity->direction,
                .pixels_per_unit = 50.0f,
                .min = -1.0f,
                .max = 1.0f,
                .flags = LISTER_ENTRY_FLAG__NORMALIZE_F4);
              lister_F1(str8("Vertical FOV"),
                &entity->camera_vertical_fov,
                .default_value = 70.0f*PI/180.0f,
                .min = PI/180.0f,
                .max = 179.0f*PI/180.0f);
              lister_F1(str8("Aperture Radius"), &entity->camera_aperture_radius);
              lister_F1(str8("Focal Distance"), &entity->camera_focal_distance, .default_value = 5.0f, .min = 0.001f);
            }
          }
        }

        if (selected_count > 0) {
          //- kti: Entity specific commands.
          lister_cmd(str8("Delete"), (Cmd){
            .kind = CMD_KIND__DELETE_SELECTED_ENTITIES,
          });
        } else {
          //- kti: List out entities
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

          //- kti: Render Settings
          lister_header(str8("Render Settings"));
          lister_L1(str8("Width"), &state->render_settings.width,
            .default_value = 1280,
            .min = 320);
          lister_L1(str8("Height"), &state->render_settings.height,
            .default_value = 720,
            .min = 160);
          lister_L1(str8("Rays Per Pixel"), &state->render_settings.rays_per_pixel,
            .default_value = 64);
          lister_L1(str8("Max Bounces"), &state->render_settings.max_num_bounces,
            .default_value = 8);

          lister_header(str8("Postprocessing"));

          lister_L1(str8("Passes"), &state->postprocessing_settings.bloom.pass_count,
            .default_value = 8);

          lister_F1(str8("Threshold"), &state->postprocessing_settings.bloom.threshold,
            .default_value = 0.5f,
            .pixels_per_unit = 50.0f,
            .max = F1_MAX);

          lister_F1(str8("Strength"), &state->postprocessing_settings.bloom.strength,
            .default_value = 0.4f,
            .min = 0.0f,
            .max = 1.0f);

          lister_F1(str8("Knee"), &state->postprocessing_settings.bloom.knee,
            .default_value = 0.5f,
            .pixels_per_unit = 50.0f,
            .max = F1_MAX);
        }

        //- kti: Actions.
        lister_header(str8("Actions"));

        lister_cmd(str8("Create Entity"), (Cmd){
          .kind = CMD_KIND__CREATE_ENTITY,
        });

        if (!has_camera) {
          lister_cmd(str8("Create Camera"), (Cmd){
            .kind = CMD_KIND__CREATE_CAMERA,
          });
        } else if (state->entity_count >= 2) {
          // only allow 1 render at a time
          if (state->active_render == 0) {
            lister_cmd(str8("Render"), (Cmd){
              .kind = CMD_KIND__RENDER,
            });
          } else {
            // grab progress values
            L1 completed = atomic_load_L1(&state->active_render->pixels_completed);
            L1 total = atomic_load_L1(&state->active_render->pixels_total);
            F1 pct = (total > 0) ? (F1)completed/(F1)total : 0.0f;

            lister_progress(str8("Tracing"), pct);

            lister_cmd(str8("Cancel"), (Cmd){
              .kind = CMD_KIND__CANCEL_RENDER,
            });
          }
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
        ui_begin_build(w->os, events, ui_cmds, &default_theme, animation_dt);

        ui_push_font(prop_fnt);

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
                ui_store_drag_struct(OS_MOUSE_BUTTON__LEFT, &drag_data);
              }
              F2 drag_data = ui_get_drag_struct(OS_MOUSE_BUTTON__LEFT, F2)[0];
              F2 drag_delta = ui_drag_delta(OS_MOUSE_BUTTON__LEFT);
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
                UI_Tag(str8("subtle")) {
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
                    UI_Tag(str8("accent"))
                    ui_build_box_from_string(UI_BOX_FLAG__DRAW_TEXT, str8("Choose view kind."));

                    UI_Row()
                    UI_Tag(str8("subtle"))
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
                  //- kti: Lister
                  case VIEW_KIND__LISTER: {
                    ui_set_next_child_layout_axis(AXIS__Y);
                    ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
                    UI_Box *lister = ui_build_box_from_stringf(
                      UI_BOX_FLAG__CLIP|
                      UI_BOX_FLAG__ALLOW_OVERFLOW_Y|
                      UI_BOX_FLAG__VIEW_SCROLL_Y|
                      UI_BOX_FLAG__VIEW_CLAMP_Y,
                      "lister%p", view);
                    ui_signal_from_box(lister);

                    UI_Parent(lister) {
                      lister_ui();
                    }
                  } break;

                  //- kti: Ray-traced render result.
                  case VIEW_KIND__RENDER_RESULT: {
                    ui_set_next_pref_width(ui_pct(1.0f, 0.0f));
                    ui_set_next_pref_height(ui_pct(1.0f, 0.0f));
                    ui_set_next_tag(str8("viewport"));
                    view->render_result_box = ui_build_box_from_stringf(
                      UI_BOX_FLAG__DRAW_BACKGROUND|
                      UI_BOX_FLAG__CLIP|
                      UI_BOX_FLAG__CLICKABLE,
                      "##render_result_%p", panel);

                    UI_Signal signal = ui_signal_from_box(view->render_result_box);
                    if (signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED) {
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
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
                    ui_set_next_tag(str8("viewport"));
                    view->viewport_box = ui_build_box_from_stringf(
                      UI_BOX_FLAG__DRAW_BACKGROUND|
                      UI_BOX_FLAG__CLIP|
                      UI_BOX_FLAG__CLICKABLE|
                      UI_BOX_FLAG__SCROLL,
                      "##viewport_%p", panel);

                    UI_Signal viewport_signal = ui_signal_from_box(view->viewport_box);
                    F2 left_drag_delta = ui_drag_delta(OS_MOUSE_BUTTON__LEFT);
                    F2 right_drag_delta = ui_drag_delta(OS_MOUSE_BUTTON__RIGHT);
                    F4 rect = view->viewport_box->rect;

                    //- kti: Hit test last frame's gizmo.
                    view->gizmo_hot_kind = GIZMO_KIND__NONE;
                    view->gizmo_hot_axis = AXIS__INVALID;
                    if (view->gizmo_visible && view->gizmo_active_kind == GIZMO_KIND__NONE) {
                      Axis hot_knob = AXIS__INVALID;
                      Axis hot_shaft = AXIS__INVALID;
                      F1 knob_dist = 9.0f;
                      F1 shaft_dist = 7.0f;
                      for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1) {
                        F2 screen_axis = view->gizmo_axes_screen[axis];
                        F1 screen_length = length_F2(screen_axis);
                        if (screen_length > 1.0f) {
                          F2 end = view->gizmo_screen_pos + screen_axis;
                          F1 dist = length_F2(ui_mouse() - end);
                          if (dist < knob_dist) {
                            knob_dist = dist;
                            hot_knob = axis;
                          }
                          F2 dir = screen_axis/screen_length;
                          dist = distance_to_segment_F2(ui_mouse(),
                            view->gizmo_screen_pos + 2.0f*dir, end - 8.0f*dir);
                          if (dist < shaft_dist) {
                            shaft_dist = dist;
                            hot_shaft = axis;
                          }
                        }
                      }
                      view->gizmo_hot_axis = hot_knob != AXIS__INVALID ? hot_knob : hot_shaft;
                      view->gizmo_hot_kind = hot_knob != AXIS__INVALID
                        ? GIZMO_KIND__SCALE
                        : hot_shaft != AXIS__INVALID
                          ? GIZMO_KIND__TRANSLATE
                          : GIZMO_KIND__NONE;

                      if (view->gizmo_hot_kind == GIZMO_KIND__NONE && view->gizmo_rotation_visible) {
                        F1 ring_dist = 7.0f;
                        for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1) {
                          for (L1 i = 0; i < GIZMO_ROTATION_SEGMENT_COUNT; i += 1) {
                            L1 next = (i + 1)%GIZMO_ROTATION_SEGMENT_COUNT;
                            if (view->gizmo_rotation_points_visible[axis][i] &&
                                view->gizmo_rotation_points_visible[axis][next]) {
                              F1 dist = distance_to_segment_F2(ui_mouse(),
                                view->gizmo_rotation_points_screen[axis][i],
                                view->gizmo_rotation_points_screen[axis][next]);
                              if (dist < ring_dist) {
                                ring_dist = dist;
                                view->gizmo_hot_axis = axis;
                                view->gizmo_hot_kind = GIZMO_KIND__ROTATE;
                              }
                            }
                          }
                        }
                      }
                    } else if (view->gizmo_active_kind != GIZMO_KIND__NONE) {
                      view->gizmo_hot_kind = view->gizmo_active_kind;
                      view->gizmo_hot_axis = view->gizmo_active_axis;
                    }

                    I1 mouse_captured = view->gizmo_active_kind != GIZMO_KIND__NONE;
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED &&
                        view->gizmo_hot_kind != GIZMO_KIND__NONE) {
                      view->gizmo_active_kind = view->gizmo_hot_kind;
                      view->gizmo_active_axis = view->gizmo_hot_axis;

                      Gizmo_Drag drag = {0};
                      if (view->gizmo_active_kind == GIZMO_KIND__ROTATE) {
                        F2 mouse = ui_mouse();
                        F1 aspect = rect[2]/rect[3];
                        F1 u = (mouse[0] - rect[0])/rect[2];
                        F1 v = (mouse[1] - rect[1])/rect[3];
                        F1 tan_half_fov = tan_F1(0.5f*view->camera.fov);
                        F4 ray_dir_camera = normalize_F4((F4){
                          (2.0f*u - 1.0f)*aspect*tan_half_fov,
                          (1.0f - 2.0f*v)*tan_half_fov,
                          1.0f,
                          0.0f,
                        });
                        M4F camera_rotation = mul_M4F(
                          rotate_x_M4F(view->camera.pitch),
                          rotate_y_M4F(view->camera.yaw));
                        Ray ray = {
                          .pos = view->camera.pos,
                          .dir = mul_M4F_F4(camera_rotation, ray_dir_camera),
                        };

                        drag.rotation_axis[view->gizmo_active_axis] = 1.0f;
                        Plane rotation_plane = {
                          .normal = V3_from_F4(drag.rotation_axis),
                          .d = -dot_F4(drag.rotation_axis, view->gizmo_pos),
                        };
                        F1 t = ray_plane_intersect(ray, rotation_plane);
                        if (t > 0.0f) {
                          drag.rotation_direction = normalize_F4(
                            ray.pos + t*ray.dir - view->gizmo_pos);
                        } else {
                          view->gizmo_active_kind = GIZMO_KIND__NONE;
                          view->gizmo_active_axis = AXIS__INVALID;
                        }
                      } else {
                        Axis axis = view->gizmo_active_axis;
                        drag.axis_screen = view->gizmo_axes_screen[axis]/
                          (GIZMO_AXIS_LENGTH_PX*view->gizmo_world_per_pixel);
                      }
                      if (view->gizmo_active_kind != GIZMO_KIND__NONE) {
                        ui_store_drag_struct(OS_MOUSE_BUTTON__LEFT, &drag);
                        mouse_captured = 1;
                        cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                      }
                    }

                    //- kti: Reset axis to 0 on middle mouse press.
                    if (!mouse_captured &&
                        viewport_signal.flags & UI_SIGNAL_FLAG__MIDDLE_PRESSED &&
                        view->gizmo_hot_kind == GIZMO_KIND__TRANSLATE) {
                      Axis axis = view->gizmo_hot_axis;
                      for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
                        if (entity->flags & ENTITY_FLAG__SELECTED) {
                          entity->pos[axis] -= view->gizmo_pos[axis];
                        }
                      }
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                    }

                    if (mouse_captured && viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING) {
                      Gizmo_Drag *drag = ui_get_drag_struct(OS_MOUSE_BUTTON__LEFT, Gizmo_Drag);
                      if (view->gizmo_active_kind == GIZMO_KIND__ROTATE) {
                        F2 mouse = ui_mouse();
                        F1 aspect = rect[2]/rect[3];
                        F1 u = (mouse[0] - rect[0])/rect[2];
                        F1 v = (mouse[1] - rect[1])/rect[3];
                        F1 tan_half_fov = tan_F1(0.5f*view->camera.fov);
                        F4 ray_dir_camera = normalize_F4((F4){
                          (2.0f*u - 1.0f)*aspect*tan_half_fov,
                          (1.0f - 2.0f*v)*tan_half_fov,
                          1.0f,
                          0.0f,
                        });
                        M4F camera_rotation = mul_M4F(
                          rotate_x_M4F(view->camera.pitch),
                          rotate_y_M4F(view->camera.yaw));
                        Ray ray = {
                          .pos = view->camera.pos,
                          .dir = mul_M4F_F4(camera_rotation, ray_dir_camera),
                        };
                        Plane rotation_plane = {
                          .normal = V3_from_F4(drag[0].rotation_axis),
                          .d = -dot_F4(drag[0].rotation_axis, view->gizmo_pos),
                        };
                        F1 t = ray_plane_intersect(ray, rotation_plane);
                        if (t > 0.0f) {
                          F4 direction = normalize_F4(ray.pos + t*ray.dir - view->gizmo_pos);
                          F1 angle = atan2f(
                            dot_F4(drag[0].rotation_axis, cross_F4(drag[0].rotation_direction, direction)),
                            dot_F4(drag[0].rotation_direction, direction));
                          M4F rotation = view->gizmo_active_axis == AXIS__X
                            ? rotate_x_M4F(angle)
                            : view->gizmo_active_axis == AXIS__Y
                              ? rotate_y_M4F(angle)
                              : rotate_z_M4F(angle);

                          for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
                            if (entity->flags & ENTITY_FLAG__SELECTED) {
                              I1 has_direction = (entity->flags & ENTITY_FLAG__CAMERA) ||
                                ((entity->flags & ENTITY_FLAG__SHAPE) && entity->shape_kind == SHAPE_KIND__PLANE);
                              if (has_direction) {
                                entity->direction = normalize_F4(mul_M4F_F4(rotation, entity->direction));
                              }
                            }
                          }
                          drag[0].rotation_direction = direction;
                        }
                      } else {
                        F1 axis_len_sq = dot_F2(drag[0].axis_screen, drag[0].axis_screen);
                        if (axis_len_sq > 0.0001f) {
                          F1 amount = dot_F2(left_drag_delta, drag[0].axis_screen)/axis_len_sq;
                          F1 change = amount - drag[0].applied_amount;
                          for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
                            if (entity->flags & ENTITY_FLAG__SELECTED) {
                              Axis axis = view->gizmo_active_axis;
                              if (view->gizmo_active_kind == GIZMO_KIND__TRANSLATE) {
                                entity->pos[axis] += change;
                              } else if (entity->shape_kind == SHAPE_KIND__BOX) {
                                entity->size[axis] = Max(0.01f, entity->size[axis] + change);
                              } else if (entity->shape_kind == SHAPE_KIND__SPHERE) {
                                entity->sphere_diameter = Max(0.01f, entity->sphere_diameter + change);
                              }
                            }
                          }
                          drag[0].applied_amount = amount;
                        }
                      }
                    }

                    //- kti: Update the gizmo after applying the transform.
                    F4 position_sum = {0};
                    L1 selected_count = 0;
                    L1 direction_count = 0;
                    for (Entity *entity = state->first_entity; !entity_is_nil(entity); entity = entity->next) {
                      if (entity->flags & ENTITY_FLAG__SELECTED) {
                        position_sum += entity->pos;
                        selected_count += 1;

                        I1 has_direction = (entity->flags & ENTITY_FLAG__CAMERA) ||
                          ((entity->flags & ENTITY_FLAG__SHAPE) && entity->shape_kind == SHAPE_KIND__PLANE);
                        if (has_direction) {
                          direction_count += 1;
                        }
                      }
                    }

                    view->gizmo_visible = selected_count != 0 && rect[2] > 0.0f && rect[3] > 0.0f;
                    view->gizmo_rotation_visible = direction_count != 0;
                    if (view->gizmo_visible) {
                      view->gizmo_pos = position_sum/(F1)selected_count;

                      M4F view_projection = camera_view_projection(view->camera, rect[2], rect[3]);
                      F4 pivot_clip = mul_M4F_F4(view_projection, F4_with_w(view->gizmo_pos, 1.0f));
                      view->gizmo_visible = pivot_clip[3] > view->camera.near_z;
                      if (view->gizmo_visible) {
                        F2 pivot_ndc = F2_from_F4(pivot_clip/pivot_clip[3]);
                        view->gizmo_screen_pos = (F2){
                          rect[0] + (pivot_ndc[0]*0.5f + 0.5f)*rect[2],
                          rect[1] + (0.5f - pivot_ndc[1]*0.5f)*rect[3],
                        };
                        view->gizmo_world_per_pixel = 2.0f*pivot_clip[3]*tan_F1(0.5f*view->camera.fov)/rect[3];
                        for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1) {
                          F4 end = view->gizmo_pos;
                          end[axis] += GIZMO_AXIS_LENGTH_PX*view->gizmo_world_per_pixel;
                          F4 end_clip = mul_M4F_F4(view_projection, F4_with_w(end, 1.0f));
                          view->gizmo_axes_screen[axis] = (F2){0};
                          if (end_clip[3] > view->camera.near_z) {
                            F2 end_ndc = F2_from_F4(end_clip/end_clip[3]);
                            F2 end_screen = {
                              rect[0] + (end_ndc[0]*0.5f + 0.5f)*rect[2],
                              rect[1] + (0.5f - end_ndc[1]*0.5f)*rect[3],
                            };
                            view->gizmo_axes_screen[axis] = end_screen - view->gizmo_screen_pos;
                          }
                        }

                        if (view->gizmo_rotation_visible) {
                          F1 radius = GIZMO_ROTATION_RADIUS_PX*view->gizmo_world_per_pixel;
                          for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1) {
                            F4 basis_a = {0};
                            F4 basis_b = {0};
                            if (axis == AXIS__X) {
                              basis_a[AXIS__Y] = 1.0f;
                              basis_b[AXIS__Z] = 1.0f;
                            } else if (axis == AXIS__Y) {
                              basis_a[AXIS__Z] = 1.0f;
                              basis_b[AXIS__X] = 1.0f;
                            } else {
                              basis_a[AXIS__X] = 1.0f;
                              basis_b[AXIS__Y] = 1.0f;
                            }

                            for (L1 i = 0; i < GIZMO_ROTATION_SEGMENT_COUNT; i += 1) {
                              F1 angle = 2.0f*PI*(F1)i/(F1)GIZMO_ROTATION_SEGMENT_COUNT;
                              F4 point = view->gizmo_pos + radius*(cos_F1(angle)*basis_a + sin_F1(angle)*basis_b);
                              F4 point_clip = mul_M4F_F4(view_projection, F4_with_w(point, 1.0f));
                              view->gizmo_rotation_points_visible[axis][i] = point_clip[3] > view->camera.near_z;
                              if (view->gizmo_rotation_points_visible[axis][i]) {
                                F2 point_ndc = F2_from_F4(point_clip/point_clip[3]);
                                view->gizmo_rotation_points_screen[axis][i] = (F2){
                                  rect[0] + (point_ndc[0]*0.5f + 0.5f)*rect[2],
                                  rect[1] + (0.5f - point_ndc[1]*0.5f)*rect[3],
                                };
                              }
                            }
                          }
                        }
                      } else {
                        view->gizmo_rotation_visible = 0;
                      }
                    } else {
                      view->gizmo_rotation_visible = 0;
                    }

                    //- kti: Dolly.
                    if (viewport_signal.scroll[1] != 0.0f) {
                      M4F camera_rotation = mul_M4F(rotate_x_M4F(view->camera.pitch), rotate_y_M4F(view->camera.yaw));
                      F4 camera_forward = mul_M4F_F4(camera_rotation, (F4){0.0f, 0.0f, 1.0f, 0.0f});
                      F1 dolly_speed = 0.025f;
                      view->target_camera.pos -= viewport_signal.scroll[1]*dolly_speed*camera_forward;
                    }

                    I1 left_is_click = dot_F2(left_drag_delta, left_drag_delta) <= Square(4.0f);

                    //- kti: Panning
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_PRESSED && !mouse_captured) {
                      F4 camera_drag_start_pos = view->camera.pos;
                      ui_store_drag_struct(OS_MOUSE_BUTTON__LEFT, &camera_drag_start_pos);
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                    }
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_DRAGGING && !left_is_click && !mouse_captured) {
                      F4 camera_drag_start_pos = ui_get_drag_struct(OS_MOUSE_BUTTON__LEFT, F4)[0];
                      M4F camera_rotation = mul_M4F(rotate_x_M4F(view->camera.pitch), rotate_y_M4F(view->camera.yaw));
                      F4 camera_right = mul_M4F_F4(camera_rotation, (F4){1.0f, 0.0f, 0.0f, 0.0f});
                      F4 camera_up = mul_M4F_F4(camera_rotation, (F4){0.0f, 1.0f, 0.0f, 0.0f});
                      F1 pan_speed = 0.01f;
                      view->target_camera.pos = camera_drag_start_pos -
                        left_drag_delta[0]*pan_speed*camera_right +
                        left_drag_delta[1]*pan_speed*camera_up;
                    }

                    //- kti: Rotating
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__RIGHT_PRESSED) {
                      view->camera_drag_start_yaw = view->camera.yaw;
                      view->camera_drag_start_pitch = view->camera.pitch;
                      cmd_push((Cmd){.kind = CMD_KIND__FOCUS_PANEL, .panel = panel});
                    }
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__RIGHT_DRAGGING) {
                      F1 rotate_speed = 0.003f;
                      view->target_camera.yaw = view->camera_drag_start_yaw + right_drag_delta[0]*rotate_speed;
                      view->target_camera.pitch = Clamp(-0.49f*PI,
                          view->camera_drag_start_pitch + right_drag_delta[1]*rotate_speed,
                          0.49f*PI);
                    }

                    //- kti: Entity Selecting / Selection
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_CLICKED && left_is_click && !mouse_captured) {
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

                    //- kti: Reset active gizmo part on mouse release.
                    if (viewport_signal.flags & UI_SIGNAL_FLAG__LEFT_RELEASED) {
                      view->gizmo_active_kind = GIZMO_KIND__NONE;
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

        ProfBegin("3D draw");
        for (Panel *panel = w->root_panel.first; panel != 0; panel = panel_rec_depth_first_pre_order(panel).next) {
          if (panel->first == 0 && panel->view_count != 0) {
            View *view = &panel->views[panel->selected_view_idx];

            ////////////////////////////////
            //~ Render Result Draw

            if (view->kind == VIEW_KIND__RENDER_RESULT && view->render_result_box != 0 && state->render_result_texture != 0) {
              F4 bounds = view->render_result_box->rect;
              GFX_Texture *texture = state->render_result_texture;
              if (bounds[2] > 0.0f && bounds[3] > 0.0f && texture->width > 0 && texture->height > 0) {
                F1 scale = Min(bounds[2]/(F1)texture->width, bounds[3]/(F1)texture->height);
                F2 fitted_size = {
                  scale*(F1)texture->width,
                  scale*(F1)texture->height,
                };
                F4 dst = {
                  bounds[0] + 0.5f*(bounds[2] - fitted_size[0]),
                  bounds[1] + 0.5f*(bounds[3] - fitted_size[1]),
                  fitted_size[0],
                  fitted_size[1],
                };

                F4 src = { 0.0f, (F1)texture->height, (F1)texture->width, -(F1)texture->height, };
                dr_push_clip(bounds);
                dr_img(dst, src, texture, (F4){1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, 0.0f);
                dr_pop_clip();
              }
            }


            ////////////////////////////////
            //~ 3D Draw


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
                    M4F transform = line_transform_M4F(entity->pos, entity->direction, thickness);
                    F4 color = {0.9f, 0.75f, 0.15f, 1.0f};
                    dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count,
                            mesh->index_buffer, 0, mesh->index_count,
                            transform, color, GFX_MESH_FEATURE__UNLIT);
                  }
                }

                //- kti: Gizmo.
                if (view->gizmo_visible) {
                  dr_clear_depth();
                  Mesh *mesh = &state->meshes[SHAPE_KIND__BOX];

                  for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1) {
                    if (length_F2(view->gizmo_axes_screen[axis]) <= 1.0f) {
                      continue;
                    }

                    I1 hot = view->gizmo_hot_axis == axis && view->gizmo_hot_kind == GIZMO_KIND__TRANSLATE;
                    I1 active = view->gizmo_active_axis == axis && view->gizmo_active_kind == GIZMO_KIND__TRANSLATE;
                    F1 base = active ? 0.35f : hot ? 0.15f : 0.0f;
                    F4 color = (F4){base, base, base, 1.0f};
                    color[axis] = active || hot ? 1.0f : 0.8f;

                    //- kti: Axis
                    F4 direction = {0};
                    direction[axis] = (GIZMO_AXIS_LENGTH_PX - 0.5f*GIZMO_SIZE_HANDLE_SIZE_PX) * view->gizmo_world_per_pixel;
                    F1 thickness = GIZMO_SHAFT_THICKNESS_PX*view->gizmo_world_per_pixel * (hot ? 1.45f : 1.0f);
                    M4F transform = line_transform_M4F(view->gizmo_pos, direction, thickness);
                    dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count, mesh->index_buffer, 0, mesh->index_count, transform, color, GFX_MESH_FEATURE__UNLIT);

                    //- kti: knob
                    hot = view->gizmo_hot_axis == axis && view->gizmo_hot_kind == GIZMO_KIND__SCALE;
                    active = view->gizmo_active_axis == axis && view->gizmo_active_kind == GIZMO_KIND__SCALE;
                    base = active ? 0.35f : hot ? 0.15f : 0.0f;
                    color = (F4){base, base, base, 1.0f};
                    color[axis] = active || hot ? 1.0f : 0.8f;
                    F1 size = GIZMO_SIZE_HANDLE_SIZE_PX*view->gizmo_world_per_pixel * (hot ? 1.25f : 1.0f);
                    F4 scale = (F4){size, size, size, 1.0f};
                    F4 pos = view->gizmo_pos;
                    pos[axis] += GIZMO_AXIS_LENGTH_PX*view->gizmo_world_per_pixel;
                    transform = mul_M4F(scale_M4F(scale), translate_M4F(pos));
                    dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count, mesh->index_buffer, 0, mesh->index_count, transform, color, GFX_MESH_FEATURE__UNLIT);
                  }

                  if (view->gizmo_rotation_visible) {
                    F1 radius = GIZMO_ROTATION_RADIUS_PX*view->gizmo_world_per_pixel;
                    for (Axis axis = AXIS__X; axis < AXIS3_COUNT; axis += 1) {
                      I1 hot = view->gizmo_hot_axis == axis && view->gizmo_hot_kind == GIZMO_KIND__ROTATE;
                      I1 active = view->gizmo_active_axis == axis && view->gizmo_active_kind == GIZMO_KIND__ROTATE;
                      F1 base = active ? 0.35f : hot ? 0.15f : 0.0f;
                      F4 color = {base, base, base, 1.0f};
                      color[axis] = active || hot ? 1.0f : 0.8f;
                      F1 thickness = 3.0f*view->gizmo_world_per_pixel*(hot ? 1.45f : 1.0f);

                      F4 basis_a = {0};
                      F4 basis_b = {0};
                      if (axis == AXIS__X) {
                        basis_a[AXIS__Y] = 1.0f;
                        basis_b[AXIS__Z] = 1.0f;
                      } else if (axis == AXIS__Y) {
                        basis_a[AXIS__Z] = 1.0f;
                        basis_b[AXIS__X] = 1.0f;
                      } else {
                        basis_a[AXIS__X] = 1.0f;
                        basis_b[AXIS__Y] = 1.0f;
                      }

                      for (L1 i = 0; i < GIZMO_ROTATION_SEGMENT_COUNT; i += 1) {
                        L1 next = (i + 1)%GIZMO_ROTATION_SEGMENT_COUNT;
                        if (view->gizmo_rotation_points_visible[axis][i] &&
                            view->gizmo_rotation_points_visible[axis][next]) {
                          F1 angle_a = 2.0f*PI*(F1)i/(F1)GIZMO_ROTATION_SEGMENT_COUNT;
                          F1 angle_b = 2.0f*PI*(F1)next/(F1)GIZMO_ROTATION_SEGMENT_COUNT;
                          F4 a = view->gizmo_pos + radius*(cos_F1(angle_a)*basis_a + sin_F1(angle_a)*basis_b);
                          F4 b = view->gizmo_pos + radius*(cos_F1(angle_b)*basis_a + sin_F1(angle_b)*basis_b);
                          M4F transform = line_transform_M4F(a, b-a, thickness);
                          dr_mesh(mesh->vertex_buffer, 0, mesh->vertex_count,
                                  mesh->index_buffer, 0, mesh->index_count,
                                  transform, color, GFX_MESH_FEATURE__UNLIT);
                        }
                      }
                    }
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

      postprocessing_dirty = !postprocessing_settings_match(postprocessing_settings_before_ui, state->postprocessing_settings);

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
            new->pos = (F4){0.0f, 0.3f, -3.0f};
            new->direction = normalize_F4(-new->pos);
            new->camera_vertical_fov = 70.0f*PI/180.0f;
            new->camera_aperture_radius = 0.0f;
            new->camera_focal_distance = 5.0f;
            entity_select(entity_handle(new), 0);
          } break;
          case CMD_KIND__DELETE_SELECTED_ENTITIES: {
            entity_delete_selected();
          } break;
          case CMD_KIND__RENDER: {
            // start a new render job
            if (state->active_render == 0) {
              // alloc and initialize job
              Arena *job_arena = arena_alloc(GiB(1));
              Render_Job *job = push_array(job_arena, Render_Job, 1);

              job->arena = job_arena;
              job->settings = state->render_settings;

              // build scene
              Entity *camera_entity = &state->nil_entity; 

              // find camera and count number of shapes
              L1 shape_count = 0;
              for (Entity *it = state->first_entity; !entity_is_nil(it); it = it->next) {
                if (it->flags & ENTITY_FLAG__CAMERA) {
                  camera_entity = it;
                }
                if (it->flags & ENTITY_FLAG__SHAPE) {
                  shape_count += 1;
                }
              }

              // build shapes and materials arrays
              L1 shape_idx = 0;
              Shape *shapes = push_array(job_arena, Shape, shape_count);
              RT_Material *materials = push_array(job_arena, RT_Material, shape_count);

              for (Entity *it = state->first_entity; !entity_is_nil(it); it = it->next) {
                if (it->flags & ENTITY_FLAG__SHAPE) {
                  shapes[shape_idx] = shape_from_entity(it);
                  materials[shape_idx] = it->material;
                  shape_idx += 1;
                }
              }

              // fill out scene
              job->scene = (RT_Scene){
                .rays_per_pixel = job->settings.rays_per_pixel,
                .max_num_bounces = job->settings.max_num_bounces,

                .camera = {
                  .pos = camera_entity->pos,  
                  .forward = camera_entity->direction,
                  .vertical_fov = camera_entity->camera_vertical_fov,
                  .aperture_radius = camera_entity->camera_aperture_radius,
                  .focal_distance = camera_entity->camera_focal_distance,
                },

                .shape_count = shape_count,
                .shapes = shapes,
                .materials = materials,
              };

              // start lanes
              Lane_Group_Params lane_params = {
                .count = Max(1, os_core_count()/2),

                .completed = &job->completed,

                .proc = render_lane,
                .user_data = job,

                .arena_size = GiB(1),
                .scratch_size = MiB(512),
              };
              lane_group_launch(lane_params);

              state->active_render = job;
            }
          } break;
          case CMD_KIND__CANCEL_RENDER: {
            if (state->active_render) {
              atomic_swap_I1(&state->active_render->cancel_requested, 1);
            }
          } break;
        }
      }
      state->cmd_count = 0;

      //- kti: End render job
      if (state->active_render && atomic_load_I1(&state->active_render->completed)) {
        Render_Job *job = state->active_render;
        state->active_render = 0;

        if (atomic_load_I1(&job->cancel_requested) || image_is_nil(job->hdr)) {
          arena_release(job->arena);
        } else {
          if (state->last_render != 0) {
            arena_release(state->last_render->arena);
          }

          state->last_render = job;
          postprocessing_dirty = 1;
        }
      }

      if (postprocessing_dirty) {
        editor_apply_postprocessing();
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
      while (os_clock() - frame_begin_time < target_frame_time) {}
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
    .count = os_core_count()/2,
    .proc = lane,

    .arena_size = GiB(1),
    .scratch_size = MiB(64),

    .lane_zero_on_caller = 1,
  };
  lane_group_launch(params);

  return 0;
}

#endif
