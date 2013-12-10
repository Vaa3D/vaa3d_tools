#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/neurolabi_config.h"
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#ifdef HAVE_LIBGSL
#  include <gsl/gsl_vector.h>
#  include <gsl/gsl_statistics_double.h>
#endif
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
#include "tz_neurotrace.h"
#include "tz_stack_threshold.h"
#include "tz_trace_utils.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_stack_attribute.h"
#include "tz_fmatrix.h"
#include "tz_fimage_lib.h"
#include "tz_arraytrans.h"
#include "tz_workspace.h"
#include "tz_neurofield.h"
#include "tz_image_io.h"
#include "tz_math.h"

static void print_command(int argc, char* const argv[])
{
  int i;
  for (i = 0; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
}

/*
 * trace_neuron - trace neuron from given seeds
 *
 * trace_neuron [!wtr] seed_file -Dsave_dir
 *   -r: write intermediate results
 *
 */
int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.01") == 1) {
    return 0;
  }

  static char *Spec[] = {
    "[!wtr] [-canvas <string>] [-mask <string>] [-res <string>] [-bs <double>]",
    "[-base_mask <string>] [-recover <int>] [-field <int>]",
    "[-minlen <double>] [-minr <double>] [-mins <double>] [-sf <string>]",
    "[-push] [-min_score <double>]",
    " <image:string> -S<string> -D<string>",
    NULL};
  
  print_command(argc, argv);

  Process_Arguments(argc, argv, Spec, 1);
  
  int i;
  char *dir = Get_String_Arg("-D");
  char file_path[500];

  /* load stack to trace */
  Stack *signal = Read_Stack_U(Get_String_Arg("image"));

  Trace_Workspace *tw = New_Trace_Workspace();
  Locseg_Chain_Default_Trace_Workspace(tw, signal);
  tw->refit = FALSE;
  tw->tune_end = TRUE;
  if (Is_Arg_Matched("-min_score")) {
    tw->min_score = Get_Double_Arg("-min_score");
  }

  tw->fit_workspace = New_Locseg_Fit_Workspace();  

  Locseg_Fit_Workspace *fw = tw->fit_workspace;
  if (Is_Arg_Matched("-field")) {
    fw->sws->field_func = Neuroseg_Slice_Field_Func(Get_Int_Arg("-field"));
  }

  //fw->sws->field_func = Neurofield_S1;

  //tw->break_refit = TRUE;
  //tw->min_chain_length = 0.0;
  sprintf(tw->save_path, "%s/%s", Get_String_Arg("-D"), "chain");

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
  
  fw->sws->fs.n = 2;
  fw->sws->fs.options[0] = STACK_FIT_DOT;
  fw->sws->fs.options[1] = STACK_FIT_CORRCOEF;

  /* Calculate seed scores and sort them */
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
      Local_Neuroseg_Score_W(locseg + i, signal, z_scale, 
			     fw->sws);
      scores[i] = fw->sws->fs.scores[1];
    }
  }
  /*****************************/

  /* find max radius */
  sprintf(file_path, "%s/%s.bn", dir, "max_r");
  int tmp;
  if (fexist(file_path)) {
    darray_read2(file_path, tw->dyvar, &tmp);
  }
  /*****************************/

  /* set minimal radius */
  if (Is_Arg_Matched("-minr")) {
    tw->dyvar[1] = Get_Double_Arg("-minr");
  }
  /*****************************/

  /* set minimal chain length */
  if (Is_Arg_Matched("-minlen")) {
    tw->min_chain_length = Get_Double_Arg("-minlen");
  }
  /*****************************/

  /* blur the signal */
  if (Is_Arg_Matched("-bs")) {
    double sigma[3];// = {1.5, 1.5, 0.5};
    sigma[0] = Get_Double_Arg("-bs");
    sigma[1] = sigma[0];
    sigma[2] = sigma[0] / 3.0;
    FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
    FMatrix *result = Filter_Stack_Fast_F(signal, filter, NULL, 0);
    
    Array_Link *a = FMatrix_To_Stack(result); 
  
    tw->sup_stack = Get_Stack_At(a);
  }

  /* trace all seeds */
  int nchain;
  Locseg_Chain **chain = 
    Trace_Locseg_S(signal, 1.0, locseg, scores, nseed, tw, &nchain);

  /* Fix the ends the chains */

  /* free tracing results */
  for (i = 0; i < nchain; i++) {
    Kill_Locseg_Chain(chain[i]);
  }

  /*
  for (i = 0; i < nseed; i++) {
    if (chain[i] != NULL) {
      if (tw->break_refit == TRUE) {
	int k = 0;
	while ((chain[i] + k)->list != NULL) {
	  Clean_Locseg_Chain(chain[i] + k);
	  k++;
	}
	Delete_Locseg_Chain(chain[i]);
      } else {
	Kill_Locseg_Chain(chain[i]);
      }
    }    
  }
  */
  free(chain);
  

  Stack *leftover = NULL; /* stack to store left-over mask */
  Stack *trace_mask = NULL; /* stack to store tracing mask */
  Stack *base_mask = NULL; /* stack to store base mask */

  int recovery = 0;

  if (Is_Arg_Matched("-recover")) { /* re-explore */
    recovery = Get_Int_Arg("-recover");
  }

  if (recovery >= 1) {
    printf("Read original mask.\n");
    leftover = Read_Stack(fullpath(dir, Get_String_Arg("-mask"), NULL));

    printf("make trace mask.\n");
    trace_mask = Make_Stack(GREY, tw->trace_mask->width, 
			    tw->trace_mask->height, tw->trace_mask->depth);
  
    printf("read base mask\n");
    base_mask = 
      Read_Stack(fullpath(dir, Get_String_Arg("-base_mask"), NULL));

    TZ_ASSERT(tw->trace_mask->kind == GREY16, "Unexpected stack kind.");
    uint16 *trace_mask_array = (uint16*) tw->trace_mask->array;

    /* combine trace_mask and base mask */
    int nvoxel = Stack_Voxel_Number(trace_mask);
    for (i = 0; i < nvoxel; i++) {
      if ((trace_mask_array[i] > 0) || (base_mask->array[i] == 1)) {
	trace_mask->array[i] = 1;
      } else {
	trace_mask->array[i] = 0;
      }
    }

    Kill_Stack(base_mask);
    base_mask = NULL;

    /*
    Kill_Stack(signal);
    signal = NULL;
    */

    printf("dilate\n");
    //Struct_Element *se = Make_Zline_Se(5);
    //Stack *submask = Stack_Dilate(trace_mask, NULL, se);
    Stack *submask = Stack_Z_Dilate(trace_mask, 5, signal, NULL);

    /* calculate left-over mask */
    printf("subtract\n");
    Stack_Bsub(leftover, submask, trace_mask);

    Kill_Stack(submask);
    submask = NULL;

    printf("remove small objects\n");
    Stack_Remove_Small_Object(trace_mask, leftover, 27, 26);
    /*
    printf("reload signal\n");
    signal = Read_Stack(Get_String_Arg("image"));
    */

    if (Stack_Is_Dark(leftover) == FALSE) {
      Stack_Seed_Workspace *ssw = New_Stack_Seed_Workspace();
      ssw->method = 2;
      //ssw->weight = signal;
      Geo3d_Scalar_Field *seed2 = Stack_Seed(leftover, ssw);
      Trace_Evaluate_Seed_Workspace *ws = New_Trace_Evaluate_Seed_Workspace();
      if (Is_Arg_Matched("-min_score")) {
	ws->min_score = Get_Double_Arg("-min_score");
      }
      ws->trace_mask = tw->trace_mask;
      ws->base_mask = Make_Stack(GREY, signal->width, signal->height,
				 signal->depth);
      Zero_Stack(ws->base_mask);
      Trace_Evaluate_Seed(seed2, signal, 1.0, ws);

      
      if (tw->refit == FALSE) {
	tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) * 2.0 - 1.0;
      } else {
	tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) * 1.5 - 1.0;
      }
      
      //tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) / 2.0;
      int nchain;
      Trace_Locseg_S(signal, 1.0, ws->locseg, ws->score, ws->nseed, tw, 
		     &nchain);

      ws->trace_mask = NULL;
      base_mask = ws->base_mask;
      ws->base_mask = NULL;
      Kill_Trace_Evaluate_Seed_Workspace(ws);
      ssw->weight = NULL;
      Kill_Stack_Seed_Workspace(ssw);
    }
  }

  if (recovery >= 2) { /* re-re-explore */
    printf("make trace mask.\n");
    trace_mask = Make_Stack(GREY, tw->trace_mask->width, 
			    tw->trace_mask->height, tw->trace_mask->depth);

    TZ_ASSERT(tw->trace_mask->kind == GREY16, "Unexpected stack kind.");

    uint16 *trace_mask_array = (uint16*) tw->trace_mask->array;

    /* combine trace_mask and base mask */
    int nvoxel = Stack_Voxel_Number(trace_mask);
    for (i = 0; i < nvoxel; i++) {
      if ((trace_mask_array[i] > 0) || (base_mask->array[i] == 1)) {
	trace_mask->array[i] = 1;
      } else {
	trace_mask->array[i] = 0;
      }
    }

    Kill_Stack(base_mask);
    base_mask = NULL;

    /*
    Kill_Stack(signal);
    signal = NULL;
    */
    printf("dilate\n");
    Struct_Element *se = Make_Zline_Se(5);
    Stack *submask = Stack_Dilate(trace_mask, NULL, se);

    printf("subtract\n");
    Stack_Bsub(leftover, submask, trace_mask);

    Kill_Stack(submask);
    submask = NULL;

    printf("remove small objects\n");
    Stack_Remove_Small_Object(trace_mask, leftover, 27, 26);

    /*
    printf("reload signal\n");
    signal = Read_Stack(Get_String_Arg("image"));
    */

    if (Stack_Is_Dark(leftover) == FALSE) {

#ifdef _DEBUG_2
      Write_Stack("../data/leftover.tif", leftover);
#endif

      Stack_Seed_Workspace *ssw = New_Stack_Seed_Workspace();
      ssw->method = 1;
      //ssw->weight = signal;
      Geo3d_Scalar_Field *seed2 = Stack_Seed(leftover, ssw);
      Trace_Evaluate_Seed_Workspace *ws = New_Trace_Evaluate_Seed_Workspace();

      ws->trace_mask = tw->trace_mask;
      ws->base_mask = Make_Stack(GREY, signal->width, signal->height,
				 signal->depth);
      Zero_Stack(ws->base_mask);
      ws->fit_option = 3;
      Trace_Evaluate_Seed(seed2, signal, 1.0, ws);

      
      if (tw->refit == FALSE) {
	tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) * 2.0 - 1.0;
      } else {
	tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) * 1.5 - 1.0;
      }
      
      //tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) / 2.0;

      int nchain;
      Trace_Locseg_S(signal, 1.0, ws->locseg, ws->score, ws->nseed, tw, 
		     &nchain);

      ws->trace_mask = NULL;
      Kill_Trace_Evaluate_Seed_Workspace(ws);
      Kill_Stack_Seed_Workspace(ssw);
    }
  }

  /* tune ends of the chains */
#if 1
  tw->trace_step = 0.1;
  //fw->sws->field_func = Neurofield_S3;

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->signal = signal;
  ws->sratio = 1.0;
  ws->sdiff = 1.0;
  //ws->option = 6;
  ws->option = LOCSEG_LABEL_OPTION_ADD;
  //ws->flag = 0;

  Zero_Stack(tw->trace_mask);

  fw->sws->mask = tw->trace_mask;

  for (i = 0; i < tw->chain_id; i++) {
    sprintf(file_path, "%s/chain%d.tb", Get_String_Arg("-D"), i);
    if (fexist(file_path)) {
      printf("id: %d\n", i);
      Locseg_Chain *chain = Read_Locseg_Chain(file_path);
      //ws->value = i + 1;
      Locseg_Chain_Label_W(chain, tw->trace_mask, 1.0, 
			   0, Locseg_Chain_Length(chain) - 1, 
			   ws); 
    }
  }
  
  //Locseg_Fit_Workspace_Enable_Cone(fw);
  tw->refit = FALSE;
  tw->min_score -= 0.05;

  double thre = 0.0;

  for (i = 0; i < tw->chain_id; i++) {
    sprintf(file_path, "%s/chain%d.tb", Get_String_Arg("-D"), i);
    if (fexist(file_path)) {
      printf("id: %d\n", i);
      Locseg_Chain *chain = Read_Locseg_Chain(file_path);
      
      /* erase the mask */
      //ws->value = 0;
      //ws->flag = i + 1;
      ws->option = LOCSEG_LABEL_OPTION_SUB;
      Locseg_Chain_Label_W(chain, tw->trace_mask, 1.0, 
			   0, Locseg_Chain_Length(chain) - 1, 
			   ws); 

      if (Is_Arg_Matched("-push")) {
	Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_HEAD);
	Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_TAIL);
	tw->trace_status[0] = TRACE_NORMAL;
	tw->trace_status[1] = TRACE_NORMAL;
	if (Locseg_Chain_Length(chain) == 1) {
	  /* break the segment into two parts */
	  Locseg_Chain_Break_Node(chain, 0, 0.5);
	} else {
	  Locseg_Chain_Adjust_Seed(chain);
	}
	Trace_Locseg(signal, 1.0, chain, tw);
	Locseg_Chain_Down_Sample(chain);
      }
      
      Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_HEAD);
      Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_TAIL);
      

      /* post processing */
      /*
      Locseg_Chain_Remove_Overlap_Ends(chain);
      Locseg_Chain_Remove_Turn_Ends(chain, 1.0);
      */
      /***********************************/

      if (Locseg_Chain_Length(chain) > 0) {
	Write_Locseg_Chain(file_path, chain);
	double min_signal = Locseg_Chain_Min_Seg_Signal(chain, signal, 1.0);
	if (thre < min_signal) {
	  thre = min_signal;
	}
	//ws->value = i + 1;
	//ws->flag = 0;
	ws->option = LOCSEG_LABEL_OPTION_ADD;
	Locseg_Chain_Label_W(chain, tw->trace_mask, 1.0, 
			     0, Locseg_Chain_Length(chain) - 1, ws); 
	if (tw->canvas != NULL) {
	  Locseg_Chain_Label(chain, tw->canvas, 1.0);
	}
      } else {
	char cmd[500];
	sprintf(cmd, "rm %s", file_path);
	system(cmd);
      }
    }
  }
#endif
  
  if (recovery >= 3) {
    tw->trace_step = 0.5;
    leftover= Copy_Stack(signal);
    Stack_Threshold_Binarize(leftover, iround(thre)-1);
    Stack_Sub(leftover, tw->trace_mask, leftover);

    /* last check */
    if (Stack_Is_Dark(leftover) == FALSE) {
#ifdef _DEBUG_2
      Write_Stack("../data/leftover.tif", leftover);
#endif

      Stack_Seed_Workspace *ssw = New_Stack_Seed_Workspace();
      ssw->method = 1;
      Geo3d_Scalar_Field *seed2 = Stack_Seed(leftover, ssw);
      Trace_Evaluate_Seed_Workspace *ws = New_Trace_Evaluate_Seed_Workspace();

      ws->trace_mask = tw->trace_mask;
      ws->base_mask = Make_Stack(GREY, signal->width, signal->height,
	  signal->depth);
      Zero_Stack(ws->base_mask);
      ws->fit_option = 3;
      Trace_Evaluate_Seed(seed2, signal, 1.0, ws);

      tw->min_chain_length = NEUROSEG_DEFAULT_H / 2.0;

      /*
	 if (tw->refit == FALSE) {
	 tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) * 2.0 - 1.0;
	 } else {
	 tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) * 1.5 - 1.0;
	 }
       */

      //tw->min_chain_length = (NEUROSEG_DEFAULT_H  - 1.0) / 2.0;

      int nchain;
      Trace_Locseg_S(signal, 1.0, ws->locseg, ws->score, ws->nseed, tw, 
	  &nchain);

      ws->trace_mask = NULL;
      Kill_Trace_Evaluate_Seed_Workspace(ws);
      Kill_Stack_Seed_Workspace(ssw);
    }
  }

  if (tw->canvas != NULL) {
    sprintf(file_path, "%s/traced.tif", Get_String_Arg("-D"));
    printf("%s saved.\n", file_path);
    Write_Stack(file_path, tw->canvas);
  }

  sprintf(file_path, "%s/trace_mask.tif", Get_String_Arg("-D"));
  printf("%s saved.\n", file_path);
  Write_Stack(file_path, tw->trace_mask);

  Kill_Stack(leftover);
  leftover = Read_Stack(fullpath(dir, Get_String_Arg("-mask"), NULL));
  Stack_Binarize(tw->trace_mask);
  Translate_Stack(tw->trace_mask, GREY, 1);
  Struct_Element *se = Make_Ball_Se(3);
  trace_mask = Stack_Dilate(tw->trace_mask, NULL, se);
  Stack_Bsub(leftover, trace_mask, leftover);
  sprintf(file_path, "%s/leftover.tif", Get_String_Arg("-D"));
  Write_Stack(file_path, leftover);

  return 0;
}
