/* tz_image_attribute.h
 *
 * 11-Dec-2003 Initial write: Ting Zhao
 */

#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_image_attribute.h"

INIT_EXCEPTION

int Image_Channel_Number(const Image *image)
{
  if (image->kind == COLOR) {
    return 3;
  } else {
    return 1;
  }
}

int Image_Pixel_Number(const Image *image)
{
  return image->width * image->height;
}

int Image_Pixel_Bsize(const Image *image)
{
  return image->kind;
}

int Image_Array_Bsize(const Image *image)
{
  return Image_Pixel_Number(image) * Image_Pixel_Bsize(image);
}
