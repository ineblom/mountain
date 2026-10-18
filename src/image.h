#ifndef IMAGE_H
#define IMAGE_H

#pragma pack(push, 1)
typedef struct Bitmap_Header Bitmap_Header;
struct Bitmap_Header
{
  W1  file_type;
  I1  file_size;
  W1  reserved1, reserved2;
  I1  bitmap_offset;
  I1  size;
  SI1 width;
  SI1 height;
  W1  planes;
  W1  bits_per_pixel;
  I1  compression;
  I1  size_of_bitmap;
  SI1 horz_resolution;
  SI1 vert_resolution;
  I1  colors_used;
  I1  colors_important;
};
#pragma pack(pop)

typedef struct Image Image;
struct Image
{
  I1  width;
  I1  height;
  L1  row_stride;
  F4 *pixels;
};

// Four horizontal RGB pixels per block. Width is in pixels; stride is in blocks.
// The last block repeats the final pixel in its unused lanes for edge sampling.
typedef struct Image_RGB Image_RGB;
struct Image_RGB
{
  I1 width;
  I1 height;
  L1 block_stride;
  F4x3 *blocks;
};

typedef struct RGBA8 RGBA8;
struct RGBA8
{
  B1 r, g, b, a;
};

typedef struct Image_RGBA8 Image_RGBA8;
struct Image_RGBA8
{
  I1     width;
  I1     height;
  L1     row_stride;
  RGBA8 *pixels;
};

typedef struct Image_Lottes_Params Image_Lottes_Params;
struct Image_Lottes_Params
{
  F1 contrast;
  F1 shoulder;
  F1 hdr_max;
  F1 mid_in;
  F1 mid_out;
};

typedef struct Image_Tonemap_Params Image_Tonemap_Params;
struct Image_Tonemap_Params
{
  F1          exposure_ev; // Stops before tonemapping; 0 is neutral.
  F1          saturation;  // Display-linear RGB; 1 is neutral, 0 is grayscale.
  Image_Lottes_Params lottes;
};

typedef struct Image_Bloom_Params Image_Bloom_Params;
struct Image_Bloom_Params
{
  L1    pass_count;
  F1    threshold;
  F1    strength;
  F1    knee;
  Image overlay;
  F1    overlay_strength;
};

#endif
