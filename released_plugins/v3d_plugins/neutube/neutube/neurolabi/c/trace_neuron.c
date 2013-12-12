#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics_double.h>
#include <math.h>
#include <utilities.h>
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
#include "tz_iarray.h"
#include "tz_darray.h"

/*
 * trace_neuron - trace neuron from given seeds
 *
 * trace_neuron [!wtr] seed_file -Dsave_dir
 *   -r: write intermediate results
 *
 */
int main(int argc, char* argv[])
{
  static char *Spec[] = {
    "[!wtr] [-canvas <string>] [-mask <string>] [-res <string>] [-minr <int>]",
    "-minlen <double>",
    " <image:string> -S<string> -D<string>",
    NULL};
  
  Process_Arguments(argc, argv, Spec, 1);
  
  char *dir = Get_String_Arg("-D");
  
  char file_path[100];
  sprintf(file_path, "%s/%s", dir, Get_String_Arg("-S"));
  printf("%s\n", file_path);

  Geo3d_Scalar_Field *seed = Read_Geo3d_Scalar_Field(file_path);

  int idx;

  sprintf(file_path, "%s/%s.bn", dir, "max_r");
  double max_r;
  int tmp;
  if (fexist(file_path)) {
    darray_read2(file_path, &max_r, &tmp);
  } else {
    max_r = darray_max(seed->values, seed->size, &idx);
  }

  printf("%g\n", max_r);

  max_r *= 1.5;

  /*
  sprintf(file_path, "%s/%s", dir, "soma0.bn");
  if (!fexist(file_path)) {
    max_r *= 2.0;
  }
  */
   
  Set_Neuroseg_Max_Radius(max_r);

  Stack *signal = Read_Stack(Get_String_Arg("image"));

  dim_type dim[3];
  dim[0] = signal->width;
  dim[1] = signal->height;
  dim[2] = signal->depth;
  /* 
  IMatrix *chord = Make_IMatrix(dim, 3);
  
  Stack *code = Make_Stack(GREY16, 
			   signal->width, signal->height, signal->depth);
  */
  Rgb_Color color;
  Set_Color(&color, 255, 0, 0);

  Stack *canvas = NULL;

  char trace_file_path[100];
  sprintf(trace_file_path, "%s/%s", dir, Get_String_Arg("-canvas"));
  
  if (fexist(trace_file_path) == 1) {
    canvas = Read_Stack((char *) trace_file_path);
  } else {
    canvas = Copy_Stack(signal);
    Stretch_Stack_Value_Q(canvas, 0.999);
    Translate_Stack(canvas, COLOR, 1);
  }

  Stack *traced = NULL;
  
  char trace_mask_path[100];
  sprintf(trace_mask_path, "%s/%s", dir, Get_String_Arg("-mask"));

  if (fexist(trace_mask_path) == 1) {
    traced = Read_Stack((char *) trace_mask_path);
  } else {
    traced = Make_Stack(GREY, signal->width, signal->height, signal->depth);
    One_Stack(traced);
  }
  

  //Object_3d *obj = NULL;
  int seed_offset = -1;

  Neurochain *chain = NULL;

  double z_scale = 1.0;

  if (Is_Arg_Matched("-res")) {
    sprintf(file_path, "%s", Get_String_Arg("-res"));

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
  }

  //sprintf(file_path, "%s/%s", dir, Get_String_Arg("-M"));
  //Stack *stack = Read_Stack(file_path);

  tic();

  FILE *fp = NULL;
  char chain_file_path[100];
  char vrml_file_path[100];

  double min_chain_length = 25.0;

  if (Is_Arg_Matched("-minlen")) {
    min_chain_length = Get_Double_Arg("-minlen");
  }

  int *indices = iarray_malloc(seed->size);
  double *values = darray_malloc(seed->size);
  int i;

  Local_Neuroseg *locseg = (Local_Neuroseg *) 
    malloc(seed->size * sizeof(Local_Neuroseg));

  int index = 0;
  for (i = 0; i < seed->size; i++) {
    printf("-----------------------------> seed: %d / %d\n", i, seed->size);
    indices[i] = i;
    index = i;
    int x = (int) seed->points[index][0];
    int y = (int) seed->points[index][1];
    int z = (int) seed->points[index][2];

    double width = seed->values[index];

    chain = New_Neurochain();

    seed_offset = Stack_Util_Offset(x, y, z, signal->width, signal->height,
				    signal->depth);

    if (width < 3.0) {
      width += 0.5;
    }
    Set_Neuroseg(&(locseg[i].seg), width, width, 12.0, 
		 0.0, 0.0, 0.0);

    double cpos[3];
    cpos[0] = x;
    cpos[1] = y;
    cpos[2] = z;
    cpos[2] *= z_scale;
    
    Set_Neuroseg_Position(&(locseg[i]), cpos, NEUROSEG_CENTER);
    Stack_Fit_Score fs;
    fs.n = 1;
    fs.options[0] = 1;
    values[i] = Local_Neuroseg_Orientation_Search_C(&(locseg[i]), signal, z_scale, &fs);
  }

  darray_qsort(values, indices, seed->size);

  /*
  for (i = 0; i < seed->size; i++) {
    indices[i] = i;
  }
  darraycpy(values, seed->values, 0, seed->size);
  darray_qsort(values, indices, seed->size);
  */

  int counter = 0;

  //  for (i = seed->size - 1; i >= seed->size - 231; i--) {
  for (i = seed->size - 1; i >= 0; i--) {
    index = indices[i];

    printf("-----------------------------> seed: %d / %d\n", i, seed->size);
    
    sprintf(chain_file_path, "%s/chain%d.bn", dir, index);
    sprintf(vrml_file_path, "%s/chain%d.wrl", dir, index);

    if (fexist(chain_file_path) == 1) {
      chain = Read_Neurochain(chain_file_path);
      if (Neurochain_Geolen(chain) >= min_chain_length) {
	Write_Neurochain_Vrml(vrml_file_path, chain);
	Neurochain_Label(canvas, chain, z_scale);
	Neurochain_Erase_E(traced, chain, z_scale, 0,
			   Neurochain_Length(chain, FORWARD),
			   1.5, 0.0);
      }

      Free_Neurochain(chain);
      printf("chain exists\n");
      continue;
    }
    
    
    int x = (int) seed->points[index][0];
    int y = (int) seed->points[index][1];
    int z = (int) seed->points[index][2];

    if (*STACK_PIXEL_8(traced, x, y, z, 0) == 0) {
      printf("traced \n");
      continue;
    }

    double width = seed->values[index];

    if (width > max_r) {
      printf("too thick\n");
      continue;
    }
    
    if (Is_Arg_Matched("-minr")) {
      int max_level = (int) (width + 0.5);
      if (max_level <= Get_Int_Arg("-minr")) {
	printf("too thin\n");
	continue;
      }
    }
    /*
    seed_offset = Stack_Util_Offset(x, y, z, signal->width, signal->height,
				    signal->depth);
    */

    chain = New_Neurochain();
    /*
    Stack_Level_Code_Constraint(stack, code, chord->array, &seed_offset, 1, 
				max_level + 1);

    Voxel_t v;
    v[0] = x;
    v[1] = y;
    v[2] = z;

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
    */

    /*
    if (width < 3.0) {
      width += 0.5;
    }
    Set_Neuroseg(&(chain->locseg.seg), width, width, 12.0, 
		 0.0, 0.0, 0.0);

    double cpos[3];
    cpos[0] = x;
    cpos[1] = y;
    cpos[2] = z;
    cpos[2] *= z_scale;
    
    //Set_Neuroseg_Position(&(chain->locseg), cpos, NEUROSEG_BOTTOM);
    Set_Neuroseg_Position(&(chain->locseg), cpos, NEUROSEG_CENTER);
    Stack_Fit_Score fs;
    fs.n = 1;
    fs.options[0] = 1;
    Local_Neuroseg_Orientation_Search_C(&(chain->locseg), signal, z_scale,
					&fs); 
    //fs.options[0] = 1;
    */

    Copy_Local_Neuroseg(&(chain->locseg), &(locseg[index]));
    Neurochain *chain_head = chain;
    
    
    if (Initialize_Tracing(signal, chain, NULL, z_scale) >= MIN_SCORE) {
      if ((Neuroseg_Hit_Traced(&(chain->locseg), traced, z_scale) == FALSE) &&
	  (chain->locseg.seg.r1 < max_r) && 
	  (chain->locseg.seg.r2 < max_r)) {
	//Initialize_Tracing(signal, chain, NULL, z_scale);
	chain = Trace_Neuron2(signal, chain, BOTH, traced, z_scale, 500);

	//Neurochain *chain_head = Neurochain_Head(chain);
	chain_head = Neurochain_Remove_Overlap_Segs(chain);
	chain_head = Neurochain_Remove_Turn_Ends(chain_head, 0.5);
	/*
	if (i == seed->size - 231) {
	  Print_Neurochain(chain_head);
	}
	*/

	fp = fopen(chain_file_path, "w");
	Neurochain_Fwrite(chain_head, fp);
	fclose(fp);
	if (Neurochain_Geolen(chain_head) >= min_chain_length) {
	  Write_Neurochain_Vrml(vrml_file_path, chain_head);

	  Neurochain_Erase_E(traced, chain_head, z_scale, 0,
			     Neurochain_Length(chain_head, FORWARD),
			     1.5, 0.0);
	  Neurochain_Label(canvas, chain_head, z_scale);

	  counter += Neurochain_Length(chain_head, FORWARD);
	  if (counter > 500) {
	    if (Is_Arg_Matched("-r")) {
	      Write_Stack((char *) trace_mask_path, traced);
	    }
	    
	    if (Is_Arg_Matched("-r")) {
	    Write_Stack((char *) trace_file_path, canvas);
	    }

	    counter = 0;
	  }
	}
      }
    }

    Free_Neurochain(chain_head);

    //Kill_Object_3d(obj);
  }

  Write_Stack((char *) trace_file_path, canvas);
  if (Is_Arg_Matched("-r")) {
    Write_Stack((char *) trace_mask_path, traced);
  }

  Kill_Geo3d_Scalar_Field(seed);

  printf("Time passed: %lld\n", toc());

  
  return 0;
}
