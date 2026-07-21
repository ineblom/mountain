// rt.c - Ray Tracer

// Coordinate system:
// +x right, +y up, +z forwards

#if (HEADER)

typedef struct RT_Material RT_Material;
struct RT_Material {
  F4 base_color;
  F1 metallic;
  F1 roughness;
  F4 emissive;
};

typedef struct RT_Sphere RT_Sphere;
struct RT_Sphere {
  F4 p;
  F1 r;
  RT_Material material;
};

typedef struct RT_Plane RT_Plane;
struct RT_Plane {
  F4 n;
  F1 d;
  RT_Material material;
};

typedef struct RT_Box RT_Box;
struct RT_Box {
  F4 min;
  F4 max;
  RT_Material material;
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
  Arena *arena;

  L1 width;
  L1 height;
  L1 rays_per_pixel;
  L1 max_num_bounces;

  RT_Camera camera;

  L1 plane_count;
  RT_Plane *planes;

  L1 sphere_count;
  RT_Sphere *spheres;

  L1 box_count;
  RT_Box *boxes;

  L1 tile_size;
  L1 tile_count;
  L1 next_tile_idx;
  L1 completed_tile_count;

  Image working_image;
  Image result;
};

#endif

#if (SOURCE)

Inline F4 sample_cosine_hemisphere(Random_State *rng, F4 n) {
  F1 x, y;
  do {
    x = 2.0f * random_unilateral(rng) - 1.0f;
    y = 2.0f * random_unilateral(rng) - 1.0f;
  } while (x*x + y*y > 1.0f);

  F1 z = sqrtf(1.0f - x*x - y*y);

  // build ONB around n
  F4 tangent = (abs_F1(n[0]) > 0.9f) ? (F4){0,1,0} : (F4){1,0,0};
  tangent = normalize_F4(cross_F4(tangent, n));
  F4 bitangent = cross_F4(n, tangent);

  // local -> world
  return normalize_F4(
    tangent * x +
    bitangent * y +
    n * z
  );
}

Inline F4 fresnel_schlick(F1 cos_theta, F4 F0) {
  return F0 + ((F4){1,1,1} - F0) * powf(1.0f - cos_theta, 5.0f);
}

Inline F1 D_GGX(F1 NoH, F1 alpha) {
  F1 a2 = alpha * alpha;
  F1 denom = (NoH*NoH)*(a2 - 1.0f) + 1.0f;
  return a2 / (PI*denom*denom);
}

Inline F1 G1_GGX(F1 NoX, F1 alpha) {
  NoX = saturate_F1(NoX);
  F1 a2 = alpha * alpha;
  F1 denom = NoX + sqrtf(a2 + (1.0f - a2) * NoX * NoX);
  return (2.0f * NoX) / Max(denom, 1e-6f);
}

Inline F1 G_Smith(F1 NoV, F1 NoL, F1 alpha) {
  return G1_GGX(NoV, alpha) * G1_GGX(NoL, alpha);
}

Inline F4 sample_GGX_half(Random_State *rng, F4 n, float alpha) {
  F1 r1 = random_unilateral(rng);
  F1 r2 = random_unilateral(rng);

  F1 phi = 2.0f * PI * r1;
  F1 cosTheta = sqrtf((1.0f - r2) / (1.0f + (alpha*alpha - 1.0f) * r2));
  F1 sinTheta = sqrtf(Max(0.0f, 1.0f - cosTheta*cosTheta));

  F1 x = sinTheta * cosf(phi);
  F1 y = sinTheta * sinf(phi);
  F1 z = cosTheta;

  // build ONB
  F4 tangent = (abs_F1(n[0]) > 0.9f) ? (F4){0,1,0} : (F4){1,0,0};
  tangent = normalize_F4(cross_F4(tangent, n));
  F4 bitangent = cross_F4(n, tangent);

  // local -> world
  return normalize_F4(
    tangent * x +
    bitangent * y +
    n * z
  );
}

Inline F1 pdf_GGX(F4 n, F4 h, F4 v, F1 alpha) {
  F1 NoH = saturate_F1(dot_F4(n,h));
  F1 VoH = saturate_F1(dot_F4(v,h));
  F1 D = D_GGX(NoH, alpha);
  F1 result = D * NoH / (4.0f * VoH);
  return result;
}

Internal F4 ray_cast(RT_Scene *scene, Random_State *rng, F4 ray_origin, F4 ray_direction) {
  F1 min_hit_distance = 0.001f;
  F1 tolerance = 0.0001f;

  F4 result = {0};
  F4 attenuation = {1, 1, 1};
  for (L1 ray_index = 0; ray_index < scene->max_num_bounces; ray_index += 1) {
    F1 hit_distance = F1_MAX;

    RT_Material const *hit_material = 0;
    F4 next_origin;
    F4 next_normal;

    // Check planes
    for (L1 plane_index = 0; plane_index < scene->plane_count; plane_index += 1) {
      RT_Plane *plane = &scene->planes[plane_index];

      F1 denom = dot_F4(plane->n, ray_direction);
      if (denom < -tolerance || denom > tolerance) {
        F1 t = (-plane->d - dot_F4(plane->n, ray_origin)) / denom;
        if (t > min_hit_distance && t < hit_distance) {
          hit_distance = t;
          hit_material = &plane->material;

          next_origin = ray_origin + t*ray_direction;
          next_normal = plane->n;
        }
      }
    }

    // Check spheres
    for (L1 sphere_index = 0; sphere_index < scene->sphere_count; sphere_index += 1) {
      RT_Sphere *sphere = &scene->spheres[sphere_index];

      F1 t = ray_sphere_intersect(ray_origin, ray_direction, sphere->p, sphere->r);
      if (t > min_hit_distance && t < hit_distance) {
        hit_distance = t;
        hit_material = &sphere->material;

        next_origin = ray_origin + t*ray_direction;
        next_normal = normalize_F4(next_origin - sphere->p);
      }
    }

    // Check boxes
    for (L1 box_index = 0; box_index < scene->box_count; box_index += 1) {
      RT_Box *box = &scene->boxes[box_index];

      F1 t_min = ray_aabb_intersect(ray_origin, ray_direction, box->min, box->max);

      if (t_min > min_hit_distance && t_min < hit_distance) {
        next_origin = ray_origin + t_min * ray_direction;
        F4 center = (box->min + box->max) * 0.5f;
        F4 local_hit = next_origin - center;
        F4 box_size = box->max - box->min;

        F4 d = abs_F4(local_hit) / (box_size * 0.5f);
        if (d[0] > d[1] && d[0] > d[2]) next_normal = sign_F1(local_hit[0]) * (F4){1,0,0};
        else if (d[1] > d[2]) next_normal = sign_F1(local_hit[1]) * (F4){0,1,0};
        else next_normal = sign_F1(local_hit[2]) * (F4){0,0,1};

        hit_distance = t_min;
        hit_material = &box->material;
      }

    }

    if (hit_material != 0) {
      RT_Material mat = *hit_material;

      result += attenuation * mat.emissive;

      F1 roughness = Max(0.05f, clamp01_F1(mat.roughness));
      F1 alpha = roughness * roughness;
      F1 metallic = clamp01_F1(mat.metallic);

      F4 f0 = lerp_F4((F4){0.04f,0.04f,0.04f}, metallic, mat.base_color);

      F1 specular_prob = 0.5f; // keep constant; MIS handles weighting
      if (random_unilateral(rng) < specular_prob) {
        F4 v = -ray_direction;
        F4 h = sample_GGX_half(rng, next_normal, alpha);
        F4 l = reflect_F4(-v, h);

        F1 NoL = saturate_F1(dot_F4(next_normal, l));
        F1 NoV = saturate_F1(dot_F4(next_normal, v));
        F1 NoH = saturate_F1(dot_F4(next_normal, h));
        F1 VoH = saturate_F1(dot_F4(v, h));

        if (NoL > 0.0f) {
          F1 D = D_GGX(NoH, alpha);
          F1 G = G_Smith(NoV, NoL, alpha);
          F4 F = fresnel_schlick(VoH, f0);

          F1 denom = Max(4.0f * NoV * NoL, 1e-6f);
          F4 f_spec = (F * (D * G)) / denom;

          // Fresnel-weighted diffuse with metallic energy conservation
          F4 kd_color = (1.0f - metallic) * (((F4){1,1,1}) - F);
          F4 f_diff = (kd_color * mat.base_color) / PI;

          F1 pdf_spec = pdf_GGX(next_normal, h, v, alpha);
          F1 pdf_diff = NoL / PI;
          F1 pdf_total = specular_prob * pdf_spec + (1.0f - specular_prob) * pdf_diff;
          F1 inv_pdf = 1.0f / Max(pdf_total, 1e-6f);

          attenuation *= (f_spec + f_diff) * (NoL * inv_pdf);
          ray_direction = l;
        } else {
          break; // absorb
        }
      } else {
        F4 v = -ray_direction;
        F4 l = sample_cosine_hemisphere(rng, next_normal);
        F1 NoL = saturate_F1(dot_F4(next_normal, l));
        if (NoL > 0.0f) {
          F4 h = normalize_F4(v + l);
          F1 NoV = saturate_F1(dot_F4(next_normal, v));
          F1 NoH = saturate_F1(dot_F4(next_normal, h));
          F1 VoH = saturate_F1(dot_F4(v, h));

          F1 D = D_GGX(NoH, alpha);
          F1 G = G_Smith(NoV, NoL, alpha);
          F4 F = fresnel_schlick(VoH, f0);

          F1 denom = Max(4.0f * NoV * NoL, 1e-6f);
          F4 f_spec = (F * (D * G)) / denom;

          F4 kd_color = (1.0f - metallic) * (((F4){1,1,1}) - F);
          F4 f_diff = (kd_color * mat.base_color) / PI;

          F1 pdf_spec = pdf_GGX(next_normal, h, v, alpha);
          F1 pdf_diff = NoL / PI;
          F1 pdf_total = specular_prob * pdf_spec + (1.0f - specular_prob) * pdf_diff;
          F1 inv_pdf = 1.0f / Max(pdf_total, 1e-6f);

          attenuation *= (f_spec + f_diff) * (NoL * inv_pdf);
          ray_direction = l;
        } else {
          break; // absorb
        }
      }

      ray_origin = next_origin + next_normal * min_hit_distance;
    } else {
      // F1 height = (ray_direction.y + 1) * 0.5;
      // F4 sky_color = lerp_F4((F4){1.0f, 1.0f, 1.0f}, height, (F4){0.2f, 0.4f, 1.0f});
      // result += attenuation * sky_color;
      break;
    }
  }

  return result;
}

Internal void rt_trace_scene(RT_Scene *scene) {
  scene->tile_size = 8;
  L1 tile_cols = (scene->width  + scene->tile_size - 1) / scene->tile_size;
  L1 tile_rows = (scene->height + scene->tile_size - 1) / scene->tile_size;

  if (lane_idx() == 0 && scene->working_image.pixels == 0) {
    scene->working_image = image_alloc(scene->arena, scene->width, scene->height, sizeof(F4));

    scene->tile_count = tile_cols*tile_rows;
    scene->next_tile_idx = 0;
    scene->completed_tile_count = 0;
  }
  lane_sync();

  F4 camera_p = scene->camera.pos;
  F4 camera_forward = normalize_F4(scene->camera.forward);
  F4 world_up = {0, 1, 0};
  F4 camera_right = normalize_F4(cross_F4(world_up, camera_forward));
  F4 camera_up = normalize_F4(cross_F4(camera_forward, camera_right));

  F1 film_dist   = 1.0f;
  F1 aspect      = (F1)scene->width/(F1)scene->height;
  F1 half_film_h = tan_F1(scene->camera.vertical_fov*0.5f)*film_dist;
  F1 half_film_w = aspect*half_film_h;
  F4 film_center = camera_p + film_dist*camera_forward;

  F1 half_pixel_w = 0.5f/(F1)scene->width;
  F1 half_pixel_h = 0.5f/(F1)scene->height;

  L1 tile_idx = atomic_add_L1(&scene->next_tile_idx, 1);
  if (tile_idx < scene->tile_count) {
    Random_State rng = {
      .state = tile_idx*7^~((tile_idx-420)*11),
      .inc = 2*tile_idx + 1,
    };
    L1 tile_x = tile_idx%tile_cols;
    L1 tile_y = tile_idx/tile_cols;

    L1 x0 = tile_x*scene->tile_size;
    L1 y0 = tile_y*scene->tile_size;
    L1 x1 = Min(x0+scene->tile_size, scene->width);
    L1 y1 = Min(y0+scene->tile_size, scene->height);

    F4 *pixels = (F4 *)scene->working_image.pixels;

    for (L1 y = y0; y < y1; y += 1) {
      for (L1 x = x0; x < x1; x += 1) {
        F1 film_x = -1 + 2 * (F1)x/(F1)scene->width;
        F1 film_y = -1 + 2 * (F1)y/(F1)scene->height;

        F4 color = {0};
        F1 contrib = 1.0f / (F1)scene->rays_per_pixel;

        for (L1 ray_index = 0; ray_index < scene->rays_per_pixel; ray_index += 1) {
          F1 off_x  = film_x + random_bilateral(&rng)*half_pixel_w;
          F1 off_y  = film_y + random_bilateral(&rng)*half_pixel_h;
          F4 film_p = film_center + off_x*half_film_w*camera_right + off_y*half_film_h*camera_up;

          F1 r = scene->camera.aperture_radius * sqrt_F1(random_unilateral(&rng));
          F1 theta = 2.0f * PI * random_unilateral(&rng);
          F4 aperture_offset = r * cos_F1(theta) * camera_right + r * sin_F1(theta) * camera_up;
          F4 ray_origin = camera_p + aperture_offset;

          F4 focus_point = camera_p + scene->camera.focal_distance  * normalize_F4(film_p - camera_p);
          F4 ray_direction = normalize_F4(focus_point - ray_origin);

          color += ray_cast(scene, &rng, ray_origin, ray_direction) * contrib;
        }

        pixels[x + y*scene->width] = color;
      }
    }

    L1 completed_before = atomic_add_L1(&scene->completed_tile_count, 1);
    if (completed_before + 1 == scene->tile_count) {
      scene->result = scene->working_image;
    }
  }
}
#endif
