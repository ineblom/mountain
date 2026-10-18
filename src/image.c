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
Global F1 image_lottes_table[IMAGE_LOTTES_TABLE_COUNT + 1];

Inline F4 tonemap_aces(F4 v)
{
  F1  a  =  2.51f;
  F1  b  =  0.03f;
  F1  c  =  2.43f;
  F1  d  =  0.59f;
  F1  e  =  0.14f;

  F4  numerator    =  v * (a * v + b);
  F4  denominator  =  v * (c * v + d) + e;
  F4  mapped       =  numerator / denominator;
  F4  result       =  clamp01_F4(mapped);

  return result;
}

Inline F4 tonemap_reinhard(F4 v)
{
  F4 result = v / (1.0f + v);

  return result;
}

Inline F1 lottes_from_linear(F1 v)
{
  if (!(v > 0.0f))
    return 0.0f;
  if (v >= 8.0f)
    return 1.0f;

  F1 position = v * (IMAGE_LOTTES_TABLE_COUNT / 8.0f);
  I1 index = (I1)position;
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

Internal void image_init(void)
{
  for (I1 i = 0; i <= IMAGE_SRGB_TABLE_COUNT; i += 1)
  {
    F1 linear = (F1)i / IMAGE_SRGB_TABLE_COUNT;
    image_srgb_table[i] = 255.0f * srgb_from_linear(linear);
  }
  image_srgb_table[IMAGE_SRGB_TABLE_COUNT] = 255.0f;

  // Lottes 2016. The curve reaches white at hdr_max, so higher inputs can
  // saturate here because the following sRGB conversion clamps to white.
  F1  a                        =  1.6f;
  F1  d                        =  0.977f;
  F1  hdr_max                  =  8.0f;
  F1  mid_in                   =  0.18f;
  F1  mid_out                  =  0.267f;
  F1  contrast_exponent        =  a * d;
  F1  mid_contrast             =  powf(mid_in, a);
  F1  hdr_contrast             =  powf(hdr_max, a);
  F1  mid_shoulder             =  powf(mid_in, contrast_exponent);
  F1  hdr_shoulder             =  powf(hdr_max, contrast_exponent);
  F1  coefficient_denominator  =  (hdr_shoulder - mid_shoulder) * mid_out;
  F1  b_numerator              =  -mid_contrast + hdr_contrast * mid_out;
  F1  c_numerator              =  hdr_shoulder * mid_contrast - hdr_contrast * mid_shoulder * mid_out;
  F1  b                        =  b_numerator / coefficient_denominator;
  F1  c                        =  c_numerator / coefficient_denominator;

  for (I1 i = 0; i <= IMAGE_LOTTES_TABLE_COUNT; i += 1)
  {
    F1  linear       =  hdr_max * (F1)i / IMAGE_LOTTES_TABLE_COUNT;
    F1  numerator    =  powf(linear, a);
    F1  denominator  =  powf(linear, contrast_exponent) * b + c;

    image_lottes_table[i] = numerator / denominator;
  }
  image_lottes_table[IMAGE_LOTTES_TABLE_COUNT] = 1.0f;
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

Inline F4 tonemap_reinhard_white(F4 v, F1 inv_white_squared)
{
  F4  compressed         =  v / (1.0f + v);
  F4  white_point_scale  =  1.0f + v * inv_white_squared;
  F4  result             =  compressed * white_point_scale;
  return result;
}

Inline F4 image_tonemap_saturation(F4 color, F1 saturation)
{
  color = clamp01_F4(color);
  F1 gray = luminance_F4(color);
  F4 result = gray + (color - gray) * saturation;

  return result;
}

typedef struct Image_Tonemap_Adjustments Image_Tonemap_Adjustments;
struct Image_Tonemap_Adjustments
{
  F1 exposure_scale;
  F1 saturation;
  F1 inv_white_squared;
};

#define IMAGE_TONEMAP_KERNEL(name, input, curve, output)                                                            \
  Internal void image_tonemap_##name(Image_RGBA8 dst, Image src, Image_Tonemap_Adjustments adjustments, Range rows) \
  {                                                                                                                 \
    for (L1 y = rows.min; y < rows.max; y += 1)                                                                     \
    {                                                                                                               \
      F4     *src_row  =  image_row(src, y);                                                                        \
      RGBA8  *dst_row  =  image_rgba8_row(dst, y);                                                                  \
                                                                                                                    \
      for (L1 x = 0; x < src.width; x += 1)                                                                         \
      {                                                                                                             \
        F4 in_px       =  (input);                                                                                  \
        F4 tonemapped  =  (curve);                                                                                  \
        F4 color       =  (output);                                                                                 \
                                                                                                                    \
        dst_row[x] = (RGBA8){                                                                                       \
          srgb8_from_linear(color[0]),                                                                              \
          srgb8_from_linear(color[1]),                                                                              \
          srgb8_from_linear(color[2]),                                                                              \
          255,                                                                                                      \
        };                                                                                                          \
      }                                                                                                             \
    }                                                                                                               \
  }

//- kti: Regular Kernels
IMAGE_TONEMAP_KERNEL(aces,     src_row[x], tonemap_aces(in_px),     tonemapped)
IMAGE_TONEMAP_KERNEL(reinhard, src_row[x], tonemap_reinhard(in_px), tonemapped)
IMAGE_TONEMAP_KERNEL(lottes,   src_row[x], tonemap_lottes(in_px),   tonemapped)
IMAGE_TONEMAP_KERNEL(linear,   src_row[x], in_px,                   tonemapped)

//- kti: Adjusted Kerlens.

//- kti: ACES
IMAGE_TONEMAP_KERNEL(aces_adjusted,
                     src_row[x] * adjustments.exposure_scale,
                     tonemap_aces(in_px),
                     image_tonemap_saturation(tonemapped, adjustments.saturation))
//- kti: Reinhard
IMAGE_TONEMAP_KERNEL(reinhard_adjusted,
                     src_row[x] * adjustments.exposure_scale,
                     tonemap_reinhard_white(in_px, adjustments.inv_white_squared),
                     image_tonemap_saturation(tonemapped, adjustments.saturation))
//- kti: Lottes
IMAGE_TONEMAP_KERNEL(lottes_adjusted,
                     src_row[x] * adjustments.exposure_scale,
                     tonemap_lottes(in_px),
                     image_tonemap_saturation(tonemapped, adjustments.saturation))
//- kti: Linear
IMAGE_TONEMAP_KERNEL(linear_adjusted,
                     src_row[x] * adjustments.exposure_scale,
                     in_px,
                     image_tonemap_saturation(tonemapped, adjustments.saturation))

#undef IMAGE_TONEMAP_KERNEL

Internal void image_tonemap(Image_RGBA8 dst, Image src, Image_Tonemap_Params params, Range rows)
{
  ProfBeginFunc();

  if (src.width == dst.width && src.height == dst.height)
  {
    Image_Tonemap_Adjustments adjustments = {
      .exposure_scale = params.exposure_ev == 0.0f ? 1.0f : exp2f(params.exposure_ev),
      .saturation = params.saturation,
    };

    if (params.kind == TONEMAP_KIND__REINHARD && params.white_point > 0.0f)
    {
      adjustments.inv_white_squared = 1.0f / (params.white_point * params.white_point);
    }

    //- kti: Select Kernel

    if (params.exposure_ev == 0.0f && params.saturation == 1.0f && adjustments.inv_white_squared == 0.0f)
    {
      //- kti: No Params
      switch (params.kind)
      {
        case TONEMAP_KIND__ACES:     image_tonemap_aces(dst, src, adjustments, rows);     break;
        case TONEMAP_KIND__REINHARD: image_tonemap_reinhard(dst, src, adjustments, rows); break;
        case TONEMAP_KIND__LOTTES:   image_tonemap_lottes(dst, src, adjustments, rows);   break;
        default:                     image_tonemap_linear(dst, src, adjustments, rows);   break;
      }
    }
    else
    {
      //- kti: With Params. Use adjustable kernels (more expensive)
      switch (params.kind)
      {
        case TONEMAP_KIND__ACES:     image_tonemap_aces_adjusted(dst, src, adjustments, rows);     break;
        case TONEMAP_KIND__REINHARD: image_tonemap_reinhard_adjusted(dst, src, adjustments, rows); break;
        case TONEMAP_KIND__LOTTES:   image_tonemap_lottes_adjusted(dst, src, adjustments, rows);   break;
        default:                     image_tonemap_linear_adjusted(dst, src, adjustments, rows);   break;
      }
    }
  }

  ProfEnd();
}

// RGB SIMD postprocessing kernels share the transfer tables above.
#include "image_rgb.c"
