#include <stdio.h>
#include <stdlib.h>
#include <image_lib.h>
#include <gsl/gsl_fit.h>
#include "tz_darray.h"
#include "tz_stack_io.h"
#include "tz_stack_stat.h"
#include "tz_image_lib.h"
#include "tz_stack_threshold.h"
#include "tz_fimage_lib.h"
#include "tz_error.h"

#include "private/alignstack.c"

int main(int argc, const char *argv[])
{
  const char *file1 = "/Users/zhaot/Data/nathan/2008-04-18/tiletest_R1_GR1_B1_L312.tif";
  const char *file2 = "/Users/zhaot/Data/nathan/2008-04-18/tiletest_R1_GR1_B1_L334.tif";

  //const char *file1 = "/Volumes/Mac HD 2/JF8904-PRV/11/11_1.tif";
  //const char *file2 = "/Volumes/Mac HD 2/JF8904-PRV/11/11_2.tif";

  if (argc == 3) {
    file1 = argv[1];
    file2 = argv[2];
  }

  Stack *stack1 = Read_Stack((char *) file1);
  Stack *stack2 = Read_Stack((char *) file2);


#if 0
  FMatrix *filter = Mexihat_3D_F(3, NULL);
  //FMatrix_Negative(filter);

  //double sigma[] = {2.0, 2.0, 3.0};
  //FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);

  FMatrix *out = Filter_Stack_Fast_F(stack1, filter, NULL, 0);
  Kill_Stack(stack1);

  dim_type sub[3];
  printf("%g\n", FMatrix_Max(out, sub));

  stack1 = Scale_Float_Stack(out->array, out->dim[0], out->dim[1], out->dim[2],
			      GREY);
  Kill_FMatrix(out);
  //Stack_Threshold_RC(stack1, 0, 65535);


  out = Filter_Stack_Fast_F(stack2, filter, NULL, 0);
  Kill_Stack(stack2);
  stack2 = Scale_Float_Stack(out->array, out->dim[0], out->dim[1], out->dim[2],
			      GREY);
  Kill_FMatrix(out);
  //Stack_Threshold_RC(stack2, 0, 65535);
#endif

#if 1  
  int chopoff1 = estimate_chopoff(stack1);
  printf("%d\n", chopoff1);
  Stack *substack1 = Crop_Stack(stack1, 0, 0, chopoff1, 
				stack1->width, stack1->height,
				stack1->depth - chopoff1, NULL);
  int chopoff2 = estimate_chopoff(stack2);
  printf("%d\n", chopoff2);
  Stack *substack2 = Crop_Stack(stack2, 0, 0, chopoff2, 
				stack2->width, stack2->height,
				stack2->depth - chopoff2, NULL);
#endif

#if 0
  Stack_Threshold_Common(substack1, 0, 65535);
  Stack_Threshold_RC(substack1, 0, 65535);
  Stack_Threshold_Common(substack2, 0, 65535);
  Stack_Threshold_RC(substack2, 0, 65535);
#endif

#if 0
  Stack_Threshold_RC(substack1, 1000, 65535);
  Stack_Threshold_RC(substack2, 1000, 65535);
#endif

  tic();
  //Write_Stack("../data/test.tif", substack1);
  
  float unnorm_maxcorr;
  int offset[3];
  //float score = Align_Stack_F(substack1, substack2, offset, &unnorm_maxcorr);
  
  
  
  int intv[] = {3, 3, 3};

  float score = Align_Stack_MR_F(substack1, substack2, intv, 1, offset, 
				 &unnorm_maxcorr);
    
  printf("%d %d %d\n", offset[0] - stack1->width + 1, offset[1]  - stack1->height + 1, offset[2] - stack1->depth + 1);
  
  //printf("fixed (RC_common)\n");
  printf("%lld\n", toc());

  Kill_Stack(stack1);
  Kill_Stack(stack2);

  return 0;
}
