////////////////////////////////
//~ kti: Utils

Internal I1 image_is_nil(Image image)
{
  I1 result = image.width == 0 || image.height == 0 || image.pixels == 0;
  return result;
}

Internal F4 *image_row(Image image, L1 y)
{
  F4 *result = image.pixels + image.row_stride * y;
  return result;
}

Internal RGBA8 *image_rgba8_row(Image_RGBA8 image, L1 y)
{
  RGBA8 *result = image.pixels + image.row_stride * y;
  return result;
}

////////////////////////////////
//~ kti: Create

Internal Image image_alloc(Arena *arena, I1 width, I1 height)
{
  ProfBeginFunc();
  Image image = {0};

  if (width > 0 && height > 0)
  {
    L1 pixel_count = (L1)width * (L1)height;

    image.width       =  width;
    image.height      =  height;
    image.row_stride  =  width;
    image.pixels      =  push_array_no_zero(arena, F4, pixel_count);
  }

  ProfEnd();

  return image;
}

Internal Image_RGBA8 image_rgba8_alloc(Arena *arena, I1 width, I1 height)
{
  ProfBeginFunc();
  Image_RGBA8 image = {0};

  if (width > 0 && height > 0)
  {
    L1 pixel_count = (L1)width * (L1)height;

    image.width       =  width;
    image.height      =  height;
    image.row_stride  =  width;
    image.pixels      =  push_array_no_zero(arena, RGBA8, pixel_count);
  }

  ProfEnd();

  return image;
}

Internal Image_RGBA8 image_read_from_file(Arena *arena, String8 filename)
{
  ProfBeginFunc();
  Temp_Arena     scratch   =  scratch_begin(&arena, 1);
  Image_RGBA8    result    =  {0};
  String8        contents  =  os_read_entire_file(scratch.arena, filename);
  Bitmap_Header  *header   =  (Bitmap_Header *)contents.str;

  if (contents.len >= sizeof(Bitmap_Header)
      && header != 0
      && header->file_type == 0x4D42
      && (header->bits_per_pixel == 24 || header->bits_per_pixel == 32)
      && header->compression == 0
      && header->size == 40
      && header->planes == 1
      && header->width > 0
      && header->height != 0)
  {
    L1  width             =  (L1)header->width;
    L1  height            =  (header->height < 0) ? (L1)(-(SL1)header->height) : (L1)header->height;
    L1  src_pixel_size    =  header->bits_per_pixel / 8;
    L1  src_row_pitch     =  AlignPow2(width * src_pixel_size, 4);
    L1  bitmap_offset     =  header->bitmap_offset;
    B1  *src_pixel_base   =  contents.str + header->bitmap_offset;
    I1  pixels_are_valid  =  bitmap_offset >= sizeof(Bitmap_Header)
                             && bitmap_offset <= contents.len
                             && height <= (contents.len - bitmap_offset) / src_row_pitch;

    if (pixels_are_valid)
    {
      result.width       =  width;
      result.height      =  height;
      result.row_stride  =  width;

      result.pixels = push_array(arena, RGBA8, result.row_stride * result.height);

      for (L1 y = 0; y < result.height; y += 1)
      {
        L1 src_y = y;

        if (header->height < 0)
        {
          src_y = result.height - 1 - y;
        }

        B1     *src_row  =  src_pixel_base + src_row_pitch * src_y;
        RGBA8  *dst_row  =  result.pixels + result.row_stride * y;

        for (L1 x = 0; x < result.width; x += 1)
        {
          B1     *src  =  src_row + x * src_pixel_size;
          RGBA8  *dst  =  &dst_row[x];

          dst->r  =  src[2];
          dst->g  =  src[1];
          dst->b  =  src[0];
          dst->a  =  (src_pixel_size == 4) ? src[3] : 0xFF;
        }
      }
    }
  }

  scratch_end(scratch);

  ProfEnd();

  return result;
}

////////////////////////////////
//~ kti: Write

Internal void image_write_to_file(Image_RGBA8 image, String8 filename)
{
  ProfBeginFunc();
  if (   image.width       >   0
      && image.height      >   0
      && image.row_stride  >=  image.width
      && image.pixels      !=  0)
  {
    L1  bitmap_row_size  =  (L1)image.width * sizeof(RGBA8);
    L1  bitmap_size      =  (L1)image.height * bitmap_row_size;
    L1  file_size        =  sizeof(Bitmap_Header) + bitmap_size;

    if (file_size <= I1_MAX)
    {
      Temp_Arena    scratch     =  scratch_begin(0, 0);
      B1            *file_data  =  push_array_no_zero(scratch.arena, B1, file_size);
      Bitmap_Header *header     =  (Bitmap_Header *)file_data;

      header[0]               =  (Bitmap_Header){0};
      header->file_type       =  0x4D42;
      header->file_size       =  (I1)file_size;
      header->bitmap_offset   =  sizeof(Bitmap_Header);
      header->size            =  40;
      header->width           =  (SI1)image.width;
      header->height          =  (SI1)image.height;
      header->planes          =  1;
      header->bits_per_pixel  =  32;
      header->compression     =  0;
      header->size_of_bitmap  =  (I1)bitmap_size;

      B1 *bitmap_pixels = file_data + header->bitmap_offset;

      for (L1 y = 0; y < image.height; y += 1)
      {
        RGBA8  *src_row  =  image.pixels + y * image.row_stride;
        B1     *dst_row  =  bitmap_pixels + y * bitmap_row_size;

        for (L1 x = 0; x < image.width; x += 1)
        {
          RGBA8  src   =  src_row[x];
          B1     *dst  =  dst_row + x * sizeof(RGBA8);

          dst[0]  =  src.b;
          dst[1]  =  src.g;
          dst[2]  =  src.r;
          dst[3]  =  src.a;
        }
      }

      L1 bytes_written = os_write_entire_file(filename, file_data, file_size);

      if (bytes_written != file_size)
      {
        fprintf(stderr, "Could not write image file.\n");
      }

      scratch_end(scratch);
    }
  }
  ProfEnd();
}

////////////////////////////////
//~ kti: Sample

Inline F4 image_sample_bilinear_F4(Image image, F1 u, F1 v)
{
  F4 result = {0};

  if (!image_is_nil(image))
  {
    u  =  clamp01_F1(u);
    v  =  clamp01_F1(v);

    F1  tex_x  =  u * (F1)(image.width - 1);
    F1  tex_y  =  v * (F1)(image.height - 1);

    L1  x0  =  Min((L1)tex_x, image.width - 1);
    L1  y0  =  Min((L1)tex_y, image.height - 1);
    L1  x1  =  Min(x0 + 1, image.width - 1);
    L1  y1  =  Min(y0 + 1, image.height - 1);

    F1  fx  =  tex_x - (F1)x0;
    F1  fy  =  tex_y - (F1)y0;

    F4  *top  =  image_row(image, y0);
    F4  *bot  =  image_row(image, y1);
    F4  c00   =  top[x0];
    F4  c10   =  top[x1];
    F4  c01   =  bot[x0];
    F4  c11   =  bot[x1];

    F4  top_row  =  lerp_F4(c00, fx, c10);
    F4  bot_row  =  lerp_F4(c01, fx, c11);

    result = lerp_F4(top_row, fy, bot_row);
  }

  return result;
}

enum
{
  IMAGE_SRGB_TABLE_COUNT    =  1024,
  IMAGE_LOTTES_TABLE_COUNT  =  4096,
};

Global F1 image_srgb_table[IMAGE_SRGB_TABLE_COUNT + 1];
Global ThreadLocal F1 image_lottes_table[IMAGE_LOTTES_TABLE_COUNT + 1];
Global ThreadLocal Image_Lottes_Params image_lottes_table_params;

// NOTE(kti): sRGB(255 * clamp01(Lottes(x))) composed into one table; the
// tonemap pass can then do one lookup per channel instead of two.
Global ThreadLocal F1 image_lottes_srgb_table[IMAGE_LOTTES_TABLE_COUNT + 1];

Internal Image_Lottes_Params image_lottes_default_params(void)
{
  Image_Lottes_Params result = {
    .contrast  =  1.6f,
    .shoulder  =  0.977f,
    .hdr_max   =  8.0f,
    .mid_in    =  0.18f,
    .mid_out   =  0.267f,
  };

  return result;
}

Internal F2 image_lottes_mid_out_bounds(Image_Lottes_Params params)
{
  F1 gray_to_white      =  params.mid_in / params.hdr_max;
  F1 shoulder_exponent  =  params.contrast * (1.0f - params.shoulder);
  F1 maximum_output     =  powf(gray_to_white, shoulder_exponent) * 0.99f;
  F1 minimum_output     =  Min(0.001f, maximum_output * 0.01f);

  F2 result = {minimum_output, maximum_output};
  return result;
}

Internal Image_Lottes_Params image_lottes_normalize_params(Image_Lottes_Params params)
{
  Image_Lottes_Params defaults = image_lottes_default_params();

  if (   params.contrast == 0 && params.shoulder == 0 && params.hdr_max == 0
      && params.mid_in == 0 && params.mid_out == 0)
  {
    params = defaults;
  }

  if (!isfinite(params.contrast)) params.contrast = defaults.contrast;
  if (!isfinite(params.shoulder)) params.shoulder = defaults.shoulder;
  if (!isfinite(params.hdr_max))  params.hdr_max  = defaults.hdr_max;
  if (!isfinite(params.mid_in))   params.mid_in   = defaults.mid_in;
  if (!isfinite(params.mid_out))  params.mid_out  = defaults.mid_out;

  params.contrast  =  Clamp(0.1f, params.contrast, 4.0f);
  params.shoulder  =  Clamp(0.01f, params.shoulder, 1.0f);
  params.hdr_max   =  Clamp(1.0f, params.hdr_max, 128.0f);
  params.mid_in    =  Clamp(0.001f, params.mid_in, Min(1.0f, params.hdr_max * 0.9f));

  F2 output_bounds = image_lottes_mid_out_bounds(params);

  params.mid_out = Clamp(output_bounds[0], params.mid_out, output_bounds[1]);

  return params;
}

Inline F1 srgb_from_linear(F1 l)
{
  if (l < 0.0f)
    l = 0.0f;
  if (l > 1.0f)
    l = 1.0f;

  F1 s = l * 12.92f;

  if (l > 0.0031308f)
  {
    s = 1.055f * powf(l, 1.0f / 2.4f) - 0.055f;
  }

  return s;
}

Internal void image_lottes_prepare(Image_Lottes_Params params)
{
  params = image_lottes_normalize_params(params);

  if (   params.contrast  == image_lottes_table_params.contrast
      && params.shoulder  == image_lottes_table_params.shoulder
      && params.hdr_max   == image_lottes_table_params.hdr_max
      && params.mid_in    == image_lottes_table_params.mid_in
      && params.mid_out   == image_lottes_table_params.mid_out)
  {
    return;
  }

  D1  a                        =  params.contrast;
  D1  d                        =  params.shoulder;
  D1  hdr_max                  =  params.hdr_max;
  D1  mid_in                   =  params.mid_in;
  D1  mid_out                  =  params.mid_out;
  D1  contrast_exponent        =  a * d;
  D1  mid_contrast             =  pow(mid_in, a);
  D1  hdr_contrast             =  pow(hdr_max, a);
  D1  mid_shoulder             =  pow(mid_in, contrast_exponent);
  D1  hdr_shoulder             =  pow(hdr_max, contrast_exponent);
  D1  coefficient_denominator  =  (hdr_shoulder - mid_shoulder) * mid_out;
  D1  b_numerator              =  -mid_contrast + hdr_contrast * mid_out;
  D1  c_numerator              =  hdr_shoulder * mid_contrast - hdr_contrast * mid_shoulder * mid_out;
  D1  b                        =  b_numerator / coefficient_denominator;
  D1  c                        =  c_numerator / coefficient_denominator;

  for (I1 i = 0; i <= IMAGE_LOTTES_TABLE_COUNT; i += 1)
  {
    D1  linear       =  hdr_max * (D1)i / IMAGE_LOTTES_TABLE_COUNT;
    D1  numerator    =  pow(linear, a);
    D1  denominator  =  pow(linear, contrast_exponent) * b + c;

    D1 mapped = numerator / denominator;

    image_lottes_table[i] = (F1)Clamp(0.0, mapped, 1.0);
  }
  image_lottes_table[IMAGE_LOTTES_TABLE_COUNT] = 1.0f;

  for (I1 i = 0; i < IMAGE_LOTTES_TABLE_COUNT; i += 1)
  {
    image_lottes_srgb_table[i] = 255.0f * srgb_from_linear(image_lottes_table[i]);
  }
  image_lottes_srgb_table[IMAGE_LOTTES_TABLE_COUNT] = 255.0f;

  image_lottes_table_params = params;
}

Inline F1 lottes_from_linear(F1 v)
{
  if (!(v > 0.0f))
    return 0.0f;
  if (v >= image_lottes_table_params.hdr_max)
    return 1.0f;

  F1 position = v * (IMAGE_LOTTES_TABLE_COUNT / image_lottes_table_params.hdr_max);
  I1 index = Min((I1)position, IMAGE_LOTTES_TABLE_COUNT - 1);
  F1 lo = image_lottes_table[index];
  F1 hi = image_lottes_table[index + 1];

  F1  blend_fraction  =  position - (F1)index;
  F1  value_delta     =  hi - lo;
  F1  interpolated    =  lo + blend_fraction * value_delta;

  return interpolated;
}

Inline F4 tonemap_lottes(F4 v)
{
  F4 result = {
    lottes_from_linear(v[0]),
    lottes_from_linear(v[1]),
    lottes_from_linear(v[2]),
    1.0f,
  };

  return result;
}

Internal void image_init(void)
{
  for (I1 i = 0; i <= IMAGE_SRGB_TABLE_COUNT; i += 1)
  {
    F1 linear = (F1)i / IMAGE_SRGB_TABLE_COUNT;
    image_srgb_table[i] = 255.0f * srgb_from_linear(linear);
  }
  image_srgb_table[IMAGE_SRGB_TABLE_COUNT] = 255.0f;

  image_lottes_prepare(image_lottes_default_params());
}

Inline B1 srgb8_from_linear(F1 l)
{
  if (!(l > 0.0f))
    return 0;
  if (l >= 1.0f)
    return 255;

  F1 position = l * IMAGE_SRGB_TABLE_COUNT;
  I1 index = (I1)position;
  F1 lo = image_srgb_table[index];
  F1 hi = image_srgb_table[index + 1];

  F1  blend_fraction  =  position - (F1)index;
  F1  value_delta     =  hi - lo;
  F1  interpolated    =  lo + blend_fraction * value_delta;

  return (B1)interpolated;
}

Inline B1 srgb8_from_lottes_linear(F1 v)
{
  if (!(v > 0.0f))
    return 0;
  if (v >= image_lottes_table_params.hdr_max)
    return 255;

  F1 position = v * (IMAGE_LOTTES_TABLE_COUNT / image_lottes_table_params.hdr_max);
  I1 index = Min((I1)position, IMAGE_LOTTES_TABLE_COUNT - 1);
  F1 lo = image_lottes_srgb_table[index];
  F1 hi = image_lottes_srgb_table[index + 1];

  F1  blend_fraction  =  position - (F1)index;
  F1  value_delta     =  hi - lo;
  F1  interpolated    =  lo + blend_fraction * value_delta + 0.5f;

  return (B1)interpolated;
}

Inline F4 image_tonemap_saturation(F4 color, F1 saturation)
{
  color = clamp01_F4(color);
  F1 gray = luminance_F4(color);
  F4 result = gray + (color - gray) * saturation;

  return result;
}

Internal void image_tonemap(Image_RGBA8 dst, Image src, Image_Tonemap_Params params, Range rows)
{
  ProfBeginFunc();

  if (src.width == dst.width && src.height == dst.height)
  {
    image_lottes_prepare(params.lottes);

    F1  exposure_scale     =  params.exposure_ev == 0.0f ? 1.0f : exp2f(params.exposure_ev);
    I1  needs_adjustments  =  params.saturation != 1.0f;

    for (L1 y = rows.min; y < rows.max; ++y)
    {
      F4     *src_row  =  image_row(src, y);
      RGBA8  *dst_row  =  image_rgba8_row(dst, y);

      for (L1 x = 0; x < src.width; ++x)
      {
        F4 color = src_row[x] * exposure_scale;

        if (!needs_adjustments)
        {
          //- kti: Fast path: one composed table maps HDR linear straight to sRGB.
          dst_row[x] = (RGBA8){
            srgb8_from_lottes_linear(color[0]),
            srgb8_from_lottes_linear(color[1]),
            srgb8_from_lottes_linear(color[2]),
            255,
          };
        }
        else
        {
          color = tonemap_lottes(color);
          color = image_tonemap_saturation(color, params.saturation);

          dst_row[x] = (RGBA8){
            srgb8_from_linear(color[0]),
            srgb8_from_linear(color[1]),
            srgb8_from_linear(color[2]),
            255,
          };
        }
      }
    }
  }

  ProfEnd();
}

// RGB SIMD postprocessing kernels share the transfer tables above.
#include "image_rgb.c"
