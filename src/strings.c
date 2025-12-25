
#if (CPU_ && TYP_)

typedef const char * CString;

typedef struct String8 String8;
struct String8 {
  B1 *str;
  L1 len;
};

typedef struct String16 String16;
struct String16 {
  W1 *str;
  L1 len;
};

typedef struct String32 String32;
struct String32 {
  I1 *str;
  L1 len;
};

typedef struct Unicode_Decode Unicode_Decode;
struct Unicode_Decode {
  I1 inc;
  I1 codepoint;
};

#define Str8_(x) (String8){ .str = (B1 *)x, .len = sizeof(x) - 1 }

#endif

#if (CPU_ && ROM_)

////////////////////////////////
//~ kti: C String

Internal L1 cstr_len(CString str) {
  L1 result = 0;

  while (*str != 0) {
    result += 1;
    str += 1;
  }

  return result;
}

Internal I1 cstr_compare(CString a, CString b) {
  L1 result = 0;

  while (*a && (*a == *b)) {
    a += 1;
    b += 1;
  }

  return *a == *b;
}

////////////////////////////////
//~ kti: String 8

Internal String8 str8_concat(Arena *arena, String8 a, String8 b) {
  String8 result = {0};
  result.len = a.len + b.len;
  result.str = push_array(arena, B1, result.len);

  memmove(result.str,       a.str, a.len);
  memmove(result.str+a.len, b.str, b.len);

  return result;
}

Internal String8 str8fv(Arena *arena, CString fmt, va_list args) {
  va_list args2;
  va_copy(args2, args);

  String8 result = {0};
  L1 bytes_needed = vsnprintf(0, 0, fmt, args)+1;
  result.str = push_array(arena, B1, bytes_needed);
  result.len = vsnprintf((SB1 *)result.str, bytes_needed, fmt, args2);

  va_end(args2);

  return result;
}

Internal String8 str8f(Arena *arena, CString fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 result = str8fv(arena, fmt, args);
  va_end(args);

  return result;
}

Internal L1 str8_hash(String8 string) {
  L1 hash = 5281;

  for EachIndex(i, string.len) {
    I1 c = string.str[i];
    hash = ((hash << 5) + hash) + c;
  }

  return hash;
}

Internal String8 push_str8_copy(Arena *arena, String8 str) {
  String8 new;
  new.len = str.len;
  new.str = push_array_no_zero(arena, B1, str.len+1);
  memmove(new.str, str.str, str.len);
  new.str[str.len] = 0;
  return new;
}

Internal I1 str8_match(String8 a, String8 b) {
	L1 result = 0;

	if (a.len == b.len) {
		result = (memcmp(a.str, b.str, a.len) == 0);
	}

	return result;
}

Internal String8 str8_substr(String8 str, L1 min, L1 max) {
	String8 result = str;
	min = Min(min, str.len);
	max = Min(max, str.len);
	result.str += min;
	result.len = max - min;
	return result;
}

////////////////////////////////
//~ kti: UTF-8 Decode

Internal Unicode_Decode utf8_decode(B1 *str, L1 cap) {
  LocalPersist B1 length[] =
  {
    1,1,1,1,
    1,1,1,1,
    1,1,1,1,
    1,1,1,1,
    0,0,0,0,
    0,0,0,0,
    2,2,2,2,
    3,3,
    4,
    0,
  };
  LocalPersist B1 first_byte_mask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
  LocalPersist B1 final_shift[] = { 0, 18, 12, 6, 0 };

  Unicode_Decode result = { 1, I1_MAX };
  if (cap > 0) {
    B1 byte = str[0];
    B1 l = length[byte >> 3];
    if (l > 0 && l <= cap) {
      I1 cp = (byte & first_byte_mask[l]) << 18;
      switch (l) {
        case 4: cp |= ((str[3] & 0x3F) << 0);
        case 3: cp |= ((str[2] & 0x3F) << 6);
        case 2: cp |= ((str[1] & 0x3F) << 12);
        default: cp >>= final_shift[l];
      }

      result.codepoint = cp;
      result.inc = l;
    }
  }
  return result;
}

////////////////////////////////
//~ kti: String 32

Internal String32 str32_from_str8(Arena *arena, String8 in) {
  String32 result = {};
  if (in.len > 0) {
    L1 cap = in.len;
    I1 *str = push_array_no_zero(arena, I1, cap + 1);
    B1 *ptr = in.str;
    B1 *opl = ptr + in.len;
    L1 len = 0;
    Unicode_Decode consume;
    for (; ptr < opl; ptr += consume.inc) {
      consume = utf8_decode(ptr, opl - ptr);
      str[len] = consume.codepoint;
      len += 1;
    }
    str[len] = 0; 
    arena_pop(arena, (cap - len)*4);
    result.str = str;
    result.len = len;
  }
  return result;
}

#endif
