/**@file imsmooth.c
 * @author Ting Zhao
 * @date 16-Apr-2013
 */

#include "tz_utilities.h"
#include "tz_stack.h"
#include "tz_image_io.h"
#include "tz_stack_attribute.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string>",
    "--sigma <double> <double> <double> [--binary]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  Stack *stack = Read_Stack_U(Get_String_Arg("input"));

  if (Is_Arg_Matched("--binary")) {
    size_t voxel_number = Stack_Voxel_Number(stack);
    size_t i;
    for (i = 0; i < voxel_number; ++i) {
      if (stack->array[i] == 1) {
        stack->array[i] = 255;
      }
    }
  }
  
  double sigma[3];
  int i;
  for (i = 0; i < 3; i++) {
    sigma[i] = Get_Double_Arg("--sigma", i + 1);
  }

  Filter_3d *filter = Gaussian_Filter_3d(sigma[0],sigma[1], sigma[2]);

  Stack *out = Filter_Stack(stack, filter);

  Write_Stack(Get_String_Arg("-o"), out);

  return 0;
}
