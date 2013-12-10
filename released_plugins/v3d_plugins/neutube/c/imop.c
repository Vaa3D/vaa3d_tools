/**@file imop.c
 * @brief image operation
 * @author Ting Zhao
 * @date 02-Feb-2010
 */

#include "tz_utilities.h"
#include "tz_mc_stack.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string> -o <string>", 
			 "[-channel <int>] [-ds <int> <int> <int>]",
			 "[-us <int> <int> <int>]",
			 "[-crop <int> <int> <int> <int> <int> <int>]",
			 "[-dark_cover] [-binarize <int> <int>]",
			 NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int channel = -1;
  if (Is_Arg_Matched("-channel")) {
    channel = Get_Int_Arg("-channel");
  }

  Mc_Stack *stack = Read_Mc_Stack(Get_String_Arg("image"), channel);
  Mc_Stack *out = stack;

  /* downsample */
  if (Is_Arg_Matched("-ds")) {
    out = Mc_Stack_Downsample_Mean(stack, Get_Int_Arg("-ds", 1), 
				   Get_Int_Arg("-ds", 2), Get_Int_Arg("-ds", 3),
				   NULL);
  }

  if (Is_Arg_Matched("-us")) {
    out = Mc_Stack_Upsample(stack, Get_Int_Arg("-us", 1),
			    Get_Int_Arg("-us", 2), Get_Int_Arg("-us", 3),
			    NULL);
  }
  
  if (Is_Arg_Matched("-dark_cover")) {
    Stack tmpstack;
    tmpstack.width = out->width;
    tmpstack.height = out->height;
    tmpstack.depth = 1;
    tmpstack.kind = out->kind;
    tmpstack.array = out->array;
    Zero_Stack(&tmpstack);

    tmpstack.array = tmpstack.array + out->width * out->height * out->kind;
    Zero_Stack(&tmpstack);
    
    tmpstack.array = out->array + 
      out->width * out->height * (out->depth - 1) * out->kind;
    Zero_Stack(&tmpstack);
  }

  if (Is_Arg_Matched("-crop")) {
    int left = Get_Int_Arg("-crop", 1);
    int top = Get_Int_Arg("-crop", 2);
    int front = Get_Int_Arg("-crop", 3);
    int width = Get_Int_Arg("-crop", 4) - left + 1;
    int height = Get_Int_Arg("-crop", 5) - top + 1;
    int depth = Get_Int_Arg("-crop", 6) - front + 1;
    Stack *tmpstack = Mc_Stack_To_Stack(stack, stack->kind, NULL);
    Stack *out2 = Crop_Stack(tmpstack, left, top, front, width, height, depth, NULL);
    Kill_Stack(tmpstack);
    out = Mc_Stack_Rewrap_Stack(out2);
  }
   
  if (Is_Arg_Matched("-binarize")) {
    int low = Get_Int_Arg("-binarize", 1);
    int high = Get_Int_Arg("-binarize", 2);
    Mc_Stack_Binarize(out, low, high);
  }

  Write_Mc_Stack(Get_String_Arg("-o"), out, NULL);

  Kill_Mc_Stack(stack);

  return 0;
}
