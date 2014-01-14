/**@file tz_image_code.h
 * @brief binary image structural coding
 * @author Ting Zhao
 * @date 19-Feb-2008
 */

#ifndef _TZ_IMAGE_CODE_H_
#define _TZ_IMAGE_CODE_H_

#include <image_lib.h>
#include "tz_cdefs.h"

__BEGIN_DECLS

/*
 * Image_Level_Code() creates a level code map for <image>. The value in the
 * code map means how far it grows from the seeds stored in <seed>. Each element
 * of <seed> is the array index of the position of a seed and there are a total
 * of <nseed> seeds. <link> is an array to store intermediate results and it
 * could be NULL. It should be at least as long as the image array. The result 
 * will be stored in <code> and the returned pointer is the same as <code> if 
 * it is not NULL, otherwise it returns a new image object.
 */
Image* Image_Level_Code(Image *image, Image *code, int *link, 
			const int *seed, int nseed);

/* 
 * Image_Branch_Code() creates a branch code map for <image> from the seed
 * index <seed>. Two pixels have the same code value when and only when they are
 * on the same branch. <parent> is an array to store tree information. The ith
 * element is the parent code of the branch with code i. It should be long
 * enough to hold the result. Unfortunately there is no function to tell you
 * how long it could be, so you should make a guess. The seed has code 0.
 */
Image* Image_Branch_Code(Image *image, Image *code, int *link, uint16 *parent, 
			 int seed);

__END_DECLS

#endif 
