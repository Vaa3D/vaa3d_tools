#include <stdio.h>
#include <stdlib.h>
#include "tz_error.h"
#include "tz_iarray.h"
#include "tz_image_array.h"
#include "tz_stack_attribute.h"
#include "tz_stack_lib.h"
#include "tz_int_histogram.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) 
{
  uint16 a[10] = {1, 2, 4, 6, 3, 3, 9, 6, 2, 3};
  uint8 mask[10] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  printf("%d\n", (int) Image_Array_Max_Index(a, GREY16, 10));
  printf("%d\n", (int) Image_Array_Max_Index_M(a, GREY16, 10, mask));

  size_t index[2];
  Image_Array_Minmax_Index(a, GREY16, 10, index);
  printf("%d %d\n", (int) index[0], (int) index[1]);
  Image_Array_Minmax_Index_M(a, GREY16, 10, mask, index);
  printf("%d %d\n", (int) index[0], (int) index[1]);

  int *hist = Image_Array_Hist_M(a, GREY16, 10, NULL, NULL);
  Print_Int_Histogram(hist);
  printf("%d\n", Int_Histogram_Mode(hist, 5, 10));
  free(hist);

  int *diff = Int_Histogram_Diff(hist, NULL);
  iarray_print2(diff, hist[0] - 1, 1);
  free(diff);

#if 1
  Stack *stack = Read_Stack("../data/test/mono4.tif");
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);

  hist = Stack_Hist_M(stack, NULL);
  //Print_Int_Histogram(hist);
  iarray_write("../data/hist.bn", hist, hist[0] + 2);
  printf("%d\n", Int_Histogram_Mode(hist, -1, -1));

  printf("%d\n", Hist_Tpthre2(hist, -1, -1));
  printf("%d\n", Hist_Rcthre(hist, 0, 65535));

  free(hist);
#endif

  return 0;
}
