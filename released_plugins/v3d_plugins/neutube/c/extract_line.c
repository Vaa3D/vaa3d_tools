/**@file extract_line.c
 * @brief >> line extraction
 * @author Ting Zhao
 * @date 06-Nov-2008
 */
#include <stdio.h>
#include <utilities.h>
#include "tz_stack_lib.h"
#include "tz_fimage_lib.h"
#include "tz_stack_threshold.h"
#include "tz_int_histogram.h"
#include "tz_iarray.h"
#include "tz_stack_bwmorph.h"
#include "tz_image_io.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> -o <string> [-s <double>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  printf("Extracting line structure ...\n");

  double sigma[] = {1.0, 1.0, 1.0};

  if (Is_Arg_Matched("-s")) {
    double scale = Is_Arg_Matched("-s");
    sigma[0] *= scale;
    sigma[1] *= scale;
    sigma[2] *= scale;
  }

  Stack *stack = Read_Stack_U(Get_String_Arg("image"));

  FMatrix *result = NULL;

  tic();
  if (stack->width * stack->height * stack->depth > 1024 * 1024 * 100) {
    result = El_Stack_L_F(stack, sigma, NULL);
  } else {
    result = El_Stack_F(stack, sigma, NULL);
  }
  printf("%llu\n", toc());

  Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
				 result->dim[2], GREY16);
  
  Kill_FMatrix(result);
  

  
  /*
  int *hist = Stack_Hist(out);
  int min = 0;
  int max = 255;

  int *map = Int_Histogram_Equalize(hist, min, max);

  int *hist2 = iarray_calloc(max - min + 3);

  hist2[0] = max - min + 1;
  hist2[1] = min;
  int i;

  for (i = 0; i < hist[0]; i++) {
    hist2[map[i + 2] - min + 2] += hist[i + 2];
  }

  int thre = Int_Histogram_Triangle_Threshold2(hist2, Int_Histogram_Min(hist2) + 1,
					       Int_Histogram_Max(hist2), 1.0);

  printf("%d\n", thre);

  for (i = 0; i < Int_Histogram_Length(hist); i++) {
    if (thre <= map[i + 2]) {
      thre = hist[1] + i;
      break;
    }
  }
  */

  
  Write_Stack(Get_String_Arg("-o"), out);

  /*
  if (Is_Arg_Matched("-m")) {
    Write_Stack(Get_String_Arg("-m"), out);
  }
  
  int thre = Stack_Find_Threshold_Locmax(out, 1, 65535);
  printf("%d\n", thre);

  Stack_Threshold(out, thre);
  Stack_Binarize(out);
  Translate_Stack(out, GREY, 1);
  Stack *result2 = Stack_Remove_Small_Object(out, NULL, 4, 26);

  Write_Stack(Get_String_Arg("-o"), result2);
  */
  return 0;
}
