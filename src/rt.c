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

Internal F4 ray_cast(RT_Scene scene, Random_State *rng, Ray ray) {
  F1 min_hit_distance = 0.001f;

  F4 result = {0};
  F4 attenuation = {1, 1, 1};
  for (L1 ray_index = 0; ray_index < scene.max_num_bounces; ray_index += 1) {
    F1 hit_distance = F1_MAX;

    L1 hit_index = 0;
    Shape *hit_shape = 0;

    for (L1 shape_index = 0; shape_index < scene.shape_count; shape_index += 1) {
      Shape *shape = &scene.shapes[shape_index]; 

      F1 t = ray_shape_intersect(ray, shape[0]);
      if (t > min_hit_distance && t < hit_distance) {
        hit_distance = t;
        hit_shape = shape;
        hit_index = shape_index;
      }
    }

    if (hit_shape != 0) {
      RT_Material mat = scene.materials[hit_index];

      F4 next_origin = ray.pos + hit_distance*ray.dir;

      F4 next_normal = {0};
      switch (hit_shape->kind) {
        case SHAPE_KIND__SPHERE: {
          F4 pos = F4_from_V3(hit_shape->sphere.pos); 
          next_normal = normalize_F4(next_origin - pos);
        } break;
        case SHAPE_KIND__PLANE: {
          F4 normal = F4_from_V3(hit_shape->plane.normal);
          next_normal = dot_F4(normal, ray.dir) < 0.0f ? -normal : normal;
        } break;
        case SHAPE_KIND__BOX: {
          F4 min = F4_from_V3(hit_shape->box.min);
          F4 max = F4_from_V3(hit_shape->box.max);

          F4 center = (min + max) * 0.5f;
          F4 local_hit = next_origin - center;
          F4 box_size = max - min;
          F4 d = abs_F4(local_hit) / (box_size * 0.5f);
          if (d[0] > d[1] && d[0] > d[2]) next_normal = sign_F1(local_hit[0]) * (F4){1,0,0};
          else if (d[1] > d[2]) next_normal = sign_F1(local_hit[1]) * (F4){0,1,0};
          else next_normal = sign_F1(local_hit[2]) * (F4){0,0,1};
        } break;
        default: break;
      }

      result += attenuation * mat.emissive;

      F1 roughness = Max(0.05f, clamp01_F1(mat.roughness));
      F1 alpha = roughness * roughness;
      F1 metallic = clamp01_F1(mat.metallic);

      F4 f0 = lerp_F4((F4){0.04f,0.04f,0.04f}, metallic, mat.base_color);

      F1 specular_prob = 0.5f; // keep constant; MIS handles weighting
      if (random_unilateral(rng) < specular_prob) {
        F4 v = -ray.dir;
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
          ray.dir = l;
        } else {
          break; // absorb
        }
      } else {
        F4 v = -ray.dir;
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
          ray.dir = l;
        } else {
          break; // absorb
        }
      }

      ray.inv_dir = 1.0f / ray.dir;
      ray.pos = next_origin + next_normal * min_hit_distance;
    } else {
      F1 height = (ray.dir.y + 1) * 0.5;
      F4 sky_color = lerp_F4((F4){1.0f, 1.0f, 1.0f}, height, (F4){0.2f, 0.4f, 1.0f});
      result += attenuation * sky_color;
      break;
    }
  }

  return result;
}

Internal void rt_trace_scene(RT_Scene scene, Image output, Range pixel_range) {
  F4 camera_p = scene.camera.pos;
  F4 camera_forward = normalize_F4(scene.camera.forward);
  F4 world_up = {0, 1, 0};
  F4 camera_right = normalize_F4(cross_F4(world_up, camera_forward));
  F4 camera_up = normalize_F4(cross_F4(camera_forward, camera_right));

  F1 film_dist   = 1.0f;
  F1 aspect      = (F1)output.width/(F1)output.height;
  F1 half_film_h = tan_F1(scene.camera.vertical_fov*0.5f)*film_dist;
  F1 half_film_w = aspect*half_film_h;
  F4 film_center = camera_p + film_dist*camera_forward;

  F4 *pixels = (F4 *)output.pixels;

  for (L1 i = pixel_range.min; i < Min(output.width*output.height, pixel_range.max); i += 1) {
    L1 x = i % output.width;
    L1 y = i / output.width;

    Random_State rng = {
      .state = i*7^~((i-420)*11),
      .inc = 2*i + 1,
    };

    F4 color = {0};
    F1 contrib = 1.0f / (F1)scene.rays_per_pixel;

    for (L1 ray_index = 0; ray_index < scene.rays_per_pixel; ray_index += 1) {
      Ray ray = {0};

      F1 sample_x = (F1)x + random_unilateral(&rng);
      F1 sample_y = (F1)y + random_unilateral(&rng);
      F1 film_x = -1.0f + 2.0f*sample_x/(F1)output.width;
      F1 film_y = -1.0f + 2.0f*sample_y/(F1)output.height;
      F4 film_p = film_center + film_x*half_film_w*camera_right + film_y*half_film_h*camera_up;

      F1 r = scene.camera.aperture_radius * sqrt_F1(random_unilateral(&rng));
      F1 theta = 2.0f * PI * random_unilateral(&rng);
      F4 aperture_offset = r * cos_F1(theta) * camera_right + r * sin_F1(theta) * camera_up;
      ray.pos = camera_p + aperture_offset;

      F4 focus_point = camera_p + scene.camera.focal_distance  * normalize_F4(film_p - camera_p);
      ray.dir = normalize_F4(focus_point - ray.pos);
      ray.inv_dir = 1.0f / ray.dir;

      color += ray_cast(scene, &rng, ray) * contrib;
    }

    pixels[i] = color;
  }
}
#endif
