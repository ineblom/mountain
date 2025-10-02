
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