
#if (HEADER)

typedef enum {
  TONEMAP_KIND__ACES,
  TONEMAP_KIND__REINHARD,
  TONEMAP_KIND__LOTTES,
} Tonemap_Kind;

#pragma pack(push, 1)
typedef struct Bitmap_Header Bitmap_Header;
struct Bitmap_Header {
  W1 file_type;
  I1 file_size;
  W1 reserved1, reserved2;
  I1 bitmap_offset;
  I1 size;
  SI1 width;
  SI1 height;
  W1 planes;
  W1 bits_per_pixel;
  I1 compression;
  I1 size_of_bitmap;
  SI1 horz_resolution;
  SI1 vert_resolution;
  I1 colors_used;
  I1 colors_important;
};
#pragma pack(pop)

typedef enum Image_Format {
  IMAGE_FORMAT__NULL,
  IMAGE_FORMAT__RGBA8_SRGB,
  IMAGE_FORMAT__RGBA32F_LINEAR,
} Image_Format;

typedef struct Image Image;
struct Image {
  I1 width;
  I1 height;
  L1 row_pitch;
  Image_Format format;
  B1 *pixels;
};

typedef struct Image_Bloom_Params Image_Bloom_Params;
struct Image_Bloom_Params {
  L1 pass_count;
  F1 threshold;
  F1 strength;
  F1 knee;
  Image overlay;
  F1 overlay_strength;
};

#endif

#if (SOURCE)

Internal I1 image_is_nil(Image image) {
  I1 result = image.width == 0 || image.height == 0 || image.pixels == 0;
  return result;
}

Internal L1 image_format_pixel_size(Image_Format format) {
  L1 result = 0;

  switch (format) {
    case IMAGE_FORMAT__RGBA8_SRGB: {
      result = sizeof(I1);
    } break;
    case IMAGE_FORMAT__RGBA32F_LINEAR: {
      result = sizeof(F4);
    } break;
    default: break;
  }

  return result;
}

Internal L1 image_format_alignment(Image_Format format) {
  L1 result = 1;

  switch (format) {
    case IMAGE_FORMAT__RGBA8_SRGB: {
      result = AlignOf(I1);
    } break;
    case IMAGE_FORMAT__RGBA32F_LINEAR: {
      result = AlignOf(F4);
    } break;
    default: break;
  }

  return result;
}

Internal Image image_alloc(Arena *arena, I1 width, I1 height, Image_Format format) {
  Image image = {0};
  I1 pixel_size = image_format_pixel_size(format);
  if (width != 0 && height != 0 && pixel_size != 0) {
    image.width = width;
    image.height = height;
    image.row_pitch = (L1)width * pixel_size;
    image.format = format;
    image.pixels = arena_push(arena, image.row_pitch*height, Max(8, image_format_alignment(format)), 1);
  }
  return image;
}

Inline F4 *image_row_F4(Image image, L1 y) {
  F4 *result = (F4 *)(image.pixels + y * image.row_pitch);
  return result;
}

Inline F4 F4_from_I1(I1 pixel) {
  F1 r = (F1)((pixel >> 16) & 0xFF);
  F1 g = (F1)((pixel >> 8) & 0xFF);
  F1 b = (F1)(pixel & 0xFF);
  F4 result = {r/255.0f,g/255.0f,b/255.0f};
  return result;
}

Internal Image image_read_from_file(Arena *arena, String8 filename) {
  Image result = {0};

  String8 contents = os_read_entire_file(arena, filename);

  Bitmap_Header *header = (Bitmap_Header *)contents.str;
  if(contents.len >= sizeof(Bitmap_Header) && header &&
      header->file_type == 0x4D42 &&
      (header->bits_per_pixel == 24 || header->bits_per_pixel == 32) &&
      header->compression == 0) {
    result.width = header->width;
    result.height = header->height;
    result.format = IMAGE_FORMAT__RGBA8_SRGB;
    result.row_pitch = image_format_pixel_size(result.format)*result.width;

    if (header->bits_per_pixel == 32) {
      result.pixels = contents.str + header->bitmap_offset;
    } else {
      L1 pixel_count = result.width * result.height;
      result.pixels = arena_push(arena, result.row_pitch*result.height, Max(8, image_format_alignment(result.format)), 1);

      B1 *src = contents.str + header->bitmap_offset;
      B1 *dst = result.pixels;
      for (L1 pixel_index = 0; pixel_index < pixel_count; pixel_index += 1) {
        B1 b = src[0];
        B1 g = src[1];
        B1 r = src[2];

        ((I1 *)dst)[0] = 0xFF000000 | (r << 16) | (g << 8) | b;

        src += 3;
        dst += 4;
      }
    }
  }

  return result;
}

Internal void image_write_to_file(Image image, String8 filename) {
  Assert(image.format == IMAGE_FORMAT__RGBA8_SRGB);

  L1 pixels_size = image.height*image.row_pitch;

  Bitmap_Header header  = {0};
  header.file_type      = 0x4D42;
  header.file_size      = sizeof(Bitmap_Header) + pixels_size;
  header.bitmap_offset  = sizeof(Bitmap_Header);
  header.size           = 40;
  header.width          = (SI1)image.width;
  header.height         = (SI1)image.height;
  header.planes         = 1;
  header.bits_per_pixel = 32;
  header.compression    = 0;
  header.size_of_bitmap = pixels_size;

  Temp_Arena scratch = scratch_begin(0, 0);
  String8 cstr_filename = push_str8_copy(scratch.arena, filename);

  // TODO: Create os_ functions for this
  L1 file = open((CString)cstr_filename.str, O_CREAT|O_WRONLY|O_TRUNC, 0666);
  if (GtSI1(file, 0)) {
    write(file, &header, sizeof(Bitmap_Header));
    write(file, image.pixels, pixels_size);
    close(file);
    printf("Image written to %s\n", cstr_filename.str);
  } else {
    fprintf(stderr, "Could not open '%s' to write image.\n", cstr_filename.str);
  }

  scratch_end(scratch);
}

Inline F4 image_sample_bilinear_F4(Image image, F1 u, F1 v) {
  F4 result = {0};
  if (!image_is_nil(image) && image.format == IMAGE_FORMAT__RGBA32F_LINEAR) {
    u = clamp01_F1(u);
    v = clamp01_F1(v);

    F1 tex_x = u * (F1)(image.width-1);
    F1 tex_y = v * (F1)(image.height-1);

    L1 x0 = Min((L1)tex_x, image.width-1);
    L1 y0 = Min((L1)tex_y, image.height-1);
    L1 x1 = Min(x0+1, image.width-1);
    L1 y1 = Min(y0+1, image.height-1);

    F1 fx = tex_x - (F1)x0;
    F1 fy = tex_y - (F1)y0;

    F4 *top = image_row_F4(image, y0);
    F4 *bot = image_row_F4(image, y1);
    F4 c00 = top[x0];
    F4 c10 = top[x1];
    F4 c01 = bot[x0];
    F4 c11 = bot[x1];

    F4 top_row = lerp_F4(c00, fx, c10);
    F4 bot_row = lerp_F4(c01, fx, c11);

    result = lerp_F4(top_row, fy, bot_row);
  }

  return result;
}

Internal Image image_apply_bloom(Arena *arena, Image hdr, Image_Bloom_Params params) {
  Image result = hdr;

  if (!image_is_nil(hdr) && params.pass_count > 0) {
    Temp_Arena scratch = scratch_begin(&arena, 1);
    Image *bloom_passes = push_array(scratch.arena, Image, 1+params.pass_count);

    bloom_passes[0] = image_alloc(scratch.arena, hdr.width, hdr.height, IMAGE_FORMAT__RGBA32F_LINEAR);

    //- kti: Copy bright pixels to pass 0.

    for (L1 y = 0; y < bloom_passes[0].height; y += 1) {
      for (L1 x = 0; x < bloom_passes[0].width; x += 1) {
        F4 color = image_row_F4(hdr, y)[x];

        F1 luminance = luminance_F4(color);

        F1 soft_threshold = params.threshold - params.knee;
        F1 knee_range = 2.0f*params.knee;
        F1 contrib = 0.0f;

        if (luminance > params.threshold + params.knee) {
          contrib = 1.0f;
        } else if (luminance > soft_threshold) {
          F1 x = luminance - soft_threshold;
          F1 knee_contrib = (x*x) / (4.0f*knee_range*knee_range);
          contrib = knee_contrib;
        }

        if (contrib > 0.0f && luminance > 1e-6f) {
          F1 excess_luminance = Max(0.0f, luminance - params.threshold);
          color = color * (excess_luminance/luminance)*contrib;
        } else {
          color = (F4){0};
        }

        image_row_F4(bloom_passes[0], y)[x] = color;
      }
    }

    //- kti: Downsample multiple times.
    L1 last_pass_index = 0;
    for (L1 pass_index = 0; pass_index < params.pass_count; pass_index += 1) {
      Image in = bloom_passes[pass_index];
      if (in.width < 2 || in.height < 2) {
        break;
      }
      Image out = image_alloc(scratch.arena, in.width/2, in.height/2, IMAGE_FORMAT__RGBA32F_LINEAR);

      bloom_passes[pass_index+1] = out;
      last_pass_index = pass_index+1;

      for (L1 y = 0; y < out.height; y += 1) {
        L1 sy = y*2;
        for (L1 x = 0; x < out.width; x += 1) {
          L1 sx = x*2;

          // TODO: look into 13-tap bilinear tent filter.

          F4 sum = {0};

          L1 left = sx;
          L1 right = Min(sx+1, in.width-1);
          L1 far_left = sx-Min(sx, 1);
          L1 far_right = Min(sx+2, in.width-1);

          F4 *top = image_row_F4(in, sy);
          F4 *bottom = image_row_F4(in, Min(sy+1, in.height-1));
          F4 *far_bottom = image_row_F4(in, Min(sy+2, in.height-1));
          F4 *far_top = image_row_F4(in, sy-Min(sy, 1));

          // Center (4)
          sum += top[left] * 4.0f;
          sum += top[right] * 4.0f;
          sum += bottom[right] * 4.0f;
          sum += bottom[left] * 4.0f;

          // Edges (2)
          sum += top[far_left] * 2.0f;
          sum += bottom[far_left] * 2.0f;

          sum += top[far_right] * 2.0f;
          sum += bottom[far_right] * 2.0f;

          sum += far_top[left] * 2.0f;
          sum += far_top[right] * 2.0f;

          sum += far_bottom[left] * 2.0f;
          sum += far_bottom[right] * 2.0f;

          // Corners (1)
          sum += far_top[far_left] * 1.0f;
          sum += far_top[far_right] * 1.0f;
          sum += far_bottom[far_left] * 1.0f;
          sum += far_bottom[far_right] * 1.0f;

          sum /= 36.0f;
          F4 karis_average = sum / (1.0f + luminance_F4(sum));

          image_row_F4(out, y)[x] = karis_average;
        }
      }
    }

    //- kti: Upsample and sum.
    for (L1 pass_index = last_pass_index; pass_index >= 1; pass_index -= 1) {
      Image in = bloom_passes[pass_index];
      Image out = bloom_passes[pass_index-1];

      for (L1 y = 0; y < out.height; y += 1) {
        L1 sy = y/2;
        for (L1 x = 0; x < out.width; x += 1) {
          L1 sx = x/2;

          F4 sum = {0};

          L1 center = Min(sx, in.width-1);
          L1 left = center-Min(center, 1);
          L1 right = Min(center+1, in.width-1);

          L1 center_y = Min(sy, in.height-1);
          F4 *middle = image_row_F4(in, center_y);
          F4 *top = image_row_F4(in, center_y-Min(center_y, 1));
          F4 *bottom = image_row_F4(in, Min(center_y+1, in.height-1));

          // center
          sum += middle[center] * 4.0f;

          // edges
          sum += middle[left] * 2.0f;
          sum += middle[right] * 2.0f;
          sum += top[center] * 2.0f;
          sum += bottom[center] * 2.0f;

          // corners
          sum += top[left] * 1.0f;
          sum += top[right] * 1.0f;
          sum += bottom[left] * 1.0f;
          sum += bottom[right] * 1.0f;

          sum /= 16.0f;

          image_row_F4(out, y)[x] += sum;
        }
      }
    }

    //- kti: Combine input image and bloom result, optionally use overlay image.
    result = image_alloc(arena, hdr.width, hdr.height, IMAGE_FORMAT__RGBA32F_LINEAR);

    F4 *in_hdr = (F4 *)hdr.pixels;
    F4 *in_bloom = (F4 *)bloom_passes[0].pixels;
    F4 *out = (F4 *)result.pixels;
    for (L1 y = 0; y < result.height; y += 1) {
      for (L1 x = 0; x < result.width; x += 1) {
        F1 u = (F1)x / (F1)result.width;
        F1 v = (F1)y / (F1)result.height;

        F4 bloom_overlay = image_sample_bilinear_F4(params.overlay, u, v);
        F4 hdr = in_hdr[0];
        F4 bloom = in_bloom[0];

        bloom *= 1.0f + luminance_F4(bloom_overlay)*params.overlay_strength;

        F4 color = hdr * (1.0f - 0.5f*params.strength) + params.strength*bloom;
        out[0] = color;

        in_hdr += 1;
        in_bloom += 1;
        out += 1;
      }
    }

    scratch_end(scratch);
  }

  return result;
}

Inline F4 tonemap_aces(F4 v) {
  const F1 a = 2.51f;
  const F1 b = 0.03f;
  const F1 c = 2.43f;
  const F1 d = 0.59f;
  const F1 e = 0.14f;
  F4 result = clamp01_F4((v * (a * v + b)) / (v * (c * v + d) + e));
  return result;
}

Inline F4 tonemap_reinhard(F4 v) {
  F4 result = v / (1.0f + v);
  return result;
}

Inline F4 tonemap_lottes(F4 v) {
  // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
  const F1 a = 1.6f;
  const F1 d = 0.977f;
  const F1 hdr_max = 8.0f;
  const F1 mid_in = 0.18f;
  const F1 mid_out = 0.267f;

  // TODO: Precompute
  const F1 b =
      (-powf(mid_in, a) + powf(hdr_max, a) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);
  const float c = (powf(hdr_max, a * d) * powf(mid_in, a) - powf(hdr_max, a) * powf(mid_in, a * d) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);

  F4 result = pow_F4(v, a) / (pow_F4(v, a * d) * b + c);
  return result;
}

Internal F4 tonemap(Tonemap_Kind kind, F4 v) {
  F4 result = v;
  switch (kind) {
    case TONEMAP_KIND__ACES:     result = tonemap_aces(v); break;
    case TONEMAP_KIND__REINHARD: result = tonemap_reinhard(v); break;
    case TONEMAP_KIND__LOTTES:   result = tonemap_lottes(v); break;
  }
  return result;
}

Inline F1 srgb_from_linear(F1 l) {
  if (l < 0.0f) l = 0.0f;
  if (l > 1.0f) l = 1.0f;

  F1 s = l * 12.92f;
  if (l > 0.0031308f) {
    s = 1.055f*powf(l, 1.0f/2.4f) - 0.055f;
  }

  return s;
}

Internal Image image_I1_from_F4_tonemap(Arena *arena, Image input, Tonemap_Kind tonemap_kind) {
  Image result = {0};
  if (input.format == IMAGE_FORMAT__RGBA32F_LINEAR) {
    result = image_alloc(arena, input.width, input.height, IMAGE_FORMAT__RGBA8_SRGB);
    for (L1 pixel_index = 0; pixel_index < input.width*input.height; pixel_index += 1) {
      F4 in_px = ((F4 *)input.pixels)[pixel_index];
      F4 tonemapped = tonemap(tonemap_kind, in_px);
      F4 out_color = {
        255.0f*srgb_from_linear(tonemapped[0]),
        255.0f*srgb_from_linear(tonemapped[1]),
        255.0f*srgb_from_linear(tonemapped[2]),
        255.0f,
      };
      I1 out_px =
        0xFF000000             |
        (I1)out_color[0] << 16 |
        (I1)out_color[1] << 8  |
        (I1)out_color[2] << 0;
      ((I1 *)result.pixels)[pixel_index] = out_px;
    }
  }

  return result;
}

#endif
