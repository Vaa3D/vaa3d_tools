#include <iostream>

#include "zargumentprocessor.h"
#include "tz_stack_lib.h"
#include "tz_arrayview.h"
#include "tz_image_io.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_math.h"

int main(int argc, char *argv[])
{
  static char const *Spec[] = {"<input:string> -o <string>"};

  ZArgumentProcessor::processArguments(argc, argv, Spec);

  Stack *stack = Read_Stack_U(ZArgumentProcessor::getStringArg("input"));
  Stack_Not(stack, stack);
  Stack* solid = Stack_Majority_Filter(stack, NULL, 8);
  Stack_Not(solid, solid);
  Free_Stack(stack);
  stack = solid;

#if 0
  Stack *perim = Stack_Perimeter(stack, NULL, 4);
  Stack_Not(perim, perim);
  Stack *dist = Stack_Bwdist_L_U16P(perim, NULL, 0);

  Free_Stack(perim);
    
  Stack *out = Copy_Stack(stack);

  Image bwslice1;
  Image bwslice2;
  Image slice1;
  Image slice2;
  Image outSlice;

  int area = Stack_Width(stack) * Stack_Height(stack);

  for (int k = 1; k < Stack_Depth(stack) - 1; k++) {
    slice1 = Image_View_Stack_Slice(dist, k - 1);
    bwslice1 = Image_View_Stack_Slice(stack, k - 1);
    slice2 = Image_View_Stack_Slice(dist, k + 1);
    bwslice2 = Image_View_Stack_Slice(stack, k + 1);
    outSlice = Image_View_Stack_Slice(out, k);

    uint16_t *distArray1 = (uint16_t*) slice1.array;
    uint16_t *distArray2 = (uint16_t*) slice2.array;

    for (int offset = 0; offset < area; offset++) {
      /* Zero if both outside */
      if ((bwslice1.array[offset] ==0) && (bwslice2.array[offset] == 0)) {
        /* Do nothing */
      } else if ((bwslice1.array[offset] == 1) && 
          (bwslice2.array[offset] == 1)) {
        outSlice.array[offset] = 1;
      } else if ((bwslice1.array[offset] > bwslice2.array[offset]) == 
          (distArray1[offset] > distArray2[offset])) {
        outSlice.array[offset] = 1;
      }
    }
  }
#endif

  Stack *out = Stack_Bwinterp(stack, NULL);

  
  Write_Stack_U(ZArgumentProcessor::getStringArg("-o"), out, NULL);

  return 0;
}
