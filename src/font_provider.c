#if (CPU_ && DEF_)
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#if (CPU_ && TYP_)

typedef struct FP_Font FP_Font;
struct FP_Font {
  FT_Face face;
};

typedef struct FP_State FP_State;
struct FP_State {
  Arena *arena;

  FT_Library library;
};

#endif

#if (CPU_ && ROM_)

Global FP_State *fp_state = 0;

Internal void fp_init() {
  Arena *arena = arena_create(MiB(64));
  fp_state = push_array(arena, FP_State, 1);
  fp_state->arena = arena;

  FT_Error err = FT_Init_FreeType(&fp_state->library);
  Assert(err == 0);
}

Internal FP_Font fp_font_open(String8 path) {
  Temp_Arena scratch = scratch_begin(0, 0);
  String8 path_copy = push_str8_copy(scratch.arena, path);
  FT_Face face;
  FT_Error err = FT_New_Face(fp_state->library, (CString)path_copy.str, 0, &face);
  Assert(err == 0);
  scratch_end(scratch);

  FP_Font result = {
    .face = face,
  };
  return result;
}

Internal void fp_font_close(FP_Font font) {
  if (font.face != 0) {
    FT_Done_Face(font.face);
  }
}

Internal void fp_raster(Arena *arena, FP_Font font, F1 size, String8 string) {
  Temp_Arena scratch = scratch_begin(&arena, 1);

  //- kti: Unpack font
  FT_Face face = font.face;
  FT_Set_Pixel_Sizes(face, 0, (FT_UInt)((96.0f/72.0f) * size));
  SL1 ascent = face->size->metrics.ascender >> 6;
  SL1 descent = abs_SL1(face->size->metrics.descender >> 6);
  SL1 height = face->size->metrics.height >> 6;

  //- kti: Unpack string

  scratch_end(scratch);
}

#endif
