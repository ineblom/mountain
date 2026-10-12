#ifndef TEXTURE_CACHE_H
#define TEXTURE_CACHE_H

typedef Hash128 TC_Key;
typedef struct TC_Node TC_Node;
struct TC_Node
{
  TC_Node     *next;
  TC_Key       key;
  GFX_Texture *texture;
};

typedef struct TC_State TC_State;
struct TC_State
{
  Arena   *arena;
  TC_Node *slots[256];
  TC_Node *free_nodes;
};

Internal void         tc_init(void);
Internal TC_Key       tc_key_from_string(String8 string);
Internal GFX_Texture *tc_texture(TC_Key key);
Internal void         tc_set(TC_Key key, Image_RGBA8 image);
Internal void         tc_release(TC_Key key);

#endif
