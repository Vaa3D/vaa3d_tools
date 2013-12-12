/* tz_locnp_chain.c
 *
 * 06-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_geo3d_utils.h"
#include "tz_locnp_chain.h"
#include "tz_trace_utils.h"

Locnp_Chain* Locnp_Chain_Trace_Init(const Stack *stack, double z_scale, 
				    Local_Neuroseg_Plane *locnp, 
				    void *ws)
{
  Stack_Fit_Score *fs = (Stack_Fit_Score*) ws;

  int var_index[LOCAL_NEUROSEG_PLANE_NPARAM];
  int nvar = 
    Local_Neuroseg_Plane_Var_Mask_To_Index(NEUROSEG_PLANE_VAR_MASK_R | 
					   NEUROSEG_PLANE_VAR_MASK_OFFSET,
					   NEUROPOS_VAR_MASK_NONE,
					   var_index);
  Fit_Local_Neuroseg_Plane(locnp, stack, var_index, nvar, z_scale, fs);

  Trace_Record *tr = New_Trace_Record();
  tr->mask = ZERO_BIT_MASK;

  Locnp_Node *p = Make_Locnp_Node(locnp, tr);
  Locnp_Chain *chain = Make_Locnp_Chain(p);
  
  return chain;
}


int Locnp_Chain_Trace_Test(void *argv[])
{
  Local_Neuroseg_Plane *locnp = (Local_Neuroseg_Plane *) argv[0];
  Locnp_Chain *chain = (Locnp_Chain *) argv[1];
  Trace_Workspace *tw = (Trace_Workspace *) argv[2];
  Trace_Record *tr = (Trace_Record *) argv[3];
  double z_scale = *((double *) argv[4]);
  double max_r = *((double *) argv[5]);

  ASSERT(chain != NULL, "Invalid tracing result");

  double center[3];
  Local_Neuroseg_Plane_Center(locnp, center);

  if (tr->fs.scores[1] < tw->min_score) {
    printf("low score: %g\n", tr->fs.scores[1]);
    return TRACE_LOW_SCORE;
  }

  if (LOCAL_NEUROSEG_PLANE_RADIUS(locnp) > max_r) {
    printf("too large\n");
    return TRACE_TOO_LARGE;
  }

  Local_Neuroseg_Plane *last_locnp = Locnp_Chain_Tail_Seg(chain);
  double center2[3];
  double npr = dmax2(LOCAL_NEUROSEG_PLANE_RADIUS(last_locnp), 
		    LOCAL_NEUROSEG_PLANE_RADIUS(locnp)) * 2.0 + 2.0;
  Local_Neuroseg_Plane_Center(last_locnp, center2);
  if (Geo3d_Dist_Sqr(locnp->np.offset_x, locnp->np.offset_y, 0, 
		     last_locnp->np.offset_x, last_locnp->np.offset_y, 0) 
      > npr * npr) {
    printf("too much shift\n");
    return TRACE_INVALID_SHAPE;
  }

  int i;
  for (i = 0; i < 3; i++) {
    if (tw->trace_range[i] >= 0.0) {
      if (center[i] < tw->trace_range[i]) {
	printf("out of bound\n");
	return TRACE_OUT_OF_BOUND;
      }
    }

    if (tw->trace_range[i + 3] >= 0.0) {
      if (center[i] > tw->trace_range[i + 3]) {
	printf("out of bound\n");
	return TRACE_OUT_OF_BOUND;
      }
    }
  }

  if (tw->trace_mask != NULL) {  
    int hit_label = Get_Stack_Pixel(tw->trace_mask, (int) round(center[0]),
				    (int) round(center[1]), 
				    (int) round(center[2] * z_scale), 0);
    
    if (hit_label > 0) {
      tr->hit_region = hit_label;
      printf("hit marked region\n");
      return TRACE_HIT_MARK;
    }
  }

  return TRACE_NORMAL;
}

int Locnp_Chain_Trace_Test2(void *argv[])
{
  Local_Neuroseg_Plane *locnp = (Local_Neuroseg_Plane *) argv[0];
  Locnp_Chain *chain = (Locnp_Chain *) argv[1];
  Trace_Workspace *tw = (Trace_Workspace *) argv[2];
  Trace_Record *tr = (Trace_Record *) argv[3];
  double z_scale = *((double *) argv[4]);
  double max_r = *((double *) argv[5]);

  ASSERT(chain != NULL, "Invalid tracing result");

  double center[3];
  Local_Neuroseg_Plane_Center(locnp, center);
  
  if (tr->fs.scores[1] < tw->min_score) {
    printf("low score: %g\n", tr->fs.scores[1]);
    return TRACE_LOW_SCORE;
  }
  
  if (LOCAL_NEUROSEG_PLANE_RADIUS(locnp) > max_r) {
    printf("too large\n");
    return TRACE_TOO_LARGE;
  }

  Local_Neuroseg_Plane *last_locnp = Locnp_Chain_Tail_Seg(chain);
  double center2[3];
  double npr = dmax2(LOCAL_NEUROSEG_PLANE_RADIUS(last_locnp), 
		    LOCAL_NEUROSEG_PLANE_RADIUS(locnp)) * 2.0 + 2.0;
  Local_Neuroseg_Plane_Center(last_locnp, center2);
  
  if (Geo3d_Dist_Sqr(locnp->np.offset_x, locnp->np.offset_y, 0, 
		     last_locnp->np.offset_x, last_locnp->np.offset_y, 0) 
      > npr * npr) {
    printf("too much shift\n");
    return TRACE_INVALID_SHAPE;
  }
  
  int i;
  for (i = 0; i < 3; i++) {
    if (tw->trace_range[i] >= 0.0) {
      if (center[i] < tw->trace_range[i]) {
	printf("out of bound\n");
	return TRACE_OUT_OF_BOUND;
      }
    }

    if (tw->trace_range[i + 3] >= 0.0) {
      if (center[i] > tw->trace_range[i + 3]) {
	printf("out of bound\n");
	return TRACE_OUT_OF_BOUND;
      }
    }
  }
  
  if (tw->trace_mask != NULL) {  
    int hit_label = Get_Stack_Pixel(tw->trace_mask, (int) round(center[0]),
				    (int) round(center[1]), 
				    (int) round(center[2] * z_scale), 0);
    
    if (hit_label > 0) {
      tr->hit_region = hit_label;
      printf("hit marked region\n");
      return TRACE_HIT_MARK;
    }
  }
  
  return TRACE_NORMAL;
}

#define TRACE_LOCNP(current_locnp, step, list_end, end_index)		\
  if (current_locnp != NULL) {						\
    current_locnp = Next_Neuroseg_Plane(current_locnp, step);		\
    Fit_Local_Neuroseg_Plane(current_locnp, stack, var_index, nvar, z_scale, &(tr.fs)); \
    argv[0] = current_locnp;						\
    cur_end_status = tw->test_func(argv);				\
    Locnp_Node *current_node = NULL;					\
    if (cur_end_status == TRACE_NORMAL) {				\
      current_node = Make_Locnp_Node(current_locnp, Copy_Trace_Record(&tr)); \
      Locnp_Chain_Add_Node(chain, current_node, list_end);		\
      i++;								\
      current_ends[end_index] = current_node;				\
    } else {								\
      if (list_end == DL_HEAD) {					\
	tw->trace_status[0] = cur_end_status;				\
      } else {								\
	tw->trace_status[1] = cur_end_status;				\
      }									\
      if ((cur_end_status == TRACE_HIT_MARK) ||				\
	  (cur_end_status == TRACE_OUT_OF_BOUND)) {			\
	current_node = Make_Locnp_Node(current_locnp,			\
				       Copy_Trace_Record(&tr));		\
	Locnp_Chain_Add_Node(chain, current_node, list_end);		\
      } else {								\
	Delete_Local_Neuroseg_Plane(current_locnp);			\
      }									\
      current_locnp = NULL;						\
    }									\
  }

void Trace_Locnp(const Stack *stack, double z_scale, Locnp_Chain *chain, 
		 Trace_Workspace *tw)
{
  if ((stack == NULL) || (chain == NULL) || (chain->list == NULL)) {
    return;
  }

  /* build variable list */
  int var_index[LOCAL_NEUROSEG_PLANE_NPARAM];
  int nvar = 
    Local_Neuroseg_Plane_Var_Mask_To_Index(NEUROSEG_PLANE_VAR_MASK_R | 
					   NEUROSEG_PLANE_VAR_MASK_OFFSET,
					   NEUROPOS_VAR_MASK_NONE,
					   var_index);

  Trace_Record tr;
  tr.mask = ZERO_BIT_MASK;
  tr.fs.n = 2;
  tr.fs.options[0] = 0;
  tr.fs.options[1] = tw->tscore_option;
  
  //Dlist_Direction_e trace_direction = (Dlist_Direction_e)DL_UNDEFINED;

  /* make parameters for validity testing */
  void *argv[6];
  argv[0] = NULL;
  argv[1] = chain;
  argv[2] = tw;
  argv[3] = &tr;
  argv[4] = &z_scale;
  argv[5] = &(tw->dyvar[0]);

  Locnp_Node *current_ends[2]; /* two ends: 0 head 1 tail*/
  current_ends[0] = Locnp_Chain_Head(chain);
  current_ends[1] = Locnp_Chain_Tail(chain);

  Local_Neuroseg_Plane *forward_locnp = NULL;
  Local_Neuroseg_Plane *backward_locnp = NULL;

  if (tw->trace_status[1] == TRACE_NORMAL) { /* tail, go forward */
    forward_locnp = current_ends[1]->locseg;
  }

  if (tw->trace_status[0] == TRACE_NORMAL) { /* head, go backward */
    backward_locnp = current_ends[0]->locseg;
  }

  if (tw->fit_first == TRUE) {
    if (tw->trace_status[1] == TRACE_NORMAL) {
      Fit_Local_Neuroseg_Plane(forward_locnp, stack, var_index, nvar, z_scale, 
			       &(tr.fs));
      argv[0] = forward_locnp;
      //trace_direction = DL_FORWARD;
      tw->trace_status[1] = tw->test_func(argv);
    } else if (tw->trace_status[0] == TRACE_NORMAL) {
      Fit_Local_Neuroseg_Plane(backward_locnp, stack, var_index, nvar, 
			       z_scale, &(tr.fs));
      argv[0] = backward_locnp;
      //trace_direction = DL_BACKWARD;
      tw->trace_status[0] = tw->test_func(argv);
    }
  }

  int i = 1;
  double step = 1.0;
  int cur_end_status = TRACE_NORMAL;

  while ((i < tw->length) && 
	 ((tw->trace_status[0] == TRACE_NORMAL) || 
	  (tw->trace_status[1] == TRACE_NORMAL))) {
    printf("segment %d\n", i);

    int end_index = 0;
    if (tw->trace_status[end_index] == TRACE_NORMAL) {
      Reset_Trace_Record(&tr);
      TRACE_LOCNP(backward_locnp, -step, DL_HEAD, end_index);
    }
    end_index = 1;
    if (tw->trace_status[end_index] == TRACE_NORMAL) {
      Reset_Trace_Record(&tr);
      TRACE_LOCNP(forward_locnp, step, DL_TAIL, end_index);
    }
  }
}
