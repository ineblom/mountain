
#if (CPU_) && (TYP_)

F1 sqrtf(F1);
F1 sinf(F1);
F1 cosf(F1);
F1 fabsf(F1);
F1 powf(F1, F1);

#define PI 3.141592653598979f
#define GOLDEN_RATIO 1.61803398875f

#endif

#if (CPU_) && (ROM_)

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

Inline F3 F3_reflect(F3 v, F3 normal) {
	F3 result = v - 2*F3_dot(v, normal) * normal;
	return result;
}

Internal F1 ray_aabb_intersect(F3 ray_origin, F3 ray_direction, F3 aabb_min, F3 aabb_max) {
	F1 t_min = (aabb_min.x - ray_origin.x) / ray_direction.x;
	F1 t_max = (aabb_max.x - ray_origin.x) / ray_direction.x;
	if (t_min > t_max) Swap(t_min, t_max);

	F1 ty_min = (aabb_min.y - ray_origin.y) / ray_direction.y;
	F1 ty_max = (aabb_max.y - ray_origin.y) / ray_direction.y;
	if (ty_min > ty_max) Swap(ty_min, ty_max);

	t_min = Max(t_min, ty_min);
	t_max = Min(t_max, ty_max);

	if (t_min > t_max) return 0.0f;

	F1 tz_min = (aabb_min.z - ray_origin.z) / ray_direction.z;
	F1 tz_max = (aabb_max.z - ray_origin.z) / ray_direction.z;
	if (tz_min > tz_max) Swap(tz_min, tz_max);

	t_min = Max(t_min, tz_min);
	t_max = Min(t_max, tz_max);

	if (t_min > t_max) return 0.0f;

	return t_min;
}

#endif