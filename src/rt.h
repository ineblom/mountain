#ifndef RT_H
#define RT_H

typedef struct RT_Material RT_Material;
struct RT_Material {
  F4 base_color;
  F1 metallic;
  F1 roughness;
  F4 emissive;
};

typedef struct RT_Camera RT_Camera;
struct RT_Camera {
  F4 pos;
  F4 forward;
  F1 vertical_fov;
  F1 aperture_radius;
  F1 focal_distance;
};

typedef struct RT_Scene RT_Scene;
struct RT_Scene {
  L1 rays_per_pixel;
  L1 max_num_bounces;

  RT_Camera camera;

  L1 shape_count;
  Shape *shapes;
  RT_Material *materials;
};

#endif
