#if (DEF_)
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#if (TYP_)

typedef struct FP_Handle FP_Handle;
struct FP_Handle {
  FT_Face face;
};

typedef struct FP_Raster_Result FP_Raster_Result;
struct FP_Raster_Result {
	SW2 atlas_dim;
	B1 *atlas;
	F1 advance;
};

typedef struct FP_Metrics FP_Metrics;
struct FP_Metrics {
	F1 design_units_per_em;
	F1 ascent;
	F1 descent;
	F1 line_gap;
	F1 capital_height;
};

typedef struct FP_State FP_State;
struct FP_State {
  Arena *arena;

  FT_Library library;
};

#endif

#if (ROM_)

Global FP_State *fp_state = 0;

Inline FP_Handle fp_handle_zero(void) {
	FP_Handle result = {0};
	return result;
}

Inline I1 fp_handle_match(FP_Handle a, FP_Handle b) {
	I1 result = (a.face == b.face);
	return result;
}

Internal void fp_init(void) {
  Arena *arena = arena_create(MiB(64));
  fp_state = push_array(arena, FP_State, 1);
  fp_state->arena = arena;

  FT_Error err = FT_Init_FreeType(&fp_state->library);
  Assert(err == 0);
}

Internal FP_Handle fp_font_open(String8 path) {
	ProfFuncBegin();

  Temp_Arena scratch = scratch_begin(0, 0);
  String8 path_copy = push_str8_copy(scratch.arena, path);
  FT_Face face = {0};
  FT_New_Face(fp_state->library, (CString)path_copy.str, 0, &face);
  scratch_end(scratch);

  FP_Handle result = {
    .face = face,
  };
	ProfEnd();
  return result;
}

Internal void fp_font_close(FP_Handle font) {
  if (font.face != 0) {
    FT_Done_Face(font.face);
  }
}

Internal FP_Metrics fp_metrics_from_font(FP_Handle font) {
	FP_Metrics result = {0};
	if (font.face != 0) {
		result.design_units_per_em = (F1)(font.face->units_per_EM);
		result.ascent              = (F1)font.face->ascender;
		result.descent             = -(F1)font.face->descender;
		result.line_gap            = (F1)(font.face->height - font.face->ascender + font.face->descender);
		result.capital_height      = (F1)(font.face->ascender);
	}
	return result;
}

Internal FP_Raster_Result fp_raster(Arena *arena, FP_Handle font, F1 size, String8 string) {
	ProfFuncBegin();

	FP_Raster_Result result = {0};
	
	if (font.face != 0) {
		Temp_Arena scratch = scratch_begin(&arena, 1);

		//- kti: Unpack font
		FT_Face face = font.face;
		if (face->num_fixed_sizes > 0) {
			FT_Select_Size(face, 0);
		} else {
			FT_Set_Pixel_Sizes(face, 0, (FT_UInt)((96.0f/72.0f) * size));
		}
		SL1 ascent = face->size->metrics.ascender >> 6;
		SL1 descent = abs_SL1(face->size->metrics.descender >> 6);
		SL1 height = face->size->metrics.height >> 6;

		//- kti: Unpack string
		String32 string32 = str32_from_str8(scratch.arena, string);

		//- kti: Render & Measure
		SI1 total_width = 0;
		for EachIndex(i, string32.len) {
			FT_Load_Char(face, string32.str[i], FT_LOAD_RENDER);
			total_width += face->glyph->advance.x >> 6;
		}

		//- kti: Allocate & fill atlas w/rasterization
		SW2 dim = { (SW1)total_width+1, height+1 };
		L1 atlas_size = dim[0] * dim[1] * 4;
		B1 *atlas = push_array(arena, B1, atlas_size);
		SI1 baseline = ascent;
		SI1 atlas_write_x = 0;
		for EachIndex(i, string32.len) {
			FT_Bitmap *bmp = &face->glyph->bitmap;
			SI1 top = face->glyph->bitmap_top;
			SI1 left = face->glyph->bitmap_left;

			for (SI1 row = 0; row < (SI1)bmp->rows; row += 1) {
				SI1 y = baseline - top + row;
				for (SI1 col = 0; col < (SI1)bmp->width; col += 1) {
					SI1 x = atlas_write_x + left + col;
					if (x >= 0 && x < dim[0] && y >= 0 && y < dim[1]) {
						B1 alpha = 0;

						if (bmp->pixel_mode == FT_PIXEL_MODE_MONO) {
							B1 byte = bmp->buffer[row*bmp->pitch + (col >> 3)];
							B1 bit = (byte >> (7 - (col & 7))) & 1;
							alpha = bit ? 255 : 0;
						} else {
							alpha = bmp->buffer[row*bmp->pitch + col];
						}

						L1 off = (y*dim[0] + x)*4;
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
	}

	ProfEnd();
	return result;
}

#endif
