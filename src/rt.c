// rt.c - Ray Tracer

// Coordinate system:
// +x right, +y up, +z forwards

Inline F3 sample_cosine_hemisphere(Random_State *rng, F3 n)
{
  F1 x, y;
  do
  {
    x  =  2.0f * random_unilateral(rng) - 1.0f;
    y  =  2.0f * random_unilateral(rng) - 1.0f;
  } while (x * x + y * y > 1.0f);

  F1 z        =  sqrtf(1.0f - x * x - y * y);
  F3 tangent  =  (abs_F1(n.x) > 0.9f) ? f3(0, 1, 0) : f3(1, 0, 0);

  tangent = normalize_F3(cross_F3(tangent, n));

  F3 bitangent = cross_F3(n, tangent);

  // local -> world
  F3 local_xy = add_F3(scale_F3(tangent, x), scale_F3(bitangent, y));
  return normalize_F3(add_scaled_F3(local_xy, z, n));
}

Inline F4 fresnel_schlick(F1 cos_theta, F4 F0)
{
  return F0 + ((F4){1, 1, 1} - F0) * powf(1.0f - cos_theta, 5.0f);
}

Inline F1 D_GGX(F1 NoH, F1 alpha)
{
  F1  a2     =  alpha * alpha;
  F1  denom  =  (NoH * NoH) * (a2 - 1.0f) + 1.0f;

  return a2 / (PI * denom * denom);
}

Inline F1 G1_GGX(F1 NoX, F1 alpha)
{
  NoX = saturate_F1(NoX);

  F1  a2     =  alpha * alpha;
  F1  denom  =  NoX + sqrtf(a2 + (1.0f - a2) * NoX * NoX);

  return (2.0f * NoX) / Max(denom, 1e-6f);
}

Inline F1 G_Smith(F1 NoV, F1 NoL, F1 alpha)
{
  return G1_GGX(NoV, alpha) * G1_GGX(NoL, alpha);
}

Inline F3 sample_GGX_half(Random_State *rng, F3 n, float alpha)
{
  F1  r1  =  random_unilateral(rng);
  F1  r2  =  random_unilateral(rng);

  F1  phi       =  2.0f * PI * r1;
  F1  cosTheta  =  sqrtf((1.0f - r2) / (1.0f + (alpha * alpha - 1.0f) * r2));
  F1  sinTheta  =  sqrtf(Max(0.0f, 1.0f - cosTheta * cosTheta));

  F1  x  =  sinTheta * cosf(phi);
  F1  y  =  sinTheta * sinf(phi);
  F1  z  =  cosTheta;

  // build ONB
  F3 tangent = (abs_F1(n.x) > 0.9f) ? f3(0, 1, 0) : f3(1, 0, 0);

  tangent = normalize_F3(cross_F3(tangent, n));

  F3 bitangent = cross_F3(n, tangent);

  // local -> world
  F3 local_xy = add_F3(scale_F3(tangent, x), scale_F3(bitangent, y));
  return normalize_F3(add_scaled_F3(local_xy, z, n));
}

Inline F1 pdf_GGX(F3 n, F3 h, F3 v, F1 alpha)
{
  F1  NoH     =  saturate_F1(dot_F3(n, h));
  F1  VoH     =  saturate_F1(dot_F3(v, h));
  F1  D       =  D_GGX(NoH, alpha);
  F1  result  =  D * NoH / (4.0f * VoH);

  return result;
}

Internal F4 ray_cast(RT_Scene scene, Random_State *rng, Ray ray)
{
  F1  min_hit_distance  =  0.001f;
  F4  result            =  {0};
  F4  attenuation       =  {1, 1, 1};

  for (L1 ray_index = 0; ray_index < scene.max_num_bounces; ray_index += 1)
  {
    F1         hit_distance     = F1_MAX;
    Shape_Kind hit_kind         = SHAPE_KIND_COUNT;
    L1         hit_packet_index = 0;
    L1         hit_lane         = 0;

    //- kti: Spheres
    L1 sphere_packet_count = (scene.sphere_count + LANE_COUNT - 1) / LANE_COUNT;
    for (L1 sphere_packet_index = 0; sphere_packet_index < sphere_packet_count; sphere_packet_index += 1)
    {
      Lane_Sphere spheres = scene.spheres[sphere_packet_index];

      Lane_F32 distances = ray_lane_sphere_intersect(ray, spheres);

      L1 lane_count = Min(scene.sphere_count - sphere_packet_index * LANE_COUNT, LANE_COUNT);
      for (L1 i = 0; i < lane_count; i += 1)
      {
        F1 t = distances[i];
        
        if (t > min_hit_distance && t < hit_distance)
        {
          hit_distance     = t;
          hit_kind         = SHAPE_KIND__SPHERE;
          hit_packet_index = sphere_packet_index;
          hit_lane         = i;
        }
      }
    }

    //- kti: Boxes
    L1 box_packet_count = (scene.box_count + LANE_COUNT - 1) / LANE_COUNT;
    for (L1 box_packet_index = 0; box_packet_index < box_packet_count; box_packet_index += 1)
    {
      Lane_Box boxes = scene.boxes[box_packet_index];

      Lane_F32 distances = ray_lane_box_intersect(ray, boxes);

      L1 lane_count = Min(scene.box_count - box_packet_index * LANE_COUNT, LANE_COUNT);
      for (L1 i = 0; i < lane_count; i += 1)
      {
        F1 t = distances[i];
        
        if (t > min_hit_distance && t < hit_distance)
        {
          hit_distance     = t;
          hit_kind         = SHAPE_KIND__BOX;
          hit_packet_index = box_packet_index;
          hit_lane         = i;
        }
      }
    }

    //- kti: Planes
    L1 plane_packet_count = (scene.plane_count + LANE_COUNT - 1) / LANE_COUNT;
    for (L1 plane_packet_index = 0; plane_packet_index < plane_packet_count; plane_packet_index += 1)
    {
      Lane_Plane planes = scene.planes[plane_packet_index];

      Lane_F32 distances = ray_lane_plane_intersect(ray, planes);

      L1 lane_count = Min(scene.plane_count - plane_packet_index * LANE_COUNT, LANE_COUNT);
      for (L1 i = 0; i < lane_count; i += 1)
      {
        F1 t = distances[i];

        if (t > min_hit_distance && t < hit_distance)
        {
          hit_distance     = t;
          hit_kind         = SHAPE_KIND__PLANE;
          hit_packet_index = plane_packet_index;
          hit_lane         = i;
        }
      }
    }

    if (hit_kind != SHAPE_KIND_COUNT)
    {
      L1 primitive_index = hit_packet_index * LANE_COUNT + hit_lane;

      L1 material_base_index = hit_kind   == SHAPE_KIND__SPHERE ? 0
                               : hit_kind == SHAPE_KIND__BOX    ? scene.sphere_count
                               : scene.sphere_count + scene.box_count;

      RT_Material  material     =  scene.materials[material_base_index + primitive_index];

      F3 next_origin = add_scaled_F3(ray.pos, hit_distance, ray.dir);
      F3 next_normal = {0};

      switch (hit_kind)
      {
        case SHAPE_KIND__SPHERE:
        {
          Lane_Sphere spheres = scene.spheres[hit_packet_index];
          F3 pos = extract_lane_F3(spheres.pos, hit_lane);

          next_normal = normalize_F3(sub_F3(next_origin, pos));
        } break;

        case SHAPE_KIND__PLANE:
        {
          Lane_Plane planes = scene.planes[hit_packet_index];
          F3 normal = extract_lane_F3(planes.normal, hit_lane);

          next_normal = dot_F3(normal, ray.dir) < 0.0f ? normal : neg_F3(normal);
        } break;

        case SHAPE_KIND__BOX:
        {
          Lane_Box boxes = scene.boxes[hit_packet_index];
          F3 min = extract_lane_F3(boxes.min, hit_lane);
          F3 max = extract_lane_F3(boxes.max, hit_lane);

          F3  center     =  scale_F3(add_F3(min, max), 0.5f);
          F3  local_hit  =  sub_F3(next_origin, center);
          F3  box_size   =  sub_F3(max, min);
          F3  d          =  div_F3(abs_F3(local_hit), scale_F3(box_size, 0.5f));

          if (d.x > d.y && d.x > d.z)
            next_normal = scale_F3(f3(1, 0, 0), sign_F1(local_hit.x));
          else if (d.y > d.z)
            next_normal = scale_F3(f3(0, 1, 0), sign_F1(local_hit.y));
          else
            next_normal = scale_F3(f3(0, 0, 1), sign_F1(local_hit.z));
        } break;

        default: break;
      }

      result += attenuation * material.emissive;

      F1  roughness     =  Max(0.05f, clamp01_F1(material.roughness));
      F1  alpha         =  roughness * roughness;
      F1  metallic      =  clamp01_F1(material.metallic);
      F4  f0            =  lerp_F4((F4){0.04f, 0.04f, 0.04f}, metallic, material.base_color);
      F1 specular_prob  =  0.5f; // keep constant; MIS handles weighting

      if (random_unilateral(rng) < specular_prob)
      {
        F3  v  =  neg_F3(ray.dir);
        F3  h  =  sample_GGX_half(rng, next_normal, alpha);
        F3  l  =  reflect_F3(neg_F3(v), h);

        F1  NoL  =  saturate_F1(dot_F3(next_normal, l));
        F1  NoV  =  saturate_F1(dot_F3(next_normal, v));
        F1  NoH  =  saturate_F1(dot_F3(next_normal, h));
        F1  VoH  =  saturate_F1(dot_F3(v, h));

        if (NoL > 0.0f)
        {
          F1  D  =  D_GGX(NoH, alpha);
          F1  G  =  G_Smith(NoV, NoL, alpha);
          F4  F  =  fresnel_schlick(VoH, f0);

          F1  denom   =  Max(4.0f * NoV * NoL, 1e-6f);
          F4  f_spec  =  (F * (D * G)) / denom;

          // Fresnel-weighted diffuse with metallic energy conservation
          F4  kd_color  =  (1.0f - metallic) * (((F4){1, 1, 1}) - F);
          F4  f_diff    =  (kd_color * material.base_color) / PI;

          F1  pdf_spec   =  pdf_GGX(next_normal, h, v, alpha);
          F1  pdf_diff   =  NoL / PI;
          F1  pdf_total  =  specular_prob * pdf_spec + (1.0f - specular_prob) * pdf_diff;
          F1  inv_pdf    =  1.0f / Max(pdf_total, 1e-6f);

          attenuation  *=  (f_spec + f_diff) * (NoL * inv_pdf);
          ray.dir      =   l;
        }
        else
        {
          break; // absorb
        }
      }
      else
      {
        F3  v    =  neg_F3(ray.dir);
        F3  l    =  sample_cosine_hemisphere(rng, next_normal);
        F1  NoL  =  saturate_F1(dot_F3(next_normal, l));

        if (NoL > 0.0f)
        {
          F3  h    =  normalize_F3(add_F3(v, l));
          F1  NoV  =  saturate_F1(dot_F3(next_normal, v));
          F1  NoH  =  saturate_F1(dot_F3(next_normal, h));
          F1  VoH  =  saturate_F1(dot_F3(v, h));

          F1  D  =  D_GGX(NoH, alpha);
          F1  G  =  G_Smith(NoV, NoL, alpha);
          F4  F  =  fresnel_schlick(VoH, f0);

          F1  denom   =  Max(4.0f * NoV * NoL, 1e-6f);
          F4  f_spec  =  (F * (D * G)) / denom;

          F4  kd_color  =  (1.0f - metallic) * (((F4){1, 1, 1}) - F);
          F4  f_diff    =  (kd_color * material.base_color) / PI;

          F1  pdf_spec   =  pdf_GGX(next_normal, h, v, alpha);
          F1  pdf_diff   =  NoL / PI;
          F1  pdf_total  =  specular_prob * pdf_spec + (1.0f - specular_prob) * pdf_diff;
          F1  inv_pdf    =  1.0f / Max(pdf_total, 1e-6f);

          attenuation *= (f_spec + f_diff) * (NoL * inv_pdf);
          ray.dir = l;
        }
        else
        {
          break; // absorb
        }
      }

      ray.inv_dir  =  safe_inv_dir_F3(ray.dir);
      ray.pos      =  add_scaled_F3(next_origin, min_hit_distance, next_normal);
    }
    else
    {
      //- kti: Sky
      // F1 height     =  (ray.dir.y + 1) * 0.5;
      // F4 sky_color  =  lerp_F4((F4){1.0f, 1.0f, 1.0f}, height, (F4){0.2f, 0.4f, 1.0f});
      // result += attenuation * sky_color;
      break;
    }
  }

  return result;
}

Internal void rt_trace_scene(RT_Scene scene, Image output, Range pixel_range)
{
  F3  camera_p        =  scene.camera.pos;
  F3  camera_forward  =  normalize_F3(scene.camera.forward);
  F3  world_up        =  f3(0, 1, 0);
  F3  camera_right    =  normalize_F3(cross_F3(world_up, camera_forward));
  F3  camera_up       =  normalize_F3(cross_F3(camera_forward, camera_right));

  F1  film_dist    =  1.0f;
  F1  aspect       =  (F1)output.width / (F1)output.height;
  F1  half_film_h  =  tan_F1(scene.camera.vertical_fov * 0.5f) * film_dist;
  F1  half_film_w  =  aspect * half_film_h;
  F3  film_center  =  add_scaled_F3(camera_p, film_dist, camera_forward);

  F4 *pixels = (F4 *)output.pixels;

  for (L1 i = pixel_range.min; i < Min(output.width * output.height, pixel_range.max); i += 1)
  {
    L1  x  =  i % output.width;
    L1  y  =  i / output.width;

    Random_State rng = {
      .state  =  i * 7 ^ ~((i - 420) * 11),
      .inc    =  2 * i + 1,
    };

    F4  color    =  {0};
    F1  contrib  =  1.0f / (F1)scene.rays_per_pixel;

    for (L1 ray_index = 0; ray_index < scene.rays_per_pixel; ray_index += 1)
    {
      Ray ray = {0};

      F1  sample_x  =  (F1)x + random_unilateral(&rng);
      F1  sample_y  =  (F1)y + random_unilateral(&rng);
      F1  film_x    =  -1.0f + 2.0f * sample_x / (F1)output.width;
      F1  film_y    =  -1.0f + 2.0f * sample_y / (F1)output.height;
      F3 film_p = add_scaled_F3(film_center, film_x * half_film_w, camera_right);
      film_p = add_scaled_F3(film_p, film_y * half_film_h, camera_up);

      F1  r                =  scene.camera.aperture_radius * sqrt_F1(random_unilateral(&rng));
      F1  theta            =  2.0f * PI * random_unilateral(&rng);
      F3  aperture_offset  =  add_F3(scale_F3(camera_right, r * cos_F1(theta)), scale_F3(camera_up, r * sin_F1(theta)));

      ray.pos = add_F3(camera_p, aperture_offset);

      F3 focus_direction = normalize_F3(sub_F3(film_p, camera_p));
      F3 focus_point = add_scaled_F3(camera_p, scene.camera.focal_distance, focus_direction);

      ray.dir     = normalize_F3(sub_F3(focus_point, ray.pos));
      ray.inv_dir = safe_inv_dir_F3(ray.dir);

      color += ray_cast(scene, &rng, ray) * contrib;
    }

    pixels[i] = color;
  }
}
