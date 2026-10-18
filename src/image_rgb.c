Inline L1 image_rgb_block_count(I1 width)
{
  return ((L1)width + 3) / 4;
}

Inline F4x3 *image_rgb_row(Image_RGB image, L1 y)
{
  return image.blocks + y * image.block_stride;
}

Inline F4x3 rgb_load_pixels(F4 *row, L1 x, I1 width)
{
  F4 a = row[x];
  F4 b = row[Min(x + 1, width - 1)];
  F4 c = row[Min(x + 2, width - 1)];
  F4 d = row[Min(x + 3, width - 1)];

  return F4x3_from_F4(a, b, c, d);
}

Internal Image_RGB image_rgb_alloc(Arena *arena, I1 width, I1 height)
{
  Image_RGB result = {0};

  if (width && height)
  {
    result         =  (Image_RGB){width, height, image_rgb_block_count(width), 0};
    result.blocks  =  push_array_no_zero(arena, F4x3, result.block_stride * height);
  }

  return result;
}

Internal void image_rgb_bloom_threshold(Image_RGB dst, Image src, Image_Bloom_Params params, Range rows)
{
  ProfBeginFunc();

  F4  zero                =  splat_F4(0);
  F4  one                 =  splat_F4(1);
  F4  threshold           =  splat_F4(params.threshold);
  F4  knee_start          =  splat_F4(params.threshold - params.knee);
  F4  knee_end            =  splat_F4(params.threshold + params.knee);
  F1  knee_width          =  2.0f * params.knee;
  F1  knee_normalization  =  4.0f * Square(knee_width);

  if (knee_normalization == 0) knee_normalization = 1.0f;

  for (L1 y = rows.min; y < rows.max; ++y)
  {
    F4    *src_row  =  image_row(src, y);
    F4x3  *dst_row  =  image_rgb_row(dst, y);

    for (L1 block = 0; block < image_rgb_block_count(src.width); ++block)
    {
      L1    first_pixel         =  block * 4;
      F4x3  color               =  rgb_load_pixels(src_row, first_pixel, src.width);
      F4    luminance           =  luminance_F4x3(color);
      F4    distance_into_knee  =  luminance - knee_start;
      F4    knee_contribution   =  Square(distance_into_knee) / knee_normalization;
      F4    soft_contribution   =  select_F4(luminance > knee_start, knee_contribution, zero);
      F4    contribution        =  select_F4(luminance > knee_end, one, soft_contribution);
      SI4   has_contribution    =  contribution > zero;
      SI4   has_luminance       =  luminance > splat_F4(1e-6f);
      SI4   active_pixels       =  has_contribution & has_luminance;
      F4    safe_luminance      =  select_F4(active_pixels, luminance, one);
      F4    excess_luminance    =  max_F4(zero, luminance - threshold);
      F4    excess_fraction     =  excess_luminance / safe_luminance;
      F4    bloom_scale         =  excess_fraction * contribution;

      color = scale_F4x3(color, bloom_scale);

      dst_row[block] = (F4x3){
        select_F4(active_pixels, color.x, zero),
        select_F4(active_pixels, color.y, zero),
        select_F4(active_pixels, color.z, zero)
      };
    }
  }

  ProfEnd();
}

Inline F4 rgb_down2_channel(F4 previous, F4 first, F4 second, F4 following)
{
  F4  first_shifted_left    =  shuffle_F4(previous, first,     F4_A3, F4_B0, F4_B1, F4_B2);
  F4  second_shifted_left   =  shuffle_F4(first,    second,    F4_A3, F4_B0, F4_B1, F4_B2);
  F4  first_shifted_right   =  shuffle_F4(first,    second,    F4_A2, F4_A3, F4_B0, F4_B1);
  F4  second_shifted_right  =  shuffle_F4(second,   following, F4_A2, F4_A3, F4_B0, F4_B1);

  F4  far_left   =  shuffle_F4(first_shifted_left,  second_shifted_left,  F4_A0, F4_A2, F4_B0, F4_B2);
  F4  left       =  shuffle_F4(first,               second,               F4_A0, F4_A2, F4_B0, F4_B2);
  F4  right      =  shuffle_F4(first,               second,               F4_A1, F4_A3, F4_B1, F4_B3);
  F4  far_right  =  shuffle_F4(first_shifted_right, second_shifted_right, F4_A0, F4_A2, F4_B0, F4_B2);

  F4  weighted_far_left   =  0.25f * far_left;
  F4  weighted_left       =  0.75f * left;
  F4  weighted_right      =  0.75f * right;
  F4  weighted_far_right  =  0.25f * far_right;
  F4  weighted_sum        =  weighted_far_left + weighted_left + weighted_right + weighted_far_right;
  F4  normalized_sum      =  weighted_sum * 0.5f;

  return normalized_sum;
}

Inline F4 rgb_up2_channel(F4 previous, F4 center, F4 next, I1 output_half)
{
  F4  left_neighbors   =  shuffle_F4(previous, center, F4_A3, F4_B0, F4_B1, F4_B2);
  F4  right_neighbors  =  shuffle_F4(center,   next,   F4_A1, F4_A2, F4_A3, F4_B0);
  F4  even_samples     =  0.25f * left_neighbors + 0.75f * center;
  F4  odd_samples      =  0.75f * center + 0.25f * right_neighbors;

  // Interleave the filtered samples into the first or second four-pixel block.
  F4 result =  output_half
               ? shuffle_F4(even_samples, odd_samples, F4_A2, F4_B2, F4_A3, F4_B3)  // {even[2], odd[2], even[3], odd[3]}
               : shuffle_F4(even_samples, odd_samples, F4_A0, F4_B0, F4_A1, F4_B1); // {even[0], odd[0], even[1], odd[1]}

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
          F1  weight               =  Max(0, 1.0f - abs_F1(normalized_distance));
          L1  index                =  (L1)Clamp(0.0f, source_position, (F1)src.width - 1);

          taps[spans[x].offset + tap_offset]  =   (RGB_Filter_Tap){ index, weight };
          spans[x].weight_sum        +=  weight;
        }
      }
    }

    for (L1 y = rows.min; y < rows.max; ++y)
    {
      F4x3  *src_row  =  image_rgb_row(src, y);
      F4x3  *dst_row  =  image_rgb_row(dst, y);

      for (L1 block = 0; block < dst_block_count; ++block)
      {
        F4x3 result = {0};

        if (is_downsample_2x || is_upsample_2x)
        {
          L1    first_src_block  =  is_downsample_2x ? (2 * block) : (block / 2);
          F4x3  first_block      =  src_row[first_src_block];
          F4x3  previous_block   =  first_src_block ? src_row[first_src_block - 1] : first_block;
          F4x3  next_block       =  src_row[Min(first_src_block + 1, src_block_count - 1)];

          if (!first_src_block)
          {
            previous_block = (F4x3){splat_F4(first_block.x[0]), splat_F4(first_block.y[0]), splat_F4(first_block.z[0])};
          }

          if (first_src_block + 1 == src_block_count)
          {
            next_block = (F4x3){
              splat_F4(first_block.x[3]),
              splat_F4(first_block.y[3]),
              splat_F4(first_block.z[3])
            };
          }

          if (is_downsample_2x)
          {
            F4x3 following_block = src_row[Min(first_src_block + 2, src_block_count - 1)];

            if (first_src_block + 2 >= src_block_count)
            {
              following_block = (F4x3){splat_F4(next_block.x[3]), splat_F4(next_block.y[3]), splat_F4(next_block.z[3])};
            }

            result = (F4x3){
              rgb_down2_channel(previous_block.x, first_block.x, next_block.x, following_block.x),
              rgb_down2_channel(previous_block.y, first_block.y, next_block.y, following_block.y),
              rgb_down2_channel(previous_block.z, first_block.z, next_block.z, following_block.z)
            };
          }
          else
          {
            I1 output_half = block & 1;

            result = (F4x3){
              rgb_up2_channel(previous_block.x, first_block.x, next_block.x, output_half),
              rgb_up2_channel(previous_block.y, first_block.y, next_block.y, output_half),
              rgb_up2_channel(previous_block.z, first_block.z, next_block.z, output_half)
            };
          }
        }
        else
        {
          RGB_Filter_Span  span[4]        =  {0};
          L1               max_tap_count  =  0;

          for (L1 lane = 0; lane < 4; ++lane)
          {
            L1 output_pixel = Min(block * 4 + lane, dst.width - 1);

            span[lane] = spans[output_pixel];
            max_tap_count = Max(max_tap_count, span[lane].count);
          }

          result = (F4x3){0};

          for (L1 tap_offset = 0; tap_offset < max_tap_count; ++tap_offset)
          {
            L1  indices[4]  =  {0};
            F4  weights     =  {0};

            for (L1 lane = 0; lane < 4; ++lane)
            {
              L1  tap_in_span  =  Min(tap_offset, span[lane].count - 1);
              L1  tap_index    =  span[lane].offset + tap_in_span;

              RGB_Filter_Tap tap = taps[tap_index];

              indices[lane] = tap.index;
              weights[lane] = tap_offset < span[lane].count ? tap.weight : 0;
            }

            F4x3  samples           =  gather_F4x3(src_row, indices);
            F4x3  weighted_samples  =  scale_F4x3(samples, weights);

            result = add_F4x3(result, weighted_samples);
          }

          F4 weight_sums = {
            span[0].weight_sum,
            span[1].weight_sum,
            span[2].weight_sum,
            span[3].weight_sum
          };

          result = (F4x3){
            result.x / weight_sums,
            result.y / weight_sums,
            result.z / weight_sums
          };
        }

        L1  remaining_pixels  =  dst.width - block * 4;
        L1  valid_lanes       =  Min((L1)4, remaining_pixels);

        dst_row[block] = pad_F4x3(result, valid_lanes);
      }
    }

    temp_arena_end(temp);
  }

  ProfEnd();
}

typedef struct RGB_Row_Tap
{
  F4x3 *row;
  F1 weight;
} RGB_Row_Tap;

Internal void image_rgb_resample_y(Image_RGB dst, Image_RGB src, Range rows, I1 karis, Arena *arena)
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
      F4x3  *dst_row     =  image_rgb_row(dst, y);
      F4x3  *first_row   =  0;
      F4x3  *second_row  =  0;
      F4x3  *third_row   =  0;
      F4x3  *fourth_row  =  0;
      F1    weight_sum   =  0;
      L1    tap_count    =  0;

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
          F1  weight               =  Max(0, 1.0f - abs_F1(normalized_distance));
          L1  source_sample        =  (L1)Clamp(0.0f, source_position, (F1)src.height - 1);

          F4x3 *sample_row = image_rgb_row(src, source_sample);

          taps[tap_offset]  =   (RGB_Row_Tap){sample_row, weight};
          weight_sum      +=  weight;
        }
      }

      for (L1 block = 0; block < image_rgb_block_count(dst.width); ++block)
      {
        F4x3 result = {0};

        if (is_downsample_2x)
        {
          F4x3  far_top_contribution     =  scale_F4x3(first_row[block], splat_F4(0.25f));
          F4x3  top_contribution         =  scale_F4x3(second_row[block], splat_F4(0.75f));
          F4x3  bottom_contribution      =  scale_F4x3(third_row[block], splat_F4(0.75f));
          F4x3  far_bottom_contribution  =  scale_F4x3(fourth_row[block], splat_F4(0.25f));

          result = add_F4x3(far_top_contribution, top_contribution);
          result = add_F4x3(result, bottom_contribution);
          result = add_F4x3(result, far_bottom_contribution);
          result = scale_F4x3(result, splat_F4(0.5f));
        }
        else if (is_upsample_2x)
        {
          I1  is_odd_row  =  y & 1;
          F4  top_weight  =  splat_F4(is_odd_row ? 0.75f : 0.25f);
          F4  bot_weight  =  splat_F4(is_odd_row ? 0.25f : 0.75f);

          F4x3  top_contribution     =  scale_F4x3(first_row[block], top_weight);
          F4x3  bottom_contribution  =  scale_F4x3(second_row[block], bot_weight);

          result = add_F4x3(top_contribution, bottom_contribution);
        }
        else
        {
          for (L1 tap_offset = 0; tap_offset < tap_count; ++tap_offset)
          {
            F4    weight           =  splat_F4(taps[tap_offset].weight);
            F4x3  source_sample    =  taps[tap_offset].row[block];
            F4x3  weighted_sample  =  scale_F4x3(source_sample, weight);

            result = add_F4x3(result, weighted_sample);
          }

          result = (F4x3){
            result.x / weight_sum,
            result.y / weight_sum,
            result.z / weight_sum
          };
        }
        if (karis)
        {
          F4  luminance          =  luminance_F4x3(result);
          F4  karis_denominator  =  1.0f + luminance;

          result = (F4x3){
            result.x / karis_denominator,
            result.y / karis_denominator,
            result.z / karis_denominator
          };
        }

        dst_row[block] = result;
      }
    }

    temp_arena_end(temp);
  }

  ProfEnd();
}

Internal void image_rgb_add(Image_RGB dst, Image_RGB src, Range rows)
{
  ProfBeginFunc();

  for (L1 y = rows.min; y < rows.max; ++y)
  {
    F4x3 *out = image_rgb_row(dst, y), *in = image_rgb_row(src, y);

    for (L1 b = 0; b < image_rgb_block_count(dst.width); ++b)
    {
      out[b] = add_F4x3(out[b], in[b]);
    }
  }

  ProfEnd();
}

Inline F4 image_table_lookup(F4 values, F1 *table, I1 count, F1 maximum)
{
  F4   zero              =  splat_F4(0);
  F4   positive_values   =  select_F4(values > zero, values, zero);
  F4   clamped_values    =  min_F4(positive_values, splat_F4(maximum));
  F1   entries_per_unit  =  count / maximum;
  F4   table_positions   =  clamped_values * entries_per_unit;
  SI4  lower_indices     =  __builtin_convertvector(table_positions, SI4);
  F4   lower_values      =  {0};
  F4   upper_values      =  {0};

  for (L1 lane = 0; lane < 4; ++lane)
  {
    I1  lower_index  =  lower_indices[lane];
    I1  upper_index  =  Min(lower_index + 1, count);

    lower_values[lane] = table[lower_index];
    upper_values[lane] = table[upper_index];
  }

  F4  lower_positions  =  __builtin_convertvector(lower_indices, F4);
  F4  blend_fractions  =  table_positions - lower_positions;
  F4  value_deltas     =  upper_values - lower_values;
  F4  interpolated     =  lower_values + blend_fractions * value_deltas;

  return interpolated;
}

Inline F4 rgb_tonemap_channel(F4 v, Tonemap_Kind kind, F1 inv_white_squared)
{
  F4 result = v;

  switch (kind)
  {
    case TONEMAP_KIND__ACES: {
      F4  numerator    =  v * (2.51f * v + 0.03f);
      F4  denominator  =  v * (2.43f * v + 0.59f) + 0.14f;
      F4  mapped       =  numerator / denominator;

      result = clamp01_F4(mapped);
    } break;

    case TONEMAP_KIND__REINHARD:
    {
      result = v / (1.0f + v);
      if (inv_white_squared > 0)
      {
        F4 white_point_scale = 1.0f + v * inv_white_squared;

        result *= white_point_scale;
      }
    } break;

    case TONEMAP_KIND__LOTTES: {
      result = image_table_lookup(v, image_lottes_table, IMAGE_LOTTES_TABLE_COUNT, 8.0f);
    } break;
  }

  return result;
}

Inline void rgb_store_srgb8(RGBA8 *out, F4x3 color, L1 count)
{
  F4  srgb_red    =  image_table_lookup(color.x, image_srgb_table, IMAGE_SRGB_TABLE_COUNT, 1);
  F4  srgb_green  =  image_table_lookup(color.y, image_srgb_table, IMAGE_SRGB_TABLE_COUNT, 1);
  F4  srgb_blue   =  image_table_lookup(color.z, image_srgb_table, IMAGE_SRGB_TABLE_COUNT, 1);

  SI4  red_bytes    =  __builtin_convertvector(srgb_red, SI4);
  SI4  green_bytes  =  __builtin_convertvector(srgb_green, SI4);
  SI4  blue_bytes   =  __builtin_convertvector(srgb_blue, SI4);
  // Pack four RGBA pixels with vector integer shifts. memcpy permits unaligned
  // destinations and writes only the valid pixels of a partial block.
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  SI4  packed_red    =  red_bytes << 24;
  SI4  packed_green  =  green_bytes << 16;
  SI4  packed_blue   =  blue_bytes << 8;
  SI4  packed_alpha  =  (SI4){255, 255, 255, 255};
#else
  SI4  packed_red    =  red_bytes;
  SI4  packed_green  =  green_bytes << 8;
  SI4  packed_blue   =  blue_bytes << 16;
  SI4  packed_alpha  =  (SI4){-16777216, -16777216, -16777216, -16777216};
#endif
  SI4  packed_pixels  =  packed_red | packed_green | packed_blue | packed_alpha;
  L1   output_size    =  count * sizeof(RGBA8);

  memcpy(out, &packed_pixels, output_size);
}

Inline F4 rgb_overlay_luminance(Image overlay, F4 u, F1 v)
{
  F4 result = {0};

  if (!image_is_nil(overlay))
  {
    F4  source_x  =  u * (F1)(overlay.width - 1);
    F1  source_y  =  v * (F1)(overlay.height - 1);
    L1  top_y     =  Min((L1)source_y, overlay.height - 1);
    L1  bottom_y  =  Min(top_y + 1, overlay.height - 1);

    F4  *top_row          =  image_row(overlay, top_y);
    F4  *bottom_row       =  image_row(overlay, bottom_y);
    F4  horizontal_blend  =  {0};

    F4 top_left_pixels[4], top_right_pixels[4], bottom_left_pixels[4], bottom_right_pixels[4];

    for (L1 lane = 0; lane < 4; ++lane)
    {
      L1  left_x   =  Min((L1)source_x[lane], overlay.width - 1);
      L1  right_x  =  Min(left_x + 1, overlay.width - 1);

      horizontal_blend[lane]   =  source_x[lane] - (F1)left_x;
      top_left_pixels[lane]    =  top_row[left_x];
      top_right_pixels[lane]   =  top_row[right_x];
      bottom_left_pixels[lane] =  bottom_row[left_x];
      bottom_right_pixels[lane] =  bottom_row[right_x];
    }

    F4x3  top_left        =  rgb_load_pixels(top_left_pixels, 0, 4);
    F4x3  top_right       =  rgb_load_pixels(top_right_pixels, 0, 4);
    F4x3  bottom_left     =  rgb_load_pixels(bottom_left_pixels, 0, 4);
    F4x3  bottom_right    =  rgb_load_pixels(bottom_right_pixels, 0, 4);
    F4x3  top_color       =  lerp_F4x3(top_left, horizontal_blend, top_right);
    F4x3  bottom_color    =  lerp_F4x3(bottom_left, horizontal_blend, bottom_right);
    F4    vertical_blend  =  splat_F4(source_y - (F1)top_y);
    F4x3  sampled_color   =  lerp_F4x3(top_color, vertical_blend, bottom_color);

    result = luminance_F4x3(sampled_color);
  }

  return result;
}

Internal void image_rgb_bloom_tonemap(Image_RGBA8 dst, Image hdr, Image_RGB bloom, Image_Bloom_Params params, Image_Tonemap_Params tonemap, Range rows)
{
  ProfBeginFunc();

  F1  exposure_scale     =  tonemap.exposure_ev == 0 ? 1.0f : exp2f(tonemap.exposure_ev);
  I1  uses_white_point   =  tonemap.kind == TONEMAP_KIND__REINHARD && tonemap.white_point > 0;
  F1  inv_white_squared  =  uses_white_point ? 1.0f / Square(tonemap.white_point) : 0;
  I1  needs_adjustments  =  tonemap.exposure_ev != 0 || tonemap.saturation != 1 || inv_white_squared != 0;

  for (L1 y = rows.min; y < rows.max; ++y)
  {
    F4     *hdr_row    =  image_row(hdr, y);
    F4x3   *bloom_row  =  image_rgb_row(bloom, y);
    RGBA8  *dst_row    =  image_rgba8_row(dst, y);
    F1     overlay_v  =  dst.height > 1 ? (F1)y / (F1)(dst.height - 1) : 0;

    for (L1 block = 0; block < image_rgb_block_count(dst.width); ++block)
    {
      L1  x             =  block * 4;
      L1  valid_pixels  =  Min((L1)4, dst.width - x);
      F4  overlay_u     =  {
        (F1)x,
        (F1)Min(x + 1, dst.width - 1),
        (F1)Min(x + 2, dst.width - 1),
        (F1)Min(x + 3, dst.width - 1)
      };

      overlay_u /= (F1)Max(dst.width - 1, 1);

      F4  overlay_luminance  =  rgb_overlay_luminance(params.overlay, overlay_u, overlay_v);
      F4  bloom_modulation   =  1.0f + overlay_luminance * params.overlay_strength;
      F4  hdr_weight         =  splat_F4(1.0f - 0.5f * params.strength);
      F4  bloom_weight       =  splat_F4(params.strength);

      F4x3  hdr_color           =  rgb_load_pixels(hdr_row, x, hdr.width);
      F4x3  hdr_contribution    =  scale_F4x3(hdr_color, hdr_weight);
      F4x3  modulated_bloom     =  scale_F4x3(bloom_row[block], bloom_modulation);
      F4x3  bloom_contribution  =  scale_F4x3(modulated_bloom, bloom_weight);
      F4x3  combined_color      =  add_F4x3(hdr_contribution, bloom_contribution);
      F4x3  color               =  scale_F4x3(combined_color, splat_F4(exposure_scale));

      color = (F4x3){
        rgb_tonemap_channel(color.x, tonemap.kind, inv_white_squared),
        rgb_tonemap_channel(color.y, tonemap.kind, inv_white_squared),
        rgb_tonemap_channel(color.z, tonemap.kind, inv_white_squared)
      };

      if (needs_adjustments)
      {
        color = (F4x3){clamp01_F4(color.x), clamp01_F4(color.y), clamp01_F4(color.z)};

        F4 gray = luminance_F4x3(color);

        F4x3 chroma = {
          color.x - gray,
          color.y - gray,
          color.z - gray
        };

        F4x3 saturated_chroma = scale_F4x3(chroma, splat_F4(tonemap.saturation));

        color = (F4x3){
          gray + saturated_chroma.x,
          gray + saturated_chroma.y,
          gray + saturated_chroma.z
        };
      }

      rgb_store_srgb8(dst_row + x, color, valid_pixels);
    }
  }

  ProfEnd();
}
