#ifndef EDITOR_H
#define EDITOR_H

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

////////////////////////////////
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

  CMD_KIND__USER_CODE_RELOAD,

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
//~ kti: User Code

typedef void (*User_Render_Func)(void);

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

  //- kti: User Code
  User_Render_Func user_render_func;
};

#endif
