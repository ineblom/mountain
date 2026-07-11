// rt.c - Ray Tracer

// Coordinate system:
// +x right, +y forwards, +z up

// TODO:
// Shadow rays (higher quality w/ lower ray counts)
// Textures
// Quads
// SIMD
// BVH (for larger scenes)

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
  I1 material_idx;
};

typedef struct RT_Plane RT_Plane;
struct RT_Plane {
  F4 n;
  F1 d;
  I1 material_idx;
};

typedef struct RT_Box RT_Box;
struct RT_Box {
  F4 min;
  F4 max;
  I1 material_idx;
};

typedef struct RT_Camera RT_Camera;
struct RT_Camera {
  F4 pos;
  F4 forward;
  F1 aperture_radius;
  F1 focal_distance;
};

typedef struct RT_Scene RT_Scene;
struct RT_Scene {
  String8 output_filename;
  L1 output_width;
  L1 output_height;
  L1 rays_per_pixel;

  L1 max_num_bounces;

  L1 bloom_pass_count;
  F1 bloom_threshold;
  F1 bloom_strength;
  F1 bloom_knee;
  String8 bloom_overlay_filename;
  F1 bloom_overlay_strength;

  RT_Camera camera;

  L1 material_count;
  RT_Material *materials;

  L1 plane_count;
  RT_Plane *planes;

  L1 sphere_count;
  RT_Sphere *spheres;

  L1 box_count;
  RT_Box *boxes;
};

typedef struct RT_State RT_State;
struct RT_State {
  L1 complete_sections_count;
  L1 section_count;
  L1 current_section_idx;

  Image bloom_overlay_image;
  Image ray_image;
  Image final_image;

  L1 total_memory_usage;
};

#endif

#if (SOURCE)

ThreadLocal Random_State *rng;
Global RT_State *rt_state;

Inline F4 sample_cosine_hemisphere(F4 n) {
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

Inline F4 sample_GGX_half(F4 n, float alpha) {
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

Internal F4 ray_cast(RT_Scene scene, F4 ray_origin, F4 ray_direction) {
  F1 min_hit_distance = 0.001f;
  F1 tolerance = 0.0001f;

  F4 result = {0};
  F4 attenuation = {1, 1, 1};
  for (L1 ray_index = 0; ray_index < scene.max_num_bounces; ray_index += 1) {
    F1 hit_distance = F1_MAX;

    I1 hit_material_idx = -1;
    F4 next_origin;
    F4 next_normal;

    // Check planes
    for (L1 plane_index = 0; plane_index < scene.plane_count; plane_index += 1) {
      RT_Plane plane = scene.planes[plane_index];

      F1 denom = dot_F4(plane.n, ray_direction);
      if (denom < -tolerance || denom > tolerance) {
        F1 t = (-plane.d - dot_F4(plane.n, ray_origin)) / denom;
        if (t > min_hit_distance && t < hit_distance) {
          hit_distance = t;
          hit_material_idx = plane.material_idx;

          next_origin = ray_origin + t*ray_direction;
          next_normal = plane.n;
        }
      }
    }

    // Check spheres
    for (L1 sphere_index = 0; sphere_index < scene.sphere_count; sphere_index += 1) {
      RT_Sphere sphere = scene.spheres[sphere_index];

      F4 sphere_relative_ray_origin = ray_origin - sphere.p;
      F1 b = 2.0f * dot_F4(ray_direction, sphere_relative_ray_origin);
      F1 c = dot_F4(sphere_relative_ray_origin, sphere_relative_ray_origin) - sphere.r*sphere.r;

      F1 root_term = sqrtf(b*b - 4.0f*c);
      if (root_term > tolerance) {
        F1 tp = (-b + root_term) / 2.0f;
        F1 tn = (-b - root_term) / 2.0f;

        F1 t = tp;
        if (tn > min_hit_distance && tn < tp) {
          t = tn;
        }

        if (t > min_hit_distance && t < hit_distance) {
          hit_distance = t;
          hit_material_idx = sphere.material_idx;

          next_origin = ray_origin + t*ray_direction;
          next_normal = normalize_F4(next_origin - sphere.p);
        }
      }
    }

    // Check boxes
    for (L1 box_index = 0; box_index < scene.box_count; box_index += 1) {
      RT_Box box = scene.boxes[box_index];

      F1 t_min = (box.min[0] - ray_origin[0]) / ray_direction[0];
      F1 t_max = (box.max[0] - ray_origin[0]) / ray_direction[0];
      if (t_min > t_max) Swap(t_min, t_max);

      F1 ty_min = (box.min[1] - ray_origin[1]) / ray_direction[1];
      F1 ty_max = (box.max[1] - ray_origin[1]) / ray_direction[1];
      if (ty_min > ty_max) Swap(ty_min, ty_max);

      t_min = Max(t_min, ty_min);
      t_max = Min(t_max, ty_max);

      if (t_min > t_max) continue;

      F1 tz_min = (box.min[2] - ray_origin[2]) / ray_direction[2];
      F1 tz_max = (box.max[2] - ray_origin[2]) / ray_direction[2];
      if (tz_min > tz_max) Swap(tz_min, tz_max);

      t_min = Max(t_min, tz_min);
      t_max = Min(t_max, tz_max);

      if (t_min > t_max) continue;

      if (t_min > min_hit_distance && t_min < hit_distance) {
        next_origin = ray_origin + t_min * ray_direction;
        F4 center = (box.min + box.max) * 0.5f;
        F4 local_hit = next_origin - center;
        F4 box_size = box.max - box.min;

        F4 d = abs_F4(local_hit) / (box_size * 0.5f);
        if (d[0] > d[1] && d[0] > d[2]) next_normal = sign_F1(local_hit[0]) * (F4){1,0,0};
        else if (d[1] > d[2]) next_normal = sign_F1(local_hit[1]) * (F4){0,1,0};
        else next_normal = sign_F1(local_hit[2]) * (F4){0,0,1};

        hit_distance = t_min;
        hit_material_idx = box.material_idx;
      }

    }

    if (hit_material_idx != (I1)(-1)) {
      RT_Material mat = scene.materials[hit_material_idx];

      result += attenuation * mat.emissive;

      F1 roughness = Max(0.05f, clamp01_F1(mat.roughness));
      F1 alpha = roughness * roughness;
      F1 metallic = clamp01_F1(mat.metallic);

      F4 f0 = lerp_F4((F4){0.04f,0.04f,0.04f}, metallic, mat.base_color);

      F1 specular_prob = 0.5f; // keep constant; MIS handles weighting
      if (random_unilateral(rng) < specular_prob) {
        F4 v = -ray_direction;
        F4 h = sample_GGX_half(next_normal, alpha);
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
        F4 l = sample_cosine_hemisphere(next_normal);
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
      // F1 height = (ray_direction.z + 1) * 0.5;
      // F4 sky_color = lerp_F4((F4){1.0f, 1.0f, 1.0f}, height, (F4){0.2f, 0.4f, 1.0f});
      // result += attenuation * sky_color;
      break;
    }
  }

  return result;
}

Inline F1 linear_to_srgb(F1 l) {
  if (l < 0.0f) l = 0.0f;
  if (l > 1.0f) l = 1.0f;

  F1 s = l * 12.92f;
  if (l > 0.0031308f) {
    s = 1.055f*powf(l, 1.0f/2.4f) - 0.055f;
  }

  return s;
}

Inline F4 tonemap_aces(F4 v) {
  const F1 a = 2.51f;
  const F1 b = 0.03f;
  const F1 c = 2.43f;
  const F1 d = 0.59f;
  const F1 e = 0.14f;
  F4 result = clamp01_F4((v * (a * v + b)) / (v * (c * v + d) + e));
  return result;
}

Inline F4 tonemap_reinhard(F4 v) {
  F4 result = v / (1.0f + v);
  return result;
}

Inline F4 tonemap_lottes(F4 v) {
  // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
  const F1 a = 1.6f;
  const F1 d = 0.977f;
  const F1 hdr_max = 8.0f;
  const F1 mid_in = 0.18f;
  const F1 mid_out = 0.267f;

  // TODO: Precompute
  const F1 b =
      (-powf(mid_in, a) + powf(hdr_max, a) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);
  const float c =
      (powf(hdr_max, a * d) * powf(mid_in, a) - powf(hdr_max, a) * powf(mid_in, a * d) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);

  F4 result = pow_F4(v, a) / (pow_F4(v, a * d) * b + c);
  return result;
}

#define tonemap tonemap_lottes

Internal void rt_trace_scene(Arena *arena, RT_Scene scene) {
  L1 start_time = os_clock();

  if (lane_idx() == 0) {
    rt_state = push_array(arena, RT_State, 1);
    
    rt_state->ray_image = image_alloc(arena, scene.output_width, scene.output_height, sizeof(F4));

    rt_state->section_count = lane_count() * 8;
    rt_state->current_section_idx = 0;
    rt_state->complete_sections_count = 0;
    rt_state->total_memory_usage = 0;

    rt_state->bloom_overlay_image = image_read_from_file(arena, scene.bloom_overlay_filename);
  }

  lane_sync();

  F4 camera_p = scene.camera.pos;
  F4 camera_z = scene.camera.forward;
  F4 camera_x = normalize_F4(cross_F4((F4){0, 0, 1}, camera_z));
  F4 camera_y = normalize_F4(cross_F4(camera_z, camera_x));

  F1 film_dist   = 1.0f;
  F1 film_w      = (F1)scene.output_width/(F1)scene.output_height;
  F1 film_h      = 1.0f;
  F1 half_film_w = film_w*0.5f;
  F1 half_film_h = film_h*0.5f;
  F4 film_center = camera_p - film_dist * camera_z;

  F1 half_pixel_w = 0.5f/(F1)scene.output_width;
  F1 half_pixel_h = 0.5f/(F1)scene.output_height;

  L1 lane_begin_time = os_clock();

  L1 section_pixel_count = scene.output_width*scene.output_height/rt_state->section_count;
  F4 *ray_pixels = push_array(arena, F4, section_pixel_count);

  while (rt_state->current_section_idx < rt_state->section_count) {
    L1 section_idx = atomic_add_L1(&rt_state->current_section_idx, 1);
    if (section_idx >= rt_state->section_count) break;
    Range pixels_range = {
      .min = section_pixel_count * section_idx,
      .max = section_pixel_count * (section_idx + 1)
    };

    F4 *out = ray_pixels;
    for (L1 pixel_index = pixels_range.min; pixel_index < pixels_range.max; pixel_index += 1) {
      I1 x = pixel_index%scene.output_width;
      I1 y = pixel_index/scene.output_width;
      F1 film_y = -1 + 2 * (F1)y/(F1)scene.output_height;
      F1 film_x = -1 + 2 * (F1)x/(F1)scene.output_width;

      F4 color = {};
      F1 contrib = 1.0f / (F1)scene.rays_per_pixel;
      for (L1 ray_index = 0; ray_index < scene.rays_per_pixel; ray_index += 1) {
        F1 off_x  = film_x + random_bilateral(rng)*half_pixel_w;
        F1 off_y  = film_y + random_bilateral(rng)*half_pixel_h;
        F4 film_p = film_center + off_x*half_film_w*camera_x + off_y*half_film_h*camera_y;

        F1 r = scene.camera.aperture_radius * sqrtf(random_unilateral(rng));
        F1 theta = 2.0f * PI * random_unilateral(rng);
        F4 aperture_offset = r * cosf(theta) * camera_x + r * sinf(theta) * camera_y;
        F4 ray_origin = camera_p + aperture_offset;

        F4 focus_point = camera_p + scene.camera.focal_distance  * normalize_F4(film_p - camera_p);
        F4 ray_direction = normalize_F4(focus_point - ray_origin);

        color += ray_cast(scene, ray_origin, ray_direction) * contrib;
      }

      out[0] = color;
      out += 1;

    }

    B1 *dest = rt_state->ray_image.pixels + sizeof(F4)*pixels_range.min;
    memmove(dest, ray_pixels, sizeof(F4)*section_pixel_count);

    atomic_add_L1(&rt_state->complete_sections_count, 1);

    L1 now         = os_clock();
    L1 duration_ms = (now - lane_begin_time)/1000000;
    F1 duration_s  = (F1)duration_ms/1000.0f;
    F1 percent     = (F1)rt_state->complete_sections_count / (F1)rt_state->section_count * 100.0f;
    printf("\r[%.2fs]: %.2f%%", duration_s, percent);
    fflush(stdout);
  }

  lane_sync();

  if (lane_idx() == 0) {
    printf("\nRay tracing complete. Postprocessing...\n");

    // TODO: Multithread

    Image *bloom_passes = push_array(arena, Image, scene.bloom_pass_count);

    bloom_passes[0] = image_alloc(arena, scene.output_width, scene.output_height, sizeof(F4));

    // Filter on bright pixels
    F4 *in = (F4 *)rt_state->ray_image.pixels;
    F4 *out = (F4 *)bloom_passes[0].pixels;
    for (L1 pixel_index = 0; pixel_index < bloom_passes[0].width*bloom_passes[0].height; pixel_index += 1) {
      F4 color = in[0];

      F1 luminance = luminance_F4(color);

      F1 soft_threshold = scene.bloom_threshold - scene.bloom_knee;
      F1 knee_range = 2.0f*scene.bloom_knee;
      F1 contrib = 0.0f;

      if (luminance > scene.bloom_threshold + scene.bloom_knee) {
        contrib = 1.0f;
      } else if (luminance > soft_threshold) {
        F1 x = luminance - soft_threshold;
        F1 knee_contrib = (x*x) / (4.0f*knee_range*knee_range);
        contrib = knee_contrib;
      }

      if (contrib > 0.0f && luminance > 1e-6f) {
        F1 excess_luminance = Max(0.0f, luminance - scene.bloom_threshold);
        color = color * (excess_luminance/luminance)*contrib;
      } else {
        color = (F4){0};
      }

      out[0] = color;

      in  += 1;
      out += 1;
    }

    // Downsample
    for (L1 pass_index = 0; pass_index < scene.bloom_pass_count-1; pass_index += 1) {
      Image in = bloom_passes[pass_index];
      Image out = image_alloc(arena, in.width/2, in.height/2, sizeof(F4));

      bloom_passes[pass_index+1] = out;

      for (L1 y = 0; y < out.height; y += 1) {
        L1 sy = y*2;
        for (L1 x = 0; x < out.width; x += 1) {
          L1 sx = x*2;

          // TODO: look into 13-tap bilinear tent filter.

          F4 sum = {0};
          F4 *in_p = (F4 *)in.pixels;

          // Center (4)
          sum += in_p[image_xy_to_index(in,sx+0,sy+0)] * 4.0f;
          sum += in_p[image_xy_to_index(in,sx+1,sy+0)] * 4.0f;
          sum += in_p[image_xy_to_index(in,sx+1,sy+1)] * 4.0f;
          sum += in_p[image_xy_to_index(in,sx+0,sy+1)] * 4.0f;

          // Edges (2)
          sum += in_p[image_xy_to_index(in,sx-1,sy+0)] * 2.0f;
          sum += in_p[image_xy_to_index(in,sx-1,sy+1)] * 2.0f;

          sum += in_p[image_xy_to_index(in,sx+2,sy+0)] * 2.0f;
          sum += in_p[image_xy_to_index(in,sx+2,sy+1)] * 2.0f;

          sum += in_p[image_xy_to_index(in,sx+0,sy-1)] * 2.0f;
          sum += in_p[image_xy_to_index(in,sx+1,sy-1)] * 2.0f;

          sum += in_p[image_xy_to_index(in,sx+0,sy+2)] * 2.0f;
          sum += in_p[image_xy_to_index(in,sx+1,sy+2)] * 2.0f;

          // Corners (1)
          sum += in_p[image_xy_to_index(in,sx-1,sy-1)] * 1.0f;
          sum += in_p[image_xy_to_index(in,sx+2,sy-1)] * 1.0f;
          sum += in_p[image_xy_to_index(in,sx-1,sy+2)] * 1.0f;
          sum += in_p[image_xy_to_index(in,sx+2,sy+2)] * 1.0f;

          sum /= 36.0f;
          F4 karis_average = sum / (1.0f + luminance_F4(sum));

          ((F4 *)out.pixels)[x + y*out.width] = karis_average;
        }
      }
    }

    // Upsample and sum
    for (L1 pass_index = scene.bloom_pass_count-1; pass_index >= 1; pass_index -= 1) {
      Image in = bloom_passes[pass_index];
      Image out = bloom_passes[pass_index-1];

      for (L1 y = 0; y < out.height; y += 1) {
        L1 sy = y/2;
        for (L1 x = 0; x < out.width; x += 1) {
          L1 sx = x/2;

          F4 sum = {0};
          F4 *in_p = (F4 *)in.pixels;

          // center
          sum += in_p[image_xy_to_index(in,sx,sy)]*4.0f;

          // edges
          sum += in_p[image_xy_to_index(in,sx-1,sy)]*2.0f;
          sum += in_p[image_xy_to_index(in,sx+1,sy)]*2.0f;
          sum += in_p[image_xy_to_index(in,sx,sy-1)]*2.0f;
          sum += in_p[image_xy_to_index(in,sx,sy+1)]*2.0f;

          // corners
          sum += in_p[image_xy_to_index(in,sx-1,sy-1)]*1.0f;
          sum += in_p[image_xy_to_index(in,sx+1,sy-1)]*1.0f;
          sum += in_p[image_xy_to_index(in,sx-1,sy+1)]*1.0f;
          sum += in_p[image_xy_to_index(in,sx+1,sy+1)]*1.0f;

          sum /= 16.0f;

          ((F4 *)out.pixels)[x + y*out.width] += sum;
        }
      }
    }

    rt_state->final_image = image_alloc(arena, scene.output_width, scene.output_height, sizeof(I1));

    // Composit & Tonemap
    F4 *in_ray    = (F4 *)rt_state->ray_image.pixels;
    F4 *in_bloom  = (F4 *)bloom_passes[0].pixels;
    I1 *final_out = (I1 *)rt_state->final_image.pixels;
    for (L1 y = 0; y < scene.output_height; y += 1) {
      for (L1 x = 0; x < scene.output_width; x += 1) {
        F1 u = (F1)x / (F1)scene.output_width;
        F1 v = (F1)y / (F1)scene.output_height;

        F4 bloom_overlay = image_sample_bilinear_I1_to_F4(rt_state->bloom_overlay_image, u, v);
        F4 ray           = in_ray[0];
        F4 bloom         = in_bloom[0];

        bloom *= 1.0f + luminance_F4(bloom_overlay)*scene.bloom_overlay_strength;

        F4 color = ray * (1.0f - 0.5f*scene.bloom_strength) + scene.bloom_strength*bloom;

        // color += dirt_color * luminance_F4(bloom) * 0.5f;

        F4 tonemapped_color = tonemap(color);
        F4 bmp_color = {
          255.0f*linear_to_srgb(tonemapped_color[0]),
          255.0f*linear_to_srgb(tonemapped_color[1]),
          255.0f*linear_to_srgb(tonemapped_color[2]),
          255.0f
        };

        final_out[0] = 0xFF000000 |
                          (I1)bmp_color[0] << 16 |
                          (I1)bmp_color[1] << 8 |
                          (I1)bmp_color[2] << 0;

        in_ray    += 1;
        in_bloom  += 1;
        final_out += 1;
      }
    }
  }

  L1 memory_usage = arena_pos(arena);
  atomic_add_L1(&rt_state->total_memory_usage, memory_usage);

  if (lane_idx() == 0) {
    image_write_to_file(rt_state->final_image, scene.output_filename);

    L1 end_time    = os_clock();
    L1 duration_ms = (end_time - start_time)/1000000;
    F1 duration_s  = (F1)duration_ms/1000.0f;

    L1 kib_used = rt_state->total_memory_usage / KiB(1);
    L1 mib_used = rt_state->total_memory_usage / MiB(1);

    printf("[%.2fs] Done, Used %llu KiB (%llu MiB)\n", duration_s, kib_used, mib_used);
  }
}

#ifdef RT_APP
Internal void lane(Arena *arena) {
  RT_Material materials[] = {
    { // ground
      .base_color  = (F4){0.63f, 0.53f, 0.13f},
      .metallic = 0.3f,
      .roughness = 0.5f,
    },
    { // left
      .base_color  = (F4){1.0f, 1.0f, 1.0f},
      .metallic = 1.0f,
      .roughness = 0.40f,
    },
    { // right
      .base_color  = (F4){0.1f, 1.0f, 1.0f},
      .metallic = 0.0f,
      .roughness = 0.10f,
    },
    { // pink glow
      .base_color  = (F4){1.0f, 0.2f, 1.0f},
      .emissive = (F4){2.0f, 0.6f, 2.0f},
      .metallic = 0.1f,
      .roughness = 1.00f,
    },
    { // blue glow
      .base_color  = (F4){1.0f, 1.0f, 1.0f},
      .emissive = (F4){1.0f, 1.0f, 6.0f},
    },
  };

  RT_Plane planes[] = {
    {
      .n = (F4){0, 0, 1},
      .d = 1.0f,
      .material_idx = 0
    }
  };

  RT_Sphere spheres[] = {
    {
      .p = (F4){-2.0f, 2.0f, 0.0f},
      .r = 1.0f,
      .material_idx = 1,
    },
    {
      .p = (F4){2.0f, 0.0f, 0.0f},
      .r = 1.0f,
      .material_idx = 2,
    },
    {
      .p = (F4){0.0f, 20.0f, 0.0f},
      .r = 1.0f,
      .material_idx = 3,
    },
  };

  RT_Box boxes[] = {
    {
      .min = {-2.6f, -2.0f,-1.0},
      .max = {-2.5f,  0.7f,-0.5},
      .material_idx = 4,
    }
  };

  RT_Camera camera = {0};
  camera.pos = (F4){0.0f, -5.0f, 0.5f};
  camera.forward = normalize_F4(camera.pos);
  camera.aperture_radius =  0.02f;
  camera.focal_distance = 5.0f;

  RT_Scene scene = {
    .output_filename = str8("output.bmp"),
    .output_width    = 1280,
    .output_height   = 720,

    .rays_per_pixel  = 128,
    .max_num_bounces = 8,

    .bloom_pass_count       = 8,
    .bloom_threshold        = 0.5f,
    .bloom_strength         = 0.4f,
    .bloom_knee             = 0.5f,
    .bloom_overlay_filename = str8("lens-dirt.bmp"),
    .bloom_overlay_strength = 2.0f,

    .camera = camera,

    .materials = materials,
    .material_count = ArrayCount(materials),

    .spheres = spheres,
    .sphere_count = ArrayCount(spheres),

    .planes = planes,
    .plane_count = ArrayCount(planes),

    .boxes = boxes,
    .box_count = ArrayCount(boxes),
  };

  trace_scene(arena, scene);
}
#endif

#endif
