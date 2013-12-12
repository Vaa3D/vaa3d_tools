#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_stack_math.h"
#include "tz_dimage_lib.h"
#include "tz_stack_threshold.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];
  sprintf(file_path, "../data/%s/lowpass.tif", neuron_name);

  Stack *stack = Read_Stack(file_path);

  Stack_Threshold_Tp3(stack, 1, 255);
  Stack_Binarize(stack);

  sprintf(file_path, "../data/%s/lowpass_binary.tif", neuron_name);
  Write_Stack(file_path, stack);

  return 0;
}
