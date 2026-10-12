#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

// Writing can only happen from the editor thread. For now.

typedef Hash128 IC_Key;

typedef struct IC_Version IC_Version;
struct IC_Version
{
  IC_Version *next;
  Arena      *arena;
  Image       image;
  L1          ref_count;
};

typedef struct IC_Ref IC_Ref;
struct IC_Ref
{
  IC_Version *version;
  Image       image;
};

typedef struct IC_Node IC_Node;
struct IC_Node
{
  IC_Node    *next;
  IC_Key      key;
  L1          generation;
  IC_Version *version;
};

typedef struct IC_State IC_State;
struct IC_State
{
  Arena      *arena;
  L1          generation;
  L1          acquired_count;
  IC_Node    *slots[256];
  IC_Node    *free_nodes;
  IC_Version *free_versions;
};

Internal void   ic_init(void);
Internal IC_Key ic_key_from_string(String8 string);
Internal Image  ic_image(IC_Key key);
Internal void   ic_set(IC_Key key, Arena **arena, Image image);
Internal void   ic_changed(IC_Key key);
Internal L1     ic_generation(IC_Key key);
Internal void   ic_release(IC_Key key);
Internal IC_Ref ic_acquire(IC_Key key);
Internal void   ic_drop(IC_Ref *ref);

#endif
