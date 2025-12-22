#if (CPU_ && DEF_)
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#if (CPU_ && TYP_)

typedef struct FP_Font FP_Font;
struct FP_Font {
  FT_Face face;
};

typedef struct FP_Raster_Result FP_Raster_Result;
struct FP_Raster_Result {
	SW2 atlas_dim;
	B1 *atlas;
	F1 advance;
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

Internal FP_Raster_Result fp_raster(Arena *arena, FP_Font font, F1 size, String8 string) {
	FP_Raster_Result result = {0};
	
  Temp_Arena scratch = scratch_begin(&arena, 1);

  //- kti: Unpack font
  FT_Face face = font.face;
	if (face->num_fixed_sizes > 0) {
		Assert(FT_Select_Size(face, 0) == 0);
	} else {
		Assert(FT_Set_Pixel_Sizes(face, 0, (FT_UInt)((96.0f/72.0f) * size)) == 0);
	}
  SL1 ascent = face->size->metrics.ascender >> 6;
  SL1 descent = abs_SL1(face->size->metrics.descender >> 6);
  SL1 height = face->size->metrics.height >> 6;

  //- kti: Unpack string
  String32 string32 = str32_from_str8(scratch.arena, string);

  //- kti: Measure
  SI1 total_width = 0;
  for EachIndex(i, string32.len) {
    FT_Load_Char(face, string32.str[i], FT_LOAD_RENDER);
    total_width += face->glyph->advance.x >> 6;
  }

  //- kti: Allocate & fill atlas w/rasterization
  SW2 dim = { (SW1)total_width+1, height+1 };
  L1 atlas_size = dim.x * dim.y * 4;
  B1 *atlas = push_array(arena, B1, atlas_size);
  SI1 baseline = ascent;
  SI1 atlas_write_x = 0;
  for EachIndex(i, string32.len) {
    FT_Load_Char(face, string32.str[i], FT_LOAD_RENDER);

		FT_Bitmap *bmp = &face->glyph->bitmap;
		SI1 top = face->glyph->bitmap_top;
		SI1 left = face->glyph->bitmap_left;

		for (SI1 row = 0; row < (SI1)bmp->rows; row += 1) {
			SI1 y = baseline - top + row;
			for (SI1 col = 0; col < (SI1)bmp->width; col += 1) {
				SI1 x = atlas_write_x + left + col;
				if (x >= 0 && x < dim.x && y >= 0 && y < dim.y) {
					B1 alpha = 0;

					if (bmp->pixel_mode == FT_PIXEL_MODE_MONO) {
						B1 byte = bmp->buffer[row*bmp->pitch + (col >> 3)];
						B1 bit = (byte >> (7 - (col & 7))) & 1;
						alpha = bit ? 255 : 0;
					} else {
						alpha = bmp->buffer[row*bmp->pitch + col];
					}

					L1 off = (y*dim.x + x)*4;
					atlas[off+0] = 255;
					atlas[off+1] = 255;
					atlas[off+2] = 255;
					atlas[off+3] = alpha;
				}
			}
		}
		atlas_write_x += (face->glyph->advance.x >> 6);
  }

	result.atlas_dim = dim;
	result.atlas = atlas;
	result.advance = (F1)total_width;
  scratch_end(scratch);

	return result;
}

#endif
