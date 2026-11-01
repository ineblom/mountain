Inline L1 image_rgb_block_count(I1 width)
{
  return ((L1)width + SIMD_WIDTH - 1) / SIMD_WIDTH;
}

Inline Simd_F4 *image_rgb_row(Image_RGB image, L1 y)
{
  return image.blocks + y * image.block_stride;
}

Inline Simd_F4 rgb_load_pixels(const F4 *row, L1 x, I1 width)
{
  Simd_F4  result  =  {0};
  L1       last_x  =  (L1)width - 1;

  for (L1 lane = 0; lane < SIMD_WIDTH; ++lane)
  {
    set_lane_F4(&result, lane, row[Min(x + lane, last_x)]);
  }

  return result;
}

// Replicate a single lane across the whole block, for edge clamping.
Inline Simd_F4 rgb_splat_lane(Simd_F4 v, L1 lane)
{
  Simd_F4 result = {
    splat_Simd_F1(v.x[lane]),
    splat_Simd_F1(v.y[lane]),
    splat_Simd_F1(v.z[lane]),
    splat_Simd_F1(v.w[lane]),
  };
  return result;
}

Internal Image_RGB image_rgb_alloc(Arena *arena, I1 width, I1 height)
{
  Image_RGB result = {0};

  if (width && height)
  {
    result         =  (Image_RGB){width, height, image_rgb_block_count(width), 0};
    result.blocks  =  push_array_no_zero(arena, Simd_F4, result.block_stride * height);
  }

  return result;
}

Internal void image_rgb_bloom_threshold(Image_RGB dst, Image src, Image_Bloom_Params params, Range rows)
{
  ProfBeginFunc();

  Simd_F1  zero                =  splat_Simd_F1(0);
  Simd_F1  one                 =  splat_Simd_F1(1);
  Simd_F1  threshold           =  splat_Simd_F1(params.threshold);
  Simd_F1  knee_start          =  splat_Simd_F1(params.threshold - params.knee);
  Simd_F1  knee_end            =  splat_Simd_F1(params.threshold + params.knee);
  F1        knee_width               =  2.0f * params.knee;
  // Normalized so the quadratic reaches one at knee_end, where the piecewise
  // select switches to full contribution, keeping the soft knee continuous.
  F1        inv_knee_normalization   =  knee_width != 0 ? 1.0f / Square(knee_width) : 1.0f;

  for (L1 y = rows.min; y < rows.max; ++y)
  {
    F4       *src_row  =  image_row(src, y);
    Simd_F4  *dst_row  =  image_rgb_row(dst, y);

    for (L1 block = 0; block < image_rgb_block_count(src.width); ++block)
    {
      L1        first_pixel         =  block * SIMD_WIDTH;
      Simd_F4   color               =  rgb_load_pixels(src_row, first_pixel, src.width);
      Simd_F1  luminance           =  luminance(color);
      Simd_F1  distance_into_knee  =  luminance - knee_start;
      Simd_F1  knee_contribution   =  Square(distance_into_knee) * inv_knee_normalization;
      Simd_F1  soft_contribution   =  select(luminance > knee_start, knee_contribution, zero);
      Simd_F1  contribution        =  select(luminance > knee_end, one, soft_contribution);
      Simd_Mask active_pixels       =  (contribution > zero) & (luminance > splat_Simd_F1(1e-6f));
      Simd_F1  safe_luminance      =  select(active_pixels, luminance, one);
      Simd_F1  excess_luminance    =  max(zero, luminance - threshold);
      Simd_F1  excess_fraction     =  excess_luminance / safe_luminance;
      Simd_F1  bloom_scale         =  excess_fraction * contribution;

      color = scale(color, bloom_scale);

      // The selects double as NaN masks for degenerate input pixels:
      // bloom_scale is zero there, but 0 * NaN would still be NaN.
      dst_row[block] = (Simd_F4){
        select(active_pixels, color.x, zero),
        select(active_pixels, color.y, zero),
        select(active_pixels, color.z, zero),
        select(active_pixels, color.w, zero)
      };
    }
  }

  ProfEnd();
}

// Input pixels are indexed relative to [first | second]: negatives read from
// the tail of previous, values past the end read from the head of following.
Inline F1 rgb_pixel_at(SL1 index, Simd_F1 previous, Simd_F1 first, Simd_F1 second, Simd_F1 following)
{
  Simd_F1  source  =  first;
  SL1       lane    =  index;

  if (index < 0)
  {
    source  =  previous;
    lane    +=  SIMD_WIDTH;
  }
  else if (index >= 2 * SIMD_WIDTH)
  {
    source  =  following;
    lane    -=  2 * SIMD_WIDTH;
  }
  else if (index >= SIMD_WIDTH)
  {
    source  =  second;
    lane    -=  SIMD_WIDTH;
  }

  return source[lane];
}

Inline Simd_F1 rgb_down2_channel(Simd_F1 previous, Simd_F1 first, Simd_F1 second, Simd_F1 following)
{
  Simd_F1 result = {0};

  for (L1 i = 0; i < SIMD_WIDTH; ++i)
  {
    SL1 center = 2 * (SL1)i;

    F1  far_left   =  rgb_pixel_at(center - 1, previous, first, second, following);
    F1  left       =  rgb_pixel_at(center,     previous, first, second, following);
    F1  right      =  rgb_pixel_at(center + 1, previous, first, second, following);
    F1  far_right  =  rgb_pixel_at(center + 2, previous, first, second, following);

    result[i] = 0.5f * (0.25f * far_left + 0.75f * left + 0.75f * right + 0.25f * far_right);
  }

  return result;
}

Inline Simd_F1 rgb_up2_channel(Simd_F1 previous, Simd_F1 center, Simd_F1 next, I1 output_half)
{
  Simd_F1 result = {0};

  for (L1 i = 0; i < SIMD_WIDTH; ++i)
  {
    L1   output_pixel  =  output_half * SIMD_WIDTH + i;
    SL1  src_index     =  (SL1)(output_pixel / 2);

    F1  in              =  center[src_index];
    F1  left_neighbor   =  src_index > 0                ? center[src_index - 1] : previous[SIMD_WIDTH - 1];
    F1  right_neighbor  =  src_index < SIMD_WIDTH - 1   ? center[src_index + 1] : next[0];

    F1  even_sample  =  0.25f * left_neighbor + 0.75f * in;
    F1  odd_sample   =  0.75f * in + 0.25f * right_neighbor;

    result[i] = (output_pixel & 1) ? odd_sample : even_sample;
  }

  return result;
}

typedef struct RGB_Filter_Tap
{
  L1 index;
  F1 weight;
} RGB_Filter_Tap;

typedef struct RGB_Filter_Span
{
  L1 offset;
  L1 count;
  F1 weight_sum;
} RGB_Filter_Span;

Internal void image_rgb_resample_x(Image_RGB dst, Image_RGB src, Range rows, Arena *arena)
{
  ProfBeginFunc();

  L1  src_block_count  =  image_rgb_block_count(src.width);
  L1  dst_block_count  =  image_rgb_block_count(dst.width);

  if (   src.width   >   0
      && dst.width   >   0
      && src.height  ==  dst.height
      && rows.min    <   rows.max)
  {
    I1               is_downsample_2x  =  src.width == (2 * (L1)dst.width);
    I1               is_upsample_2x    =  dst.width == (2 * (L1)src.width);
    Temp_Arena       temp              =  temp_arena_begin(arena);
    RGB_Filter_Span  *spans            =  0;
    RGB_Filter_Tap   *taps             =  0;

    if (!is_downsample_2x && !is_upsample_2x)
    {
      //- kti: Build the horizontal filter once per invocation, rather than per row.

      F1  source_step      =  (F1)src.width / dst.width;
      F1  filter_radius    =  Max(1.0f, source_step);
      L1  total_tap_count  =  0;

      spans = push_array_no_zero(arena, RGB_Filter_Span, dst.width);

      for (L1 x = 0; x < dst.width; ++x)
      {
        F1  source_center       =  ((F1)x + 0.5f) * source_step - 0.5f;
        F1  first_source_pixel  =  ceil_F1(source_center - filter_radius);
        F1  last_source_pixel   =  floor_F1(source_center + filter_radius);
        L1  sample_count        =  (L1)(last_source_pixel - first_source_pixel) + 1;

        spans[x] = (RGB_Filter_Span){total_tap_count, sample_count, 0};
        total_tap_count += sample_count;
      }

      taps = push_array_no_zero(arena, RGB_Filter_Tap, total_tap_count);

      for (L1 x = 0; x < dst.width; ++x)
      {
        F1  source_center       =  ((F1)x + 0.5f) * source_step - 0.5f;
        F1  first_source_pixel  =  ceil_F1(source_center - filter_radius);

        for (L1 tap_offset = 0; tap_offset < spans[x].count; ++tap_offset)
        {
          F1  source_position      =  first_source_pixel + (F1)tap_offset;
          F1  normalized_distance  =  (source_position - source_center) / filter_radius;
          F1  weight               =  Max(0, 1.0f - abs(normalized_distance));
          L1  index                =  (L1)Clamp(0.0f, source_position, (F1)src.width - 1);

          taps[spans[x].offset + tap_offset]  =   (RGB_Filter_Tap){ index, weight };
          spans[x].weight_sum        +=  weight;
        }
      }
    }

    for (L1 y = rows.min; y < rows.max; ++y)
    {
      Simd_F4  *src_row  =  image_rgb_row(src, y);
      Simd_F4  *dst_row  =  image_rgb_row(dst, y);

      for (L1 block = 0; block < dst_block_count; ++block)
      {
        Simd_F4 result = {0};

        if (is_downsample_2x || is_upsample_2x)
        {
          L1       first_src_block  =  is_downsample_2x ? (2 * block) : (block / 2);
          Simd_F4  first_block      =  src_row[first_src_block];
          Simd_F4  previous_block   =  first_src_block ? src_row[first_src_block - 1] : first_block;
          Simd_F4  next_block       =  src_row[Min(first_src_block + 1, src_block_count - 1)];

          if (!first_src_block)
          {
            previous_block = rgb_splat_lane(first_block, 0);
          }

          if (first_src_block + 1 == src_block_count)
          {
            next_block = rgb_splat_lane(first_block, SIMD_WIDTH - 1);
          }

          if (is_downsample_2x)
          {
            Simd_F4 following_block = src_row[Min(first_src_block + 2, src_block_count - 1)];

            if (first_src_block + 2 >= src_block_count)
            {
              following_block = rgb_splat_lane(next_block, SIMD_WIDTH - 1);
            }

            result = (Simd_F4){
              rgb_down2_channel(previous_block.x, first_block.x, next_block.x, following_block.x),
              rgb_down2_channel(previous_block.y, first_block.y, next_block.y, following_block.y),
              rgb_down2_channel(previous_block.z, first_block.z, next_block.z, following_block.z),
              rgb_down2_channel(previous_block.w, first_block.w, next_block.w, following_block.w)
            };
          }
          else
          {
            I1 output_half = block & 1;

            result = (Simd_F4){
              rgb_up2_channel(previous_block.x, first_block.x, next_block.x, output_half),
              rgb_up2_channel(previous_block.y, first_block.y, next_block.y, output_half),
              rgb_up2_channel(previous_block.z, first_block.z, next_block.z, output_half),
              rgb_up2_channel(previous_block.w, first_block.w, next_block.w, output_half)
            };
          }
        }
        else
        {
          RGB_Filter_Span  span[SIMD_WIDTH]  =  {0};
          L1               max_tap_count     =  0;

          for (L1 lane = 0; lane < SIMD_WIDTH; ++lane)
          {
            L1 output_pixel = Min(block * SIMD_WIDTH + lane, (L1)dst.width - 1);

            span[lane] = spans[output_pixel];
            max_tap_count = Max(max_tap_count, span[lane].count);
          }

          result = (Simd_F4){0};

          for (L1 tap_offset = 0; tap_offset < max_tap_count; ++tap_offset)
          {
            L1        indices[SIMD_WIDTH]  =  {0};
            Simd_F1  weights              =  {0};

            for (L1 lane = 0; lane < SIMD_WIDTH; ++lane)
            {
              L1  tap_in_span  =  Min(tap_offset, span[lane].count - 1);
              L1  tap_index    =  span[lane].offset + tap_in_span;

              RGB_Filter_Tap tap = taps[tap_index];

              indices[lane] = tap.index;
              weights[lane] = tap_offset < span[lane].count ? tap.weight : 0;
            }

            Simd_F4  samples           =  gather_Simd_F4(src_row, indices);
            Simd_F4  weighted_samples  =  scale(samples, weights);

            result = add(result, weighted_samples);
          }

          Simd_F1 weight_sums = {0};

          for (L1 lane = 0; lane < SIMD_WIDTH; ++lane)
          {
            weight_sums[lane] = span[lane].weight_sum;
          }

          Simd_F1 inv_weight_sums = splat_Simd_F1(1.0f) / weight_sums;

          result = scale(result, inv_weight_sums);
        }

        L1  remaining_pixels  =  (L1)dst.width - block * SIMD_WIDTH;
        L1  valid_lanes       =  Min((L1)SIMD_WIDTH, remaining_pixels);

        dst_row[block] = pad_Simd_F4(result, valid_lanes);
      }
    }

    temp_arena_end(temp);
  }

  ProfEnd();
}

typedef struct RGB_Row_Tap
{
  Simd_F4  *row;
  Simd_F1 weight; // Splatted once per row instead of once per block.
} RGB_Row_Tap;

// NOTE(kti): With accumulate set, dst is read and summed into rather than
// overwritten; the caller must guarantee dst is fully initialized.
Internal void image_rgb_resample_y(Image_RGB dst, Image_RGB src, Range rows, I1 karis, I1 accumulate, Arena *arena)
{
  ProfBeginFunc();

  if (src.height != 0 && dst.height != 0 && src.width == dst.width && rows.min < rows.max)
  {
    I1           is_downsample_2x  =  src.height == 2 * (L1)dst.height;
    I1           is_upsample_2x    =  dst.height == 2 * (L1)src.height;
    F1           source_step       =  (F1)src.height / dst.height;
    F1           filter_radius     =  Max(1.0f, source_step);
    Temp_Arena   temp              =  temp_arena_begin(arena);
    RGB_Row_Tap  *taps             =  0;

    if (!is_downsample_2x && !is_upsample_2x)
    {
      L1 max_tap_count = (L1)ceil_F1(2.0f * filter_radius) + 2;

      taps = push_array_no_zero(arena, RGB_Row_Tap, max_tap_count);
    }

    for (L1 y = rows.min; y < rows.max; ++y)
    {
      Simd_F4  *dst_row     =  image_rgb_row(dst, y);
      Simd_F4  *first_row   =  0;
      Simd_F4  *second_row  =  0;
      Simd_F4  *third_row   =  0;
      Simd_F4  *fourth_row  =  0;
      F1       weight_sum   =  0;
      L1       tap_count    =  0;

      if (is_downsample_2x)
      {
        L1 first_src_row = 2 * y;

        L1  far_top_index     =  first_src_row ? first_src_row - 1 : 0;
        L1  top_index         =  first_src_row;
        L1  bottom_index      =  first_src_row + 1;
        L1  far_bottom_index  =  Min(first_src_row + 2, src.height - 1);

        first_row  = image_rgb_row(src, far_top_index);
        second_row = image_rgb_row(src, top_index);
        third_row  = image_rgb_row(src, bottom_index);
        fourth_row = image_rgb_row(src, far_bottom_index);
      }
      else if (is_upsample_2x)
      {
        L1 first_src_row = y / 2;

        I1  is_odd_row     =  y & 1;
        L1  previous_row   =  first_src_row ? first_src_row - 1 : 0;
        L1  next_row       =  Min(first_src_row + 1, src.height - 1);
        L1  top_row_index  =  is_odd_row ? first_src_row : previous_row;
        L1  bot_row_index  =  is_odd_row ? next_row : first_src_row;

        first_row  = image_rgb_row(src, top_row_index);
        second_row = image_rgb_row(src, bot_row_index);
      }
      else
      {
        // Filter positions and weights are shared by every block in this row.
        F1  source_center     =  ((F1)y + 0.5f) * source_step - 0.5f;
        F1  first_source_row  =  ceil_F1(source_center - filter_radius);
        F1  last_source_row   =  floor_F1(source_center + filter_radius);

        tap_count = (L1)(last_source_row - first_source_row) + 1;

        for (L1 tap_offset = 0; tap_offset < tap_count; ++tap_offset)
        {
          F1  source_position      =  first_source_row + (F1)tap_offset;
          F1  normalized_distance  =  (source_position - source_center) / filter_radius;
          F1  weight               =  Max(0, 1.0f - abs(normalized_distance));
          L1  source_sample        =  (L1)Clamp(0.0f, source_position, (F1)src.height - 1);

          Simd_F4 *sample_row = image_rgb_row(src, source_sample);

          taps[tap_offset]  =   (RGB_Row_Tap){sample_row, splat_Simd_F1(weight)};
          weight_sum      +=  weight;
        }
      }

      for (L1 block = 0; block < image_rgb_block_count(dst.width); ++block)
      {
        Simd_F4 result = {0};

        if (is_downsample_2x)
        {
          Simd_F4  far_top_contribution     =  scale(first_row[block], splat_Simd_F1(0.25f));
          Simd_F4  top_contribution         =  scale(second_row[block], splat_Simd_F1(0.75f));
          Simd_F4  bottom_contribution      =  scale(third_row[block], splat_Simd_F1(0.75f));
          Simd_F4  far_bottom_contribution  =  scale(fourth_row[block], splat_Simd_F1(0.25f));

          result = add(far_top_contribution, top_contribution);
          result = add(result, bottom_contribution);
          result = add(result, far_bottom_contribution);
          result = scale(result, splat_Simd_F1(0.5f));
        }
        else if (is_upsample_2x)
        {
          I1        is_odd_row  =  y & 1;
          Simd_F1  top_weight  =  splat_Simd_F1(is_odd_row ? 0.75f : 0.25f);
          Simd_F1  bot_weight  =  splat_Simd_F1(is_odd_row ? 0.25f : 0.75f);

          Simd_F4  top_contribution     =  scale(first_row[block], top_weight);
          Simd_F4  bottom_contribution  =  scale(second_row[block], bot_weight);

          result = add(top_contribution, bottom_contribution);
        }
        else
        {
          for (L1 tap_offset = 0; tap_offset < tap_count; ++tap_offset)
          {
            Simd_F4  source_sample    =  taps[tap_offset].row[block];
            Simd_F4  weighted_sample  =  scale(source_sample, taps[tap_offset].weight);

            result = add(result, weighted_sample);
          }

          Simd_F1 inv_weight_sum = splat_Simd_F1(1.0f / weight_sum);

          result = scale(result, inv_weight_sum);
        }
        if (karis)
        {
          Simd_F1  luminance     =  luminance(result);
          Simd_F1  inv_karis     =  splat_Simd_F1(1.0f) / (1.0f + luminance);

          result = scale(result, inv_karis);
        }

        if (accumulate)
        {
          result = add(dst_row[block], result);
        }

        dst_row[block] = result;
      }
    }

    temp_arena_end(temp);
  }

  ProfEnd();
}

Inline Simd_F1 image_table_lookup(Simd_F1 values, F1 *table, I1 count, F1 maximum)
{
  Simd_F1  zero              =  splat_Simd_F1(0);
  Simd_F1  positive_values   =  select(values > zero, values, zero);
  Simd_F1  clamped_values    =  min(positive_values, splat_Simd_F1(maximum));
  F1        entries_per_unit  =  count / maximum;
  Simd_F1  table_positions   =  clamped_values * entries_per_unit;
  Simd_SI1  lower_indices     =  __builtin_convertvector(table_positions, Simd_SI1);
  Simd_F1  lower_values      =  {0};
  Simd_F1  upper_values      =  {0};

  for (L1 lane = 0; lane < SIMD_WIDTH; ++lane)
  {
    I1  lower_index  =  lower_indices[lane];
    I1  upper_index  =  Min(lower_index + 1, count);

    lower_values[lane] = table[lower_index];
    upper_values[lane] = table[upper_index];
  }

  Simd_F1  lower_positions  =  __builtin_convertvector(lower_indices, Simd_F1);
  Simd_F1  blend_fractions  =  table_positions - lower_positions;
  Simd_F1  value_deltas     =  upper_values - lower_values;
  Simd_F1  interpolated     =  lower_values + blend_fractions * value_deltas;

  return interpolated;
}

Inline Simd_F1 rgb_tonemap_lottes(Simd_F1 values)
{
  Simd_F1 result = image_table_lookup(values, image_lottes_table, IMAGE_LOTTES_TABLE_COUNT, image_lottes_table_params.hdr_max);
  return result;
}

Inline void rgb_pack_srgb8(RGBA8 *out, Simd_F1 srgb_red, Simd_F1 srgb_green, Simd_F1 srgb_blue, L1 count)
{
  Simd_F1  half         =  splat_Simd_F1(0.5f);
  Simd_SI1  red_bytes    =  __builtin_convertvector(srgb_red + half, Simd_SI1);
  Simd_SI1  green_bytes  =  __builtin_convertvector(srgb_green + half, Simd_SI1);
  Simd_SI1  blue_bytes   =  __builtin_convertvector(srgb_blue + half, Simd_SI1);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  Simd_SI1  packed_red    =  red_bytes << 24;
  Simd_SI1  packed_green  =  green_bytes << 16;
  Simd_SI1  packed_blue   =  blue_bytes << 8;
  Simd_SI1  packed_alpha  =  splat_Simd_SI1(255);
#else
  Simd_SI1  packed_red    =  red_bytes;
  Simd_SI1  packed_green  =  green_bytes << 8;
  Simd_SI1  packed_blue   =  blue_bytes << 16;
  Simd_SI1  packed_alpha  =  splat_Simd_SI1(-16777216);
#endif
  Simd_SI1  packed_pixels  =  packed_red | packed_green | packed_blue | packed_alpha;
  L1        output_size    =  count * sizeof(RGBA8);

  memcpy(out, &packed_pixels, output_size);
}

Inline void rgb_store_srgb8(RGBA8 *out, Simd_F4 color, L1 count)
{
  Simd_F1  srgb_red    =  image_table_lookup(color.x, image_srgb_table, IMAGE_SRGB_TABLE_COUNT, 1);
  Simd_F1  srgb_green  =  image_table_lookup(color.y, image_srgb_table, IMAGE_SRGB_TABLE_COUNT, 1);
  Simd_F1  srgb_blue   =  image_table_lookup(color.z, image_srgb_table, IMAGE_SRGB_TABLE_COUNT, 1);

  rgb_pack_srgb8(out, srgb_red, srgb_green, srgb_blue, count);
}

Inline Simd_F1 rgb_overlay_luminance(Image overlay, Simd_F1 u, F1 v)
{
  Simd_F1 result = {0};

  if (!image_is_nil(overlay))
  {
    Simd_F1  source_x  =  u * (F1)(overlay.width - 1);
    F1        source_y  =  v * (F1)(overlay.height - 1);
    L1        top_y     =  Min((L1)source_y, (L1)overlay.height - 1);
    L1        bottom_y  =  Min(top_y + 1, (L1)overlay.height - 1);

    F4        *top_row          =  image_row(overlay, top_y);
    F4        *bottom_row       =  image_row(overlay, bottom_y);
    Simd_F1  horizontal_blend  =  {0};

    F4 top_left_pixels[SIMD_WIDTH], top_right_pixels[SIMD_WIDTH], bottom_left_pixels[SIMD_WIDTH], bottom_right_pixels[SIMD_WIDTH];

    for (L1 lane = 0; lane < SIMD_WIDTH; ++lane)
    {
      L1  left_x   =  Min((L1)source_x[lane], (L1)overlay.width - 1);
      L1  right_x  =  Min(left_x + 1, (L1)overlay.width - 1);

      horizontal_blend[lane]     =  source_x[lane] - (F1)left_x;
      top_left_pixels[lane]      =  top_row[left_x];
      top_right_pixels[lane]     =  top_row[right_x];
      bottom_left_pixels[lane]   =  bottom_row[left_x];
      bottom_right_pixels[lane]  =  bottom_row[right_x];
    }

    Simd_F4   top_left        =  pack_Simd_F4(top_left_pixels);
    Simd_F4   top_right       =  pack_Simd_F4(top_right_pixels);
    Simd_F4   bottom_left     =  pack_Simd_F4(bottom_left_pixels);
    Simd_F4   bottom_right    =  pack_Simd_F4(bottom_right_pixels);
    Simd_F4   top_color       =  lerp(top_left, horizontal_blend, top_right);
    Simd_F4   bottom_color    =  lerp(bottom_left, horizontal_blend, bottom_right);
    Simd_F1  vertical_blend  =  splat_Simd_F1(source_y - (F1)top_y);
    Simd_F4   sampled_color   =  lerp(top_color, vertical_blend, bottom_color);

    result = luminance(sampled_color);
  }

  return result;
}

Internal void image_rgb_bloom_tonemap(Image_RGBA8 dst, Image hdr, Image_RGB bloom, Image_Bloom_Params params, Image_Tonemap_Params tonemap, Range rows)
{
  ProfBeginFunc();

  image_lottes_prepare(tonemap.lottes);

  // NOTE(kti): Exposure alone needs no grade block; the composed table clamps.
  I1  needs_adjustments  =  tonemap.saturation != 1;
  I1  has_overlay        =  !image_is_nil(params.overlay);
  F1  exposure_scale     =  tonemap.exposure_ev == 0 ? 1.0f : exp2f(tonemap.exposure_ev);
  F1  lottes_hdr_max     =  image_lottes_table_params.hdr_max;

  for (L1 y = rows.min; y < rows.max; ++y)
  {
    F4       *hdr_row    =  image_row(hdr, y);
    Simd_F4  *bloom_row  =  image_rgb_row(bloom, y);
    RGBA8    *dst_row    =  image_rgba8_row(dst, y);
    F1       overlay_v   =  dst.height > 1 ? (F1)y / (F1)(dst.height - 1) : 0;

    for (L1 block = 0; block < image_rgb_block_count(dst.width); ++block)
    {
      L1        x             =  block * SIMD_WIDTH;
      L1        valid_pixels  =  Min((L1)SIMD_WIDTH, (L1)dst.width - x);
      Simd_F1  hdr_weight    =  splat_Simd_F1(1.0f - 0.5f * params.strength);
      Simd_F1  bloom_weight  =  splat_Simd_F1(params.strength);

      Simd_F1  bloom_modulation  =  splat_Simd_F1(1.0f);

      if (has_overlay)
      {
        Simd_F1 overlay_u = {0};

        for (L1 lane = 0; lane < SIMD_WIDTH; ++lane)
        {
          overlay_u[lane] = (F1)Min(x + lane, (L1)dst.width - 1);
        }

        overlay_u /= (F1)Max((SL1)dst.width - 1, 1);

        Simd_F1 overlay_luminance = rgb_overlay_luminance(params.overlay, overlay_u, overlay_v);

        bloom_modulation = 1.0f + overlay_luminance * params.overlay_strength;
      }

      Simd_F4  hdr_color           =  rgb_load_pixels(hdr_row, x, hdr.width);
      Simd_F4  hdr_contribution    =  scale(hdr_color, hdr_weight);
      Simd_F4  modulated_bloom     =  scale(bloom_row[block], bloom_modulation);
      Simd_F4  bloom_contribution  =  scale(modulated_bloom, bloom_weight);
      Simd_F4  combined_color      =  add(hdr_contribution, bloom_contribution);
      Simd_F4  color               =  scale(combined_color, splat_Simd_F1(exposure_scale));

      if (!needs_adjustments)
      {
        Simd_F1  srgb_red    =  image_table_lookup(color.x, image_lottes_srgb_table, IMAGE_LOTTES_TABLE_COUNT, lottes_hdr_max);
        Simd_F1  srgb_green  =  image_table_lookup(color.y, image_lottes_srgb_table, IMAGE_LOTTES_TABLE_COUNT, lottes_hdr_max);
        Simd_F1  srgb_blue   =  image_table_lookup(color.z, image_lottes_srgb_table, IMAGE_LOTTES_TABLE_COUNT, lottes_hdr_max);

        rgb_pack_srgb8(dst_row + x, srgb_red, srgb_green, srgb_blue, valid_pixels);
      }
      else
      {
        color = (Simd_F4){
          rgb_tonemap_lottes(color.x),
          rgb_tonemap_lottes(color.y),
          rgb_tonemap_lottes(color.z),
          color.w
        };

        color = (Simd_F4){
          min(max(color.x, splat_Simd_F1(0)), splat_Simd_F1(1)),
          min(max(color.y, splat_Simd_F1(0)), splat_Simd_F1(1)),
          min(max(color.z, splat_Simd_F1(0)), splat_Simd_F1(1)),
          color.w
        };

        Simd_F1 gray = luminance(color);

        Simd_F4 chroma = {
          color.x - gray,
          color.y - gray,
          color.z - gray,
          splat_Simd_F1(0)
        };

        Simd_F4 saturated_chroma = scale(chroma, splat_Simd_F1(tonemap.saturation));

        color = (Simd_F4){
          gray + saturated_chroma.x,
          gray + saturated_chroma.y,
          gray + saturated_chroma.z,
          color.w
        };

        rgb_store_srgb8(dst_row + x, color, valid_pixels);
      }
    }
  }

  ProfEnd();
}
