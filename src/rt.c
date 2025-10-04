// rt.c - Ray Tracer 

// Coordinate system:
// +x right, +y forwards, +z up

// TODO:
// Shadow rays, higher quality w. lower ray counts 
// Quads
// BVH (for larger scenes)

#if (CPU_ && TYP_)

#pragma pack(push, 1)
typedef struct Bitmap_Header Bitmap_Header;
struct Bitmap_Header {
	W1 file_type;
	I1 file_size;
	W1 reserved1, reserved2;
	I1 bitmap_offset;
	I1 size;
	SI1 width;
	SI1 height;
	W1 planes;
	W1 bits_per_pixel;
	I1 compression;
	I1 size_of_bitmap;
	SI1 horz_resolution;
	SI1 vert_resolution;
	I1 colors_used;
	I1 colors_important;
};
#pragma pack(pop)

typedef struct Image Image;
struct Image {
	I1 width;
	I1 height;
	I1 bytes_per_pixel;
	L1 pixels;
};

typedef struct Material Material;
struct Material {
	F3 base_color;
	F1 metallic;
	F1 roughness;
	F3 emissive;
	// F1 specular;
	// F1 subsurface;
	// F1 specular_tint;
	// F1 antisotropic;
	// F1 sheen;
	// F1 sheen_tint;
	// F1 clearcoat;
	// F1 clearcoat_gloss;
};

typedef struct Sphere Sphere;
struct Sphere {
	F3 p;
	F1 r;
	I1 material_idx;
};

typedef struct Plane Plane;
struct Plane {
	F3 n;
	F1 d;
	I1 material_idx;
};

typedef struct Box Box;
struct Box {
	F3 min;
	F3 max;
	I1 material_idx;
};

typedef struct World World;
struct World {
	L1 material_count;
	L1 materials;

	L1 plane_count;
	L1 planes;

	L1 sphere_count;
	L1 spheres;

	L1 box_count;
	L1 boxes;
};

F1 sqrtf(F1);
F1 sinf(F1);
F1 cosf(F1);
F1 fabsf(F1);
F1 powf(F1, F1);

#define PI 3.141592653598979f

#endif

#if (CPU_ && RAM_)

L1 complete_sections_count;
L1 section_count;
L1 current_section_idx;

L1 start_time;
Image ray_image;
Image final_image;

L1 total_memory_usage;

#endif

#if (CPU_ && ROM_)

Inline L1 image_pixels_size(Image image) {
	return image.bytes_per_pixel * image.width * image.height;
}

Inline Image image_alloc(L1 arena, I1 width, I1 height, I1 bytes_per_pixel) {
	Image image  = {0};
	image.width  = width;
	image.height = height;
	image.bytes_per_pixel = bytes_per_pixel;
	image.pixels = arena_push(arena, image_pixels_size(image));
	return image;
}

Inline L1 image_xy_to_index(Image image, L1 x, L1 y) {
	if (LtSL1(x, 0)) x = 0;
	if (LtSL1(y, 0)) y = 0;
	if (x > image.width-1) x = image.width-1;	
	if (y > image.height-1) y = image.height-1;	
	L1 result = x + y*image.width;
	return result;
}

Internal void image_write_to_file(Image image, CString filename) {
	Assert(image.bytes_per_pixel == 4);

	L1 pixels_size = image_pixels_size(image);

	Bitmap_Header header  = {0};
	header.file_type      = 0x4D42;
 	header.file_size      = sizeof(Bitmap_Header) + pixels_size;
	header.bitmap_offset  = sizeof(Bitmap_Header);
	header.size           = 40;
	header.width          = SI1_(image.width);
	header.height         = SI1_(image.height);
	header.planes         = 1;
	header.bits_per_pixel = 32;
	header.compression    = 0;
	header.size_of_bitmap = pixels_size;

	// TODO: Create os_ functions for this
	L1 file = open(filename, O_CREAT | O_WRONLY, 0666);
	if (GtSI1(file, 0)) {
		write(file, L1_(&header), sizeof(Bitmap_Header));
		write(file, image.pixels, pixels_size);
		close(file);
		printf("Image written to %s\n", filename);
	} else {
		fprintf(stderr, "Could not open '%s' to write image.\n", filename);
	}
}

Inline F1 F1_clamp01(F1 x) { return Min(Max(x, 0.0f), 1.0f); }
Inline F1 F1_saturate(F1 x) { return F1_clamp01(x); }


Inline F1 F1_sign(F1 a) {
	F1 result = -1.0f;
	if (a > 0.0f) result = 1.0f;
	return result;	
}

Inline F1 F3_dot(F3 a, F3 b) {
	F1 result = a.x*b.x + a.y*b.y + a.z*b.z;
	return result;
}

Inline F1 F3_length_sq(F3 v) {
	F1 result = F3_dot(v, v);
	return result;
}

Inline F1 F3_length(F3 v) {
	F1 result = sqrtf(F3_length_sq(v));
	return result;
}

Inline F3 F3_normalize(F3 v) {
	F3 result = v * (1.0f / F3_length(v));
	return result;
}

Inline F3 F3_cross(F3 a, F3 b) {
	F3 result;

	result.x = a.y*b.z - a.z*b.y;
	result.y = a.z*b.x - a.x*b.z;
	result.z = a.x*b.y - a.y*b.x;

	return result;
}

Inline F3 F3_lerp(F3 a, F1 t, F3 b) {
	F3 result = a * (1-t) + b*t;
	return result;
}

Inline F3 F3_abs(F3 v) {
	F3 result = { fabsf(v.x), fabsf(v.y), fabsf(v.z) };
	return result;
}

Inline F3 F3_clamp01(F3 v) {
	F3 result= {
		F1_clamp01(v.x),
		F1_clamp01(v.y),
		F1_clamp01(v.z) 
	};
	return result;
}

Inline F1 F3_luminance(F3 v) {
	F1 result = 0.2126f*v.x + 0.7152f*v.y + 0.0722f*v.z;
	return result;
}

Inline F3 F3_pow(F3 v, F1 exp) {
	F3 result = {
		powf(v.x, exp),
		powf(v.y, exp),
		powf(v.z, exp)
	};
	return result;
}

typedef struct Random_State Random_State;
struct Random_State {
	L1 state;
	L1 inc;
};

// TODO: Take as parameter to rand_pcg
ThreadLocal Random_State rng;

Inline I1 rand_pcg() {
	L1 oldstate = rng.state;
  rng.state = oldstate * 6364136223846793005ULL + (rng.inc|1);
  I1 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
  I1 rot = oldstate >> 59u;
  I1 result = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
  return result;
}

Inline F1 random_unilateral(void) {
	F1 result = F1_(rand_pcg()) / F1_(I1_MAX);
	return result;
}

Inline F1 random_bilateral(void) {
	F1 result = -1 + 2 * random_unilateral();
	return result;
}

Inline F3 F3_reflect(F3 v, F3 normal) {
	F3 result = v - 2*F3_dot(v, normal) * normal;
	return result;
}

Inline F3 sample_cosine_hemisphere(F3 n) {
  F1 x, y;
  do {
    x = 2.0f * random_unilateral() - 1.0f;
    y = 2.0f * random_unilateral() - 1.0f;
  } while (x*x + y*y > 1.0f);

  F1 z = sqrtf(1.0f - x*x - y*y);

  // build ONB around n
  F3 tangent = (fabsf(n.x) > 0.9f) ? (F3){0,1,0} : (F3){1,0,0};
  tangent = F3_normalize(F3_cross(tangent, n));
  F3 bitangent = F3_cross(n, tangent);

  // local -> world
  return F3_normalize(
    tangent * x +
    bitangent * y +
    n * z
  );
}

// Schlick Fresnel approximation
Inline F3 fresnel_schlick(F1 cos_theta, F3 F0) {
  return F0 + ((F3){1,1,1} - F0) * powf(1.0f - cos_theta, 5.0f);
}

// GGX / Trowbridge-Reitz normal distribution function (NDF)
Inline F1 D_GGX(F1 NoH, F1 alpha) {
  F1 a2 = alpha * alpha;
  F1 denom = (NoH*NoH)*(a2 - 1.0f) + 1.0f;
  return a2 / (PI*denom*denom);
}

// Smith's GGX geometry term (separable)
Inline F1 G1_GGX(F1 NoX, F1 alpha) {
  NoX = F1_saturate(NoX);
  F1 a2 = alpha * alpha;
  F1 denom = NoX + sqrtf(a2 + (1.0f - a2) * NoX * NoX);
  return (2.0f * NoX) / Max(denom, 1e-6f);
}

Inline F1 G_Smith(F1 NoV, F1 NoL, F1 alpha) {
  return G1_GGX(NoV, alpha) * G1_GGX(NoL, alpha);
}

// GGX importance sample half-vector
Inline F3 sample_GGX_half(F3 n, float alpha) {
  F1 r1 = random_unilateral();
  F1 r2 = random_unilateral();

  F1 phi = 2.0f * PI * r1;
  F1 cosTheta = sqrtf( (1.0f - r2) / (1.0f + (alpha*alpha - 1.0f) * r2) );
  F1 sinTheta = sqrtf(Max(0.0f, 1.0f - cosTheta*cosTheta));

  F1 x = sinTheta * cosf(phi);
  F1 y = sinTheta * sinf(phi);
  F1 z = cosTheta;

  // build ONB
  F3 tangent = (fabsf(n.x) > 0.9f) ? (F3){0,1,0} : (F3){1,0,0};
  tangent = F3_normalize(F3_cross(tangent, n));
  F3 bitangent = F3_cross(n, tangent);

  // local -> world
  return F3_normalize(
    tangent * x +
    bitangent * y +
    n * z
  );
}

// PDF for GGX reflection sampling
Inline F1 pdf_GGX(F3 n, F3 h, F3 v, F1 alpha) {
  F1 NoH = F1_saturate(F3_dot(n,h));
  F1 VoH = F1_saturate(F3_dot(v,h));
  F1 D = D_GGX(NoH, alpha);
  F1 result = D * NoH / (4.0f * VoH);
  return result;
}


Internal F3 ray_cast(World world, F3 ray_origin, F3 ray_direction, L1 max_num_bounces) {
	F1 min_hit_distance = 0.001f;
	F1 tolerance = 0.0001f;
 
	F3 result = {0};
	F3 attenuation = {1, 1, 1};
	for EachIndex(ray_index, max_num_bounces) {
		F1 hit_distance = F1_MAX;

		I1 hit_material_idx = -1;
		F3 next_origin;
		F3 next_normal;

		// Check planes
		for EachIndex(plane_index, world.plane_count) {
			Plane plane = TR_(Plane, world.planes)[plane_index];

			F1 denom = F3_dot(plane.n, ray_direction);
			if (denom < -tolerance || denom > tolerance) {
				F1 t = (-plane.d - F3_dot(plane.n, ray_origin)) / denom;
				if (t > min_hit_distance && t < hit_distance) {
					hit_distance = t;
					hit_material_idx = plane.material_idx;

					next_origin = ray_origin + t*ray_direction;
					next_normal = plane.n;
				}
			}
		}

		// Check spheres
		for EachIndex(sphere_index, world.sphere_count) {
			Sphere sphere = TR_(Sphere, world.spheres)[sphere_index];

			F3 sphere_relative_ray_origin = ray_origin - sphere.p;
			F1 b = 2.0f * F3_dot(ray_direction, sphere_relative_ray_origin);
			F1 c = F3_dot(sphere_relative_ray_origin, sphere_relative_ray_origin) - sphere.r*sphere.r;

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
					next_normal = F3_normalize(next_origin - sphere.p);
				}
			}
		}

		// Check boxes
		for EachIndex(box_index, world.box_count) {
			Box box = TR_(Box, world.boxes)[box_index];

			F1 t_min = (box.min.x - ray_origin.x) / ray_direction.x;
			F1 t_max = (box.max.x - ray_origin.x) / ray_direction.x;
			if (t_min > t_max) Swap(t_min, t_max);

			F1 ty_min = (box.min.y - ray_origin.y) / ray_direction.y;
			F1 ty_max = (box.max.y - ray_origin.y) / ray_direction.y;
			if (ty_min > ty_max) Swap(ty_min, ty_max);

			t_min = Max(t_min, ty_min);
			t_max = Min(t_max, ty_max);

			if (t_min > t_max) continue;

			F1 tz_min = (box.min.z - ray_origin.z) / ray_direction.z;
			F1 tz_max = (box.max.z - ray_origin.z) / ray_direction.z;
			if (tz_min > tz_max) Swap(tz_min, tz_max);

			t_min = Max(t_min, tz_min);
			t_max = Min(t_max, tz_max);

			if (t_min > t_max) continue;

			if (t_min > min_hit_distance && t_min < hit_distance) {
				next_origin = ray_origin + t_min * ray_direction;
        F3 center = (box.min + box.max) * 0.5f;
        F3 local_hit = next_origin - center;
        F3 box_size = box.max - box.min;
        
        F3 d = F3_abs(local_hit) / (box_size * 0.5f);
        if (d.x > d.y && d.x > d.z) next_normal = F1_sign(local_hit.x) * (F3){1,0,0};
        else if (d.y > d.z) next_normal = F1_sign(local_hit.y) * (F3){0,1,0};
        else next_normal = F1_sign(local_hit.z) * (F3){0,0,1};

        hit_distance = t_min;
        hit_material_idx = box.material_idx;
			}

		}

		if (hit_material_idx != I1_(-1)) {
			Material mat = TR_(Material, world.materials)[hit_material_idx];

			result += attenuation * mat.emissive;

			F1 roughness = Max(0.05f, F1_clamp01(mat.roughness));
			F1 alpha = roughness * roughness;
			F1 metallic = F1_clamp01(mat.metallic);

			F3 f0 = F3_lerp((F3){0.04f,0.04f,0.04f}, metallic, mat.base_color);

			F1 specular_prob = 0.5f; // keep constant; MIS handles weighting
			if (random_unilateral() < specular_prob) {
				F3 v = -ray_direction;
				F3 h = sample_GGX_half(next_normal, alpha);
				F3 l = F3_reflect(-v, h);

				F1 NoL = F1_saturate(F3_dot(next_normal, l));
				F1 NoV = F1_saturate(F3_dot(next_normal, v));
				F1 NoH = F1_saturate(F3_dot(next_normal, h));
				F1 VoH = F1_saturate(F3_dot(v, h));

				if (NoL > 0.0f) {
					F1 D = D_GGX(NoH, alpha);
					F1 G = G_Smith(NoV, NoL, alpha);
					F3 F = fresnel_schlick(VoH, f0);

					F1 denom = Max(4.0f * NoV * NoL, 1e-6f);
					F3 f_spec = (F * (D * G)) / denom;

					// Fresnel-weighted diffuse with metallic energy conservation
					F3 kd_color = (1.0f - metallic) * (((F3){1,1,1}) - F);
					F3 f_diff = (kd_color * mat.base_color) / PI;

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
				F3 v = -ray_direction;
				F3 l = sample_cosine_hemisphere(next_normal);
				F1 NoL = F1_saturate(F3_dot(next_normal, l));
				if (NoL > 0.0f) {
					F3 h = F3_normalize(v + l);
					F1 NoV = F1_saturate(F3_dot(next_normal, v));
					F1 NoH = F1_saturate(F3_dot(next_normal, h));
					F1 VoH = F1_saturate(F3_dot(v, h));

					F1 D = D_GGX(NoH, alpha);
					F1 G = G_Smith(NoV, NoL, alpha);
					F3 F = fresnel_schlick(VoH, f0);

					F1 denom = Max(4.0f * NoV * NoL, 1e-6f);
					F3 f_spec = (F * (D * G)) / denom;

					F3 kd_color = (1.0f - metallic) * (((F3){1,1,1}) - F);
					F3 f_diff = (kd_color * mat.base_color) / PI;

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
			// F3 sky_color = F3_lerp((F3){1.0f, 1.0f, 1.0f}, height, (F3){0.2f, 0.4f, 1.0f});
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

Inline F3 tonemap_aces(F3 v) {
	const F1 a = 2.51f;
	const F1 b = 0.03f;
	const F1 c = 2.43f;
	const F1 d = 0.59f;
	const F1 e = 0.14f;
	F3 result = F3_clamp01((v * (a * v + b)) / (v * (c * v + d) + e));
	return result;
} 

Inline F3 tonemap_reinhard(F3 v) {
	F3 result = v / (1.0f + v);
	return result;
}

F3 tonemap_lottes(F3 v) {
  // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
  const F1 a = 1.6f;
  const F1 d = 0.977f;
  const F1 hdr_max = 8.0f;
  const F1 mid_in = 0.18f;
  const F1 mid_out = 0.267f;

  // Can be precomputed
  const F1 b =
      (-powf(mid_in, a) + powf(hdr_max, a) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);
  const float c =
      (powf(hdr_max, a * d) * powf(mid_in, a) - powf(hdr_max, a) * powf(mid_in, a * d) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);

  F3 result = F3_pow(v, a) / (F3_pow(v, a * d) * b + c);
  return result;
}

#define tonemap tonemap_lottes

#define output_width  1920
#define output_height 1080

#define rays_per_pixel  512
#define max_num_bounces 8

#define aperture_radius 0.06f
#define focal_distance  5.0f

#define bloom_pass_count 8
#define bloom_threshold  0.5f
#define bloom_strength   0.1f
#define bloom_knee       0.3f

Internal void lane(L1 arena) {

	if (lane_idx() == 0) {
		ramR->ray_image = image_alloc(arena, output_width, output_height, sizeof(F3));

		ramR->start_time = os_clock();

		ramR->section_count = lane_count() * 8;
		ramR->current_section_idx = 0;
	}

	lane_sync();

	Material materials[] = {
		{ // ground
			.base_color  = (F3){0.53f, 0.43f, 0.13f},
			.metallic = 0.9f,
			.roughness = 0.4f,
		},
		{ // left
			.base_color  = (F3){1.0f, 1.0f, 1.0f},
			.metallic = 1.0f,
			.roughness = 0.40f,
		},
		{ // right
			.base_color  = (F3){0.1f, 1.0f, 1.0f},
			.metallic = 0.0f,
			.roughness = 0.10f,
		},
		{ // pink glow
			.base_color  = (F3){1.0f, 0.2f, 1.0f},
			.emissive = (F3){2.0f, 0.6f, 2.0f},
			.metallic = 0.1f,
			.roughness = 1.00f,
		},
		{ // blue glow
			.base_color  = (F3){1.0f, 1.0f, 1.0f},
			.emissive = (F3){1.0f, 1.0f, 6.0f},
		},
	};

	Plane planes[] = {
		{
			.n = (F3){0, 0, 1},
			.d = 1.0f,
			.material_idx = 0
		}
	};

	Sphere spheres[] = {
		{
			.p = (F3){-2.0f, 2.0f, 0.0f},
			.r = 1.0f,
			.material_idx = 1,
		},
		{
			.p = (F3){2.0f, 0.0f, 0.0f},
			.r = 1.0f,
			.material_idx = 2,
		},
		{
			.p = (F3){0.0f, 20.0f, 0.0f},
			.r = 1.0f,
			.material_idx = 3,
		},
	};

	Box boxes[] = {
		{
			.min = {-2.6f, -2.0f,-1.0},
			.max = {-2.5f,  0.7f,-0.5},
			.material_idx = 4,
	 	}
	};

	F3 camera_p = (F3){0.0f, -5.0f, 0.5f};
	F3 camera_z = F3_normalize(camera_p);
	F3 camera_x = F3_normalize(F3_cross((F3){0, 0, 1}, camera_z));
	F3 camera_y = F3_normalize(F3_cross(camera_z, camera_x));

	World world = {0};
	world.material_count = ArrayLength(materials);
	world.materials      = L1_(materials);
	world.plane_count    = ArrayLength(planes);
	world.planes         = L1_(planes);
	world.sphere_count   = ArrayLength(spheres);
	world.spheres        = L1_(spheres);
	world.box_count    = ArrayLength(boxes);
	world.boxes          = L1_(boxes);

	F1 film_dist   = 1.0f;
	F1 film_w      = F1_(output_width)/F1_(output_height);
	F1 film_h      = 1.0f;
	F1 half_film_w = film_w*0.5f;
	F1 half_film_h = film_h*0.5f;
	F3 film_center = camera_p - film_dist * camera_z;

	F1 half_pixel_w = 0.5f/F1_(output_width);
	F1 half_pixel_h = 0.5f/F1_(output_height);

	L1 lane_begin_time = os_clock();

	L1 section_pixel_count = output_width*output_height/ramR->section_count;
	L1 ray_pixels = arena_push(arena, sizeof(F3)*section_pixel_count);

	while (ramR->current_section_idx < ramR->section_count) {
		L1 section_idx = AtomicAddL1(&ramV->current_section_idx, 1);
		if (section_idx >= ramR->section_count) break;
		Range pixels_range = {
			.min = section_pixel_count * section_idx,
			.max = section_pixel_count * (section_idx + 1)
		};

		L1 out = ray_pixels;
		for EachInRange(pixel_index, pixels_range) {
			I1 x = pixel_index%output_width;
			I1 y = pixel_index/output_width;
			F1 film_y = -1 + 2 * F1_(y)/F1_(output_height);
			F1 film_x = -1 + 2 * F1_(x)/F1_(output_width);

			F3 color = {};
			F1 contrib = 1.0f / F1_(rays_per_pixel);
			for EachIndex(ray_index, rays_per_pixel) {
				F1 off_x  = film_x + random_bilateral()*half_pixel_w;
				F1 off_y  = film_y + random_bilateral()*half_pixel_h;
				F3 film_p = film_center +
										off_x*half_film_w*camera_x +
										off_y*half_film_h*camera_y;

				F1 r = aperture_radius * sqrtf(random_unilateral());
				F1 theta = 2.0f * PI * random_unilateral();
				F3 aperture_offset = r * cosf(theta) * camera_x + r * sinf(theta) * camera_y;
				F3 ray_origin = camera_p + aperture_offset;

				F3 focus_point = camera_p + focal_distance  * F3_normalize(film_p - camera_p);
				F3 ray_direction = F3_normalize(focus_point - ray_origin);

				color += ray_cast(world, ray_origin, ray_direction, max_num_bounces) * contrib;
			}

			TR_(F3, out)[0] = color;
			out += sizeof(F3);

		}

		L1 dest = ramR->ray_image.pixels + sizeof(F3)*pixels_range.min;
		memmove(dest, ray_pixels, sizeof(F3)*section_pixel_count);

		AtomicAddL1(&ramV->complete_sections_count, 1);

		L1 now         = os_clock();
		L1 duration_ms = (now - lane_begin_time)/1000000;
		F1 duration_s  = F1_(duration_ms)/1000.0f;
		F1 percent     = F1_(ramR->complete_sections_count) / F1_(ramR->section_count) * 100.0f;
		printf("\r[%.2fs]: %.2f%%", duration_s, percent);
		fflush(stdout);
	}

	lane_sync();

	if (lane_idx() == 0) {
		printf("\nRay tracing complete. Postprocessing...\n");

		// TODO: Multithread

		Image bloom_passes[bloom_pass_count] = {0};

		L1 ray_img_w = ramR->ray_image.width;
		L1 ray_img_h = ramR->ray_image.height;
		bloom_passes[0] = image_alloc(arena, ray_img_w, ray_img_h, sizeof(F3));

		// Filter on bright pixels
		L1 in = ramR->ray_image.pixels;
		L1 out = bloom_passes[0].pixels;
		for EachIndex(pixel_index, ray_img_w*ray_img_h) {
			F3 color = TR_(F3, in)[0];

			F1 luminance = F3_luminance(color);

			F1 soft_threshold = bloom_threshold - bloom_knee;
			F1 knee_range = 2.0f*bloom_knee;
			F1 contrib = 0.0f;

			if (luminance > bloom_threshold + bloom_knee) {
				contrib = 1.0f;
			} else if (luminance > soft_threshold) {
				F1 x = luminance - soft_threshold;
				F1 knee_contrib = (x*x) / (4.0f*knee_range*knee_range);
				contrib = knee_contrib;
			}

			if (contrib > 0.0f && luminance > 1e-6f) {
				F1 excess_luminance = Max(0.0f, luminance - bloom_threshold);
				color = color * (excess_luminance/luminance)*contrib;
			} else {
				color = (F3){0};
			}

			TR_(F3, out)[0] = color;

			in  += sizeof(F3);
			out += sizeof(F3);
		}

		// Downsample
		for EachIndex(pass_index, bloom_pass_count-1) {
			Image in = bloom_passes[pass_index];
			Image out = image_alloc(arena, in.width/2, in.height/2, sizeof(F3));

			bloom_passes[pass_index+1] = out;

			for EachIndex(y, out.height) {
				L1 sy = y*2;
				for EachIndex(x, out.width) {
					L1 sx = x*2;

					// TODO: look into 13-tap bilinear tent filter. 

					// 16-tap downsample
					// (sx,sy)
					//    1\2 2 1
					//    2 4 4 2  
					//    2 4 4 2  
					//    1 2 2 1

					F3 sum = {0};

					// Center (4)
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+0,sy+0)] * 4.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+1,sy+0)] * 4.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+1,sy+1)] * 4.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+0,sy+1)] * 4.0f;

					// Edges (2)
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx-1,sy+0)] * 2.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx-1,sy+1)] * 2.0f;

					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+2,sy+0)] * 2.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+2,sy+1)] * 2.0f;

					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+0,sy-1)] * 2.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+1,sy-1)] * 2.0f;

					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+0,sy+2)] * 2.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+1,sy+2)] * 2.0f;

					// Corners (1)
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx-1,sy-1)] * 1.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+2,sy-1)] * 1.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx-1,sy+2)] * 1.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+2,sy+2)] * 1.0f;

					sum /= 36.0f;
					F3 karis_average = sum / (1.0f + F3_luminance(sum));

					TR_(F3, out.pixels)[x + y*out.width] = karis_average;
				}
			}
		}

		// Upsample and sum
		for (L1 pass_index = bloom_pass_count-1; pass_index >= 1; pass_index -= 1) {
			Image in = bloom_passes[pass_index];
			Image out = bloom_passes[pass_index-1];

			for EachIndex(y, out.height) {
				L1 sy = y/2;
				for EachIndex(x, out.width) {
					L1 sx = x/2;

					F3 sum = {0};

					// center
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx,sy)]*4.0f;

					// edges
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx-1,sy)]*2.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+1,sy)]*2.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx,sy-1)]*2.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx,sy+1)]*2.0f;

					// corners
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx-1,sy-1)]*1.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+1,sy-1)]*1.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx-1,sy+1)]*1.0f;
					sum += TR_(F3, in.pixels)[image_xy_to_index(in,sx+1,sy+1)]*1.0f;

					sum /= 16.0f;

					TR_(F3, out.pixels)[x + y*out.width] += sum;

				}
			}
		}

		ramR->final_image = image_alloc(arena, output_width, output_height, sizeof(I1));

		// Composit & Tonemap
		L1 in_ray    = ramR->ray_image.pixels;
		L1 in_bloom  = bloom_passes[0].pixels;
		out          = ramR->final_image.pixels;
		for EachIndex(pixel_index, output_width*output_height) {
			F3 ray_color = TR_(F3, in_ray)[0];
			F3 bloom_color = TR_(F3, in_bloom)[0];
			F3 color = ray_color * (1.0f - 0.5f*bloom_strength) + bloom_strength*bloom_color;
			// F3 color = F3_lerp(ray_color, 1-bloom_strength, bloom_color);
			// F3 color = ray_color + bloom_strength*bloom_color;

			F3 tonemapped_color = tonemap(color);

			F4 bmp_color = {
				255.0f*linear_to_srgb(tonemapped_color.x),
				255.0f*linear_to_srgb(tonemapped_color.y),
				255.0f*linear_to_srgb(tonemapped_color.z),
				255.0f
			};

			TR_(I1, out)[0] = 0xFF000000 |
												I1_(bmp_color.x) << 16 |
												I1_(bmp_color.y) << 8 |
												I1_(bmp_color.z) << 0;

			in_ray    += sizeof(F3);
			in_bloom  += sizeof(F3);
			out       += sizeof(I1);
		}
	}

	L1 memory_usage = TR_(Arena, arena)->pos;
	AtomicAddL1(&ramV->total_memory_usage, memory_usage);

	if (lane_idx() == 0) {
		image_write_to_file(ramR->final_image, "output.bmp");

		L1 end_time    = os_clock();
		L1 duration_ms = (end_time - ramR->start_time)/1000000;
		F1 duration_s  = F1_(duration_ms)/1000.0f;

		L1 kib_used = ramR->total_memory_usage / KiB(1);
		L1 mib_used = ramR->total_memory_usage / MiB(1);

		printf("[%.2fs] Done, Used %llu KiB (%llu MiB)\n", duration_s, kib_used, mib_used);
	}
}

#endif
