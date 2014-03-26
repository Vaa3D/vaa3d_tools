/* tz_neurotrace.c
 *
 * 31-Oct-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <math.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_darray.h"
#include "tz_3dgeom.h"
#include "tz_geoangle_utils.h"
#include "tz_geo3d_utils.h"
#include "tz_geo3d_vector.h"
#include "tz_stack_draw.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_cont_fun.h"
#include "tz_trace_utils.h"
#include "tz_neurotrace.h"

INIT_EXCEPTION

#include "private/tz_neurotrace.c"

double Set_Neuroseg_Max_Radius(double r)
{
  double old_r = Neuroseg_Max_Radius;
  Neuroseg_Max_Radius = r;
  return old_r;
}

/* Initialize_Tracing(): Initialize neuron tracing.
 *
 * Args: stack - stack for tracing;
 *       chain - chain for initialization;
 *       locseg - neuron segment to initialize the chain;
 *       z_scale - z scale factor.
 * 
 * Return: the score of fitting.  
 */
double Initialize_Tracing(const Stack *stack, Neurochain *chain,
			  const Local_Neuroseg *locseg,
			  double z_scale)
{
  if (chain == NULL) {
    THROW(ERROR_POINTER_NULL);
  }

  Init_Neurochain(chain, locseg);

  int var_index[LOCAL_NEUROSEG_NPARAM];
  //int nvar = Init_Local_Neuroseg_Var_Index(NEUROSEG_PARAM_NO_H, TRUE, var_index);
  int nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_NONE,
				     NEUROSEG_VAR_MASK_ALL, var_index);

  Neuroseg_Fit_Score fs;
  fs.n = 3;
  fs.options[0] = 0;
  fs.options[1] = 1;
  fs.options[2] = 4;
  Fit_Local_Neuroseg_P(&(chain->locseg), stack, var_index, nvar, NULL,
		       z_scale, &fs);

  Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL & 
				     ~NEUROSEG_VAR_MASK_HEIGHT &
				     ~NEUROSEG_VAR_MASK_CURVATURE,
				     NEUROSEG_VAR_MASK_NONE, var_index);
  Fit_Local_Neuroseg_P(&(chain->locseg), stack, var_index, nvar, NULL,
		       z_scale, &fs);

  /* record fitting history */
  if (chain->hist == NULL) {
    chain->hist = New_Trace_History();
  }
  Trace_History_Set_Score(chain->hist, &fs);
  Trace_History_Set_Index(chain->hist, 0);

  return fs.scores[0];
}

/* Trace_Neuron(): neuron tracing.
 *
 * Args: stack - the stack of a neuron;
 *       chain - start of the chain;
 *       d - direction.
 *       mask - mask for traced regions;
 *       z_scale - z scale  of the stack;
 *       nseg - maximum number of segments. 
 *
 * Return: a neuron chain.
 */
Neurochain *Trace_Neuron2(const Stack *stack, Neurochain *chain, Direction_e d,
			  const Stack *mask, double z_scale, int nseg)
{
  if ((stack == NULL) || (chain == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }
  
  Neurochain *chain2 = NULL;

  switch(d) {
  case BACKWARD:
    Flip_Local_Neuroseg(&(chain->locseg));
    break;
  case FORWARD:
    break;
  case BOTH:
    Trace_Neuron2(stack, chain, FORWARD, mask, z_scale, nseg);
    chain2 = New_Neurochain();
    Init_Neurochain(chain2, &(chain->locseg));
    Append_Neurochain(chain, chain2);
    Trace_Neuron2(stack, chain2, BACKWARD, mask, z_scale, nseg);
    chain = Neurochain_Cut(chain2);

    if (Neurochain_Length(chain2, FORWARD) == 1) {
      Free_Neurochain(chain2);
    } else {
      chain2->prev = NULL;
      chain2 = Neurochain_Flip_Link(chain2, FORWARD);
      Neurochain_Remove_Last(chain2, 1);
      chain = Neurochain_Tail(Append_Neurochain(chain2, chain));
    }

    return chain;
  case LONG:
    Trace_Neuron2(stack, chain, FORWARD, mask, z_scale, nseg);
    chain2 = New_Neurochain();
    Init_Neurochain(chain2, &(chain->locseg));
    chain2 = Trace_Neuron2(stack, chain2, BACKWARD, mask, z_scale, nseg);

    if (Neurochain_Length(chain, FORWARD) >= 
	Neurochain_Length(chain2, FORWARD)) {
      Free_Neurochain(chain2);
    } else {
      Cross_Neurochain(chain->next, chain2->next);
      Free_Neurochain(chain2);
    }
    return chain;
  case STRONG:
    Trace_Neuron2(stack, chain, FORWARD, mask, z_scale, nseg);
    chain2 = New_Neurochain();
    Init_Neurochain(chain2, &(chain->locseg));
    chain2 = Trace_Neuron2(stack, chain2, BACKWARD, mask, z_scale, nseg);

    double score = Neurochain_Score(chain, stack, z_scale, NULL) / 
      (Neurochain_Length(chain, FORWARD) + 1);
    double score2 = Neurochain_Score(chain2, stack, z_scale, NULL) / 
      (Neurochain_Length(chain2, FORWARD) + 1);
    if (score >= score2) {
      Free_Neurochain(chain2);
    } else {
      Cross_Neurochain(chain->next, chain2->next);
      Free_Neurochain(chain2);
    }
    return chain;
  default:
    THROW(ERROR_DATA_VALUE);
    return NULL;
  }

  /* start tracing */
  BOOL is_valid = TRUE;
  BOOL hit_traced[nseg];
  double theta[nseg];
  double psi[nseg];


  int var_index[LOCAL_NEUROSEG_NPARAM];
  /*
  int nvar = Init_Local_Neuroseg_Var_Index(NEUROSEG_PARAM_NO_H, FALSE, 
					   var_index);
  */
  int nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ALL & 
				     ~NEUROSEG_VAR_MASK_HEIGHT &
				     ~NEUROSEG_VAR_MASK_CURVATURE,
				     NEUROSEG_VAR_MASK_NONE, var_index);
  /*
  int h_var_index[LOCAL_NEUROSEG_NPARAM];
  int h_nvar = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_HEIGHT |
						NEUROSEG_VAR_MASK_R,
						NEUROPOS_VAR_MASK_NONE,
						h_var_index);
  */

  Neuroseg_Fit_Score fs;  
  fs.n = 3;
  fs.options[0] = 4;
  fs.options[1] = 0;
  fs.options[2] = 1;

  Neuroseg_Fit_Score ort_fs;
  ort_fs.n = 1;
  ort_fs.options[0] = 1;

  int invalid_code = 0;
  int length = 1;
  BOOL stop = FALSE;
  Local_Neuroseg *sensor_seg = New_Local_Neuroseg();
  BOOL add_sensor_seg = FALSE;
  Trace_History *hist = New_Trace_History();

  invalid_code = 0;

  while ((length < nseg) && (stop == FALSE)) {
    hist->mask = 0;
    Trace_History_Set_Direction(hist, d);
    printf("Length: %d\n", length);

    if (invalid_code == 2) {
      Next_Local_Neuroseg(&(chain->locseg), sensor_seg, 0.8);
    } else {
      Next_Local_Neuroseg(&(chain->locseg), sensor_seg, 0.4);
    }
    printf("-------begin\n");
    Print_Local_Neuroseg(sensor_seg);
    Fit_Local_Neuroseg_P(sensor_seg, stack, var_index, nvar, NULL,
			 z_scale, &fs); 
    /*
    int var_r_index[2];
    int nvar_r = Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R,
						  NEUROSEG_VAR_MASK_NONE, 
						  var_r_index);
    Fit_Local_Neuroseg_P(sensor_seg, stack, var_r_index, nvar_r, 
			 z_scale, &fs);
    */
    Trace_History_Set_Score(hist, &fs);
    Print_Local_Neuroseg(sensor_seg);
    printf("-------end\n");

    printf("trace score: %g\n", hist->fit_stat);
    /*
    fs.options[0] = 4;
    printf("trace stat: %g\n", Local_Neuroseg_Score(sensor_seg, stack, 
						    z_scale, &fs));
    */
    is_valid = is_trace_valid(sensor_seg, mask, chain, hist->fit_corr, theta,
			      psi, hit_traced, length - 1, z_scale, 
			      &invalid_code);

    if (is_valid == TRUE) {
      add_sensor_seg = TRUE;
    } else if (invalid_code != 2) {
      printf("refitting ...\n");
      
      /*
      Fit_Local_Neuroseg(&(chain->locseg), stack, h_var_index, h_nvar, 
			 z_scale, &fs);
      */
      //Local_Neuroseg_Height_Search(&(chain->locseg), stack, z_scale, 1.0, NULL);

      Local_Neuroseg_Height_Search_P(&(chain->locseg), stack, z_scale);

      double bound_pos[3];
      Local_Neuroseg_Axis_Position(&(chain->locseg), bound_pos, 
				   chain->locseg.seg.h + 
				   NEUROSEG_R2(&(chain->locseg.seg)));

      if ((bound_pos[0] >= 0.0) && (bound_pos[1] >= 0.0) && 
	  (bound_pos[2] >= 0.0) && (bound_pos[0] < (double) stack->width) && 
	  (bound_pos[1] < (double) stack->height) && 
	  (bound_pos[2] < (double) stack->depth)) {
#ifdef _DEBUG_
	Print_Local_Neuroseg(&(chain->locseg));
#endif
	double pos_step = 1.0 - NEUROSEG_R2(&(chain->locseg.seg)) / chain->locseg.seg.h;
	if (pos_step < 0.5) {
	  pos_step = 0.5;
	}
	Next_Local_Neuroseg(&(chain->locseg), sensor_seg, pos_step);
	//Neuroseg_Change_Thickness(&(sensor_seg->seg), 1.0, 1.0);
	
	Local_Neuroseg_Orientation_Search_B(sensor_seg, stack, z_scale, &ort_fs);
	double v[3];
	Geo3d_Orientation_Normal(chain->locseg.seg.theta, chain->locseg.seg.psi,
				 v, v + 1, v + 2);
	/*
	if (Neuroseg_Angle_Between(&(chain->locseg.seg), &(sensor_seg->seg))
	    > TZ_PI / 4.0) {
	  Local_Neuroseg_Position_Search(sensor_seg, stack, z_scale, v, 
					 chain->locseg.seg.h, -0.2, NULL);
	}
	*/
	Print_Local_Neuroseg(sensor_seg);

	int var_link[LOCAL_NEUROSEG_NPARAM];
	bzero(var_link, sizeof(int) * LOCAL_NEUROSEG_NPARAM);
	//Variable_Set_Add_Link(var_link, 0, 1);
	Fit_Local_Neuroseg_P(sensor_seg, stack, var_index, nvar, var_link,
			     z_scale, &fs);	

	Trace_History_Set_Score(hist, &fs);
	printf("refit score: %g\n", hist->fit_stat);
	
	is_valid = is_trace_valid(sensor_seg, mask, chain, hist->fit_corr, 
				  theta, psi, hit_traced, length - 1, z_scale,
				  &invalid_code);
	
	if (is_valid == TRUE) {
	  Trace_History_Set_Is_Refit(hist, TRUE);
	  add_sensor_seg = TRUE;
	} else {
	  add_sensor_seg = FALSE;
	  //stop = TRUE;
	}
      } else {
	printf("trace out of bound:\n");
	add_sensor_seg = FALSE;
      }
    } else {
      add_sensor_seg = FALSE;
    }

    if (add_sensor_seg == TRUE) {
      Trace_History_Set_Index(hist, length);
      Print_Trace_History(hist);
      chain = Extend_Neurochain_Spec(chain, sensor_seg, hist);
      length++;
    } else {
      /*
      if (chain->prev != NULL) {
	double top[3];
	double bottom[3];
	Local_Neuroseg_Top(&(chain->locseg), top);
	if ((Local_Neuroseg_Hit_Test(&(chain->prev->locseg), top[0], top[1],
				     top[2]) == TRUE) &&
	    (Local_Neuroseg_Hit_Test(&(chain->prev->locseg), bottom[0], 
				     bottom[1], bottom[2]) == TRUE)) {
	  printf("Tail overlap. Removed.\n");
	  Print_Neurochain(chain);
	  chain = chain->prev;
	  Neurochain_Remove_Last(chain, 1);
	}
      }
      */
      stop = TRUE;
    }
  }

  Free_Trace_History(hist);
  Delete_Local_Neuroseg(sensor_seg);

  return chain;
}


/* Object_To_Neurochain(): Trace neuron in an object.
 *
 * Args: obj - object to chain;
 *       chain - chain to store results.
 *
 * Return: traced neurochain.
 */
Neurochain *Object_To_Neurochain(Object_3d *obj, Neurochain *chain, 
				 double z_scale, int nseg)
{
  if (chain == NULL) {
    chain = New_Neurochain();
    Set_Neuroseg(&(chain->locseg.seg), 5, 5, 12, -TZ_PI_2, 0.0, 0.0, 0.0, 1.0);
  }

  Set_Neuropos(chain->locseg.pos, 
	       obj->voxels[0][0], obj->voxels[0][1], obj->voxels[0][2]);

  int offset[3];
  Stack *stack = Object_To_Stack_Bw(obj, 1, offset);
  Trace_Neuron2(stack, chain, BOTH, NULL, z_scale, nseg);
  Neurochain_Translate(chain, offset[0], offset[1], offset[2]);

  return chain;
}

BOOL Neuroseg_Hit_Traced(const Local_Neuroseg *locseg, const Stack *mask, 
			 double z_scale)
{
  if (Neuroseg_Hit_At(locseg, mask, z_scale) >=0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

int Neuroseg_Hit_At(const Local_Neuroseg *locseg, const Stack *mask,
		    double z_scale)
{
  if (Stack_Kind(mask) != GREY) {
    THROW(ERROR_DATA_TYPE);
  }

  double dpos[3];
  Local_Neuroseg_Top(locseg, dpos);

#if 0 /* remove lrint dependence */
  int pos[3];
  pos[0] = lrint(dpos[0]);
  pos[1] = lrint(dpos[1]);
  pos[2] = lrint(dpos[2] / z_scale);
#endif  

  int offset = Stack_Util_Offset((int) round(dpos[0]),
				 (int) round(dpos[1]), 
				 (int) round(dpos[2] * z_scale), 
				 mask->width, mask->height, mask->depth);

  if (offset >= 0) {
    if (mask->array[offset] != 0) {
      offset = -1;
    } 
  }

  return offset;  
}

void Process_Neurochain(Neurochain *chain[], int n, Stack *region)
{
  
}

Soma_Trace_Workspace* New_Soma_Trace_Workspace()
{
  Soma_Trace_Workspace *stw = (Soma_Trace_Workspace *) 
    Guarded_Malloc(sizeof(Soma_Trace_Workspace), "New_Soma_Trace_Workspace");
  Default_Trace_Workspace(&(stw->tw));
  stw->tw.min_score = 0.4;
  stw->curv_thre = -0.2;

  return stw;
}

void Delete_Soma_Trace_Workspace(Soma_Trace_Workspace *stw)
{
  free(stw);
}

Locne_Chain* Trace_Soma(const Stack *stack, double z_scale, 
			Local_Neuroseg_Ellipse *seed, 
			Soma_Trace_Workspace *stw)
{
  int default_workspace = (stw == NULL);
  if (default_workspace) {
    stw = New_Soma_Trace_Workspace();
  }

  Local_Neuroseg_Ellipse *trace_seed = Copy_Local_Neuroseg_Ellipse(seed);
  Locne_Chain *chain = Locne_Chain_Trace_Init(stack, z_scale, trace_seed, NULL);

  Trace_Locne(stack, z_scale, chain, &(stw->tw));
  
  int length = Locne_Chain_Length(chain);
  double *area_array = darray_malloc(length);
  double *area_array2 = darray_malloc(length);
  double *tmp_area_array = darray_malloc(length);

  int i = 0;
  Locne_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg_Ellipse *locne = NULL;
  while ((locne = Locne_Chain_Next_Seg(chain)) != NULL) {
    area_array[i] = locne->np.rx * locne->np.ry;
    i++;
  }

  darray_avgsmooth(area_array, length, 3, area_array2);
  darray_scale(area_array2, length, 1, length);

#ifdef _DEBUG_
  darray_write("../data/test.bn", area_array2, length);
#endif

  darray_curvature(area_array2, length, area_array);

#ifdef _DEBUG_
  darray_write("../data/test2.bn", area_array, length);
#endif

  darray_avgsmooth(area_array, length, 5, tmp_area_array);

  darray_div(tmp_area_array, area_array2, length);


  int range[2];

  find_soma_range(area_array, length, stw->curv_thre,// * 2 / stw->tw.length , 
		  range);
  
  free(area_array);
  free(area_array2);
  free(tmp_area_array);

  Locne_Chain_Subchain(chain, range[0], range[1]);
  
  if (default_workspace) {
    Delete_Soma_Trace_Workspace(stw);
  }

  return chain;
}

double Soma_Score(Locne_Chain *soma)
{
  int length;
  double *area = Locne_Chain_Area_Array(soma, NULL, &length);
  double score = soma_score_from_area(area, length);
  free(area);
  
  return score;
}

void Soma_Stack_Mask(Locne_Chain *soma, Stack *stack, double z_scale,
		     double v)
{
  Locne_Chain_Iterator_Start(soma, DL_HEAD);
  Local_Neuroseg_Ellipse *locne = NULL;
  int x, y, z;
  coordinate_3d_t center;
  int x_range[2];
  int z_range[2];
  while ((locne = Locne_Chain_Next_Seg(soma)) != NULL) {
    center[0] = locne->np.offset_x + locne->pos[0];
    center[1] = locne->pos[1];
    center[2] = locne->np.offset_y + locne->pos[2];
    x_range[0] = (int) (center[0] - locne->np.rx + 0.5);
    x_range[1] = (int) (center[0] + locne->np.rx + 0.5);
    z_range[0] = (int) ((center[2] - locne->np.ry) * z_scale + 0.5);
    z_range[1] = (int) ((center[2] + locne->np.ry) * z_scale + 0.5);
    y = (int) center[1];
    for (x = x_range[0]; x <= x_range[1]; x++) {
      for (z = z_range[0]; z < z_range[1]; z++) {
	if (dsqr(x - center[0]) / dsqr(locne->np.rx) + 
	    dsqr(z - center[2]) / dsqr(locne->np.ry * z_scale) <= 1.0 ) {
	  Set_Stack_Pixel(stack, x, y, z, 0, v);
	}
      }
    }
  }
}
