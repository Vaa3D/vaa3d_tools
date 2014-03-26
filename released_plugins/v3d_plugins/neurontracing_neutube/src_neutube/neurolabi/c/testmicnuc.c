#include <stdio.h>
#include <stdlib.h>
#include <image_filters.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_dimage_lib.h"
#include "tz_stack_attribute.h"
#include "tz_stack_lib.h"
#include "tz_image_lib.h"
#include "tz_int_histogram.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) 
{

#if 0
  Image *image = Read_Image("../data/threshold.tif");

#if 0
  Image *filter = Mexican_Hat_2D_Filter(3);
  Filter_Image(filter, image, 1);
  Translate_Image(image, GREY, 1);
#endif

#if 0
  DMatrix *filter = Mexihat_2D_D(1, NULL);
  DMatrix_Negative(filter);
  DMatrix_Print(filter);
  DMatrix *out = Filter_Image_D(image, filter, NULL);
  /*
  DMatrix *in = Get_Double_Matrix3(stack);
  darray_sub(out->array, in->array, matrix_size(out->dim, out->ndim));
  */

  Kill_Image(image);
  image = Scale_Double_Image(out->array, out->dim[0], out->dim[1], GREY);
#endif

  Write_Image("../data/test.tif", image);
#endif

#if 0
  Stack *stack = Read_Stack("../data/threshold.tif");
  DMatrix *filter = Mexihat_3D_D(3, NULL);
  DMatrix_Negative(filter);
  //DMatrix_Print(filter);
  DMatrix *out = Filter_Stack_Fast_D(stack, filter, NULL, 0);
  stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1], out->dim[2],
			     GREY);
  Write_Stack("../data/test.tif", stack);
#endif
  
#if 0
  Image *image = Read_Image("../data/test/mono4.tif");
  int *hist = Image_Hist(image);
  //int threshold = Int_Histogram_Triangle_Threshold(hist, 0, 65535);
  int threshold = Hist_Tpthre2(hist, 0, hist[0] + hist[1] - 1);
  printf("%d\n", threshold);
#endif

#if 0
  Image *image = Read_Image("../data/threshold.tif");
  DMatrix *filter = Mexihat_2D_D(1.5, NULL);
  DMatrix_Negative(filter);
  DMatrix *out = Filter_Image_D(image, filter, NULL);
  Kill_Image(image);
  image = Scale_Double_Image(out->array, out->dim[0], out->dim[1], GREY);
  Write_Image("../data/test/threshold_edge.tif", image);
#endif

#if 0
  double d = 0.0;
  int i = 0;
  for (d = 0.5; d < 2147400400.0; d += 1.0, i++) {
    int t1 = (int) floor(d);
    int t2 = (int) (floor(d) + 0.5);
    //printf("%d ",  t1);
    if (t1 != i) {
      printf("unmatch found: %d, %d\n", t1, t2);
    }
  }
  printf("\n");
#endif

#if 1
  double sigma = 1.0;
  DMatrix *filter = Mexihat_3D_D(sigma, NULL);
  Stack *stack = Read_Stack("../data/Gaussian_stack.tif");
  DMatrix *out = Filter_Stack_Fast_D(stack, filter, NULL, 0);
  Stack *result = 
    Scale_Double_Stack(out->array, out->dim[0], out->dim[1], out->dim[2], GREY);

  Write_Stack("../data/test.tif", result);
#endif

  return 0;
}
