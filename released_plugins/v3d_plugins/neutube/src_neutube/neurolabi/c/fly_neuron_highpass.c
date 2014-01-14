#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_math.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"
#include "tz_objdetect.h"
#include "tz_imatrix.h"
#include "tz_stack_math.h"
#include "tz_stack_bwdist.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_dimage_lib.h"
#include "tz_fimage_lib.h"
#include "tz_arrayview.h"
#include "tz_dmatrix.h"
#include "tz_stack_attribute.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];
  sprintf(file_path, "../data/%s.tif", neuron_name);

  Stack *stack = Read_Stack(file_path);

  int nvoxel = Stack_Voxel_Number(stack);

  /* 1.5GB for double */
  if (nvoxel < iround(962592768.0 / (8.0 + stack->kind))) { 
    DMatrix *filter = Mexihat_2D_D(2.0, NULL);
    filter->ndim = 3;
    filter->dim[2] = 1;
    
    DMatrix *out = Filter_Stack_Fast_D(stack, filter, NULL, 0);
    
    Kill_Stack(stack);
    Kill_DMatrix(filter);

    stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1],
			       out->dim[2], GREY);
    Kill_DMatrix(out);
  } else {
    printf("Large volume data. Float version activated.\n");

    FMatrix *filter = Mexihat_2D_F(2.0, NULL);
    filter->ndim = 3;
    filter->dim[2] = 1;
    
    FMatrix *out = Filter_Stack_Fast_F(stack, filter, NULL, 0);
    
    Kill_Stack(stack);
    Kill_FMatrix(filter);

    stack = Scale_Float_Stack(out->array, out->dim[0], out->dim[1],
			       out->dim[2], GREY);
    Kill_FMatrix(out);
  }

  sprintf(file_path, "../data/%s/highpass.tif", neuron_name);
  Write_Stack(file_path, stack);
  printf("%s created.\n", file_path);

  Kill_Stack(stack);
  /*
  DMatrix_Negative(out);
  stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1],
			     out->dim[2], GREY16);
  */


  //Stack_View sv = Stack_View_DMatrix(out);
  /*
  Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);

  sprintf(file_path, "../data/%s/highpass_locmin.tif", neuron_name);
  Write_Stack(file_path, locmax);
  printf("%s created.\n", file_path);

  Kill_Stack(locmax);
  */

  return 0;
}
