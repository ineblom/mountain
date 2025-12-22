
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

////////////////////////////////
//~ kti: UTF-8 Decode

// TODO: Simplify
Internal Unicode_Decode utf8_decode(B1 *str, L1 max) {
  LocalPersist B1 utf8_class[32] =
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
    5,
  };

  Unicode_Decode result = { 1, I1_MAX };
  B1 byte = str[0];
  B1 byte_class = utf8_class[byte >> 3];
  switch (byte_class) {
    case 1: {
      result.codepoint = byte;
    } break;
    case 2: {
      if (1 < max) {
        B1 cont_byte = str[1];
        if (utf8_class[cont_byte >> 3] == 0) {
          result.codepoint = (byte & bitmask5) << 6;
          result.codepoint |= (cont_byte & bitmask6);
          result.inc = 2;
        }
      }
    } break;
    case 3: {
      if (2 < max) {
        B1 cont_byte[2] = { str[1], str[2] };
        if (utf8_class[cont_byte[0] >> 3] == 0 &&
            utf8_class[cont_byte[1] >> 3] == 0) {
          result.codepoint = (byte & bitmask4) << 12;
          result.codepoint |= (cont_byte[0] & bitmask6) << 6;
          result.codepoint |= (cont_byte[1] & bitmask6);
          result.inc = 3;
        }
      }
    } break;
    case 4: {
      if (3 < max) {
        B1 cont_byte[3] = { str[1], str[2], str[3]};
        if (utf8_class[cont_byte[0] >> 3] == 0 &&
            utf8_class[cont_byte[1] >> 3] == 0 &&
            utf8_class[cont_byte[2] >> 3] == 0) {
          result.codepoint = (byte & bitmask3) << 18;
          result.codepoint |= (cont_byte[0] & bitmask6) << 12;
          result.codepoint |= (cont_byte[1] & bitmask6) << 6;
          result.codepoint |= (cont_byte[2] & bitmask6);
          result.inc = 4;
        }
      }
    } break;
  }
  return result;
}

////////////////////////////////
//~ kti: String 32

Internal String32 str32_from_8(Arena *arena, String8 in) {
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
