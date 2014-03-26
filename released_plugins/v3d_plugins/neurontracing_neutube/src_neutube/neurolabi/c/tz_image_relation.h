/**@file tz_image_relation.h
 * @brief image comparison
 * @author Ting Zhao
 * @date 12-Dec-2007
 */

/* relationship between two images */

#ifndef _TZ_IMAGE_RELATION_H_
#define _TZ_IMAGE_RELATION_H_

#include <image_lib.h>
#include "tz_cdefs.h"

__BEGIN_DECLS

/*
 * Image_Same_Size() returns TRUE if the sizes of <image1> and <image2> are the
 * same, otherwise it returns FASLE. Image_Same_Kind() returns TRUE if the kinds
 * of <image1> and <image2> are the same, otherwise it returns FASLE.
 * Image_Same_Attribute() returns TRUE if all the attributes of <image1> and
 * <image2> are the same, otherwise it returns FASLE.
 */
BOOL Image_Same_Size(const Image *image1, const Image *image2);
BOOL Image_Same_Kind(const Image *image1, const Image *image2);
BOOL Image_Same_Attribute(const Image *image1, const Image *image2);

/*
 * Image_Identical() returns TRUE if <image1> and <image2> have exactly the same
 * attributes and voxel values, otherwise it returns FALSE.
 */
BOOL Image_Identical(const Image *image1, const Image *image2);

__END_DECLS

#endif

