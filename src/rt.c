// rt.c - Ray Tracer 

// Coordinate system:
// +x right, +y forwards, +z up

// TODO:
// Planes (finite)
// Optimize

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

L1 memmove(L1, L1, L1);

#define PI 3.141592653598979f

#endif

#if (CPU_ && RAM_)

L1 section_count;
L1 current_section_idx;

L1 start_time;
Image output_image;

#endif

#if (CPU_ && ROM_)

Inline L1 image_pixels_size(Image image) {
	return sizeof(I1) * image.width * image.height;
}

Inline Image image_alloc(L1 arena, I1 width, I1 height) {
	Image image  = {0};
	image.width  = width;
	image.height = height;
	image.pixels = arena_push(arena, image_pixels_size(image));
	return image;
}

Internal void image_write_to_file(Image image, CString filename) {
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

typedef struct Random_State Random_State;
struct Random_State {
	L1 state;
	L1 inc;
};

// TODO: Take as parameter to rand_pcg
ThreadLocal Random_State rng = {
	.state = 0,
	.inc = 109384234
};

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

Inline F1 clamp01(F1 x) { return Min(Max(x, 0.0f), 1.0f); }
Inline F1 saturate(F1 x) { return clamp01(x); }

#if 1
// optimized to not use trig functions
Inline F3 sample_cosine_hemisphere(F3 n) {
  F1 x, y;
  do {
    x = 2.0f * random_unilateral() - 1.0f;
    y = 2.0f * random_unilateral() - 1.0f;
  } while (x*x + y*y > 1.0f); // Reject if outside unit circle (accept ~78.5%)

  F1 z = sqrtf(1.0f - x*x - y*y); // Only 1 sqrt, no trig

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
#else
Inline F3 sample_cosine_hemisphere(F3 n) {
  F1 r1 = random_unilateral();
  F1 r2 = random_unilateral();

  F1 phi = 2.0f * PI * r1;
  F1 r = sqrtf(r2);
  F1 x = r * cosf(phi);
  F1 y = r * sinf(phi);
  F1 z = sqrtf(1.0f - r2);

  // build an ONB around n
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
#endif

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
  NoX = saturate(NoX);
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
  F1 NoH = saturate(F3_dot(n,h));
  F1 VoH = saturate(F3_dot(v,h));
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

			F1 roughness = Max(0.05f, clamp01(mat.roughness));
			F1 alpha = roughness * roughness;
			F1 metallic = clamp01(mat.metallic);

			F3 f0 = F3_lerp((F3){0.04f,0.04f,0.04f}, metallic, mat.base_color);

			F1 specular_prob = 0.5f; // keep constant; MIS handles weighting
			if (random_unilateral() < specular_prob) {
				F3 v = -ray_direction;
				F3 h = sample_GGX_half(next_normal, alpha);
				F3 l = F3_reflect(-v, h);

				F1 NoL = saturate(F3_dot(next_normal, l));
				F1 NoV = saturate(F3_dot(next_normal, v));
				F1 NoH = saturate(F3_dot(next_normal, h));
				F1 VoH = saturate(F3_dot(v, h));

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
				F1 NoL = saturate(F3_dot(next_normal, l));
				if (NoL > 0.0f) {
					F3 h = F3_normalize(v + l);
					F1 NoV = saturate(F3_dot(next_normal, v));
					F1 NoH = saturate(F3_dot(next_normal, h));
					F1 VoH = saturate(F3_dot(v, h));

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
					ray_direction = F3_normalize(l);
				} else {
					break; // absorb
				}
			}

			ray_origin = next_origin + next_normal * min_hit_distance;
		} else {
			F1 mult = 0.0f;
			F1 height = (ray_direction.z + 1) * 0.5;
			F3 sky_color = F3_lerp((F3){mult, mult, mult}, height, (F3){0.2f, 0.4f, 2.0f} * mult);
			result += attenuation * sky_color; 
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

Internal void lane(L1 arena) {
	I1 output_width = 640;
	I1 output_height = 480;
	L1 rays_per_pixel = 1024;
	L1 max_num_bounces = 8;

	if (lane_idx() == 0) {
		ramR->output_image = image_alloc(arena, output_width, output_height);
		ramR->start_time = os_clock();

		ramR->section_count = lane_count() * 8;
		ramR->current_section_idx = 0;
	}

	lane_sync();

	Material materials[] = {
		{ // ground
			.base_color  = (F3){0.33f, 0.33f, 0.13f},
			.metallic = 1.0f,
			.roughness = 0.4f,
		},
		{ // light
			.base_color  = (F3){1.0f, 1.0f, 1.0f},
			.emissive = (F3){1.0f, 1.0f, 1.0f},
			.metallic = 1.0f,
			.roughness = 0.8f,
		},
		{ // mirror
			.base_color  = (F3){0.9f, 1.0f, 0.9f},
			.metallic = 1.0f,
			.roughness = 0.40f,
		},
		{ // right
			.base_color  = (F3){0.1f, 1.0f, 0.9f},
			.metallic = 0.0f,
			.roughness = 0.01f,
		},
		{ // center
			.base_color  = (F3){1.0f, 1.0f, 1.0f},
			.metallic = 0.1f,
			.roughness = 0.01f,
		},
	};

	Plane planes[] = {
		{
			.n = (F3){0, 0, 1},
			.material_idx = 0
		}
	};

	Sphere spheres[] = {
		{
			.p = (F3){-1.5f, 0.0f, 1.5f},
			.r = 1.0f,
			.material_idx = 2,
		},
		{
			.p = (F3){1.5f, 0.0f, 1.5f},
			.r = 1.0f,
			.material_idx = 3,
		},
		/*{
			.p = (F3){0.0f, 0.0f, 1.0f},
			.r = 1.0f,
			.material_idx = 4,
		},*/
	};

	Box boxes[] = {
		{
			.min = {-3.0f, 1.8f, 5.7f},
			.max = {3.0f, -1.8f, 5.8f},
			.material_idx = 1,
		},
		{
			.min = {-3.0f, -2.0f, 0.0f},
			.max = {3.0f, 2.0f, 0.5f},
			.material_idx = 4,
		}
	};

	F3 camera_p = (F3){-4.0f, -7.0f, 3.5f};
	F3 camera_z = F3_normalize(camera_p - (F3){0, 0, 1.0f});
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

	L1 section_num_pixels = output_width*output_height/ramR->section_count;
	L1 section_pixels = arena_push(arena, sizeof(I1)* section_num_pixels);

	while (ramR->current_section_idx < ramR->section_count) {
		L1 my_section_idx = AtomicAddL1(&ramV->current_section_idx, 1);
		if (my_section_idx >= ramR->section_count) break;
		Range section_pixels_range = {
			.min = section_num_pixels * my_section_idx,
			.max = section_num_pixels * (my_section_idx + 1)
		};

		L1 out = section_pixels;
		for EachInRange(pixel_index, section_pixels_range) {
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

				F3 ray_origin = camera_p;
				F3 ray_direction = F3_normalize(film_p - camera_p);

				color += ray_cast(world, ray_origin, ray_direction, max_num_bounces) * contrib;
			}

			F4 bmp_color = {
				255.0f*linear_to_srgb(color.x),
				255.0f*linear_to_srgb(color.y),
				255.0f*linear_to_srgb(color.z),
				255.0f
			};

			TR_(I1, out)[0] = 0xFF000000 |
												I1_(bmp_color.x) << 16 |
												I1_(bmp_color.y) << 8 |
												I1_(bmp_color.z) << 0;
			out += sizeof(I1);
		}

		L1 dest = ramR->output_image.pixels + sizeof(I1)*section_pixels_range.min;
		memmove(dest, section_pixels, sizeof(I1)*section_num_pixels);
	}

	L1 lane_end_time    = os_clock();
	L1 lane_duration_ms = (lane_end_time - lane_begin_time)/1000000;
	F1 lane_duration_s  = F1_(lane_duration_ms)/1000.0f;
	printf("Lane %llu - %.2fs\n", lane_idx(), lane_duration_s);

	lane_sync();

	if (lane_idx() == 0) {
		image_write_to_file(ramR->output_image, "output.bmp");

		L1 end_time    = os_clock();
		L1 duration_ms = (end_time - ramR->start_time)/1000000;
		F1 duration_s  = F1_(duration_ms)/1000.0f;
		printf("Total time: %.2fs\n", duration_s);
	}
}

#endif
