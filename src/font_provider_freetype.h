#ifndef FONT_PROVIDER_FREETYPE_H
#define FONT_PROVIDER_FREETYPE_H
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct FP_Handle FP_Handle;
struct FP_Handle {
  FT_Face face;
};

typedef struct FP_Raster_Result FP_Raster_Result;
struct FP_Raster_Result {
  SW2 atlas_dim;
  B1 *atlas;
  F1 advance;
  F1 baseline;
  F1 raster_scale;
};

typedef struct FP_Metrics FP_Metrics;
struct FP_Metrics {
  F1 ascent;
  F1 descent;
  F1 line_gap;
  F1 capital_height;
  F1 x_height;
};

typedef struct FP_State FP_State;
struct FP_State {
  Arena *arena;

  FT_Library library;
};

#endif
