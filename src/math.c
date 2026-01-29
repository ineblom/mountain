
#if (CPU_) && (TYP_)

#define PI 3.141592653598979f
#define GOLDEN_RATIO 1.61803398875f

typedef struct Random_State Random_State;
struct Random_State {
	L1 state;
	L1 inc;
};

#endif

#if (CPU_) && (ROM_)

#define abs_SL1(x) llabs(x)
#define mod_F1(x, y) fmodf(x, y)
#define floor_F1(x) floorf(x)
#define round_F1(x) roundf(x)
#define ceil_F1(x) ceilf(x)

Inline F1 clamp01_F1(F1 x) { return Min(Max(x, 0.0f), 1.0f); }
Inline F1 saturate_F(F1 x) { return clamp01_F1(x); }

Inline F1 sign_F1(F1 a) {
	F1 result = -1.0f;
	if (a > 0.0f) result = 1.0f;
	return result;
}

Inline F1 dot_F3(F3 a, F3 b) {
	F1 result = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
	return result;
}

Inline F1 length_sq_F3(F3 v) {
	F1 result = dot_F3(v, v);
	return result;
}

Inline F1 length_F3(F3 v) {
	F1 result = sqrtf(length_sq_F3(v));
	return result;
}

Inline F3 normalize_F3(F3 v) {
	F3 result = v * (1.0f / length_F3(v));
	return result;
}

Inline F3 cross_F3(F3 a, F3 b) {
	F3 result = {0};

	result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];

	return result;
}

Inline F3 lerp_F3(F3 a, F1 t, F3 b) {
	F3 result = a * (1-t) + b*t;
	return result;
}

Inline F3 abs_F3(F3 v) {
	F3 result = { fabsf(v[0]), fabsf(v[1]), fabsf(v[2]) };
	return result;
}

Inline F3 clamp01_F3(F3 v) {
	F3 result= {
		clamp01_F1(v[0]),
		clamp01_F1(v[1]),
		clamp01_F1(v[2])
	};
	return result;
}

Inline F1 luminance_F3(F3 v) {
	F1 result = 0.2126f*v[0] + 0.7152f*v[1] + 0.0722f*v[2];
	return result;
}

Inline F3 pow_F3(F3 v, F1 exp) {
	F3 result = {
		powf(v[0], exp),
		powf(v[1], exp),
		powf(v[2], exp)
	};
	return result;
}

Inline F3 reflect_F3(F3 v, F3 normal) {
	F3 result = v - 2*dot_F3(v, normal) * normal;
	return result;
}

Internal F1 ray_aabb_intersect(F3 ray_origin, F3 ray_direction, F3 aabb_min, F3 aabb_max) {
	F1 t_min = (aabb_min[0] - ray_origin[0]) / ray_direction[0];
	F1 t_max = (aabb_max[0] - ray_origin[0]) / ray_direction[0];
	if (t_min > t_max) Swap(t_min, t_max);

	F1 ty_min = (aabb_min[1] - ray_origin[1]) / ray_direction[1];
	F1 ty_max = (aabb_max[1] - ray_origin[1]) / ray_direction[1];
	if (ty_min > ty_max) Swap(ty_min, ty_max);

	t_min = Max(t_min, ty_min);
	t_max = Min(t_max, ty_max);

	if (t_min > t_max) return 0.0f;

	F1 tz_min = (aabb_min[2] - ray_origin[2]) / ray_direction[2];
	F1 tz_max = (aabb_max[2] - ray_origin[2]) / ray_direction[2];
	if (tz_min > tz_max) Swap(tz_min, tz_max);

	t_min = Max(t_min, tz_min);
	t_max = Min(t_max, tz_max);

	if (t_min > t_max) return 0.0f;

	return t_min;
}

Inline I1 rand_pcg(Random_State *rng) {
	L1 oldstate = rng->state;
  rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
  I1 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
  I1 rot = oldstate >> 59u;
  I1 result = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
  return result;
}

Inline F1 random_unilateral(Random_State *rng) {
	F1 result = (F1)rand_pcg(rng) / (F1)I1_MAX;
	return result;
}

Inline F1 random_bilateral(Random_State *rng) {
	F1 result = -1 + 2 * random_unilateral(rng);
	return result;
}

Inline F4 intersect_rects(F4 a, F4 b) {
	F4 result = {0};

	result[0] = Max(a[0], b[0]);
	result[1] = Max(a[1], b[1]);
	result[2] = Min(a[0]+a[2], b[0]+b[2]) - result[0];
	result[3] = Min(a[1]+a[3], b[1]+b[3]) - result[1];

	return result;
}

Inline I1 rect_contains(F4 rect, F2 point) {
	I1 result = 0;

	if (point[0] >= rect[0] &&
			point[1] >= rect[1] &&
			point[0] <= rect[0]+rect[2] &&
			point[1] <= rect[1]+rect[3]) {
		result = 1;
	}

	return result;
}

#endif
