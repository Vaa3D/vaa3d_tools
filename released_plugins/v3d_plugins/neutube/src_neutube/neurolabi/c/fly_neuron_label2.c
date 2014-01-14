#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_voxel_graphics.h"
#include "tz_neurotrace.h"
#include "tz_stack_math.h"
#include "tz_stack_utils.h"
#include "tz_objdetect.h"
#include "tz_voxeltrans.h"
#include "tz_stack_stat.h"
#include "tz_stack_draw.h"
#include "tz_stack_io.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_bwdist.h"
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

  sprintf(file_path, "../data/%s.tif", neuron_name);
  Stack *signal = Read_Stack(file_path);
  Translate_Stack(signal, GREY, 1);
  Stack *stack = Translate_Stack(signal, COLOR, 0);

  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Stack *label = Read_Stack(file_path);
  Stack *label2 = Copy_Stack(label);

  Stack_Threshold(label, 2);
  Stack_Binarize(label);

  Stack_Threshold(label2, 1);
  Stack_Binarize(label2);
  Stack_Xor(label, label2, label2);

  Print_Stack_Info(signal);

  Stack_Label_Color(stack, label, 0.0, 1.0, signal);
  Stack_Label_Color(stack, label2, 4.0, 1.0, signal);

  int i;
  for (i = 0; i < 3; i++) {
    Stack_Channel_Extraction(stack, i, signal);
    sprintf(file_path, "../data/%s/label_%d.tif", neuron_name, i);
    Write_Stack(file_path, signal);
  }

  Kill_Stack(signal);
  Kill_Stack(stack);
  Kill_Stack(label);
  Kill_Stack(label2);

  sprintf(file_path, "../data/%s/traced.tif", neuron_name);
  Stack *canvas = Read_Stack(file_path);
  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Stack *soma = Read_Stack(file_path);
  Stack_Blend_Mc_L(canvas, soma, 2, 4.0);
  Stack_Blend_Mc_L(canvas, soma, 3, 1.0);
  sprintf(file_path, "../data/%s/label_color.tif", neuron_name);
  Write_Stack(file_path, canvas);

  return 0;
}
