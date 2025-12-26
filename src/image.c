
#if (CPU_ && TYP_)

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

typedef struct Image Image;
struct Image {
	I1 width;
	I1 height;
	I1 bytes_per_pixel;
	B1 *pixels;
};

#endif


#if (CPU_ && ROM_)

Inline L1 image_pixels_size(Image image) {
	return image.bytes_per_pixel * image.width * image.height;
}

Inline Image image_alloc(Arena *arena, I1 width, I1 height, I1 bytes_per_pixel) {
	Image image  = {0};
	image.width  = width;
	image.height = height;
	image.bytes_per_pixel = bytes_per_pixel;
	image.pixels = arena_push(arena, image_pixels_size(image), Max(8, bytes_per_pixel), 1);
	return image;
}

Inline L1 image_xy_to_index(Image image, L1 x, L1 y) {
	if (LtSL1(x, 0)) x = 0;
	if (LtSL1(y, 0)) y = 0;
	if (x > image.width-1) x = image.width-1;
	if (y > image.height-1) y = image.height-1;
	L1 result = x + y*image.width;
	return result;
}

Inline F3 I1_to_F3(I1 pixel) {
	F1 r = (F1)((pixel >> 16) & 0xFF);
	F1 g = (F1)((pixel >> 8) & 0xFF);
	F1 b = (F1)(pixel & 0xFF);
	F3 result = {r/255.0f,g/255.0f,b/255.0f};
	return result;
}

Inline F3 image_sample_bilinear_F3(Image image, F1 u, F1 v) {
	Assert(image.bytes_per_pixel == sizeof(F3));

	u = F1_clamp01(u);
	v = F1_clamp01(v);

	F1 tex_x = u * (F1)(image.width-1);
	F1 tex_y = v * (F1)(image.height-1);

	L1 x0 = (L1)tex_x;
	L1 y0 = (L1)tex_y;
	L1 x1 = Min(x0+1, image.width-1);
	L1 y1 = Min(y0+1, image.height-1);

	F1 fx = tex_x - (F1)x0;
	F1 fy = tex_y - (F1)y0;

	F3 *p = (F3 *)image.pixels;
	F3 c00 = p[x0+y0*image.width];
	F3 c10 = p[x1+y0*image.width];
	F3 c01 = p[x0+y1*image.width];
	F3 c11 = p[x1+y1*image.width];

	F3 top_row = F3_lerp(c00, fx, c10);
	F3 bot_row = F3_lerp(c01, fx, c11);
	F3 result  = F3_lerp(top_row, fy, bot_row);

	return result;
}

Inline F3 image_sample_bilinear_I1_to_F3(Image image, F1 u, F1 v) {
	Assert(image.bytes_per_pixel == sizeof(I1));

	u = F1_clamp01(u);
	v = F1_clamp01(v);

	F1 tex_x = u * (F1)(image.width-1);
	F1 tex_y = v * (F1)(image.height-1);

	L1 x0 = (L1)tex_x;
	L1 y0 = (L1)tex_y;
	L1 x1 = Min(x0+1, image.width-1);
	L1 y1 = Min(y0+1, image.height-1);

	F1 fx = tex_x - (F1)x0;
	F1 fy = tex_y - (F1)y0;

	I1 *p = (I1 *)image.pixels;
	I1 c00 = p[x0+y0*image.width];
	I1 c10 = p[x1+y0*image.width];
	I1 c01 = p[x0+y1*image.width];
	I1 c11 = p[x1+y1*image.width];

	F3 c00rgb = I1_to_F3(c00);
	F3 c10rgb = I1_to_F3(c10);
	F3 c01rgb = I1_to_F3(c01);
	F3 c11rgb = I1_to_F3(c11);

	F3 top_row = F3_lerp(c00rgb,  fx, c10rgb);
	F3 bot_row = F3_lerp(c01rgb,  fx, c11rgb);
	F3 result  = F3_lerp(top_row, fy, bot_row);

	return result;
}

Internal Image image_read_from_file(Arena *arena, String8 filename) {
	String8 contents = os_read_entire_file(arena, filename);
	Assert(contents.len > 0 && "failed to read image file");

	Bitmap_Header *header = (Bitmap_Header *)contents.str;
	Assert(header->file_type == 0x4D42);
	Assert(header->bits_per_pixel == 24 || header->bits_per_pixel == 32);
	Assert(header->compression == 0);

	Image result = {0};

	result.width = header->width;
	result.height = header->height;
	result.bytes_per_pixel = 4;

	if (header->bits_per_pixel == 32) {
		result.pixels = contents.str + header->bitmap_offset;
	} else {
		L1 pixel_count = result.width * result.height;
		result.pixels = push_array(arena, B1, pixel_count*result.bytes_per_pixel);

		B1 *src = contents.str + header->bitmap_offset;
		B1 *dst = result.pixels;
		for EachIndex(pixel_index, pixel_count) {
			B1 b = src[0];
			B1 g = src[1];
			B1 r = src[2];

			((I1 *)dst)[0] = 0xFF000000 | (r << 16) | (g << 8) | b;

			src += 3;
			dst += 4;
		}
	}

	return result;
}

Internal void image_write_to_file(Image image, String8 filename) {
	Assert(image.bytes_per_pixel == 4);

	L1 pixels_size = image_pixels_size(image);

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

	// Alternatives to capping filename length:
	// - Alloc to arena.
	// - Just take a CString and let the user decide.
	Assert(filename.len < MAX_FILENAME_LEN);
	char cstr_filename[MAX_FILENAME_LEN] = {0};
	memmove(cstr_filename, filename.str, filename.len);

	// TODO: Create os_ functions for this
	L1 file = open(cstr_filename, O_CREAT | O_WRONLY, 0666);
	if (GtSI1(file, 0)) {
		write(file, &header, sizeof(Bitmap_Header));
		write(file, image.pixels, pixels_size);
		close(file);
		printf("Image written to %s\n", cstr_filename);
	} else {
		fprintf(stderr, "Could not open '%s' to write image.\n", cstr_filename);
	}
}

#endif
