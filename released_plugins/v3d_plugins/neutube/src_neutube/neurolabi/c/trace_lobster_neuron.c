#include <stdio.h>
#include <stdlib.h>
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
#include "tz_geo3d_vector.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_bwdist.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[])
{
#if 1
  Stack *stack = Read_Stack("../data/lobster_neuron_seed.tif");

  Voxel_List *list = Stack_To_Voxel_List(stack);

  //Print_Voxel_List(list);

  Pixel_Array *pa = Pixel_Array_Read("../data/lobster_neuron_seeds.pa");
  //Print_Pixel_Array(pa);

  Voxel *seed;

  int i;
  double *pa_array = (double *) pa->array;

  printf("mean: %g, std: %g\n", gsl_stats_mean(pa_array, 1, pa->size), 
	 sqrt(gsl_stats_variance(pa_array, 1, pa->size)));
  
  double threshold = gsl_stats_mean(pa_array, 1, pa->size) + 
    3.0 * sqrt(gsl_stats_variance(pa_array, 1, pa->size));

  double max_r = gsl_stats_max(pa_array, 1, pa->size);
  
  printf("%g, %g\n", threshold, max_r);

  max_r = 30.0;
  Set_Neuroseg_Max_Radius(max_r);

  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  
  IMatrix *chord = Make_IMatrix(dim, 3);
  
  Stack *code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);

  Kill_Stack(stack);

  stack = Read_Stack("../data/lobster_neuron_single.tif");

  Rgb_Color color;
  Set_Color(&color, 255, 0, 0);

  Stack *signal = Read_Stack("../data/lobster_neuron.tif");

  Stack *canvas = Translate_Stack(signal, COLOR, 0);


  /************** soma detection *************/
#if 0
  Struct_Element *se = Make_Ball_Se(((int) threshold));
  Stack *stack1 = Stack_Erode_Fast(stack, NULL, se);
  Stack *soma = Stack_Dilate(stack1, NULL, se);
  Kill_Stack(stack1);  

  Stack_And(stack, soma, soma);

  Stack_Label_Bwc(canvas, soma, color);

  Kill_Stack(soma);

  Write_Stack("../data/test.tif", canvas);
  return 0;
#endif
  /*******************************************/
  

  Object_3d *obj = NULL;
  int seed_offset = -1;

  Neurochain *chain = NULL;

  double z_scale = 0.488 / 0.588;

  Set_Zscale(z_scale);
 
  Stack *traced = 
    Make_Stack(GREY, signal->width, signal->height, signal->depth);
  One_Stack(traced);

  for (i = 0; i < pa->size; i++) {
    seed = Voxel_Queue_De(&list);
    printf("------------------------------------------> seed: %d\n", i);
    if (*STACK_PIXEL_8(traced, seed->x, seed->y, seed->z, 0) == 0) {
      continue;
    }

    double width = pa_array[i];

    if (width > max_r) {
      continue;
    }

    chain = New_Neurochain();

    Print_Voxel(seed);
    printf("%g\n", width);

    
    int max_level = (int) (width + 0.5);
    if (max_level < 6) {
      max_level = 6;
    }

    seed_offset = Stack_Util_Offset(seed->x, seed->y, seed->z, 
				    stack->width, stack->height, stack->depth);
    Stack_Level_Code_Constraint(stack, code, chord->array, &seed_offset, 1, 
				max_level + 1);

    Voxel_t v;
    Voxel_To_Tvoxel(seed, v);

    Print_Tvoxel(v);

    Stack *tmp_stack = Copy_Stack(stack);
    obj = Stack_Grow_Object_Constraint(tmp_stack, 1, v, chord, code, 
				       max_level);
    Free_Stack(tmp_stack);

    Print_Object_3d_Info(obj);

    double vec[3];
    Object_3d_Orientation_Zscale(obj, vec, MAJOR_AXIS, z_scale);

    double theta, psi;
    Geo3d_Vector obj_vec;
    Set_Geo3d_Vector(&obj_vec, vec[0], vec[1], vec[2]);

    Geo3d_Vector_Orientation(&obj_vec, &theta, &psi);
    
    Set_Neuroseg(&(chain->seg), width, width, 10.0, 
		 theta, psi);

    double cpos[3];
    cpos[0] = seed->x;
    cpos[1] = seed->y;
    cpos[2] = seed->z;
    cpos[2] *= z_scale;

    double bpos[3];
    Neuroseg_Pos_Center_To_Bottom(&(chain->seg), cpos, bpos);
    Set_Position(chain->position, bpos[0], bpos[1], bpos[2]);

    if (Initialize_Tracing(signal, chain, NULL, NULL) >= MIN_SCORE) {
      if ((chain->seg.r1 < max_r) && (chain->seg.r2 < max_r)) {
	chain = Trace_Neuron(signal, chain, BOTH, traced);

	Print_Neurochain(chain);

	//Stack_Draw_Object_Bwc(canvas, obj, color);

	Neurochain_Erase(traced, Neurochain_Head(chain));
	Neurochain_Label(canvas, Neurochain_Head(chain));
      }
    }

    Free_Neurochain(chain);

    free(seed);

    Kill_Object_3d(obj);
  }

  Write_Stack("../data/lobster_neuron_traced.tif", canvas);
  
#endif

  
  return 0;
}
