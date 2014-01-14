/**@file tz_image_attribute.h
 * @brief image attributes
 * @author Ting Zhao
 * @date 18-Feb-2008
 */

/* Rountines for getting attributes of an $image. 
 * See ../lib/myers.pack/image_lib.h for $image definition. 
 */

#ifndef _TZ_IMAGE_ATTRIBUTE_H_
#define _TZ_IMAGE_ATTRIBUTE_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

/*
 * Image_Wdith(), Image_Height() and Image_Kind() returns the widht, height and
 * kind of an image.
 */
static inline int Image_Width(const Image *image) { 
  if (image == NULL) {
    return 0;
  }
  return image->width;
}

static inline int Image_Height(const Image *image) {
  if (image == NULL) {
    return 0;
  }
  return image->height;
}

static inline int Image_Kind(const Image *image) {
  if (image == NULL) {
    return -1;
  }
  return image->kind;
}

/*
 * Image_Channel_Number() returns the number of the channels of an image.
 */
int Image_Channel_Number(const Image *image);

/*
 * Image_Voxel_Number() returns the number of voxels of an image. 
 * Image_Voxel_Bsize() returns the number of bytes of each voxel. 
 * Image_Array_Bsize() returns the total number of bytes of all voxels.
 */
int Image_Pixel_Number(const Image *image);
int Image_Pixel_Bsize(const Image *image);
int Image_Array_Bsize(const Image *image);

__END_DECLS

#endif
