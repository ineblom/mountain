Global FP_State *fp_state = 0;

Inline FP_Handle fp_handle_zero(void)
{
  FP_Handle result = {0};
  return result;
}

Inline I1 fp_handle_match(FP_Handle a, FP_Handle b)
{
  I1 result = (a.face == b.face);
  return result;
}

Internal void fp_init(void)
{
  Arena *arena = arena_alloc(MiB(64));

  fp_state         =  push_array(arena, FP_State, 1);
  fp_state->arena  =  arena;

  FT_Init_FreeType(&fp_state->library);
}

Internal FP_Handle fp_font_open(String8 path)
{
  ProfBeginFunc();

  Temp_Arena  scratch    =  scratch_begin(0, 0);
  String8     path_copy  =  push_str8_copy(scratch.arena, path);
  FT_Face     face       =  {0};

  FT_New_Face(fp_state->library, (CString)path_copy.str, 0, &face);
  scratch_end(scratch);

  FP_Handle result = {
    .face = face,
  };

  ProfEnd();

  return result;
}

Internal void fp_font_close(FP_Handle font)
{
  if (font.face != 0)
  {
    FT_Done_Face(font.face);
  }
}

Inline I1 fp_face_is_bitmap_only(FT_Face face)
{
  return !FT_IS_SCALABLE(face) && face->num_fixed_sizes > 0;
}

//- kti: Returns the scale from the selected strike to size. 1 for scalable fonts.
Internal F1 fp_select_size(FT_Face face, F1 size)
{
  F1 result = 1.0f;

  if (fp_face_is_bitmap_only(face))
  {
    L1  best_idx   =  0;
    F1  best_diff  =  F1_MAX;

    for (L1 i = 0; i < (L1)face->num_fixed_sizes; i += 1)
    {
      F1 diff = abs_F1((F1)face->available_sizes[i].height - size);

      if (diff < best_diff)
      {
        best_diff  =  diff;
        best_idx   =  i;
      }
    }

    FT_Select_Size(face, (FT_Int)best_idx);

    F1 strike_height = (F1)face->size->metrics.height / 64.0f;

    if (strike_height > 0)
    {
      result = size / strike_height;
    }
  }
  else
  {
    FT_F26Dot6 char_size = (FT_F26Dot6)(size * 64.0f + 0.5f);

    FT_Set_Char_Size(face, 0, Max(char_size, 1), 72, 72);
  }

  return result;
}

Internal F1 fp_kerning(FP_Handle font, F1 size, F1 raster_scale, I1 left_codepoint, I1 right_codepoint)
{
  F1       result  =  0;
  FT_Face  face    =  font.face;

  if (face != 0 && size > 0 && FT_HAS_KERNING(face))
  {
    FT_UInt left_glyph   =  FT_Get_Char_Index(face, left_codepoint);
    FT_UInt right_glyph  =  FT_Get_Char_Index(face, right_codepoint);

    if (left_glyph != 0 && right_glyph != 0)
    {
      raster_scale = Max(1.0f, raster_scale);

      F1         strike_scale  =  fp_select_size(face, size * raster_scale);
      FT_Vector  kerning       =  {0};

      if (FT_Get_Kerning(face, left_glyph, right_glyph, FT_KERNING_DEFAULT, &kerning) == 0)
      {
        result = (F1)kerning.x / 64.0f * strike_scale / raster_scale;
      }
    }
  }

  return result;
}

Internal FP_Metrics fp_metrics_from_font(FP_Handle font, F1 size)
{
  FP_Metrics  result  =  {0};
  FT_Face     face    =  font.face;

  if (face != 0 && size > 0)
  {
    F1 ascent          =  0;
    F1 descent         =  0;
    F1 height          =  0;
    F1 capital_height  =  0;
    F1 x_height        =  0;

    if (FT_IS_SCALABLE(face) && face->units_per_EM > 0)
    {
      F1 units_to_px = size / (F1)face->units_per_EM;

      ascent   =  (F1)face->ascender * units_to_px;
      descent  =  -(F1)face->descender * units_to_px;
      height   =  (F1)face->height * units_to_px;

      TT_OS2 *os2 = FT_Get_Sfnt_Table(face, FT_SFNT_OS2);

      if (os2 != 0 && os2->version != 0xFFFF && os2->version >= 2)
      {
        capital_height  =  (F1)os2->sCapHeight * units_to_px;
        x_height        =  (F1)os2->sxHeight * units_to_px;
      }

      if (capital_height <= 0 && FT_Load_Char(face, 'H', FT_LOAD_NO_SCALE) == 0)
      {
        capital_height = (F1)face->glyph->metrics.height * units_to_px;
      }

      if (x_height <= 0 && FT_Load_Char(face, 'x', FT_LOAD_NO_SCALE) == 0)
      {
        x_height = (F1)face->glyph->metrics.height * units_to_px;
      }
    }
    else
    {
      F1               strike_scale  =  fp_select_size(face, size);
      FT_Size_Metrics  *m            =  &face->size->metrics;

      ascent   =  (F1)m->ascender / 64.0f * strike_scale;
      descent  =  -(F1)m->descender / 64.0f * strike_scale;
      height   =  (F1)m->height / 64.0f * strike_scale;

      if (FT_Load_Char(face, 'H', FT_LOAD_DEFAULT) == 0)
      {
        capital_height = (F1)face->glyph->metrics.height / 64.0f * strike_scale;
      }

      if (FT_Load_Char(face, 'x', FT_LOAD_DEFAULT) == 0)
      {
        x_height = (F1)face->glyph->metrics.height / 64.0f * strike_scale;
      }
    }

    result.ascent          =  ascent;
    result.descent         =  descent;
    result.line_gap        =  Max(0.0f, height - ascent - descent);
    result.capital_height  =  capital_height > 0 ? capital_height : ascent;
    result.x_height        =  x_height > 0 ? x_height : result.capital_height;
  }

  return result;
}

Internal FP_Raster_Result fp_raster(Arena *arena, FP_Handle font, F1 size, F1 raster_scale, String8 string)
{
  ProfBeginFunc();

  FP_Raster_Result result = {0};

  if (font.face != 0)
  {
    Temp_Arena scratch = scratch_begin(&arena, 1);

    //- kti: Unpack font
    FT_Face face = font.face;

    raster_scale = Max(1.0f, raster_scale);

    F1  strike_scale  =  fp_select_size(face, size * raster_scale);
    SI1 ascent        =  (SI1)(face->size->metrics.ascender >> 6);
    SI1 descent       =  (SI1)abs(face->size->metrics.descender >> 6);

    //- kti: Unpack string
    String32 string32 = str32_from_str8(scratch.arena, string);

    //- kti: Render & Measure
    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT;

    if (FT_HAS_COLOR(face))
    {
      load_flags |= FT_LOAD_COLOR;
    }

    SI1  total_width  =  0;
    F1   ink_ascent   =  -F1_MAX;
    F1   ink_descent  =  -F1_MAX;

    for (L1 i = 0; i < string32.len; i += 1)
    {
      if (FT_Load_Char(face, string32.str[i], load_flags) != 0)
      {
        continue;
      }

      FT_GlyphSlot glyph = face->glyph;

      if (glyph->bitmap.rows > 0 && glyph->bitmap.width > 0)
      {
        ink_ascent   =  Max(ink_ascent, (F1)glyph->bitmap_top);
        ink_descent  =  Max(ink_descent, (F1)glyph->bitmap.rows - (F1)glyph->bitmap_top);
      }

      total_width += glyph->advance.x >> 6;
    }

    if (ink_ascent + ink_descent <= 0)
    {
      ink_ascent   =  0;
      ink_descent  =  0;
    }

    SI1 above = Max(ascent, (SI1)ink_ascent);
    SI1 below = Max(descent, (SI1)ink_descent);

    //- kti: Allocate & fill atlas w/rasterization
    SW2  dim            =  {{(SW1)total_width, (SW1)(above + below)}};
    L1   atlas_size     =  dim.x * dim.y * 4;
    B1   *atlas         =  push_array(arena, B1, atlas_size);

    //- kti: Unwritten texels are transparent white, not transparent black, so
    // bilinear filtering at glyph edges doesn't pull the color toward black.
    for (L1 off = 0; off < atlas_size; off += 4)
    {
      atlas[off + 0]  =  255;
      atlas[off + 1]  =  255;
      atlas[off + 2]  =  255;
      atlas[off + 3]  =  0;
    }
    SI1  baseline       =  above;
    SI1  atlas_write_x  =  0;

    for (L1 i = 0; i < string32.len; i += 1)
    {
      if (FT_Load_Char(face, string32.str[i], load_flags) != 0)
      {
        continue;
      }

      FT_Bitmap  *bmp  =  &face->glyph->bitmap;
      SI1        top   =  face->glyph->bitmap_top;
      SI1        left  =  face->glyph->bitmap_left;

      for (SI1 row = 0; row < (SI1)bmp->rows; row += 1)
      {
        SI1 y = baseline - top + row;

        for (SI1 col = 0; col < (SI1)bmp->width; col += 1)
        {
          SI1 x = atlas_write_x + left + col;

          if (x >= 0 && x < dim.x && y >= 0 && y < dim.y)
          {
            L1 off = (y * dim.x + x) * 4;

            if (bmp->pixel_mode == FT_PIXEL_MODE_MONO)
            {
              B1  byte   =  bmp->buffer[row * bmp->pitch + (col >> 3)];
              B1  bit    =  (byte >> (7 - (col & 7))) & 1;
              B1  alpha  =  bit ? 255 : 0;

              atlas[off + 0]  =  255;
              atlas[off + 1]  =  255;
              atlas[off + 2]  =  255;
              atlas[off + 3]  =  alpha;
            }
            else if (bmp->pixel_mode == FT_PIXEL_MODE_BGRA)
            {
              //- kti: FreeType BGRA is premultiplied; our blending is straight alpha.
              L1  src    =  row * bmp->pitch + col * 4;
              B1  alpha  =  bmp->buffer[src + 3];

              if (alpha != 0)
              {
                atlas[off + 0]  =  (B1)Min(255, (bmp->buffer[src + 2] * 255 + alpha / 2) / alpha);
                atlas[off + 1]  =  (B1)Min(255, (bmp->buffer[src + 1] * 255 + alpha / 2) / alpha);
                atlas[off + 2]  =  (B1)Min(255, (bmp->buffer[src + 0] * 255 + alpha / 2) / alpha);
              }
              atlas[off + 3]  =  alpha;
            }
            else
            {
              B1 alpha = bmp->buffer[row * bmp->pitch + col];

              atlas[off + 0]  =  255;
              atlas[off + 1]  =  255;
              atlas[off + 2]  =  255;
              atlas[off + 3]  =  alpha;
            }
          }
        }
      }

      atlas_write_x += (face->glyph->advance.x >> 6);
    }

    result.atlas_dim     =  dim;
    result.atlas         =  atlas;
    result.advance       =  (F1)total_width;
    result.baseline      =  (F1)baseline;
    result.ink_ascent    =  ink_ascent;
    result.ink_descent   =  ink_descent;
    result.strike_scale  =  strike_scale;

    scratch_end(scratch);
  }

  ProfEnd();
  return result;
}
