#ifndef HASH_H
#define HASH_H

// Portable, endian-stable cache keys. The 64-bit lanes use the xxHash64
// algorithm with independent seeds; this is intentionally non-cryptographic.

typedef union Hash128 Hash128;
union Hash128 {
  L1 l1[2];
  B1 bytes[16];
};

Internal Hash128 hash128(void *data, L1 size);

#endif
