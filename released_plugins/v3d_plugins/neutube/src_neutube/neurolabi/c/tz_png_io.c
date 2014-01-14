#include "tz_png_io.h"
#include "tz_error.h"

void Png_Attribute(const char *file_path, int *kind, int *width, int *height)
{
#if defined(HAVE_LIBPNG)
  png_structp png_ptr;
  png_infop info_ptr;
  unsigned int sig_read = 0;
  png_uint_32 png_width, png_height;
  int bit_depth, color_type, interlace_type;
  FILE *fp;

  if ((fp = fopen(file_path, "rb")) == NULL) {
    return;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

  if (png_ptr == NULL)
  {
    fclose(fp);
    return;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
  {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, sig_read);

  int png_transforms = PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_PACKING | 
    PNG_TRANSFORM_SWAP_ENDIAN;
  png_read_png(png_ptr, info_ptr, png_transforms, NULL);
     
  png_get_IHDR(png_ptr, info_ptr, &png_width, &png_height, &bit_depth, 
      &color_type, &interlace_type, NULL, NULL);
  *width = png_width;
  *height = png_height;

  *kind = GREY;
  switch (color_type) {
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      if (bit_depth != 8) {
        *kind = 0;
      } else {
        *kind = COLOR;
      }
      break;
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
    case PNG_COLOR_TYPE_PALETTE:
      *kind = (bit_depth + 7) / 8;
      break;
    default:
      *kind = 0;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif
}

Stack* Read_Png(const char *file_path)
{
  Stack *stack = NULL;

#if defined(HAVE_LIBPNG)
  png_structp png_ptr;
  png_infop info_ptr;
  unsigned int sig_read = 0;
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  FILE *fp;

  if ((fp = fopen(file_path, "rb")) == NULL) {
    return NULL;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

  if (png_ptr == NULL)
  {
    fclose(fp);
    return NULL;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
  {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return NULL;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, sig_read);

  int png_transforms = PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_PACKING | 
    PNG_TRANSFORM_SWAP_ENDIAN;
  png_read_png(png_ptr, info_ptr, png_transforms, NULL);
     
  /*
  png_read_info(png_ptr, info_ptr);
  switch(bit_depth) {

  }
*/
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
      &interlace_type, NULL, NULL);

  int kind = GREY;
  switch (color_type) {
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      if (bit_depth != 8) {
        kind = 0;
      } else {
        kind = COLOR;
      }
      break;
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
    case PNG_COLOR_TYPE_PALETTE:
      kind = (bit_depth + 7) / 8;
      break;
    default:
      kind = 0;
  }

  if (kind != 0) {
    stack = Make_Stack(kind, width, height, 1);
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
      //(png_bytep*) malloc(sizeof(png_bytep) * height);
    int i;
    for (i = 0; i < height; i++) {
      memcpy(stack->array + i * kind * width, row_pointers[i], 
          kind * width);
      //row_pointers[i] = stack->array + i * kind * width;
    }
    //png_set_rows(png_ptr, info_ptr, row_pointers);

    //free(row_pointers);
  }

#  ifdef _DEBUG_2
  printf("%d, %d, %d, %d\n", width, height, bit_depth, color_type);
  Write_Stack("../data/test.tif", stack);
#  endif
  fclose(fp);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#endif

  return stack;
}

BOOL Is_Png(const char *file_path)
{
#if defined(HAVE_LIBPNG)
#define PNG_BYTES_TO_CHECK 4
  FILE *fp = fopen(file_path, "rb");
  if (fp == NULL) {
    return FALSE;
  }

  char buffer[PNG_BYTES_TO_CHECK];
  if (fread(buffer, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
    fclose(fp);
    return FALSE;
  }

  if (png_sig_cmp((png_const_bytep) buffer, 0, PNG_BYTES_TO_CHECK) == 0) {
    fclose(fp);
    return TRUE;
  }

  fclose(fp);
#endif
  return FALSE;
}

Mc_Stack* Read_Png_M(const char *file_path)
{
  Mc_Stack *stack = NULL;

#if defined(HAVE_LIBPNG)
  png_structp png_ptr;
  png_infop info_ptr;
  unsigned int sig_read = 0;
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  FILE *fp;

  if ((fp = fopen(file_path, "rb")) == NULL) {
    return NULL;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

  if (png_ptr == NULL)
  {
    fclose(fp);
    return NULL;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
  {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return NULL;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, sig_read);

  int png_transforms = PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_PACKING | 
    PNG_TRANSFORM_SWAP_ENDIAN;
  png_read_png(png_ptr, info_ptr, png_transforms, NULL);
     
  /*
  png_read_info(png_ptr, info_ptr);
  switch(bit_depth) {

  }
*/
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
      &interlace_type, NULL, NULL);

  int kind = GREY;
  switch (color_type) {
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      if (bit_depth != 8) {
        kind = 0;
      } else {
        kind = COLOR;
      }
      break;
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
    case PNG_COLOR_TYPE_PALETTE:
      kind = (bit_depth + 7) / 8;
      break;
    default:
      kind = 0;
  }

  if (kind != 0) {
    stack = Make_Mc_Stack(kind, width, height, 1, 1);
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
      //(png_bytep*) malloc(sizeof(png_bytep) * height);
    int i;
    for (i = 0; i < height; i++) {
      memcpy(stack->array + i * kind * width, row_pointers[i], 
          kind * width);
      //row_pointers[i] = stack->array + i * kind * width;
    }
    //png_set_rows(png_ptr, info_ptr, row_pointers);

    //free(row_pointers);
  }

#  ifdef _DEBUG_2
  printf("%d, %d, %d, %d\n", width, height, bit_depth, color_type);
  Write_Stack("../data/test.tif", stack);
#  endif
  fclose(fp);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#endif

  return stack;
}
