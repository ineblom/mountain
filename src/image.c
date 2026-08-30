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
  case IMAGE_FORMAT__RGBA8_SRGB:
    result = AlignOf(I1);
    break;
  case IMAGE_FORMAT__RGBA32F_LINEAR:
    result = AlignOf(F4);
    break;
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

Internal Image image_read_from_file(Arena *arena, String8 filename) {
  Image result = {0};

  String8 contents = os_read_entire_file(arena, filename);

  Bitmap_Header *header = (Bitmap_Header *)contents.str;
  if(contents.len >= sizeof(Bitmap_Header) && header &&
     header->file_type == 0x4D42 &&
     (header->bits_per_pixel == 24 || header->bits_per_pixel == 32) &&
     header->compression == 0 &&
     header->width > 0 && header->height != 0) {

    L1 width = (L1)header->width;
    L1 height = (header->height < 0) ? (L1)(-(SL1)header->height) : (L1)header->height;

    L1 src_pixel_size = header->bits_per_pixel / 8;
    L1 src_row_pitch = AlignPow2(width * src_pixel_size, 4);
    L1 bitmap_offset = header->bitmap_offset;

    I1 pixels_are_valid =
      bitmap_offset >= sizeof(Bitmap_Header) &&
      bitmap_offset <= contents.len &&
      height <= (contents.len - bitmap_offset) / src_row_pitch;

    if (pixels_are_valid) {
      result.width = width;
      result.height = height;
      result.format = IMAGE_FORMAT__RGBA8_SRGB;
      result.row_pitch = image_format_pixel_size(result.format)*result.width;

      B1 *pixel_base = contents.str + header->bitmap_offset;

      result.pixels = arena_push(arena, result.row_pitch*result.height, Max(8, image_format_alignment(result.format)), 1);

      for (L1 y = 0; y < result.height; y += 1) {
        L1 src_y = y;

        if (header->height < 0) {
          src_y = result.height - 1 - y;
        }

        B1 *src_row = pixel_base + src_row_pitch*src_y;
        B1 *dst_row = result.pixels + y*result.row_pitch;

        for (L1 x = 0; x < result.width; x += 1) {
          B1 *src = src_row + x*src_pixel_size;
          B1 *dst = dst_row + x*4;
          dst[0] = src[2];
          dst[1] = src[1];
          dst[2] = src[0];
          dst[3] = (src_pixel_size == 4) ? src[3] : 0xFF;
        }
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
  B1 *bitmap_pixels = push_array_no_zero(scratch.arena, B1, pixels_size);

  for (L1 y = 0; y < image.height; y += 1) {
    B1 *src_row = image.pixels + y*image.row_pitch;
    B1 *dst_row = bitmap_pixels + y*image.row_pitch;
    for (L1 x = 0; x < image.width; x += 1) {
      B1 *src = src_row + x*4;
      B1 *dst = dst_row + x*4;
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
      dst[3] = src[3];
    }
  }

  // TODO: Create os_ functions for this
  L1 file = open((CString)cstr_filename.str, O_CREAT|O_WRONLY|O_TRUNC, 0666);
  if (GtSI1(file, 0)) {
    write(file, &header, sizeof(Bitmap_Header));
    write(file, bitmap_pixels, pixels_size);
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

  if (!image_is_nil(hdr) && params.pass_count > 0 && hdr.format == IMAGE_FORMAT__RGBA32F_LINEAR) {
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

    for (L1 y = 0; y < result.height; y += 1) {
      for (L1 x = 0; x < result.width; x += 1) {
        F1 u = (result.width > 1) ? (F1)x / (F1)(result.width-1) : 0;
        F1 v = (result.height > 1) ? (F1)y / (F1)(result.height-1) : 0;

        F4 bloom_overlay = image_sample_bilinear_F4(params.overlay, u, v);
        F4 hdr_px = image_row_F4(hdr, y)[x];
        F4 bloom_px = image_row_F4(bloom_passes[0], y)[x];

        bloom_px *= 1.0f + luminance_F4(bloom_overlay)*params.overlay_strength;

        F4 color = hdr_px * (1.0f - 0.5f*params.strength) + params.strength*bloom_px;
        image_row_F4(result, y)[x] = color;
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
    for (L1 y = 0; y < input.height; y += 1) {
      for (L1 x = 0; x < input.width; x += 1) {
        F4 in_px = image_row_F4(input, y)[x];
        F4 tonemapped = tonemap(tonemap_kind, in_px);
        F4 out_color = {
          255.0f*srgb_from_linear(tonemapped[0]),
          255.0f*srgb_from_linear(tonemapped[1]),
          255.0f*srgb_from_linear(tonemapped[2]),
          255.0f,
        };
        B1 *out_px = result.pixels + y*result.row_pitch + x*4;
        out_px[0] = (B1)out_color[0];
        out_px[1] = (B1)out_color[1];
        out_px[2] = (B1)out_color[2];
        out_px[3] = (B1)out_color[3];
      }
    }
  }

  return result;
}

#endif
