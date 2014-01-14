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
#include "tz_stack_code.h"
#include "tz_stack_math.h"
#include "tz_stack_utils.h"
#include "tz_objdetect.h"
#include "tz_voxeltrans.h"
#include "tz_stack_stat.h"
#include "tz_stack_draw.h"
#include "tz_geo3d_vector.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_bwdist.h"
#include "tz_neurotrace.h"
#include "tz_stack_threshold.h"
#include "tz_darray.h"

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

#if 1
  sprintf(file_path, "../data/%s/bundle_seed.tif", neuron_name);
  Stack *stack = Read_Stack(file_path);
  Voxel_List *list = Stack_To_Voxel_List(stack);

  sprintf(file_path, "../data/%s/bundle_seeds.pa", neuron_name);
  Pixel_Array *pa = Pixel_Array_Read(file_path);

  Voxel *seed;

  int i;
  double *pa_array = (double *) pa->array;

  printf("mean: %g, std: %g\n", gsl_stats_mean(pa_array, 1, pa->size), 
	 sqrt(gsl_stats_variance(pa_array, 1, pa->size)));
  
  double threshold = gsl_stats_mean(pa_array, 1, pa->size) + 
    3.0 * sqrt(gsl_stats_variance(pa_array, 1, pa->size));

  double max_r = gsl_stats_max(pa_array, 1, pa->size);
  
  printf("%g, %g\n", threshold, max_r);

  max_r = max_r * 2;
  Set_Neuroseg_Max_Radius(max_r / 2.0);

  dim_type dim[3];
  dim[0] = stack->width;
  dim[1] = stack->height;
  dim[2] = stack->depth;
  
  IMatrix *chord = Make_IMatrix(dim, 3);
  
  Stack *code = Make_Stack(GREY16, stack->width, stack->height, stack->depth);

  Kill_Stack(stack);

  sprintf(file_path, "../data/%s/bundle.tif", neuron_name);
  stack = Read_Stack(file_path);

  Rgb_Color color;
  Set_Color(&color, 255, 0, 0);

  sprintf(file_path, "../data/%s.tif", neuron_name);
  Stack *signal = Read_Stack(file_path);

  Stack *canvas = NULL;

  char trace_mask_path[100]; 
  sprintf(trace_mask_path, "../data/%s/trace_mask.tif", neuron_name);
  char trace_file_path[100];
  sprintf(trace_file_path, "../data/%s/traced.tif", neuron_name);
  
  if (fexist(trace_file_path) == 1) {
    canvas = Read_Stack((char *) trace_file_path);
  } else {
    canvas = Translate_Stack(signal, COLOR, 0);
  }

  Stack *traced = NULL;
  

  if (fexist(trace_mask_path) == 1) {
    traced = Read_Stack((char *) trace_mask_path);
  } else {
    traced = Make_Stack(GREY, signal->width, signal->height, signal->depth);
    One_Stack(traced);
  }
  

  Object_3d *obj = NULL;
  int seed_offset = -1;

  Neurochain *chain = NULL;

  double z_scale = 1.0;
  sprintf(file_path, "../data/%s.res", neuron_name);
  if (fexist(file_path)) {
    double res[3];
    int length;
    darray_read(file_path, res, &length);
    if (res[0] != res[1]) {
      perror("Different X-Y resolutions.");
      TZ_ERROR(ERROR_DATA_VALUE);
    }
    z_scale = res[0] / res[2];
  }

 
  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Stack *soma = Read_Stack(file_path);
  Stack_Threshold(soma, 1);
  Stack_Binarize(soma);
  Stack_Not(soma, soma);
  Stack_And(traced, soma, traced);
  Kill_Stack(soma);

  tic();

  FILE *fp = NULL;
  char chain_file_path[100];
  char vrml_file_path[100];
  int seg_idx = 109;
  int min_chain_length = 5;
  for (i = 0; i < pa->size; i++) {
    /* do not skip seed dequeueing */
    seed = Voxel_Queue_De(&list);
    printf("---------------------------------> seed: %d / %d\n", i, pa->size);
    
    sprintf(chain_file_path, "../data/%s/chain%d.bn", neuron_name, i);
    sprintf(vrml_file_path, "../data/%s/chain%d.wrl", neuron_name, i);

    if (fexist(chain_file_path) == 1) {
      chain = Read_Neurochain(chain_file_path);
      if (Neurochain_Length(chain, FORWARD) >= min_chain_length) {
	Write_Neurochain_Vrml(vrml_file_path, chain);
      }
      Free_Neurochain(chain);
      continue;
    }

    /* for debugging */
    if (i < seg_idx) {
      continue;
    }
    /****************/
    
    if (*STACK_PIXEL_8(traced, seed->x, seed->y, seed->z, 0) == 0) {
      continue;
    }

    double width = pa_array[i];

    if (width > max_r) {
      continue;
    }

    Print_Voxel(seed);
    printf("%g\n", width);
    
    int max_level = (int) (width + 0.5);
    if (max_level < 3) {		       
      continue;
      //max_level = 6;
    }

    /* for debugging */
    if (i > seg_idx) {
      break;
    }
    /****************/

    chain = New_Neurochain();

    seed_offset = Stack_Util_Offset(seed->x, seed->y, seed->z, 
				    stack->width, stack->height, stack->depth);
    Stack_Level_Code_Constraint(stack, code, chord->array, &seed_offset, 1, 
				max_level + 1);

    Voxel_t v;
    Voxel_To_Tvoxel(seed, v);

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
    
    Set_Neuroseg(&(chain->locseg.seg), width, width, 12.0, 
		 theta, psi, 0.0);

    double cpos[3];
    cpos[0] = seed->x;
    cpos[1] = seed->y;
    cpos[2] = seed->z;
    cpos[2] *= z_scale;
    
    Set_Neuroseg_Position(&(chain->locseg), cpos, NEUROSEG_CENTER);

    if (Initialize_Tracing(signal, chain, NULL, z_scale) >= MIN_SCORE) {
      if ((Neuroseg_Hit_Traced(&(chain->locseg), traced, z_scale) == FALSE) &&
	  (chain->locseg.seg.r1 < max_r / 2.0) && 
	  (chain->locseg.seg.r2 < max_r / 2.0)) {
	chain = Trace_Neuron2(signal, chain, BOTH, traced, z_scale, 100);

	//Print_Neurochain(Neurochain_Head(chain));
	//Stack_Draw_Object_Bwc(canvas, obj, color);
	Neurochain *chain_head = Neurochain_Head(chain);

	fp = fopen(chain_file_path, "w");
	Neurochain_Fwrite(chain_head, fp);
	fclose(fp);
	if (Neurochain_Length(chain_head, FORWARD) >= min_chain_length) {
	  Write_Neurochain_Vrml(vrml_file_path, chain_head);
	}
	//Neurochain_Erase(traced, Neurochain_Head(chain), z_scale);
	Neurochain_Erase_E(traced, chain_head, z_scale, 0,
			   Neurochain_Length(chain_head, FORWARD),
			   1.5, 0.0);
	Neurochain_Label(canvas, chain_head, z_scale);
	Write_Stack((char *) trace_mask_path, traced);
	Write_Stack((char *) trace_file_path, canvas);
      }
    }

    Free_Neurochain(chain);

    free(seed);

    Kill_Object_3d(obj);
  }

  printf("Time passed: %lld\n", toc());
  
#endif

  
  return 0;
}
