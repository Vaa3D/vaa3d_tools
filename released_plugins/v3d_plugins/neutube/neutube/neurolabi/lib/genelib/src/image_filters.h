/*****************************************************************************************\
*                                                                                         *
*  Image Filters                                                                          *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*                                                                                         *
\*****************************************************************************************/

#ifndef _IMAGE_FILTERS

#define _IMAGE_FILTERS

#include "image_lib.h"

/* FILTERS & CONVOLUTION */

  /*  The routines below build and return a "filter" which is small FLOAT32 image
      of weights that can then be convolved with an image to produce a smoothed or
      filtered result by Filter_Image below.  If the filter is w x h in dimensions
      then the center of the filter is (w/2,h/2).  The Gaussian and Mexican Hat
      filters produce filters that are 2*floor(3*sigma)+1 pixels in each dimension,
      the other filters are 2*radius+1 in each dimension.
  */

Image *Gaussian_2D_Filter(double sigma);
Image *Mexican_Hat_2D_Filter(double sigma);
Image *Average_Round_2D_Filter(int radius);
Image *Average_Square_2D_Filter(int radius);

  /*  Convolve filter with image and return the result as a FLOAT32 image.  Image
      is first translated (see Translate_Image in image_lib.[ch]) to a FLOAT32 image
      before being convolved and if requested this will be done in_place.
  */

Image *Convolve_Image(Image *filter, Image *image, int in_place);

Image *Median_2D_Filter(int radius, Image *image, int in_place);   //  TO BE WRITTEN!
Image *Variance_2D_Filter(int radius, Image *image, int in_place); //  TO BE WRITTEN!

  /*  The 3D/Stack equivalents of the routines above  */

Stack *Gaussian_3D_Filter(double sigma);          //  NEEDS TESTING!
Stack *Mexican_Hat_3D_Filter(double sigma);       //  NEEDS TESTING!
Stack *Average_Round_3D_Filter(int radius);       //  TO BE WRITTEN!
Stack *Average_Square_3D_Filter(int radius);      //  NEEDS TESTING!

Stack *Convolve_Stack(Stack *filter, Stack *stack, int in_place);    //  TO BE WRITTEN!
Stack *Median_3D_Filter(int radius, Stack *stack, int in_place);     //  TO BE WRITTEN!
Stack *Variance_3D_Filter(int radius, Stack *stack, int in_place);   //  TO BE WRITTEN!

#endif
