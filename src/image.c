////////////////////////////////
//~ kti: Utils

Internal I1 image_is_nil( Image image ) {

  I1 result = image.width  == 0
           || image.height == 0 
           || image.pixels == 0;

  return result;

}

Internal F4 *image_row( Image image, L1 y ) {
  
  F4 *result = image.pixels + image.row_stride*y;
  return result; 

}

Internal RGBA8 *image_rgba8_row( Image_RGBA8 image, L1 y ) {
  
  RGBA8 *result = image.pixels + image.row_stride*y;
  return result; 

}

////////////////////////////////
//~ kti: Create

Internal Image image_alloc( Arena *arena, I1 width, I1 height ) {

  Image image = {0};

  if ( width > 0 && height > 0 ) {

    L1 pixel_count  =  (L1)width * (L1)height;

    image.width       =  width;
    image.height      =  height;
    image.row_stride  =  width;
    image.pixels      =  push_array( arena, F4, pixel_count );

  }

  return image;
}

Internal Image_RGBA8 image_rgba8_alloc( Arena *arena, I1 width, I1 height ) {

  Image_RGBA8 image = {0};

  if ( width > 0 && height > 0 ) {

    L1 pixel_count  =  (L1)width * (L1)height;

    image.width       =  width;
    image.height      =  height;
    image.row_stride  =  width;
    image.pixels      =  push_array( arena, RGBA8, pixel_count );

  }

  return image;
}

Internal Image_RGBA8 image_read_from_file( Arena *arena, String8 filename ) {

  Temp_Arena     scratch   =  scratch_begin( &arena, 1 );
  Image_RGBA8    result    =  {0};
  String8        contents  =  os_read_entire_file( scratch.arena, filename );
  Bitmap_Header  *header   =  (Bitmap_Header *)contents.str;

  if (    contents.len >= sizeof(Bitmap_Header)
       && header             !=  0
       && header->file_type  ==  0x4D42
       && ( header->bits_per_pixel == 24 || header->bits_per_pixel == 32 )
       && header->compression  ==  0
       && header->size         ==  40
       && header->planes       ==  1
       && header->width        >   0
       && header->height       !=  0 ) {

    L1 width   =  (L1)header->width;
    L1 height  =  ( header->height < 0 ) ? (L1)(-(SL1)header->height) : (L1)header->height;

    L1 src_pixel_size  =  header->bits_per_pixel / 8;
    L1 src_row_pitch   =  AlignPow2( width*src_pixel_size, 4 );
    L1 bitmap_offset   =  header->bitmap_offset;

    I1 pixels_are_valid =
         bitmap_offset  >=  sizeof( Bitmap_Header )
      && bitmap_offset  <=  contents.len
      && height         <=  ( contents.len - bitmap_offset ) / src_row_pitch;

    if ( pixels_are_valid ) {

      result.width       =  width;
      result.height      =  height;
      result.row_stride  =  width;

      B1  *src_pixel_base  =  contents.str + header->bitmap_offset;

      result.pixels = push_array( arena, RGBA8, result.row_stride*result.height );

      for ( L1 y=0;  y<result.height;  y+=1 ) {

        L1  src_y  =  y;

        if ( header->height < 0 ) {
          src_y = result.height - 1 - y;
        }

        B1     *src_row  =  src_pixel_base + src_row_pitch*src_y;
        RGBA8  *dst_row  =  result.pixels  + result.row_stride*y;

        for (L1 x = 0; x < result.width; x += 1) {
          B1     *src  =  src_row + x*src_pixel_size;
          RGBA8  *dst  =  &dst_row[x];

          dst->r  =  src[2];
          dst->g  =  src[1];
          dst->b  =  src[0];
          dst->a  =  ( src_pixel_size == 4 ) ? src[3] : 0xFF;
        }
      }
    }
  }

  scratch_end( scratch );

  return result;
}

////////////////////////////////
//~ kti: Write

Internal void image_write_to_file( Image_RGBA8 image, String8 filename ) {

  if (    image.width       >   0
       && image.height      >   0
       && image.row_stride  >=  image.width
       && image.pixels      !=  0 ) {

    L1  bitmap_row_size  =  (L1)image.width * sizeof( RGBA8 );
    L1  bitmap_size      =  (L1)image.height * bitmap_row_size;
    L1  file_size        =  sizeof( Bitmap_Header ) + bitmap_size;

    if ( file_size <= I1_MAX ) {

      Temp_Arena  scratch     =  scratch_begin( 0, 0 );
      B1         *file_data   =  push_array_no_zero( scratch.arena, B1, file_size );

      Bitmap_Header  *header  =  (Bitmap_Header *)file_data;
      *header                  =  (Bitmap_Header){0};
      header->file_type        =  0x4D42;
      header->file_size        =  (I1)file_size;
      header->bitmap_offset    =  sizeof( Bitmap_Header );
      header->size             =  40;
      header->width            =  (SI1)image.width;
      header->height           =  (SI1)image.height;
      header->planes           =  1;
      header->bits_per_pixel   =  32;
      header->compression      =  0;
      header->size_of_bitmap   =  (I1)bitmap_size;

      B1  *bitmap_pixels  =  file_data + header->bitmap_offset;

      for ( L1 y=0;  y<image.height;  y+=1 ) {

        RGBA8  *src_row  =  image.pixels + y*image.row_stride;
        B1     *dst_row  =  bitmap_pixels + y*bitmap_row_size;

        for ( L1 x=0;  x<image.width;  x+=1 ) {

          RGBA8  src  =  src_row[x];
          B1     *dst =  dst_row + x*sizeof( RGBA8 );

          dst[0]  =  src.b;
          dst[1]  =  src.g;
          dst[2]  =  src.r;
          dst[3]  =  src.a;

        }
      }

      L1  bytes_written  =  os_write_entire_file( filename, file_data, file_size );
      if ( bytes_written != file_size ) {
        fprintf( stderr, "Could not write image file.\n" );
      }

      scratch_end( scratch );

    }
  }
}

////////////////////////////////
//~ kti: Sample
/* TODO
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
*/

Internal void image_bloom_threshold( Image dst, Image src, Image_Bloom_Params params, Range rows ) {
  //- kti: Equal dimensions.
  if (    src.width  == dst.width
       && src.height == dst.height ) {

    for ( L1 y=rows.min;  y<rows.max;  y+=1 ) {

      F4  *src_row  =  image_row( src, y );
      F4  *dst_row  =  image_row( dst, y );

      for ( L1 x=0;  x<src.width;  x+=1 ) {

        F4 color = src_row[x];

        F1   luminance       =  luminance_F4( color );
        F1   soft_threshold  =  params.threshold - params.knee;
        F1   knee_range      =  2.0f * params.knee;
        F1   contrib         =  0.0f;

        if (  luminance  >  params.threshold + params.knee  ) {

          contrib = 1.0f;

        } else if ( luminance > soft_threshold ) {

          F1  x             =  luminance - soft_threshold;
          F1  knee_contrib  =  Square( x ) / ( 4.0f * Square( knee_range ) );
          contrib           =  knee_contrib;

        }

        if (  contrib > 0.0f  &&  luminance > 1e-6f  ) {

          F1 excess_luminance  =   Max( 0.0f, luminance - params.threshold );
          color                *=  excess_luminance/luminance * contrib;

        } else {

          color  =  (F4){0};

        }

        dst_row[x]  =  color;

      }

    }

  }

}

Internal void image_resample_x( Image dst, Image src, Range rows ) {

  if ( src.height == dst.height ) {

    L1 src_dim = src.width;
    L1 dst_dim = dst.width;

    F1  scale          =  (F1)src_dim / (F1)dst_dim;
    F1  filter_scale   =  Max( 1, scale );
    F1  filter_radius  =  1.0f;
    F1  support        =  filter_radius*filter_scale;

    for ( L1 y=rows.min; y<rows.max; y+=1 ) {

      F4 *src_row = image_row( src, y );
      F4 *dst_row = image_row( dst, y );

      for ( L1 dst_x=0;  dst_x<dst_dim;  dst_x+=1 ) {

        F1  center         =  ( (F1)dst_x + 0.5f ) * scale - 0.5f;
        F1  first_src_pos  =  ceil_F1( center - support );
        F1  last_src_pos   =  floor_F1( center + support );
        L1  src_pos_count  =  (L1)( last_src_pos - first_src_pos ) + 1;
        F4  sum            =  {0};
        F1  weight_sum     =  0;

        for ( L1 src_pos_idx=0;  src_pos_idx<src_pos_count; src_pos_idx+=1 ) {

          F1  src_pos   =  first_src_pos + (F1)src_pos_idx;
          F1  distance  =  ( src_pos - center ) / filter_scale;
          F1  weight    =  Max( 0, 1.0f - abs_F1( distance ) );

          L1  sample_x     =  (L1)Clamp( 0.0f, src_pos, (F1)src_dim - 1.0f );
              sum         +=  src_row[sample_x] * weight;
              weight_sum  +=  weight;

        }

        if ( weight_sum > 0.0f ) {

          dst_row[dst_x] = sum / weight_sum;

        } else {

          L1 nearest_x = (L1)Clamp(0.0f, floor_F1(center+0.5f), (F1)src_dim-1.0f);
          dst_row[dst_x] = src_row[nearest_x];

        }
      }
    }
  }
}

Internal void image_resample_y(Image dst, Image src, Range rows) {

  if ( src.width == dst.width ) {

    L1 src_dim = src.height;
    L1 dst_dim = dst.height;

    F1  scale          =  (F1)src_dim / (F1)dst_dim;
    F1  filter_scale   =  Max(1, scale);
    F1  filter_radius  =  1.0f;
    F1  support        =  filter_radius*filter_scale;

    for ( L1 dst_y=rows.min;  dst_y<rows.max;  dst_y += 1 ) {

      F1  weight_sum     =  0;
      F1  center         =  ( (F1)dst_y + 0.5f ) * scale - 0.5f;
      F1  first_src_pos  =  ceil_F1( center - support );
      F1  last_src_pos   =  floor_F1( center + support );
      L1  src_pos_count  =  (L1)( last_src_pos - first_src_pos ) + 1;
      F4 *dst_row        =  image_row(dst, dst_y);

      //- kti: Clear dst row.
      for ( L1 x=0;  x<dst.width;  x+=1 ) {
        dst_row[x] = (F4){0};
      }

      for ( L1 src_pos_idx=0;  src_pos_idx<src_pos_count;  src_pos_idx += 1 ) {

        F1  src_pos   =  first_src_pos + (F1)src_pos_idx;
        F1  distance  =  ( src_pos - center ) / filter_scale;
        F1  weight    =  Max( 0, 1.0f - abs_F1( distance ) );
        L1  sample_y  =  (L1)Clamp( 0.0f, src_pos, (F1)src_dim-1.0f );
        F4 *src_row   =  image_row( src, sample_y );

        for (L1 x = 0; x < dst.width; x += 1) {

          dst_row[x] += src_row[x]*weight;

        }
        
        weight_sum += weight;

      }

      if ( weight_sum > 0.0f ) {

        for ( L1 x=0;  x<dst.width;  x+=1 ) {
          dst_row[x] /= weight_sum;
        }

      } else {

        L1  nearest_y  =  (L1)Clamp( 0.0f, floor_F1( center+0.5f ), (F1)src_dim - 1.0f );
        F4  *src_row   =  image_row( src, nearest_y );
        for ( L1 x=0;  x<dst.width;  x+=1 ) {

          dst_row[x] = src_row[x];

        }

      }
    }
  }
}

Internal void image_apply_karis( Image image, Range rows ) {

  for ( L1 y=rows.min;  y<rows.max;  y+=1 ) {

    F4 *row = image_row(image, y);

    for ( L1 x=0;  x<image.width;  x+=1 ) {

      F4 color   =  row[x];
         row[x]  =  color / ( 1.0f + luminance_F4( color ) );

    }
  }
}

Internal void image_add( Image dst, Image src, Range rows ) {

  if (    src.width  == dst.width
       && src.height == dst.height ) {

    for (L1 y = rows.min; y < rows.max; y += 1) {

      F4 *src_row = image_row(src, y);
      F4 *dst_row = image_row(dst, y);

      for (L1 x = 0; x < src.width; x += 1) {
        dst_row[x] += src_row[x];
      }

    }

  }

}

Inline F4 tonemap_aces( F4 v ) {
  F1 a = 2.51f;
  F1 b = 0.03f;
  F1 c = 2.43f;
  F1 d = 0.59f;
  F1 e = 0.14f;

  F4 result = clamp01_F4( (v * (a * v + b)) / (v * (c * v + d) + e) );

  return result;
}

Inline F4 tonemap_reinhard( F4 v ) {

  F4 result = v / ( 1.0f + v );

  return result;

}

Inline F4 tonemap_lottes( F4 v ) {

  // Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
  F1 a = 1.6f;
  F1 d = 0.977f;
  F1 hdr_max = 8.0f;
  F1 mid_in = 0.18f;
  F1 mid_out = 0.267f;

  // TODO: Precompute
  const F1 b =
      (-powf(mid_in, a) + powf(hdr_max, a) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);
  const float c =
      (powf(hdr_max, a * d) * powf(mid_in, a) - powf(hdr_max, a) * powf(mid_in, a * d) * mid_out) /
      ((powf(hdr_max, a * d) - powf(mid_in, a * d)) * mid_out);

  F4 result = pow_F4(v, a) / (pow_F4(v, a * d) * b + c);

  return result;

}

Internal F4 tonemap(Tonemap_Kind kind, F4 v) {

  F4 result = v;

  switch (kind) {
    case TONEMAP_KIND__ACES     :  result = tonemap_aces     ( v ); break;
    case TONEMAP_KIND__REINHARD :  result = tonemap_reinhard ( v ); break;
    case TONEMAP_KIND__LOTTES   :  result = tonemap_lottes   ( v ); break;
  }

  return result;

}

Inline F1 srgb_from_linear( F1 l ) {

  if ( l < 0.0f ) l = 0.0f;
  if ( l > 1.0f ) l = 1.0f;

  F1 s = l * 12.92f;

  if ( l > 0.0031308f ) {
    s = 1.055f * powf( l, 1.0f/2.4f ) - 0.055f;
  }

  return s;

}

Internal Image_RGBA8 image_tonemap(Arena *arena, Image input, Tonemap_Kind tonemap_kind) {

  Image_RGBA8 result = image_rgba8_alloc(arena, input.width, input.height);

  for ( L1 y=0;  y<input.height;  y+=1 ) {

    F4     *in_row   =  image_row( input, y );
    RGBA8  *out_row  =  image_rgba8_row( result, y );

    for ( L1 x=0;  x<input.width;  x+=1 ) {

      F4     in_px       =  in_row [x];
      RGBA8 *out_px      =  &out_row[x];
      F4     tonemapped  =  tonemap( tonemap_kind, in_px );

      F4 out_color = {
        255.0f*srgb_from_linear(tonemapped[0]),
        255.0f*srgb_from_linear(tonemapped[1]),
        255.0f*srgb_from_linear(tonemapped[2]),
        255.0f,
      };

      out_px->r = (B1)out_color[0];
      out_px->g = (B1)out_color[1];
      out_px->b = (B1)out_color[2];
      out_px->a = (B1)out_color[3];

    }

  }

  return result;

}
