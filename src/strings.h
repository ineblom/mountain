#ifndef STRINGS_H
#define STRINGS_H

typedef const char * CString;

typedef struct String8 String8;
struct String8 {
  B1 *str;
  L1 len;
};

typedef struct String8_Array String8_Array;
struct String8_Array {
  String8 *v;
  L1 count;
  L1 total_length;
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
  L1 node_count;
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
