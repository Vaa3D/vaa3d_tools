#include <stdio.h>
#include <string.h>
#include <gsl/gsl_statistics_double.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_stack_bwmorph.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_stack_draw.h"
#include "tz_voxel_graphics.h"

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
  Stack *canvas = Read_Stack(file_path);
  
  Translate_Stack(canvas, COLOR, 1);
  Print_Stack_Info(canvas);

  sprintf(file_path, "../data/%s/grow_bundle.tif", neuron_name);
  Stack *bundle = Read_Stack(file_path);
  Stack *bundle_boundary = Stack_Perimeter(bundle, NULL, 4);
  Kill_Stack(bundle);

  sprintf(file_path, "../data/%s/bundle_seed.tif", neuron_name);
  Stack *seed = Read_Stack(file_path);

  Rgb_Color color;
  Set_Color(&color, 0, 255, 0);
  Stack_Label_Bwc(canvas, bundle_boundary, color);
  Set_Color(&color, 255, 0, 0);
  Stack_Label_Bwc(canvas, seed, color);

  sprintf(file_path, "../data/%s/bundle_label.tif", neuron_name);
  Write_Stack(file_path, canvas);

  Kill_Stack(canvas);
  Kill_Stack(bundle_boundary);
  Kill_Stack(seed);

  return 0;
}
