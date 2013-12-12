/* sort_seed.c
 *
 * 28-Oct-2008 Initial write: Ting Zhao
 */

/* sort_seed - sort_seed image_file -s seed_file -o out_file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
//#include "tz_stack_bwdist.h"
#include "tz_stack_threshold.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_array.h"
#include "tz_workspace.h"
#include "tz_neurofield.h"
#include "tz_image_io.h"
#include "tz_locseg_chain.h"

int main(int argc, char* argv[])
{
  if (Show_Version(argc, argv, "1.00") == 1) {
    return 0;
  }

  static char *Spec[] = {
    " <image:string> -s <string> -o <string> [-e <string>] [-fo <int>] "
    "[-z <double> | -res <string>] [-field <int>] [-min_score <double>]",
    NULL};
  
  Process_Arguments(argc, argv, Spec, 1);
  
  Geo3d_Scalar_Field *seed = Read_Geo3d_Scalar_Field(Get_String_Arg("-s"));

  size_t idx;
  double max_r = darray_max(seed->values, seed->size, &idx);

  max_r *= 1.5;

  //Set_Neuroseg_Max_Radius(max_r);

  Stack *signal = Read_Stack_U(Get_String_Arg("image"));

  dim_type dim[3];
  dim[0] = signal->width;
  dim[1] = signal->height;
  dim[2] = signal->depth;

  Rgb_Color color;
  Set_Color(&color, 255, 0, 0);

  int seed_offset = -1;

  double z_scale = 1.0;

  if (Is_Arg_Matched("-res")) {
    if (fexist(Get_String_Arg("-res"))) {
      double res[3];
      int length;
      darray_read2(Get_String_Arg("-res"), res, &length);
      if (res[0] != res[1]) {
	perror("Different X-Y resolutions.");
	TZ_ERROR(ERROR_DATA_VALUE);
      }
      z_scale = res[0] / res[2] * 2.0;
    }
  }
  
  if (Is_Arg_Matched("-z")) {
    z_scale = Get_Double_Arg("-z");
  }

  printf("z scale: %g\n", z_scale);

  tic();


  double *values = darray_malloc(seed->size);

  int i;
  Local_Neuroseg *locseg = (Local_Neuroseg *) 
    malloc(seed->size * sizeof(Local_Neuroseg));


  int index = 0;

  //int ncol = LOCAL_NEUROSEG_NPARAM + 1 + 23;
  //double *features = darray_malloc(seed->size * ncol);
  //double *tmpfeats = features;

  Stack *seed_mask = Make_Stack(GREY, signal->width, signal->height, 
				signal->depth);
  Zero_Stack(seed_mask);

  Locseg_Fit_Workspace *fws = New_Locseg_Fit_Workspace();
  
  if (Is_Arg_Matched("-field")) {
    fws->sws->field_func = Neuroseg_Slice_Field_Func(Get_Int_Arg("-field"));
  }

  fws->sws->fs.n = 2;
  fws->sws->fs.options[0] = STACK_FIT_DOT;
  fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;

  if (Is_Arg_Matched("-fo")) {
    fws->sws->fs.options[1] = Get_Int_Arg("-fo");
  }

  for (i = 0; i < seed->size; i++) {
    printf("-----------------------------> seed: %d / %d\n", i, seed->size);

    index = i;
    int x = (int) seed->points[index][0];
    int y = (int) seed->points[index][1];
    int z = (int) seed->points[index][2];

    double width = seed->values[index];

    seed_offset = Stack_Util_Offset(x, y, z, signal->width, signal->height,
				    signal->depth);

    if (width < 3.0) {
      width += 0.5;
    }
    Set_Neuroseg(&(locseg[i].seg), width, 0.0, NEUROSEG_DEFAULT_H, 
		 0.0, 0.0, 0.0, 0.0, 1.0);

    double cpos[3];
    cpos[0] = x;
    cpos[1] = y;
    cpos[2] = z;
    cpos[2] /= z_scale;
    
    Set_Neuroseg_Position(&(locseg[i]), cpos, NEUROSEG_CENTER);

    if (seed_mask->array[seed_offset] > 0) {
      printf("labeled\n");
      values[i] = 0.0;
      continue;
    }

    //Local_Neuroseg_Optimize(locseg + i, signal, z_scale, 0);
    Local_Neuroseg_Optimize_W(locseg + i, signal, z_scale, 0, fws);

    values[i] = fws->sws->fs.scores[1];
    /*
    Stack_Fit_Score fs;
    fs.n = 1;
    fs.options[0] = 1;
    values[i] = Local_Neuroseg_Score(locseg + i, signal, z_scale, &fs);
    */

    //values[i] = Local_Neuroseg_Score_W(locseg + i, signal, z_scale, sws);

    printf("%g\n", values[i]);

    double min_score = LOCAL_NEUROSEG_MIN_CORRCOEF;
    if (Is_Arg_Matched("-min_score")) {
      min_score = Get_Double_Arg("-min_score");
    }

    if (values[i] > min_score) {
      Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 2, z_scale);
    } else {
      Local_Neuroseg_Label_G(locseg + i, seed_mask, -1, 1, z_scale);
    }

    /*
    tmpfeats += Local_Neuroseg_Param_Array(locseg + i, z_scale, tmpfeats);
    
    tmpfeats += Local_Neuroseg_Stack_Feature(locseg + i, signal, z_scale, 
					     tmpfeats); 
    */
  }

  if (Is_Arg_Matched("-e")) {
    Write_Stack(Get_String_Arg("-e"), seed_mask);
  }
  Write_Local_Neuroseg_Array(Get_String_Arg("-o"), locseg, seed->size);

  char file_path[MAX_PATH_LENGTH];
  sprintf(file_path, "%s_score", Get_String_Arg("-o"));
  darray_write(file_path, values, seed->size);

  //sprintf(file_path, "%s_feat", Get_String_Arg("-o"));
  //darray_write(file_path, features, seed->size * ncol); 

  Kill_Geo3d_Scalar_Field(seed);

  printf("Time passed: %lld\n", toc());

  
  return 0;
}

