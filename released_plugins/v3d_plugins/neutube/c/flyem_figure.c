/**@file flyem_figure.c
 * @author Ting Zhao
 * @date 18-Apr-2013
 */

#include "tz_utilities.h"
#include "tz_image_io.h"
#include "tz_image_trans.h"
#include "tz_math.h"
#include "tz_stack_attribute.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  Mc_Stack *stack = Read_Mc_Stack(Get_String_Arg("input"), 0);

  Stack stack_view = Stack_View_Mc_Stack(stack, 0);

  double factor = 255.0 / 200.0;

  size_t nvoxel = Stack_Voxel_Number(&stack_view);
  size_t i;
  for (i = 0; i < nvoxel; ++i) {
    int v = iround(factor * stack_view.array[i]);
    if (v > 255) {
      v = 255;
    }
    stack_view.array[i] = v;
  }
  
  Write_Stack_U(Get_String_Arg("-o"), &stack_view, NULL);

  return 0;
}
