#if (HEADER)

// Portable, endian-stable cache keys. The 64-bit lanes use the xxHash64
// algorithm with independent seeds; this is intentionally non-cryptographic.

typedef union Hash128 Hash128;
union Hash128 {
  L1 l1[2];
  B1 bytes[16];
};

Internal Hash128 hash128(void *data, L1 size);

#endif

#if (SOURCE)

#define HASH_XX_P1 11400714785074694791ULL
#define HASH_XX_P2 14029467366897019727ULL
#define HASH_XX_P3  1609587929392839161ULL
#define HASH_XX_P4  9650029242287828579ULL
#define HASH_XX_P5  2870177450012600261ULL

Inline L1 hash_rotate_left(L1 x, I1 amount) {
  return (x << amount) | (x >> (64 - amount));
}

Inline I1 hash_read_u32_le(B1 const *p) {
  return ((I1)p[0] << 0) | ((I1)p[1] << 8) | ((I1)p[2] << 16) | ((I1)p[3] << 24);
}

Inline L1 hash_read_u64_le(B1 const *p) {
  return (L1)hash_read_u32_le(p) | ((L1)hash_read_u32_le(p + 4) << 32);
}

Inline L1 hash_xx_round(L1 accumulator, L1 input) {
  accumulator += input * HASH_XX_P2;
  accumulator = hash_rotate_left(accumulator, 31);
  accumulator *= HASH_XX_P1;
  return accumulator;
}

Inline L1 hash_xx_merge_round(L1 accumulator, L1 value) {
  accumulator ^= hash_xx_round(0, value);
  accumulator = accumulator * HASH_XX_P1 + HASH_XX_P4;
  return accumulator;
}

Internal L1 hash64_seed(void *data, L1 size, L1 seed) {
  LocalPersist B1 empty;
  B1 const *at = data != 0 ? (B1 const *)data : &empty;
  B1 const *end = at + size;
  L1 result;

  if (size >= 32) {
    B1 const *block_end = end - 32;
    L1 v1 = seed + HASH_XX_P1 + HASH_XX_P2;
    L1 v2 = seed + HASH_XX_P2;
    L1 v3 = seed;
    L1 v4 = seed - HASH_XX_P1;
    do {
      v1 = hash_xx_round(v1, hash_read_u64_le(at + 0));
      v2 = hash_xx_round(v2, hash_read_u64_le(at + 8));
      v3 = hash_xx_round(v3, hash_read_u64_le(at + 16));
      v4 = hash_xx_round(v4, hash_read_u64_le(at + 24));
      at += 32;
    } while (at <= block_end);

    result = hash_rotate_left(v1, 1) + hash_rotate_left(v2, 7) +
             hash_rotate_left(v3, 12) + hash_rotate_left(v4, 18);
    result = hash_xx_merge_round(result, v1);
    result = hash_xx_merge_round(result, v2);
    result = hash_xx_merge_round(result, v3);
    result = hash_xx_merge_round(result, v4);
  } else {
    result = seed + HASH_XX_P5;
  }

  result += size;
  while (at + 8 <= end) {
    L1 lane = hash_xx_round(0, hash_read_u64_le(at));
    result ^= lane;
    result = hash_rotate_left(result, 27) * HASH_XX_P1 + HASH_XX_P4;
    at += 8;
  }
  if (at + 4 <= end) {
    result ^= (L1)hash_read_u32_le(at) * HASH_XX_P1;
    result = hash_rotate_left(result, 23) * HASH_XX_P2 + HASH_XX_P3;
    at += 4;
  }
  while (at < end) {
    result ^= (L1)*at * HASH_XX_P5;
    result = hash_rotate_left(result, 11) * HASH_XX_P1;
    at += 1;
  }

  result ^= result >> 33;
  result *= HASH_XX_P2;
  result ^= result >> 29;
  result *= HASH_XX_P3;
  result ^= result >> 32;
  return result;
}

Internal Hash128 hash128(void *data, L1 size) {
  Hash128 result;
  result.l1[0] = hash64_seed(data, size, 0);
  result.l1[1] = hash64_seed(data, size, HASH_XX_P1);
  return result;
}

#undef HASH_XX_P1
#undef HASH_XX_P2
#undef HASH_XX_P3
#undef HASH_XX_P4
#undef HASH_XX_P5

#endif
