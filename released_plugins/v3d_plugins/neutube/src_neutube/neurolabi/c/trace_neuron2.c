#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/neurolabi_config.h"
#ifdef HAVE_LIBGSL
#  include <gsl/gsl_vector.h>
#  include <gsl/gsl_statistics_double.h>
#endif
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
#include "tz_trace_utils.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_stack_attribute.h"

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
    "[!wtr] [-canvas <string>] [-mask <string>] [-res <string>]",
    "-minlen <double> [-minr <double>] [-mins <double>] [-sf <string>]",
    " <image:string> -S<string> -D<string>",
    NULL};
  
  Process_Arguments(argc, argv, Spec, 1);
  
  int i;
  char *dir = Get_String_Arg("-D");
  char file_path[100];

  /* load stack to trace */
  Stack *signal = Read_Stack(Get_String_Arg("image"));

  /* set z_scale */
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
      z_scale = res[0] / res[2] * 2.0;
    }
  }

  printf("z_scale: %g\n", z_scale);
  /*****************************/

  /* load seeds */
  sprintf(file_path, "%s/%s", dir, Get_String_Arg("-S"));
  printf("%s\n", file_path);

  int nseed = 0;
  Local_Neuroseg *locseg = Read_Local_Neuroseg_Array(file_path, &nseed);
  /*****************************/
  
  /* Calculate seed sores and sort them */
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_CORRCOEF;

  double *scores = NULL;
  if (Is_Arg_Matched("-sf")) {
    int length;
    sprintf(file_path, "%s/%s", dir, Get_String_Arg("-sf"));
    scores = darray_read(file_path, &length);
    printf("%d %d\n", length, nseed);
    TZ_ASSERT(length == nseed, "Wrong seed file.");
  } else {
    scores = darray_malloc(nseed);
    for (i = 0; i < nseed; i++) {
      scores[i] = Local_Neuroseg_Score(locseg + i, signal, z_scale, &fs);
    }
  }
  int *indices = iarray_malloc(nseed);
  darray_qsort(scores, indices, nseed);
  /*****************************/

  /* find max radius */
  sprintf(file_path, "%s/%s.bn", dir, "max_r");
  double max_r;
  int tmp;
  if (fexist(file_path)) {
    darray_read2(file_path, &max_r, &tmp);
  } else {
    max_r = Local_Neuroseg_Array_Maxr(locseg, nseed);
  }
  printf("%g\n", max_r);
  /*****************************/

  /* estimate maxium radius a segment can be */
  max_r *= 1.5;

  sprintf(file_path, "%s/%s", dir, "soma0.bn");
  if (!fexist(file_path)) {
    max_r *= 2.0;
  }
   
  max_r = dmax2(max_r, 100.0);
  //Set_Neuroseg_Max_Radius(max_r);
  /******************************************/

  /* set minimal radius */
  double min_r = 0.0;

  if (Is_Arg_Matched("-minr")) {
    min_r = Get_Double_Arg("-minr");
  }
  /*****************************/

  /* create canvas */
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
  /*****************************/

  /* load or create the tracing mask */  
  Stack *traced = NULL;
  char trace_mask_path[100];
  sprintf(trace_mask_path, "%s/%s", dir, Get_String_Arg("-mask"));

  if (fexist(trace_mask_path) == 1) {
    traced = Read_Stack((char *) trace_mask_path);
    Stack_Not(traced, traced);
    if (traced->kind == GREY) {
      Translate_Stack(traced, GREY16, 1);
    }
  } else {
    traced = Make_Stack(GREY16, signal->width, signal->height, signal->depth);
    Zero_Stack(traced);
  }
  /*****************************/

  tic();

  /* set minimal chain length */
  double min_chain_length = 25.0;

  if (Is_Arg_Matched("-minlen")) {
    min_chain_length = Get_Double_Arg("-minlen");
  }
  /*****************************/

  int counter = 0;

  /* initialize tracing workspace */
  /*
  Trace_Workspace *tw = New_Trace_Workspace();
  tw->length = 200;
  tw->fit_first = FALSE;
  tw->tscore_option = fs.options[0];
  tw->min_score = 0.3;
  tw->trace_direction = DL_BOTHDIR;
  tw->trace_status[0] = TRACE_NORMAL;
  tw->trace_status[1] = TRACE_NORMAL;
  tw->trace_mask = traced;
  tw->dyvar[0] = max_r;
  tw->dyvar[1] = min_r;
  tw->test_func = Locseg_Chain_Trace_Test;
  tw->trace_range[0] = 0;
  tw->trace_range[1] = 0;
  tw->trace_range[2] = 0;
  tw->trace_range[3] = signal->width - 1;
  tw->trace_range[4] = signal->height - 1;
  tw->trace_range[5] = signal->depth - 1;
  */

  Trace_Workspace *tw = Locseg_Chain_Default_Trace_Workspace(NULL, signal);
  //tw->dyvar[2] = 0.5;
  
  tw->tscore_option = fs.options[0];
  tw->trace_mask = traced;
  tw->dyvar[0] = max_r;
  tw->dyvar[1] = min_r;
  if (Is_Arg_Matched("-mins")) {
    tw->min_score = Get_Double_Arg("-mins");
  }
  /*****************************/

  int chain_id = 100;

  char chain_info_file_path[100];
  sprintf(chain_info_file_path, "%s/chain_info.txt", dir);
  FILE *chain_info_file = fopen(chain_info_file_path, "w");

  /* trace all seeds */
  for (i = nseed - 1; i >= 0; i--) {
    int index = indices[i];

    printf("-------------------> seed: %d (%d) / %d\n", i, index, nseed);

    /* ignore low score seeds */
    if (scores[i] < tw->min_score) {
      printf("low score: %g\n", scores[i]);
      continue;
    }
    /******************************/
 
    char chain_file_path[100];
    char vrml_file_path[100];
    sprintf(chain_file_path, "%s/chain%d.bn", dir, index);
    sprintf(vrml_file_path, "%s/chain%d.wrl", dir, index);

    /* the seed has been traced before */
    if (fexist(chain_file_path) == 1) {
      printf("chain exists\n");
      continue;
    }
    /***************************/
    
    /* test if the location has been traced */
    int x = (int) (locseg[index].pos[0] + 0.5);
    int y = (int) (locseg[index].pos[1] + 0.5);
    int z = (int) (locseg[index].pos[2] * z_scale + 0.5);

    if (Stack_Contain_Point(signal, x, y, z) == TRUE) {
      if (*STACK_PIXEL_16(traced, x, y, z, 0) > 0) {
	printf("traced \n");
	continue;
      }
    }
    /***********************************/

    /* ignore too thick segments */
    //double width = dmax2(locseg[index].seg.r1, locseg[index].seg.r2);
    double width = NEUROSEG_RB(&(locseg[index].seg));

    if (width > max_r) {
      printf("too thick");
      continue;
    }
    /***********************************/

    /* The seed passes quals, let's start tracing */
    printf("Start: \n");

    /* Initialize a chain */
    Trace_Record *tr = New_Trace_Record();
    tr->mask = ZERO_BIT_MASK;
    Locseg_Node *p = Make_Locseg_Node(Copy_Local_Neuroseg(locseg + index), tr);
    Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

    /* trace! */
    tw->trace_status[0] = TRACE_NORMAL;
    tw->trace_status[1] = TRACE_NORMAL;
    Trace_Locseg(signal, z_scale, locseg_chain, tw);
    /***********************/

    /* post processing */
    Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
    Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);
    /***********************************/

    /* save the the chain */
    Write_Locseg_Chain(chain_file_path, locseg_chain);
    
    /* record chains that are long enough */
    if (Locseg_Chain_Geolen(locseg_chain) >= min_chain_length) {
      Locseg_Chain_To_Vrml_File(locseg_chain, vrml_file_path);

      Locseg_Chain_Label_G(locseg_chain, traced, z_scale, 0,
			   Locseg_Chain_Length(locseg_chain),
			   1.5, 0.0, chain_id);
      fprintf(chain_info_file, "%d %d %d\n", i, chain_id, 
	      Locseg_Chain_Length(locseg_chain));
      chain_id++;
      Locseg_Chain_Label(locseg_chain, canvas, z_scale);

      counter += Locseg_Chain_Length(locseg_chain);
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

    /* Clean the traced chain */
    Kill_Locseg_Chain(locseg_chain);
  }

  Write_Stack((char *) trace_mask_path, traced);
  Write_Stack((char *) trace_file_path, canvas);

  printf("Time passed: %lld\n", toc());

  
  return 0;
}
