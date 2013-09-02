#include "canvas.h"
#include "color.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

#include <math.h>


Canvas *
new_canvas(int width,
           int height) {
    
	Canvas * c = (Canvas *) malloc(sizeof(Canvas));
	c->w = width;
	c->h = height;
	c->data = (Color *) calloc(width * height + 1, sizeof(Color));
	return c;
}

void
release_canvas(Canvas * c) {
	free(c->data);
	free(c);
}

void
clear_canvas(Canvas * canv) {
    memset(canv->data, 0, canv->w * canv->h * sizeof(Color));
}

// Just adapted from http://zarb.org/~gc/html/libpng.html
// TODO: refactoring

void
abort_(const char * s,
       ...) {
    
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void
write_png(char file_name[],
          Canvas * canv) {
    
    // create file
    FILE *fp = fopen(file_name, "wb");
    if (!fp)
        abort_("[write_png_file] File %s could not be opened for writing", file_name);
    
    // initialize stuff
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        abort_("[write_png_file] png_create_write_struct failed");
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[write_png_file] png_create_info_struct failed");
    
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during init_io");
    
    png_init_io(png_ptr, fp);
    
    
    // write header
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing header");
    
    png_byte bit_depth = 8;
    png_byte color_type = PNG_COLOR_TYPE_RGB;
    
    png_set_IHDR(png_ptr, info_ptr, canv->w, canv->h,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(png_ptr, info_ptr);
    
    
    png_bytep * row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * canv->h);
    int y;
    int x;
    for (y=0; y < canv->h; y++) {
        row_pointers[y] = (png_byte*) malloc(canv->w * 3 * bit_depth);
        png_byte* row = row_pointers[y];
        for(x = 0; x < canv->w; x++) {
            Color c = get_pixel(x, y, canv);
            png_byte * ptr = &(row[x * 3]);
            ptr[0] = c.r;
            ptr[1] = c.g;
            ptr[2] = c.b;
        }
    }
    
    
    // write bytes
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing bytes");
    
    png_write_image(png_ptr, row_pointers);
    
    
    // end write
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during end of write");
    
    png_write_end(png_ptr, NULL);
    
    // cleanup heap allocation
    for (y=0; y < canv->h; y++)
        free(row_pointers[y]);
    free(row_pointers);
    
    fclose(fp);
}

Canvas *
read_png(char * file_name) {
    
    unsigned char header[8];    // 8 is the maximum size that can be checked
    
    // open file and test for it being a png
    FILE *fp = fopen(file_name, "rb");
    if (!fp)
        abort_("[read_png_file] File %s could not be opened for reading", file_name);
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
        abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);
    
    
    // initialize stuff
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if (!png_ptr)
        abort_("[read_png_file] png_create_read_struct failed");
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[read_png_file] png_create_info_struct failed");
    
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during init_io");
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    
    png_read_info(png_ptr, info_ptr);
    
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    
    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    
    
    // read file
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[read_png_file] Error during read_image");
    
    png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    int y;
    for (y = 0; y < height; y++)
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    
    png_read_image(png_ptr, row_pointers);
    
    Canvas * canvas = new_canvas(width, height);
    int x;
    for (y=0; y < canvas->h; y++) {
        png_byte * row = row_pointers[y];
        for(x = 0; x < canvas->w; x++) {
            png_byte * ptr = &(row[x * 3]);
            
            set_pixel(x, y, rgb(ptr[0], ptr[1], ptr[2]), canvas);
        }
    }
    
    
    // cleanup heap allocation
    for (y=0; y < canvas->h; y++)
        free(row_pointers[y]);
    free(row_pointers);
    
    fclose(fp);
    
    return canvas;
}