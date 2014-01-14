/* testbwmorph.c
 *
 * 08-May-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include "tz_error.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_lib.h"
#include "tz_stack_draw.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_utils.h"
#include "tz_stack_attribute.h"
#include "tz_stack_threshold.h"
#include "tz_stack.h"
#include "tz_testdata.h"
#include "tz_stack_io.h"

int main(int argc, char *argv[])
{
  printf("Testing ...\n");

  static char *Spec[] = {"[-t] [-d]", NULL};

  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    Stack *stack = Make_Stack(GREY, 3, 3, 3);
    One_Stack(stack);

    Stack *out = Stack_Bwshrink(stack, NULL);
    Stack_Threshold_Binarize(out, 200);

    if (Stack_Foreground_Size(out) != 1) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected foregound size.");
      return 1;
    }

    Set_Stack_Pixel(stack, 1, 1, 1, 0, 0);

    out = Stack_Bwshrink(stack, out);
    Stack_Threshold_Binarize(out, 200);

    if (Stack_Foreground_Size(out) != 6) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected foregound size.");
      return 1;
    }

    Set_Stack_Pixel(stack, 1, 1, 0, 0, 0);
    Set_Stack_Pixel(stack, 1, 1, 1, 0, 0);
    Set_Stack_Pixel(stack, 1, 1, 2, 0, 0);

    out = Stack_Bwshrink(stack, out);
    Stack_Threshold_Binarize(out, 200);

    if (Stack_Foreground_Size(out) != 4) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected foregound size.");
      return 1;
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  Stack *stack = Read_Stack("../data/mouse_neuron3_org/mask.tif");
  Stack *out  = Stack_Majority_Filter(stack, NULL, 26);

  int nvoxel = Stack_Voxel_Number(stack);

  int i;
  int s[26];
  int nfg;
  int nbg;
  int x, y, z;
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] == 1) {
      Stack_Util_Coord(i, stack->width, stack->height, &x, &y, &z);
      Stack_Neighbor_Sampling(stack, 26, x, y, z, -1, s);
      int j;
      nfg = 0;
      nbg = 0;
      for (j = 0; j < 26; j++) {
	if (s[j] == 0) {
	  nbg++;
	} else {
	  nfg++;
	}
      }
      if (out->array[i] == 0) {
	TZ_ASSERT(nfg <= nbg, "bug found");
      } else {
	TZ_ASSERT(nfg > nbg, "bug found");	
      }
    }
  }

  Write_Stack("../data/test.tif", out);
  Kill_Stack(stack);
  Kill_Stack(out);
#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_neuron3_org/mask.tif");
  Stack_Seed_Workspace *ws = New_Stack_Seed_Workspace();
  Geo3d_Scalar_Field *seed = Stack_Seed(stack, ws);

  Print_Geo3d_Scalar_Field(seed);

  Stack_Brighten_Bw(stack);
  Translate_Stack(stack, COLOR, 1);

  Stack_Draw_Voxel_Mc(stack, ws->seed_mask, 4, 255, 0, 0);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/test.tif");
  printf("%g\n", Stack_Volume_Surface_Ratio(stack, 6));
#endif

#if 0
  Stack *stack = Make_Stack(1, 10, 10, 10);

  stack->array[100 * 5 + 10 * 5 + 5] = 1;
  stack->array[100 * 4 + 10 * 5 + 5] = 1;

  Stack *signal = Copy_Stack(stack);

  Stack *out = Stack_Z_Dilate(stack, 3, signal, NULL);
  Write_Stack("../data/test.tif", out);
#endif

#if 0
  Stack *stack = Read_Stack("../data/test/soma2.tif");

  int thre = Stack_Find_Threshold_A(stack, THRESHOLD_LOCMAX_TRIANGLE);
  Filter_3d *filter = Gaussian_Filter_3d(1.0, 1.0, 0.5);
  Stack *out = Filter_Stack(stack, filter);
  stack = Copy_Stack(out);
  Stack_Threshold_Binarize(out, thre);
  Stack *out2 = Stack_Bwdist_L_U16(out, NULL, 0);
  Write_Stack("../data/test.tif", out2);
#endif

#if 0
  int start[3] = { 1, 1, 0};
  int end[3] = {11, 11, 0};
  Stack *stack = Line_Stack(start, end, 0);
  Print_Stack_Value(stack);
  Stack *out = Stack_Region_Expand(stack, 8, 3, NULL);
  Print_Stack_Value(out);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/rice_label.tif");
  Stack *out = Stack_Region_Expand(stack, 8, 3, NULL);
  Write_Stack("../data/test.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 3, 3, 1);
  Zero_Stack(stack);
  Set_Stack_Pixel(stack, 1, 1, 0, 0, 1);
  Stack *out = stack;
  //Stack *out = Stack_Region_Expand(stack, 8, 1, NULL);

  Print_Stack_Value(out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 5, 5, 3);
  Zero_Stack(stack);
  stack->array[0] = 1;
  stack->array[1] = 1;
  stack->array[2] = 1;
  stack->array[3] = 1;
  stack->array[5] = 1;
  stack->array[11] = 1;
  stack->array[12] = 1;
  stack->array[8] = 1;

  Print_Stack_Value(stack);

  Stack *out = Stack_Bwshrink(stack, NULL);

  Print_Stack_Value(out);
#endif

#if 0
  //Stack *stack = Read_Stack("../data/benchmark/binary/2d/btrig2.tif");
  //Stack *stack = Read_Stack("/Users/zhaot/Work/neutube/neurolabi/data/flyem/skeletonization/session1/215_old.tif");
  //Stack *stack = Read_Stack("../data/body.tif");
  //stack->depth = 160;
  //stack = Stack_Fill_Hole_N(stack, NULL, 1, 6, NULL);
  Stack *stack = Read_Stack("../data/flyem/skeletonization/session3/Tm6_14_70108.tif");
  Stack_Binarize(stack);
  Write_Stack("../data/test2.tif", stack);

  Stack *out = Stack_Bwshrink(stack, NULL);

  /*
  Stack_Threshold_Binarize(out, 200);
  out = Stack_Bwshrink(out, NULL);

  Stack_Threshold_Binarize(out, 200);
  out = Stack_Bwshrink(out, NULL);

  Stack_Threshold_Binarize(out, 200);
  out = Stack_Bwshrink(out, NULL);
  */

  //Stack_Threshold_Binarize(out, 200);
  //out = Stack_Bwshrink(out, NULL);
  Write_Stack("../data/test.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  /*
  One_Stack(stack);
  Set_Stack_Pixel(stack, 1, 2, 0, 0, 0);
  Set_Stack_Pixel(stack, 1, 1, 1, 0, 0);
  Set_Stack_Pixel(stack, 1, 0, 2, 0, 0);
  Set_Stack_Pixel(stack, 1, 2, 1, 0, 0);
  Set_Stack_Pixel(stack, 1, 2, 2, 0, 0);
  */
  Zero_Stack(stack);
  /*
  stack->array[7] = 1;
  stack->array[12] = 1;
  stack->array[14] = 1;
  stack->array[19] = 1;
  stack->array[1] = 1;
  //stack->array[4] = 1;
  */

  stack->array[1] = 1;
  stack->array[5] = 1;
  stack->array[7] = 1;
  stack->array[9] = 1;
  stack->array[15] = 1;
  stack->array[21] = 1;

  stack->array[12] = 1;
  //stack->array[4] = 1;

  Stack *out = Stack_Bwshrink_Z3(stack, NULL);

  Print_Stack(out);

  if (Stack_Has_Hole_Z3(stack, NULL) == TRUE) {
    printf("Hole detected\n");
  }
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 8, 8, 3);
  One_Stack(stack);
  Stack *out = Stack_Bwthin(stack, NULL);

  Print_Stack_Value(out);

#endif

#if 1
  Stack *stack = Read_Stack("../data/benchmark/binary/3d/diadem_e1.tif");
  //Stack *stack = Read_Stack("../data/flyem/skeletonization/session3/C2_214.tif");
  tic();
  Stack *out = Stack_Bwpeel(stack, NORMAL_THINNING, NULL);
  ptoc();
  Write_Stack("../data/test.tif", out);
#endif

#if 0
  printf("Simple point number: %d\n", Stack_Bwthin_Count_Simple_Point());
#endif

#if 0
  Stack *stack = Read_Stack("../data/test.tif");
  Stack_Bwpeel(stack, NORMAL_THINNING, NULL);

#endif

  printf("Done.\n");

  return 0;
}
