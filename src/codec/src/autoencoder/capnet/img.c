/* This file contains the code to process various image formats */

#include "config.h"

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

// Process PNG
struct png_d *readpngtobitmap(char *filename)
{
  png_structp png_ptr;
  png_infop info_ptr;
  int r;
  png_bytepp rows;
  FILE *fp;
  
  struct png_d *pd = malloc(sizeof(struct png_d));

  if ((fp = fopen(filename, "rb")) == NULL) return NULL; // open in binary-mode

  /* Allocate/initialize the memory for image information. */
  if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
    fclose(fp);
    return NULL;
  }
  if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return NULL;
  }
  /* when libpng encounters an error, it expects to longjmp() back to this routine */
  if (setjmp(png_jmpbuf(png_ptr))) {
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    if (pd->image) free(pd->image);
    free(pd);
    fclose(fp);
    return NULL;
  }

  /* Set up the input control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  png_read_png(png_ptr, info_ptr, 0, NULL);

  pd->width  = (int)png_get_image_width(png_ptr, info_ptr);
  pd->height = (int)png_get_image_height(png_ptr, info_ptr);
  pd->bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  pd->channels = png_get_channels(png_ptr, info_ptr);

//  printf("image %s: %dx%dx%dx%d\n", filename, *width, *height, bit_depth, channels);

  if ((rows = png_get_rows(png_ptr, info_ptr)) == NULL) printf("No rows.\n");
  
  pd->image = xmalloc((sizeof(unsigned char *) * (pd->width) * (pd->height) * pd->channels) + 0x10);

  for (r=0; r < pd->height; r++)
    memcpy(pd->image+(r*(pd->width)*pd->channels), rows[r], (pd->width) * pd->channels);

  /* clean up after the read, and free any memory allocated - REQUIRED */
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  fclose(fp);
  return pd;
}

unsigned char *readgiftobitmap(char *filename, int *width, int *height)
{
  struct ColorMapObject *cmap;
  struct GifColorType *ce;
  GifFileType *gif;
  unsigned char *data;
  int w, h, x, y, p;

  if ((gif = DGifOpenFileName(filename)) == NULL) {
    PrintGifError();
    return NULL;
  }
  if (DGifSlurp(gif) == GIF_ERROR) {
    PrintGifError();
    return NULL;
  }

  w = *width = gif->SWidth;
  h = *height = gif->SHeight;
  cmap = gif->Image.ColorMap? gif->Image.ColorMap : gif->SColorMap;
  data = xmalloc(sizeof(char) * w * (h+1) * 3);
  for(y=0; y<h; y++) {
    for(x=0; x<w; x++) {
      p = x*3 + (y*w*3);
      ce = &cmap->Colors[gif->SavedImages[0].RasterBits[x+(y*w)]];
      data[p] = ce->Red;
      data[p+1] = ce->Green;
      data[p+2] = ce->Blue;
    }
  }
  if (DGifCloseFile(gif) == GIF_ERROR) PrintGifError();
  return data;
}

// Allocate and blank the memory.
void *bmalloc(size_t size)
{
  void *m = xmalloc(size);

  memset(m,0,size);
  return m;
}

// Allocate memory or exit on failure.
void *xmalloc(size_t size)
{
  register void *m = malloc(size);

  if (m == NULL) {
    fprintf(stderr,"System memory exhausted.\n");
    exit(1);
  }
  return m;
}