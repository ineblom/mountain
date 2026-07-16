
#if (HEADER)

typedef const char * CString;

typedef struct String8 String8;
struct String8 {
  B1 *str;
  L1 len;
};

typedef struct String8_Node String8_Node;
struct String8_Node {
  String8_Node *next;
  String8_Node *prev;
  String8 value;
};

typedef struct String8_List String8_List;
struct String8_List {
  String8_Node *first;
  String8_Node *last;
  L1 total_length;
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

#define str8(x) (String8){ .str = (B1 *)x, .len = sizeof(x) - 1 }

#endif

#if (SOURCE)

////////////////////////////////
//~ kti: String <-> Integer Tables

Read_Only Global B1 integer_symbols[16] = {
  '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',
};

// NOTE(kti): Includes reverses for uppercase and lowercase hex.
Read_Only Global B1 integer_symbol_reverse[128] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

Read_Only Global B1 base64[64] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '_', '$',
};

Read_Only Global B1 base64_reverse[128] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
  0xFF,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,
  0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0xFF,0xFF,0xFF,0xFF,0x3E,
  0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
  0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0xFF,0xFF,0xFF,0xFF,0xFF,
};

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

Internal I1 char_is_space(char c) {
  I1 result = (c == ' ' ||  c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
  return result;
}

Internal I1 char_is_upper(B1 c) {
  I1 result = ('A' <= c && c <= 'Z');
  return result;
}

Internal I1 char_is_lower(B1 c) {
  I1 result = ('a' <= c && c <= 'z');
  return result;
}

Internal I1 char_is_alpha(B1 c) {
  I1 result = (char_is_upper(c) || char_is_lower(c));
  return result;
}

Internal I1 char_is_slash(B1 c) {
  I1 result = (c == '/' || c == '\\');
  return result;
}

Internal I1 char_is_digit(B1 c, I1 base) {
  I1 result = 0;
  if(0 < base && base <= 16) {
    B1 val = integer_symbol_reverse[c];
    if(val < base) {
      result = 1;
    }
  }
  return result;
}

Internal B1 lower_from_char(B1 c) {
  if(char_is_upper(c)) {
    c += ('a' - 'A');
  }
  return c;
}

Internal B1 upper_from_char(B1 c) {
  if(char_is_lower(c)) {
    c += ('A' - 'a');
  }
  return c;
}

Internal B1 correct_slash_from_char(B1 c) {
  if (char_is_slash(c)) {
    c = '/';
  }
  return c;
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

  for (L1 i = 0; i < string.len; i += 1) {
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

Internal I1 try_F1_from_str8(String8 string, F1 *out) {
  B1 *ptr = string.str;
  B1 *opl = string.str + string.len;

  while (ptr < opl && char_is_space(ptr[0]))  { ptr += 1; }
  while (opl > ptr && char_is_space(opl[-1])) { opl -= 1; }

  D1 sign = 1.0;
  if (ptr < opl && (ptr[0] == '+' || ptr[0] == '-')) {
    sign = (ptr[0] == '-') ? -1.0 : 1.0;
    ptr += 1;
  }

  D1 mantissa = 0.0;
  SL1 exponent = 0;
  L1 digit_count = 0;
  for (; ptr < opl && '0' <= ptr[0] && ptr[0] <= '9'; ptr += 1) {
    mantissa = mantissa*10.0 + (D1)(ptr[0] - '0');
    digit_count += 1;
  }
  if (ptr < opl && ptr[0] == '.') {
    ptr += 1;
    for (; ptr < opl && '0' <= ptr[0] && ptr[0] <= '9'; ptr += 1) {
      mantissa = mantissa*10.0 + (D1)(ptr[0] - '0');
      exponent -= 1;
      digit_count += 1;
    }
  }

  I1 is_valid = (digit_count > 0);

  if (is_valid && ptr < opl && (ptr[0] == 'e' || ptr[0] == 'E')) {
    ptr += 1;
    SL1 exp_sign = 1;
    if (ptr < opl && (ptr[0] == '+' || ptr[0] == '-')) {
      exp_sign = (ptr[0] == '-') ? -1 : 1;
      ptr += 1;
    }
    L1 exp_digit_count = 0;
    SL1 exp_value = 0;
    for (; ptr < opl && '0' <= ptr[0] && ptr[0] <= '9'; ptr += 1) {
      exp_value = Min(exp_value*10 + (ptr[0] - '0'), 512);
      exp_digit_count += 1;
    }
    exponent += exp_sign*exp_value;
    is_valid = (exp_digit_count > 0);
  }

  is_valid = is_valid && (ptr == opl);

  if (is_valid && mantissa != 0.0) {
    LocalPersist D1 pow10[9] = { 1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256 };
    SL1 e = Clamp(-511, exponent, 511);
    D1 scale = 1.0;
    for (L1 bit = 0, n = (e < 0) ? -e : e; n > 0; n >>= 1, bit += 1) {
      if (n & 1) {
        scale *= pow10[bit];
      }
    }
    mantissa = (e < 0) ? mantissa/scale : mantissa*scale;
    is_valid = (mantissa <= F1_MAX);
  }

  if (is_valid) {
    out[0] = (F1)(sign*mantissa);
  }

  return is_valid;
}

Internal F1 F1_from_str8(String8 string) {
  F1 result = 0;
  try_F1_from_str8(string, &result);
  return result;
}

Internal I1 str8_match(String8 a, String8 b) {
  L1 result = 0;

  if (a.len == b.len) {
    result = (memcmp(a.str, b.str, a.len) == 0);
  }

  return result;
}

Internal String8 str8_prefix(String8 str, L1 len) {
  str.len = Min(len, str.len);
  return str;
}

Internal String8 str8_substr(String8 str, L1 min, L1 max) {
  String8 result = str;
  min = Min(min, str.len);
  max = Min(max, str.len);
  result.str += min;
  result.len = max - min;
  return result;
}

Internal String8_Node *str8_list_push(Arena *arena, String8_List *list, String8 str) {
  String8_Node *node = push_array(arena, String8_Node, 1);
  node->value = str;
  DLLPushBack(list->first, list->last, node);
  list->total_length += str.len;
  return node;
}

Internal String8_Node *str8_list_pushf(Arena *arena, String8_List *list, CString fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 string = str8fv(arena, fmt, args);
  va_end(args);

  String8_Node *node = str8_list_push(arena, list, string);
  return node;
}

Internal String8 str8_list_join(Arena *arena, String8_List *list) {
  String8 result = {0};

  result.str = push_array(arena, B1, list->total_length);
  for (String8_Node *n = list->first; n != 0; n = n->next) {
    memmove(result.str+result.len, n->value.str, n->value.len);
    result.len += n->value.len;
  }

  return result;
}


////////////////////////////////
//~ kti: UTF-8 Decode

Internal I1 utf8_byte_is_continuation(B1 byte) {
  I1 result = ((byte & 0xC0) == 0x80);
  return result;
}

Internal L1 utf8_boundary_left_from_column(String8 string, L1 column) {
  L1 result = Clamp(0, column, string.len);
  if (result > 0) {
    do {
      result -= 1;
    } while (result > 0 && utf8_byte_is_continuation(string.str[result]));
  }
  return result;
}

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

Internal L1 utf8_boundary_right_from_column(String8 string, L1 column) {
  L1 result = Clamp(0, column, string.len);
  if (result < string.len) {
    Unicode_Decode decode = utf8_decode(string.str+result, string.len-result);
    result += decode.inc;
    while (result < string.len && utf8_byte_is_continuation(string.str[result])) {
      result += 1;
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
