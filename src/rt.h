#ifndef RT_H
#define RT_H

typedef struct RT_Material RT_Material;
struct RT_Material
{
  F4 base_color;
  F1 metallic;
  F1 roughness;
  F4 emissive;
};

typedef struct RT_Camera RT_Camera;
struct RT_Camera
{
  F4 pos;
  F4 forward;
  F1 vertical_fov;
  F1 aperture_radius;
  F1 focal_distance;
};

typedef struct RT_Sphere4_Packet RT_Sphere4_Packet;
struct RT_Sphere4_Packet
{
  Sphere4 geometry;
  L1 material_indices[4];
};

typedef struct RT_Box4_Packet RT_Box4_Packet;
struct RT_Box4_Packet
{
  Box4 geometry;
  L1 material_indices[4];
};

typedef struct RT_Plane4_Packet RT_Plane4_Packet;
struct RT_Plane4_Packet
{
  Plane4 geometry;
  L1 material_indices[4];
};

typedef struct RT_Scene RT_Scene;
struct RT_Scene
{
  L1 rays_per_pixel;
  L1 max_num_bounces;

  RT_Camera camera;

  L1                 sphere_count;
  RT_Sphere4_Packet *spheres;

  L1              box_count;
  RT_Box4_Packet *boxes;

  L1                plane_count;
  RT_Plane4_Packet *planes;

  RT_Material *materials;
};

#endif
