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

Internal void image_bloom_threshold(Image dst, Image src, Image_Bloom_Params params, Range rows)
{
  ProfBeginFunc();
  //- kti: Equal dimensions.
  if (src.width == dst.width && src.height == dst.height)
  {
    for (L1 y = rows.min; y < rows.max; y += 1)
    {
      F4  *src_row  =  image_row(src, y);
      F4  *dst_row  =  image_row(dst, y);

      for (L1 x = 0; x < src.width; x += 1)
      {
        F4 color = src_row[x];

        F1  luminance       =  luminance_F4(color);
        F1  soft_threshold  =  params.threshold - params.knee;
        F1  knee_range      =  2.0f * params.knee;
        F1  contrib         =  0.0f;

        if (luminance > params.threshold + params.knee)
        {
          contrib = 1.0f;
        }
        else if (luminance > soft_threshold)
        {
          F1  x             =  luminance - soft_threshold;
          F1  knee_contrib  =  Square(x) / (4.0f * Square(knee_range));

          contrib = knee_contrib;
        }

        if (contrib > 0.0f && luminance > 1e-6f)
        {
          F1 excess_luminance = Max(0.0f, luminance - params.threshold);
          color *= excess_luminance / luminance * contrib;
        }
        else
        {
          color = (F4){0};
        }

        dst_row[x] = color;
      }
    }
  }
  ProfEnd();
}

Internal void image_resample_x(Image dst, Image src, Range rows)
{
  ProfBeginFunc();
  if (src.height == dst.height)
  {
    L1  src_dim  =  src.width;
    L1  dst_dim  =  dst.width;

    if (dst_dim > 0 && src_dim == 2 * dst_dim)
    {
      for (L1 y = rows.min; y < rows.max; y += 1)
      {
        F4  *src_row  =  image_row(src, y);
        F4  *dst_row  =  image_row(dst, y);

        for (L1 x = 0; x < dst_dim; x += 1)
        {
          L1  base  =  2 * x;
          F4  a     =  src_row[base > 0 ? base - 1 : 0];
          F4  b     =  src_row[base];
          F4  c     =  src_row[base + 1];
          F4  d     =  src_row[Min(base + 2, src_dim - 1)];

          dst_row[x] = (0.25f * a + 0.75f * b + 0.75f * c + 0.25f * d) * 0.5f;
        }
      }
    }
    else if (src_dim > 0 && dst_dim == 2 * src_dim)
    {
      for (L1 y = rows.min; y < rows.max; y += 1)
      {
        F4  *src_row  =  image_row(src, y);
        F4  *dst_row  =  image_row(dst, y);

        for (L1 x = 0; x < src_dim; x += 1)
        {
          F4  prev  =  src_row[x > 0 ? x - 1 : 0];
          F4  curr  =  src_row[x];
          F4  next  =  src_row[Min(x + 1, src_dim - 1)];

          dst_row[2 * x]      =  0.25f * prev + 0.75f * curr;
          dst_row[2 * x + 1]  =  0.75f * curr + 0.25f * next;
        }
      }
    }
    else
    {
      F1  scale          =  (F1)src_dim / (F1)dst_dim;
      F1  filter_scale   =  Max(1, scale);
      F1  filter_radius  =  1.0f;
      F1  support        =  filter_radius * filter_scale;

      for (L1 y = rows.min; y < rows.max; y += 1)
      {
        F4  *src_row  =  image_row(src, y);
        F4  *dst_row  =  image_row(dst, y);

        for (L1 dst_x = 0; dst_x < dst_dim; dst_x += 1)
        {
          F1  center         =  ((F1)dst_x + 0.5f) * scale - 0.5f;
          F1  first_src_pos  =  ceil_F1(center - support);
          F1  last_src_pos   =  floor_F1(center + support);
          L1  src_pos_count  =  (L1)(last_src_pos - first_src_pos) + 1;
          F4  sum            =  {0};
          F1  weight_sum     =  0;

          for (L1 src_pos_idx = 0; src_pos_idx < src_pos_count; src_pos_idx += 1)
          {
            F1  src_pos   =  first_src_pos + (F1)src_pos_idx;
            F1  distance  =  (src_pos - center) / filter_scale;
            F1  weight    =  Max(0, 1.0f - abs_F1(distance));
            L1  sample_x  =  (L1)Clamp(0.0f, src_pos, (F1)src_dim - 1.0f);

            sum         +=  src_row[sample_x] * weight;
            weight_sum  +=  weight;
          }

          if (weight_sum > 0.0f)
          {
            dst_row[dst_x] = sum / weight_sum;
          }
          else
          {
            L1 nearest_x = (L1)Clamp(0.0f, floor_F1(center + 0.5f), (F1)src_dim - 1.0f);

            dst_row[dst_x] = src_row[nearest_x];
          }
        }
      }
    }
  }
  ProfEnd();
}

Internal void image_resample_y(Image dst, Image src, Range rows)
{
  ProfBeginFunc();
  if (src.width == dst.width)
  {
    L1  src_dim  =  src.height;
    L1  dst_dim  =  dst.height;

    if (dst_dim > 0 && src_dim == 2 * dst_dim)
    {
      for (L1 y = rows.min; y < rows.max; y += 1)
      {
        L1  base      =  2 * y;
        F4  *a        =  image_row(src, base > 0 ? base - 1 : 0);
        F4  *b        =  image_row(src, base);
        F4  *c        =  image_row(src, base + 1);
        F4  *d        =  image_row(src, Min(base + 2, src_dim - 1));
        F4  *dst_row  =  image_row(dst, y);

        for (L1 x = 0; x < dst.width; x += 1)
        {
          dst_row[x] = (0.25f * a[x] + 0.75f * b[x] + 0.75f * c[x] + 0.25f * d[x]) * 0.5f;
        }
      }
    }
    else if (src_dim > 0 && dst_dim == 2 * src_dim)
    {
      for (L1 y = rows.min; y < rows.max; y += 1)
      {
        L1  base      =  y / 2;
        F4  *dst_row  =  image_row(dst, y);

        if (y & 1)
        {
          F4  *a  =  image_row(src, base);
          F4  *b  =  image_row(src, Min(base + 1, src_dim - 1));

          for (L1 x = 0; x < dst.width; x += 1)
          {
            dst_row[x] = 0.75f * a[x] + 0.25f * b[x];
          }
        }
        else
        {
          F4  *a  =  image_row(src, base > 0 ? base - 1 : 0);
          F4  *b  =  image_row(src, base);

          for (L1 x = 0; x < dst.width; x += 1)
          {
            dst_row[x] = 0.25f * a[x] + 0.75f * b[x];
          }
        }
      }
    }
    else
    {
      F1  scale          =  (F1)src_dim / (F1)dst_dim;
      F1  filter_scale   =  Max(1, scale);
      F1  filter_radius  =  1.0f;
      F1  support        =  filter_radius * filter_scale;

      for (L1 dst_y = rows.min; dst_y < rows.max; dst_y += 1)
      {
        F1  weight_sum     =  0;
        F1  center         =  ((F1)dst_y + 0.5f) * scale - 0.5f;
        F1  first_src_pos  =  ceil_F1(center - support);
        F1  last_src_pos   =  floor_F1(center + support);
        L1  src_pos_count  =  (L1)(last_src_pos - first_src_pos) + 1;
        F4  *dst_row       =  image_row(dst, dst_y);

        //- kti: Clear dst row.
        for (L1 x = 0; x < dst.width; x += 1)
        {
          dst_row[x] = (F4){0};
        }

        for (L1 src_pos_idx = 0; src_pos_idx < src_pos_count; src_pos_idx += 1)
        {
          F1  src_pos   =  first_src_pos + (F1)src_pos_idx;
          F1  distance  =  (src_pos - center) / filter_scale;
          F1  weight    =  Max(0, 1.0f - abs_F1(distance));
          L1  sample_y  =  (L1)Clamp(0.0f, src_pos, (F1)src_dim - 1.0f);
          F4  *src_row  =  image_row(src, sample_y);

          for (L1 x = 0; x < dst.width; x += 1)
          {
            dst_row[x] += src_row[x] * weight;
          }

          weight_sum += weight;
        }

        if (weight_sum > 0.0f)
        {
          for (L1 x = 0; x < dst.width; x += 1)
          {
            dst_row[x] /= weight_sum;
          }
        }
        else
        {
          L1  nearest_y  =  (L1)Clamp(0.0f, floor_F1(center + 0.5f), (F1)src_dim - 1.0f);
          F4  *src_row   =  image_row(src, nearest_y);

          for (L1 x = 0; x < dst.width; x += 1)
          {
            dst_row[x] = src_row[x];
          }
        }
      }
    }
  }
  ProfEnd();
}

Internal void image_apply_karis(Image image, Range rows)
{
  ProfBeginFunc();
  for (L1 y = rows.min; y < rows.max; y += 1)
  {
    F4 *row = image_row(image, y);

    for (L1 x = 0; x < image.width; x += 1)
    {
      F4 color = row[x];

      row[x] = color / (1.0f + luminance_F4(color));
    }
  }
  ProfEnd();
}

Internal void image_add(Image dst, Image src, Range rows)
{
  ProfBeginFunc();
  if (src.width == dst.width && src.height == dst.height)
  {
    for (L1 y = rows.min; y < rows.max; y += 1)
    {
      F4  *src_row  =  image_row(src, y);
      F4  *dst_row  =  image_row(dst, y);

      for (L1 x = 0; x < src.width; x += 1)
      {
        dst_row[x] += src_row[x];
      }
    }
  }
  ProfEnd();
}

Internal void image_bloom_combine(Image dst, Image hdr, Image bloom, Image_Bloom_Params params, Range rows)
{
  ProfBeginFunc();
  if (   dst.width == hdr.width
      && dst.height == hdr.height
      && bloom.width == hdr.width
      && bloom.height == hdr.height)
  {
    for (L1 y = rows.min; y < rows.max; y += 1)
    {
      F4  *dst_row    =  image_row(dst, y);
      F4  *hdr_row    =  image_row(hdr, y);
      F4  *bloom_row  =  image_row(bloom, y);
      F1  v           =  dst.height > 1 ? (F1)y / (F1)(dst.height - 1) : 0.0f;

      for (L1 x = 0; x < dst.width; x += 1)
      {
        F1  u              =  dst.width > 1 ? (F1)x / (F1)(dst.width - 1) : 0.0f;
        F4  bloom_overlay  =  image_sample_bilinear_F4(params.overlay, u, v);
        F4  bloom_px       =  bloom_row[x];

        bloom_px    *=  1.0f + luminance_F4(bloom_overlay) * params.overlay_strength;
        dst_row[x]  =   hdr_row[x] * (1.0f - 0.5f * params.strength) + bloom_px * params.strength;
      }
    }
  }
  ProfEnd();
}

Inline F4 tonemap_aces(F4 v)
{
  F1  a  =  2.51f;
  F1  b  =  0.03f;
  F1  c  =  2.43f;
  F1  d  =  0.59f;
  F1  e  =  0.14f;

  F4 result = clamp01_F4((v * (a * v + b)) / (v * (c * v + d) + e));

  return result;
}

Inline F4 tonemap_reinhard(F4 v)
{
  F4 result = v / (1.0f + v);

  return result;
}

Inline F4 tonemap_lottes(F4 v)
{
  // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
  F1  a        =  1.6f;
  F1  d        =  0.977f;
  F1  hdr_max  =  8.0f;
  F1  mid_in   =  0.18f;
  F1  mid_out  =  0.267f;
  F1  b        =  (-powf(mid_in, a) + powf(hdr_max, a) * mid_out) / ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);
  F1  c        =  (powf(hdr_max, a * d) * powf(mid_in, a) - powf(hdr_max, a) * powf(mid_in, a * d) * mid_out) / ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);
  //- kti: Skip alpha powers, but keep the remaining arithmetic vectorized.
  F4 numerator = {
    powf(v[0], a),
    powf(v[1], a),
    powf(v[2], a),
    1.0f,
  };
  F4 denominator_power = {
    powf(v[0], a * d),
    powf(v[1], a * d),
    powf(v[2], a * d),
    1.0f,
  };
  F4 result = numerator / (denominator_power * b + c);

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

//- kti: Samples of (B1)(255 * srgb_from_linear(i / 1024.0f)).
// If adjacent samples agree, monotonicity makes the whole interval exact.
// Intervals crossing a byte boundary use the original conversion instead.
// The upper endpoint is conservative: float rounding of white can vary.
Global const B1 srgb8_from_linear_table[1025] = {
  0, 3, 6, 9, 12, 15, 17, 19, 21, 23, 25, 26, 28, 29, 30, 32,
  33, 34, 35, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 46, 47, 48,
  49, 50, 51, 51, 52, 53, 54, 54, 55, 56, 57, 57, 58, 59, 59, 60,
  61, 61, 62, 63, 63, 64, 64, 65, 66, 66, 67, 67, 68, 69, 69, 70,
  70, 71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 77, 77, 77, 78,
  78, 79, 79, 80, 80, 81, 81, 82, 82, 83, 83, 84, 84, 84, 85, 85,
  86, 86, 87, 87, 88, 88, 88, 89, 89, 90, 90, 90, 91, 91, 92, 92,
  92, 93, 93, 94, 94, 94, 95, 95, 96, 96, 96, 97, 97, 97, 98, 98,
  99, 99, 99, 100, 100, 100, 101, 101, 101, 102, 102, 103, 103, 103, 104, 104,
  104, 105, 105, 105, 106, 106, 106, 107, 107, 107, 108, 108, 108, 109, 109, 109,
  110, 110, 110, 111, 111, 111, 112, 112, 112, 112, 113, 113, 113, 114, 114, 114,
  115, 115, 115, 116, 116, 116, 116, 117, 117, 117, 118, 118, 118, 119, 119, 119,
  119, 120, 120, 120, 121, 121, 121, 121, 122, 122, 122, 123, 123, 123, 123, 124,
  124, 124, 124, 125, 125, 125, 126, 126, 126, 126, 127, 127, 127, 127, 128, 128,
  128, 129, 129, 129, 129, 130, 130, 130, 130, 131, 131, 131, 131, 132, 132, 132,
  132, 133, 133, 133, 133, 134, 134, 134, 134, 135, 135, 135, 135, 136, 136, 136,
  136, 137, 137, 137, 137, 138, 138, 138, 138, 139, 139, 139, 139, 140, 140, 140,
  140, 141, 141, 141, 141, 142, 142, 142, 142, 142, 143, 143, 143, 143, 144, 144,
  144, 144, 145, 145, 145, 145, 145, 146, 146, 146, 146, 147, 147, 147, 147, 147,
  148, 148, 148, 148, 149, 149, 149, 149, 149, 150, 150, 150, 150, 151, 151, 151,
  151, 151, 152, 152, 152, 152, 152, 153, 153, 153, 153, 154, 154, 154, 154, 154,
  155, 155, 155, 155, 155, 156, 156, 156, 156, 156, 157, 157, 157, 157, 157, 158,
  158, 158, 158, 158, 159, 159, 159, 159, 160, 160, 160, 160, 160, 161, 161, 161,
  161, 161, 162, 162, 162, 162, 162, 162, 163, 163, 163, 163, 163, 164, 164, 164,
  164, 164, 165, 165, 165, 165, 165, 166, 166, 166, 166, 166, 167, 167, 167, 167,
  167, 168, 168, 168, 168, 168, 168, 169, 169, 169, 169, 169, 170, 170, 170, 170,
  170, 170, 171, 171, 171, 171, 171, 172, 172, 172, 172, 172, 173, 173, 173, 173,
  173, 173, 174, 174, 174, 174, 174, 175, 175, 175, 175, 175, 175, 176, 176, 176,
  176, 176, 176, 177, 177, 177, 177, 177, 178, 178, 178, 178, 178, 178, 179, 179,
  179, 179, 179, 179, 180, 180, 180, 180, 180, 180, 181, 181, 181, 181, 181, 181,
  182, 182, 182, 182, 182, 183, 183, 183, 183, 183, 183, 184, 184, 184, 184, 184,
  184, 185, 185, 185, 185, 185, 185, 186, 186, 186, 186, 186, 186, 187, 187, 187,
  187, 187, 187, 188, 188, 188, 188, 188, 188, 188, 189, 189, 189, 189, 189, 189,
  190, 190, 190, 190, 190, 190, 191, 191, 191, 191, 191, 191, 192, 192, 192, 192,
  192, 192, 192, 193, 193, 193, 193, 193, 193, 194, 194, 194, 194, 194, 194, 195,
  195, 195, 195, 195, 195, 195, 196, 196, 196, 196, 196, 196, 197, 197, 197, 197,
  197, 197, 197, 198, 198, 198, 198, 198, 198, 199, 199, 199, 199, 199, 199, 199,
  200, 200, 200, 200, 200, 200, 200, 201, 201, 201, 201, 201, 201, 202, 202, 202,
  202, 202, 202, 202, 203, 203, 203, 203, 203, 203, 203, 204, 204, 204, 204, 204,
  204, 204, 205, 205, 205, 205, 205, 205, 205, 206, 206, 206, 206, 206, 206, 207,
  207, 207, 207, 207, 207, 207, 208, 208, 208, 208, 208, 208, 208, 209, 209, 209,
  209, 209, 209, 209, 210, 210, 210, 210, 210, 210, 210, 210, 211, 211, 211, 211,
  211, 211, 211, 212, 212, 212, 212, 212, 212, 212, 213, 213, 213, 213, 213, 213,
  213, 214, 214, 214, 214, 214, 214, 214, 215, 215, 215, 215, 215, 215, 215, 215,
  216, 216, 216, 216, 216, 216, 216, 217, 217, 217, 217, 217, 217, 217, 218, 218,
  218, 218, 218, 218, 218, 218, 219, 219, 219, 219, 219, 219, 219, 220, 220, 220,
  220, 220, 220, 220, 220, 221, 221, 221, 221, 221, 221, 221, 222, 222, 222, 222,
  222, 222, 222, 222, 223, 223, 223, 223, 223, 223, 223, 223, 224, 224, 224, 224,
  224, 224, 224, 224, 225, 225, 225, 225, 225, 225, 225, 226, 226, 226, 226, 226,
  226, 226, 226, 227, 227, 227, 227, 227, 227, 227, 227, 228, 228, 228, 228, 228,
  228, 228, 228, 229, 229, 229, 229, 229, 229, 229, 229, 230, 230, 230, 230, 230,
  230, 230, 230, 231, 231, 231, 231, 231, 231, 231, 231, 232, 232, 232, 232, 232,
  232, 232, 232, 233, 233, 233, 233, 233, 233, 233, 233, 234, 234, 234, 234, 234,
  234, 234, 234, 235, 235, 235, 235, 235, 235, 235, 235, 236, 236, 236, 236, 236,
  236, 236, 236, 236, 237, 237, 237, 237, 237, 237, 237, 237, 238, 238, 238, 238,
  238, 238, 238, 238, 239, 239, 239, 239, 239, 239, 239, 239, 239, 240, 240, 240,
  240, 240, 240, 240, 240, 241, 241, 241, 241, 241, 241, 241, 241, 241, 242, 242,
  242, 242, 242, 242, 242, 242, 243, 243, 243, 243, 243, 243, 243, 243, 243, 244,
  244, 244, 244, 244, 244, 244, 244, 244, 245, 245, 245, 245, 245, 245, 245, 245,
  246, 246, 246, 246, 246, 246, 246, 246, 246, 247, 247, 247, 247, 247, 247, 247,
  247, 247, 248, 248, 248, 248, 248, 248, 248, 248, 248, 249, 249, 249, 249, 249,
  249, 249, 249, 250, 250, 250, 250, 250, 250, 250, 250, 250, 251, 251, 251, 251,
  251, 251, 251, 251, 251, 252, 252, 252, 252, 252, 252, 252, 252, 252, 253, 253,
  253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254,
  255,
};

Inline B1 srgb8_from_linear(F1 l)
{
  if (l <= 0.0f)
    return 0;
  if (l >= 1.0f)
    return (B1)(255.0f * srgb_from_linear(1.0f));

  I1 index = (I1)(l * 1024.0f);
  B1 lo = srgb8_from_linear_table[index];
  B1 hi = srgb8_from_linear_table[index + 1];

  if (lo == hi)
    return lo;

  return (B1)(255.0f * srgb_from_linear(l));
}

//- kti: Specialize each pixel loop so kind selection happens once per call.
#define IMAGE_TONEMAP_KERNEL(name, color)                                                 \
  Internal void image_tonemap_##name(Image_RGBA8 dst, Image src, Range rows)              \
  {                                                                                       \
    for (L1 y = rows.min; y < rows.max; y += 1)                                           \
    {                                                                                     \
      F4     *src_row  =  image_row(src, y);                                              \
      RGBA8  *dst_row  =  image_rgba8_row(dst, y);                                        \
                                                                                          \
      for (L1 x = 0; x < src.width; x += 1)                                               \
      {                                                                                   \
        F4 in_px = src_row[x];                                                            \
        F4 tonemapped = (color);                                                          \
                                                                                          \
        dst_row[x] = (RGBA8){                                                             \
          srgb8_from_linear(tonemapped[0]),                                               \
          srgb8_from_linear(tonemapped[1]),                                               \
          srgb8_from_linear(tonemapped[2]),                                               \
          255,                                                                            \
        };                                                                                \
      }                                                                                   \
    }                                                                                     \
  }

IMAGE_TONEMAP_KERNEL(aces,     tonemap_aces(in_px))
IMAGE_TONEMAP_KERNEL(reinhard, tonemap_reinhard(in_px))
IMAGE_TONEMAP_KERNEL(lottes,   tonemap_lottes(in_px))
IMAGE_TONEMAP_KERNEL(linear,   in_px)

#undef IMAGE_TONEMAP_KERNEL

Internal void image_tonemap(Image_RGBA8 dst, Image src, Tonemap_Kind kind, Range rows)
{
  ProfBeginFunc();
  if (src.width == dst.width && src.height == dst.height)
  {
    switch (kind)
    {
      case TONEMAP_KIND__ACES:     image_tonemap_aces(dst, src, rows);     break;
      case TONEMAP_KIND__REINHARD: image_tonemap_reinhard(dst, src, rows); break;
      case TONEMAP_KIND__LOTTES:   image_tonemap_lottes(dst, src, rows);   break;
      default:                   image_tonemap_linear(dst, src, rows);   break;
    }
  }
  ProfEnd();
}
