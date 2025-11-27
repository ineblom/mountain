
#if (CPU_ && TYP_)

typedef const char *Restrict CString;
#define CString_(x) ((CString)x)

typedef struct String8 String8;
struct String8 {
  B1 *str;
  L1 len;
};

#define Str8_(x) (String8){ .str = (B1 *)x, .len = sizeof(x) - 1 }

#endif

#if (CPU_ && ROM_)

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

#endif