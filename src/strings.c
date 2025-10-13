
#if (CPU_ && TYP_)

typedef const char *Restrict CString;
#define CString_(x) ((CString)x)

typedef struct String8 String8;
struct String8 {
  L1 str;
  L1 len;
};

#define Str8_(x) (String8){ .str = L1_(x), .len = sizeof(x) - 1 }

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

#endif