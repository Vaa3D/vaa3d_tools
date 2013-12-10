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
#include "tz_locseg_chain.h"
#include "tz_trace_utils.h"
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
    if (traced->kind != GREY16) {
      Translate_Stack(traced, GREY16, 1);
    }
  } else {
    traced = Make_Stack(GREY16, signal->width, signal->height, signal->depth);
    Zero_Stack(traced);
  }
  
  Neurochain *chain = NULL;

  double z_scale = 1.0;
  sprintf(file_path, "../data/%s.res", neuron_name);
  if (fexist(file_path)) {
    double res[3];
    int length;
    darray_read2(file_path, res, &length);
    if (res[0] != res[1]) {
      perror("Different X-Y resolutions.");
      TZ_ERROR(ERROR_DATA_VALUE);
    }
    z_scale = res[0] / res[2];
  }

  /*
  sprintf(file_path, "../data/%s/soma.tif", neuron_name);
  Stack *soma = Read_Stack(file_path);
  Stack_Threshold(soma, 1);
  Stack_Binarize(soma);
  Stack_Not(soma, soma);
  Stack_And(traced, soma, traced);
  Kill_Stack(soma);
  */

  tic();

  FILE *fp = NULL;
  char chain_file_path[100];
  char vrml_file_path[100];
  int seg_idx = 229;
  int min_chain_length = 4;

#define TRY_NEXT_SEED(current_seed)			\
  Kill_Voxel(current_seed);				\
  continue;

#define STOP_TRACING(current_seed)			\
  Kill_Voxel(current_seed);				\
  break;

  Trace_Workspace *tw = New_Trace_Workspace();
  tw->length = 200;
  tw->fit_first = FALSE;
  tw->tscore_option = 1;
  tw->min_score = 0.4;
  tw->trace_direction = DL_BOTHDIR;
  tw->trace_mask = traced;
  tw->dyvar[0] = max_r;
  tw->test_func = Locseg_Chain_Trace_Test;

  int chain_id = 100;

  char chain_info_file_path[100];
  sprintf(chain_info_file_path, "../data/%s/chain_info.txt", neuron_name);
  FILE *chain_info_file = fopen(chain_info_file_path, "w");

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
      Neurochain_Label(canvas, chain, z_scale);
      Free_Neurochain(chain);
      TRY_NEXT_SEED(seed);
    }

    /* for debugging */
    if (i < seg_idx) {
      //TRY_NEXT_SEED(seed);
    }
    /****************/
    
    if (*STACK_PIXEL_16(traced, seed->x, seed->y, seed->z, 0) > 0) {
      TRY_NEXT_SEED(seed);
    }

    double width = pa_array[i];

    if (width > max_r) {
      TRY_NEXT_SEED(seed);
    }

    Print_Voxel(seed);
    printf("%g\n", width);
    
    int max_level = (int) (width + 0.5);
    if (max_level < 1.1) {
      TRY_NEXT_SEED(seed);
      //max_level = 6;
    }

    /* for debugging */
    if (i > seg_idx) {
      //STOP_TRACING(seed);
    }
    /****************/

    
    double cpos[3];
    cpos[0] = seed->x;
    cpos[1] = seed->y;
    cpos[2] = seed->z;
    cpos[2] *= z_scale;
    
    Local_Neuroseg *startseg = New_Local_Neuroseg();;

    Set_Neuroseg_Position(startseg, cpos, NEUROSEG_CENTER);
    Reset_Neuroseg(&(startseg->seg));
    startseg->seg.r1 = width;
    startseg->seg.r2 = width;
    
    Stack_Fit_Score fs;
    fs.n = 1;
    fs.options[0] = tw->tscore_option;

    printf("Search orientation ...\n");
    Local_Neuroseg_Orientation_Search_C(startseg, signal, z_scale, &fs); 

    Print_Local_Neuroseg(startseg);

    printf("Start: \n");

    Locseg_Chain* locseg_chain = 
      Locseg_Chain_Trace_Init(signal, z_scale, startseg, &fs);
    
    
    if (fs.scores[0] >= tw->min_score) {
      Locseg_Chain_Iterator_Start(locseg_chain, DL_HEAD);
      Local_Neuroseg *head = Locseg_Chain_Next(locseg_chain);
      if ((head->seg.r1 < max_r) && (head->seg.r2 < max_r)) {
	Trace_Locseg(signal, z_scale, locseg_chain, tw);
	chain = Neurochain_From_Locseg_Chain(locseg_chain);
	//chain = Trace_Neuron2(signal, chain, BOTH, traced, z_scale, 100);

	printf("Remove ends ...\n");
	chain = Neurochain_Remove_Overlap_Ends(chain);
	chain = Neurochain_Remove_Turn_Ends(chain, 1.0);

	//Print_Neurochain(Neurochain_Head(chain));
	//Stack_Draw_Object_Bwc(canvas, obj, color);
	Neurochain *chain_head = Neurochain_Head(chain);

	printf("Saving ...\n");
	fp = fopen(chain_file_path, "w");
	Neurochain_Fwrite(chain_head, fp);
	fclose(fp);
	if (Neurochain_Length(chain_head, FORWARD) >= min_chain_length) {
	  printf("Labeling ...\n");
	  Write_Neurochain_Vrml(vrml_file_path, chain_head);
	  Neurochain_Label_G(trac, chain_head, z_scale, 0,
			     Neurochain_Length(chain_head, FORWARD),
			     1.5, 0.0, chain_id);
	  fprintf(chain_info_file, "%d %d %d\n", i, chain_id, 
		  Neurochain_Length(chain_head, FORWARD));
	  chain_id++;
	  Neurochain_Label(canvas, chain_head, z_scale);
	  Write_Stack((char *) trace_mask_path, traced);
	}
	//Neurochain_Erase(traced, Neurochain_Head(chain), z_scale);
	Free_Neurochain(chain);
      }
    } else {
      printf("Ignored seed:\n");
     
    }

    Kill_Locseg_Chain(locseg_chain);
    Kill_Voxel(seed);
  }

  fclose(chain_info_file);

  Write_Stack((char *) trace_file_path, canvas);

  Kill_Pixel_Array(pa);
  Voxel_List_Removeall(&list);

  printf("Time passed: %lld\n", toc());
  
#endif

  
  return 0;
}
