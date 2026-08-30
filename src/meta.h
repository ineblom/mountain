#ifndef META_H
#define META_H

#define MAX_PARAM_COUNT 256

typedef struct Line Line;
struct Line {
  Line *next;
  Line *prev;

  String8 str;
  L1 param_count;
  String8 params[MAX_PARAM_COUNT];
};

typedef struct Line_List Line_List;
struct Line_List {
  Line *first;
  Line *last;
  L1 count;
};

#endif
