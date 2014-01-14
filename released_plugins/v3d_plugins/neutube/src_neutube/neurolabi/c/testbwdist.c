/* testbwdist.c
 *
 * 07-01-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <math.h>
#include "tz_error.h"
#include "tz_stack_draw.h"
#include "tz_image_lib.h"
#include "tz_stack_io.h"
#include "tz_stack_bwdist.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_attribute.h"
#include "tz_int_histogram.h"
#include "tz_stack_utils.h"
#include "tz_stack_math.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
#if 0
  Stack *stack = Read_Stack("../data/binimg.tif");
 
  Set_Matlab_Path("/Applications/MATLAB74/bin/matlab");
  Stack *dist = Stack_Bwdist(stack);

  Stack* seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_ALTER1);

  Stack *out = Scale_Double_Stack((double *) dist->array, stack->width, 
				  stack->height, stack->depth, GREY);

  Translate_Stack(out, COLOR, 1);

  Rgb_Color color;
  Set_Color(&color, 255, 0, 0);

  Stack_Label_Bwc(out, seeds, color);

  Print_Stack_Info(dist);

  Write_Stack("../data/test.tif", out);
#endif 

#if 1
  Stack *stack = Read_Stack("../data/benchmark/sphere_bw.tif");
  //Stack *stack = Read_Stack("../data/sphere_data.tif");
  //Stack_Not(stack, stack);

  int i;
  /*
  uint8 *array = stack->array + 512 * 600;
  for (i = 1; i < 512; i++) {
    array[i] = 1;
  }
  */
  //stack->depth = 50;
  
  /*
  long int *label = (long int *) malloc(sizeof(long int) * 
					Stack_Voxel_Number(stack));
  */
  tic();
  Stack *out = Stack_Bwdist_L_U16(stack, NULL, 0);
  uint16 *out_array = (uint16 *) out->array;

  printf("%llu\n", toc());

  //int *hist = Stack_Hist(out);
  //Print_Int_Histogram(hist);

  
  Stack *out2 = Stack_Bwdist_L(stack, NULL, NULL);
  float *out2_array = (float *) out2->array;

  int n = Stack_Voxel_Number(out);

  int t = 0;
  int x, y, z;
  for (i = 0; i < n; i++) {
    uint16 d2 = (uint16) out2_array[i];
    if (out_array[i] != d2){
      int area = stack->width * stack->height;
      STACK_UTIL_COORD(i, stack->width, area, x, y, z);
      printf("(%d %d %d)", x, y, z);
      printf("%d %d %d\n", out_array[i], d2, stack->array[i]);
      t++;
    }
  }

  printf("%d error\n", t);

#  if 0
  //Translate_Stack(out, GREY, 1);
  float *out_array = (float *) out->array;
  int i;
  int n = Stack_Voxel_Number(out);
  /*
  for (i = 0; i < n; i++) {
    out_array[i] = sqrt(out_array[i]);
  }
  Stack *out2 = Scale_Float_Stack((float *)out->array, out->width, out->height,
    out->depth, GREY);
  */
  
  Stack *out2 = Make_Stack(GREY, out->width, out->height, out->depth);
  for (i = 0; i < n; i++) {
    out2->array[i] = (uint8) round(sqrt(out_array[i]));
  }
  
  Write_Stack("../data/test.tif", out2);
#  endif
  
  Write_Stack("../data/test.tif", out);
  Kill_Stack(out);
  Kill_Stack(out2);
#endif

  return 0;
}
