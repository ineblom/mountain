#ifndef IMAGE_H
#define IMAGE_H

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
