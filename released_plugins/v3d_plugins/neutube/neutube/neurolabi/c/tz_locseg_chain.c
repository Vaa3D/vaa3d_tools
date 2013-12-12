/* tz_locseg_chain.c
 *
 * 06-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#if defined(_WIN32) || defined(_WIN64)
  #define PCRE_STATIC
  #include <pcreposix.h>
#else
  #include <regex.h>
#endif
#include <string.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include "tz_dirent.h"
#endif
#include <utilities.h>
#include "tz_constant.h"
#include "tz_error.h"
#include "tz_interface.h"
#include "tz_stack_sampling.h"
#include "tz_trace_utils.h"
#include "tz_darray.h"
#include "tz_cont_fun.h"
#include "tz_locseg_chain.h"
#include "tz_vrml_material.h"
#include "tz_vrml_io.h"
#include "tz_geo3d_utils.h"
#include "tz_string.h"
#include "tz_geo3d_point_array.h"
#include "tz_stack_graph.h"
#include "tz_image_array.h"
#include "tz_swc_cell.h"
#include "tz_u8array.h"
#include "tz_iarray.h"
#include "tz_math.h"
#include "tz_stack_attribute.h"
#include "tz_geo3d_utils.h"
#include "tz_stack_math.h"
#include "tz_workspace.h"
#include "tz_geoangle_utils.h"
#include "tz_unipointer_linked_list.h"
#include "tz_stack_utils.h"
#include "tz_int_histogram.h"
#include "tz_stack_threshold.h"
#include "private/tz_locseg_chain_p.h"
#include "private/tzp_locseg_chain.c"

Locseg_Chain* Copy_Locseg_Chain(Locseg_Chain *chain)
{
#ifdef _MSC_VER
  Locseg_Chain *chain_copy = New_Locseg_Chain();
#else
  Locseg_Chain *chain_copy = New_Locseg_Chain(chain);
#endif
  
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  
  Locseg_Node *node = NULL;

  while ((node = Locseg_Chain_Next(chain)) != NULL) {
    Locseg_Chain_Add_Node(chain_copy, Copy_Locseg_Node(node), DL_TAIL);
  }

  return chain_copy;
}

BOOL Locseg_Chain_Is_Identical(Locseg_Chain *chain1, Locseg_Chain *chain2)
{
  if (chain1 == chain2) {
    return TRUE;
  }

  Locseg_Chain_Iterator_Start(chain1, DL_HEAD);
  Locseg_Chain_Iterator_Start(chain2, DL_HEAD);

  Locseg_Node *node1 = NULL;
  Locseg_Node *node2 = NULL;

  while ((node1 = Locseg_Chain_Next(chain1)) != NULL) {
    if ((node2 = Locseg_Chain_Next(chain2)) == NULL) {
      return FALSE;
    }

    if (Locseg_Node_Is_Identical(node1, node2) == FALSE) {
      return FALSE;
    }
  }

  return TRUE;
}

double Locseg_Chain_Geolen(Locseg_Chain *chain)
{
  if (chain == NULL) {
    return 0.0;
  }
  
  if (chain->list == NULL) {
    return 0.0;
  }

  /* alloc <ka> */
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  double prev_pos[3], pos[3];
  
  double dist = 0.0;
  Local_Neuroseg *locseg = NULL;  
  Locseg_Chain_Iterator_Start(ka->chain, DL_HEAD);

  int knot_index = 0;
  int index = 0;

  Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
  knot_index++;

  /* For each knot */
  while ((locseg = Locseg_Chain_Next_Seg(ka->chain)) != NULL) {
     while (knot != NULL) {
       if (knot->id == index) {
	 /* Set pos to the current knot */
	 Local_Neuroseg_Axis_Coord_N(locseg, knot->offset, pos);
	 /* if it is not the first knot */ 
	 if (knot_index > 1) {
	   /* Accumulate distance */
	   dist += Geo3d_Dist(pos[0], pos[1], pos[2], prev_pos[0],
			      prev_pos[1], prev_pos[2]);
	 }

	 /* Set prev_pos to pos */
	 prev_pos[0] = pos[0];
	 prev_pos[1] = pos[1];
	 prev_pos[2] = pos[2];

	 knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
	 knot_index++;
       } else {
	 break;
       } 
     }
     index++;
  }

  /* free <ka> */
  Kill_Locseg_Chain_Knot_Array(ka);


  return dist;
}

double Locseg_Chain_Geolen_Z(Locseg_Chain *chain, double z_scale)
{
  if (chain == NULL) {
    return 0.0;
  }

  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  double prev_pos[3], pos[3];
  
  double dist = 0.0;
  Local_Neuroseg *locseg = NULL;  
  Locseg_Chain_Iterator_Start(ka->chain, DL_HEAD);

  int knot_index = 0;
  int index = 0;

  Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
  knot_index++;

  /* For each knot */
  while ((locseg = Locseg_Chain_Next_Seg(ka->chain)) != NULL) {
     while (knot != NULL) {
       if (knot->id == index) {
	 /* Set pos to the current knot */
	 Local_Neuroseg_Axis_Coord_N(locseg, knot->offset, pos);
	 pos[2] /= z_scale;

	 /* if it is not the first knot */ 
	 if (knot_index > 1) {
	   /* Accumulate distance */
	   dist += Geo3d_Dist(pos[0], pos[1], pos[2], prev_pos[0],
			      prev_pos[1], prev_pos[2]);
	 }

	 /* Set prev_pos to pos */
	 prev_pos[0] = pos[0];
	 prev_pos[1] = pos[1];
	 prev_pos[2] = pos[2];

	 knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
	 knot_index++;
       } else {
	 break;
       } 
     }
     index++;
  }

  return dist;
}

Trace_Workspace* 
Locseg_Chain_Default_Trace_Workspace(Trace_Workspace *tw, 
				     const Stack *stack)
{
  if (tw == NULL) {
    tw = New_Trace_Workspace();
  } else {
    Clean_Trace_Workspace(tw);
  }

  //tw->length = 500;
  //tw->fit_first = 0;
  //tw->tscore_option = STACK_FIT_CORRCOEF;
  //tw->min_score = LOCAL_NEUROSEG_MIN_CORRCOEF;
  //tw->trace_direction = DL_BOTHDIR;
  //tw->stop_reason[0] = 0; /* obsolete */
  //tw->stop_reason[1] = 0; /* obsolete */
  //tw->trace_status[0] = TRACE_NORMAL;
  //tw->trace_status[1] = TRACE_NORMAL;
  //tw->canvas_updating = TRUE;

  Default_Trace_Workspace(tw);

  if (stack != NULL) {
    tw->trace_range[0] = 0;
    tw->trace_range[1] = 0;
    tw->trace_range[2] = 0;
    tw->trace_range[3] = stack->width - 1;
    tw->trace_range[4] = stack->height - 1;
    tw->trace_range[5] = stack->depth - 1;
  }

  tw->dyvar[0] = 25.0; /* Max radius */
  tw->dyvar[2] = -1.0; /* Score threshold that triggers height adjustment */
  tw->test_func = Locseg_Chain_Trace_Test;

  return tw;
}

Locseg_Chain* Locseg_Chain_Trace_Init(const Stack *stack, double z_scale, 
				      Local_Neuroseg *locseg, 
				      void *ws)
{
  Locseg_Fit_Workspace *fws = (Locseg_Fit_Workspace*) ws;

  Locseg_Fit_Workspace_Set_Var(fws, NEUROSEG_VAR_MASK_NONE,
			       NEUROPOS_VAR_MASK_ALL);
  Fit_Local_Neuroseg_W(locseg, stack, z_scale, fws);

  Locseg_Fit_Workspace_Set_Var(fws, (NEUROSEG_VAR_MASK_R |
				    NEUROSEG_VAR_MASK_ORIENTATION) &
			       ~NEUROSEG_VAR_MASK_HEIGHT &
			       ~NEUROSEG_VAR_MASK_CURVATURE,
			       NEUROPOS_VAR_MASK_NONE);
  Fit_Local_Neuroseg_W(locseg, stack, z_scale, fws);

  Trace_Record *tr = New_Trace_Record();
  tr->mask = ZERO_BIT_MASK;
  
  Locseg_Node *p = Make_Locseg_Node(locseg, tr);
  Locseg_Chain *chain = Make_Locseg_Chain(p);
  
  return chain;
}

int Locseg_Chain_Trace_Test(void *argv[])
{
  Local_Neuroseg *locseg = (Local_Neuroseg *) argv[0];
  Locseg_Chain *chain = (Locseg_Chain *) argv[1];
  Trace_Workspace *tw = (Trace_Workspace *) argv[2];
  Trace_Record *tr = (Trace_Record *) argv[3];
  double z_scale = *((double *) argv[4]);
  double max_r = *((double *) argv[5]);
  Dlist_Direction_e trace_direction = *((Dlist_Direction_e *) argv[6]);
  double min_r = *((double *) argv[7]);
  Local_Neuroseg *prev_locseg = (Local_Neuroseg*) argv[8];
  Stack *stack = (Stack*) argv[9];

  double pos[3];
  double ipos[3];
  pos[0] = -1.0;
  double segr1 = 1.0;
  double segr2 = 1.0;

  if (locseg != NULL) {
    if (trace_direction == DL_FORWARD) {
      Local_Neuroseg_Top(locseg, pos);
    } else if (trace_direction == DL_BACKWARD){
      Local_Neuroseg_Bottom(locseg, pos);
    } else {
      pos[0] = -1.0;
    }

    if (tw->resolution[0] > 0.0) {
      segr1 = 
        sqrt(Neuroseg_Rx_P(&(locseg->seg), tw->resolution, NEUROSEG_BOTTOM) * 
            Neuroseg_Ry_P(&(locseg->seg), tw->resolution, NEUROSEG_BOTTOM));
      segr2 = 
        sqrt(Neuroseg_Rx_P(&(locseg->seg), tw->resolution, NEUROSEG_TOP) * 
            Neuroseg_Ry_P(&(locseg->seg), tw->resolution, NEUROSEG_TOP));
    } else {
      segr1 = 
        sqrt(Neuroseg_Rx(&(locseg->seg), NEUROSEG_BOTTOM) * 
            Neuroseg_Ry(&(locseg->seg), NEUROSEG_BOTTOM));
      segr2 = 
        sqrt(Neuroseg_Rx(&(locseg->seg), NEUROSEG_TOP) * 
            Neuroseg_Ry(&(locseg->seg), NEUROSEG_TOP));      
    }
    //NEUROSEG_RC(&(locseg->seg)) * sqrt(locseg->seg.scale);
    //dmin2(locseg->seg.r1, locseg->seg.r2) * sqrt(locseg->seg.scale);
  }

  ipos[0] = pos[0];
  ipos[1] = pos[1];
  ipos[2] = pos[2] * z_scale;
  if (tr != NULL) {
    if (Local_Neuroseg_Good_Score(locseg, tr->fs.scores[tw->tscore_option],
          tw->min_score) == FALSE) {
      printf("low score: %g\n", tr->fs.scores[1]);
      return TRACE_LOW_SCORE;
    }

    if (tw->sup_stack != NULL) {
      Stack_Fit_Score fs;
      fs.n = 1;
      fs.options[0] = tw->tscore_option;
      double sup_score = Local_Neuroseg_Score(locseg, tw->sup_stack, z_scale, 
          &fs);
      if (Local_Neuroseg_Good_Score(locseg, sup_score, tw->min_score) 
          == FALSE) {
        printf("low sup score: %g\n", sup_score);
        return TRACE_LOW_SCORE;
      }
    }
  }

  /*
     if ((prev_locseg != NULL) && (locseg != NULL)) {
     if (locseg_overlap(prev_locseg, locseg, trace_direction) == TRUE) {
     printf("trace overlaped\n");
     return TRACE_OVERLAP;
     }
     }
     */
  if (locseg != NULL) {
    if ((segr1 > max_r) || (segr2 > max_r)) { /* too thick*/
      printf("too thick\n");
      return TRACE_TOO_LARGE;
    }

    if ((segr1 < min_r) || (segr2 < min_r)) { /* too thin*/
      printf("too thin\n");
      return TRACE_TOO_SMALL;
    }

    double max_change = max_r / 2;
    if (fabs(locseg->seg.c * locseg->seg.h) >= max_change) { /* irregular */
      printf("irregular tube 1\n");
      return TRACE_INVALID_SHAPE;
    } else if (NEUROSEG_RA(&(locseg->seg)) > 2.5) {
      if (NEUROSEG_RB(&(locseg->seg)) / 
          NEUROSEG_RA(&(locseg->seg)) > 3.0) {
        printf("irregular tube 2\n");
        return TRACE_INVALID_SHAPE;
      }
    }
  }

  if ((chain != NULL) && (pos[0] >= 0.0)) {
    if ((Locseg_Chain_Hit_Test(chain, DL_BACKWARD, 
            pos[0], pos[1], pos[2]) > 0) ||
        (Locseg_Chain_Form_Loop(chain, locseg, trace_direction) == TRUE)) {
      printf("Loop formed\n");
      return TRACE_LOOP_FORMED;
    }
  }

  if (prev_locseg != NULL) {
    double r1 = NEUROSEG_CRC(&(prev_locseg->seg));
    double r2 = NEUROSEG_CRC(&(locseg->seg));
    if (r2 > 1.0) {
      double ratio = r1 / r2;
      if ((ratio > 2.0) || (ratio < 0.5)) {
        return TRACE_SIZE_CHANGE;
      }
    }

    double intensity1 = Local_Neuroseg_Average_Signal(locseg, stack, z_scale);
    double intensity2 = Local_Neuroseg_Average_Signal(prev_locseg, stack, 
        z_scale);
    if (intensity1 / intensity2 < 0.5) {
      return TRACE_SIGNAL_CHANGE;
    }
  }

  if (locseg != NULL) {

#if 0
    if (ipos[0] >= 0) {
      int i;
      for (i = 0; i < 3; i++) {
        if (tw->trace_range[i] >= 0.0) {
          if (ipos[i] < tw->trace_range[i]) {
            printf("out of bound\n");
            return TRACE_OUT_OF_BOUND;
          }
        }

        if (tw->trace_range[i + 3] >= 0.0) {
          if (ipos[i] > tw->trace_range[i + 3]) {
            printf("out of bound\n");
            return TRACE_OUT_OF_BOUND;
          }
        }
      }
    }
#endif

#if 1
    if (Trace_Workspace_Point_In_Bound(tw, ipos) == FALSE) {
      printf("out of bound\n");
      return TRACE_OUT_OF_BOUND;
    }
#endif

    int hit_label = Trace_Workspace_Mask_Value(tw, ipos); 
    if(hit_label > 0) {
      tr->hit_region = hit_label;
      printf("hit marked region\n");
      return TRACE_HIT_MARK;
    }    

#if 0
    if (tw->trace_mask != NULL) {
      int hit_label = 0;
      if (ipos[0] >= 0) {
        hit_label = Get_Stack_Pixel(tw->trace_mask, (int) round(ipos[0]),
            (int) round(ipos[1]), 
            (int) round(ipos[2]), 0);
      }

      /*      
              if (hit_label == 0) {
              double center[3];
              Local_Neuroseg_Center(locseg, center);
              hit_label = Get_Stack_Pixel(tw->trace_mask, (int) round(center[0]),
              (int) round(center[1]), 
              (int) round(center[2] * z_scale), 0);
              }
              */     
      if(hit_label > 0) {
        tr->hit_region = hit_label;
        printf("hit marked region\n");
        return TRACE_HIT_MARK;
      }    
    }
#endif
  }

  return TRACE_NORMAL;
}

#define RECORD_TRACE_SCORE(record, fit_workspace)			\
  record.fs.scores[0] = fit_workspace->sws->fs.scores[0];		\
  record.fs.scores[1] = fit_workspace->sws->fs.scores[fit_workspace->sws->fs.n-1];

#define LOCSEG_CHAIN_BEGIN_FIT(locseg, list_end)			\
  if (list_end == DL_HEAD) {						\
    trace_direction = DL_BACKWARD;					\
    Flip_Local_Neuroseg(locseg);					\
  } else {								\
    trace_direction = DL_FORWARD;					\
  }

#define LOCSEG_CHAIN_END_FIT(locseg, list_end)				\
  if (list_end == DL_HEAD) {						\
    Flip_Local_Neuroseg(locseg);					\
  }  


#define LOCSEG_CHAIN_FIT(locseg)					\
  Fit_Local_Neuroseg_W(locseg, stack, z_scale, fit_ws);			\
  RECORD_TRACE_SCORE(tr, fit_ws);

#define LOCSEG_CHAIN_FIT_H(locseg)					\
  Fit_Local_Neuroseg_W(locseg, stack, z_scale, fit_ws_h);		\
  RECORD_TRACE_SCORE(tr, fit_ws_h);

/*
#define LOCSEG_CHAIN_FIT(locseg)					\
  Fit_Local_Neuroseg_P(locseg, stack, fit_ws->var_index, fit_ws->nvar, fit_ws->var_link, z_scale, &(tr.fs));

#define LOCSEG_CHAIN_FIT_H(locseg)					\
  Fit_Local_Neuroseg_P(locseg, stack, fit_ws_h->var_index, fit_ws_h->nvar, fit_ws_h->var_link, z_scale, &(tr.fs));
*/

#define LOCSEG_CHAIN_REFIT(refit_locseg, list_end, end_index)		\
  printf("refitting ...\n");						\
  nrefit++;								\
  Locseg_Node *p = current_ends[end_index];				\
  ASSERT(Trace_Record_Fit_Height(p->tr, end_index) != 1, "bug found");	\
  LOCSEG_CHAIN_BEGIN_FIT(p->locseg, list_end);				\
  printf("  Adjust the height of the current chain end.\n");		\
  /*Local_Neuroseg_Height_Search_E(p->locseg, 2, stack, z_scale);*/	\
  Local_Neuroseg_Height_Search_W(p->locseg, stack, z_scale, fit_ws->sws); \
  printf("  Label it as height adjusted.\n");				\
  Trace_Record_Set_Fit_Height(p->tr, end_index, 1);			\
  LOCSEG_CHAIN_END_FIT(p->locseg, list_end);				\
  cur_end_status = TRACE_NORMAL;					\
  while ((p->locseg->seg.h < NEUROSEG_DEFAULT_H / 3.0)			\
      && (i > 1)) { /* requires improvement */				\
    printf("  The current end is too short after adjustment, remove it.\n"); \
    Locseg_Chain_Remove_End(chain, list_end);				\
    i--;								\
    Locseg_Chain_Iterator_Start(chain, list_end);			\
    printf("  Update the current end.");				\
    if (list_end == DL_HEAD) {						\
      current_ends[end_index] = Locseg_Chain_Next(chain);		\
      p = current_ends[end_index];					\
    } else {								\
      current_ends[end_index] = Locseg_Chain_Prev(chain);		\
      p = current_ends[end_index];					\
    }									\
    if (dmax2(Neuroseg_Rx(&(p->locseg->seg), NEUROSEG_CENTER), Neuroseg_Ry(&(p->locseg->seg), NEUROSEG_CENTER)) >= NEUROSEG_DEFAULT_H / 2.0) { \
      printf("  Too large for refit");					\
      cur_end_status = TRACE_TOO_LARGE;					\
    } else {								\
      if (Trace_Record_Fit_Height(p->tr, end_index) == 0) {		\
	LOCSEG_CHAIN_BEGIN_FIT(p->locseg, list_end);			\
	printf("  Adjust the height of the current end.\n");		\
	/*Local_Neuroseg_Height_Search_E(p->locseg, 2, stack, z_scale);*/ \
	Local_Neuroseg_Height_Search_W(p->locseg, stack, z_scale, fit_ws->sws);	\
	printf("  Label it as height adjusted.\n");			\
	Trace_Record_Set_Fit_Height(p->tr, end_index, 1);		\
	LOCSEG_CHAIN_END_FIT(p->locseg, list_end);			\
      } else {								\
	printf("  Trace repeated because the current end had been height adjusted before.\n"); \
	cur_end_status = TRACE_REPEATED;				\
      }									\
    }									\
  }									\
  									\
  if (dmax2(Neuroseg_Rx(&(p->locseg->seg), NEUROSEG_CENTER),		\
	    Neuroseg_Ry(&(p->locseg->seg), NEUROSEG_CENTER))		\
      >= NEUROSEG_DEFAULT_H / 2.0) {					\
    printf("  Too large for refit\n");					\
    cur_end_status = TRACE_TOO_LARGE;					\
  }									\
									\
  LOCSEG_CHAIN_BEGIN_FIT(p->locseg, list_end);				\
  if (tw->trace_mask != NULL) {						\
    if (Local_Neuroseg_Top_Sample(p->locseg, tw->trace_mask, z_scale) > 0) { \
      cur_end_status = TRACE_NOT_ASSIGNED;				\
    }									\
  }									\
  LOCSEG_CHAIN_END_FIT(p->locseg, list_end);				\
									\
  if (cur_end_status == TRACE_NORMAL) {					\
    double bound_pos[3];						\
    double bound_offset = -p->locseg->seg.r1;				\
    if (list_end == DL_TAIL) {						\
      bound_offset = p->locseg->seg.h + NEUROSEG_R2(&(p->locseg->seg)); \
    }									\
    Local_Neuroseg_Axis_Position(p->locseg, bound_pos, bound_offset);	\
    									\
    if ((bound_pos[0] >= 0.0) && (bound_pos[1] >= 0.0) &&		\
	(bound_pos[2] >= 0.0) && (bound_pos[0] < (double) stack->width) && \
	(bound_pos[1] < (double) stack->height) &&			\
	(bound_pos[2] * z_scale < (double) stack->depth)) {		\
      double pos_step = 1.0 - NEUROSEG_R2(&(p->locseg->seg)) / p->locseg->seg.h; \
      if (pos_step < 0.75) {						\
	pos_step = 0.75;						\
      }									\
      if (0) /*(list_end == DL_HEAD)*/ {				\
	double t = p->locseg->seg.r1 / p->locseg->seg.h;		\
	if (t > 0.25) {							\
	  t = 0.25;							\
	}								\
	pos_step = t - refit_locseg->seg.h / p->locseg->seg.h;		\
      }									\
      LOCSEG_CHAIN_BEGIN_FIT(p->locseg, list_end);			\
      printf("  Extend current end as current_locseg\n");		\
      Next_Local_Neuroseg(p->locseg, refit_locseg, pos_step);		\
      /*Print_Local_Neuroseg(p->locseg);*/				\
      /*LOCSEG_CHAIN_BEGIN_FIT(refit_locseg, list_end);*/		\
      LOCSEG_CHAIN_END_FIT(p->locseg, list_end);			\
      /*printf("before refitting\n");*/					\
      /*Local_Neuroseg_Score_P(refit_locseg, stack, z_scale, &(tr.fs));*/ \
      /*printf("score: %g\n", tr.fs.scores[1]);*/			\
      /*Print_Local_Neuroseg(refit_locseg);*/				\
      printf("  Search the orientation of current_locseg.\n");		\
      Local_Neuroseg_Orientation_Search_B(refit_locseg, stack, z_scale, &ort_fs); \
      if (Neuropos_Reference == NEUROSEG_CENTER) {			\
	printf("  Adjust the position of current_locseg.\n");		\
	Local_Neuroseg_Position_Adjust(refit_locseg, stack, z_scale);	\
      }									\
      /*Local_Neuroseg_Orientation_Adjust(refit_locseg, stack, z_scale);*/ \
      /*Fit_Local_Neuroseg_P(refit_locseg, stack, var_index_o, nvar_o, var_link, z_scale, &(tr.fs)); */	\
      printf("  Fit current_locseg.\n");				\
      /*Fit_Local_Neuroseg_P(refit_locseg, stack, var_index, nvar, var_link, z_scale, &(tr.fs));*/ \
      LOCSEG_CHAIN_FIT(refit_locseg);					\
      if (Local_Neuroseg_Good_Score(refit_locseg, tr.fs.scores[1], tw->min_score) == FALSE) { \
	printf("  The score is too low, adjust its height.\n");		\
	/*Local_Neuroseg_Height_Search_P(refit_locseg, stack, z_scale);*/ \
	Local_Neuroseg_Height_Search_W(p->locseg, stack, z_scale, fit_ws->sws);	\
	/*Local_Neuroseg_Score_P(refit_locseg, stack, z_scale, &(tr.fs));*/ \
	LOCSEG_CHAIN_FIT(refit_locseg);					\
      }									\
      LOCSEG_CHAIN_END_FIT(refit_locseg, list_end);			\
      argv[0] = refit_locseg;						\
      argv[8] = p->locseg;						\
      cur_end_status = tw->test_func(argv);				\
    } else {								\
      printf("  trace out of bound:\n");				\
      cur_end_status = TRACE_SEED_OUT_OF_BOUND;				\
    }									\
  }

#define TRACE_LOCSEG(current_locseg, trace_step, list_end, end_index)	\
  if (current_locseg != NULL) {						\
    cur_end_status = tw->trace_status[end_index]; \
    if (cur_end_status == TRACE_NORMAL) {				\
      prev_locseg = current_locseg;					\
      LOCSEG_CHAIN_BEGIN_FIT(prev_locseg, list_end);			\
      printf("Extend current end as current_locseg.\n");		\
      current_locseg = Next_Local_Neuroseg(prev_locseg, NULL, trace_step); \
      if (Neuropos_Reference == NEUROSEG_CENTER) {			\
        printf("Adjust the position of current_locseg.\n");		\
        Local_Neuroseg_Position_Adjust(current_locseg, stack, z_scale);	\
      }									\
      /*LOCSEG_CHAIN_BEGIN_FIT(current_locseg, list_end);*/		\
      /*Local_Neuroseg_Orientation_Adjust(current_locseg, stack, z_scale);*/ \
      printf("Fit current_locseg.\n");					\
      /*Fit_Local_Neuroseg_P(current_locseg, stack, var_index, nvar, var_link, z_scale, &(tr.fs));*/ \
      LOCSEG_CHAIN_FIT(current_locseg);					\
      /* test height */							\
      if (tr.fs.scores[1] < fit_height_threshold) {			\
        printf("The score is a bit low, reset current_locseg to the extend of current end.\n"); \
        Next_Local_Neuroseg(prev_locseg, current_locseg, trace_step);	\
        printf("Fit current_locseg including height.\n");		\
        /*Fit_Local_Neuroseg_P(current_locseg, stack, var_index_h, nvar_h, var_link, z_scale, &(tr.fs));*/ \
        LOCSEG_CHAIN_FIT_H(current_locseg);				\
        /* current_locseg->seg.h /= 2.0; */				\
        printf("Fit current_locseg again without height change.\n");	\
        /*Fit_Local_Neuroseg_P(current_locseg, stack, var_index, nvar, var_link, z_scale, &(tr.fs));*/ \
        LOCSEG_CHAIN_FIT(current_locseg);				\
        printf("Chop the overlap part of prev_locseg.\n");		\
        prev_locseg->seg.h *= trace_step;				\
      }									\
      /*********/							\
      LOCSEG_CHAIN_END_FIT(prev_locseg, list_end);			\
      LOCSEG_CHAIN_END_FIT(current_locseg, list_end);			\
      argv[0] = current_locseg;						\
      argv[8] = prev_locseg;						\
      ASSERT(tw->test_func != NULL, "NULL validating function");	\
      cur_end_status = tw->test_func(argv);				\
    }									\
    \
    if ((cur_end_status != TRACE_NORMAL) && (tw->refit == TRUE) &&	\
        (cur_end_status != TRACE_HIT_MARK) &&				\
        (cur_end_status != TRACE_OUT_OF_BOUND) &&			\
        (cur_end_status != TRACE_NOT_ASSIGNED) &&			\
        (i > 1)) {							\
      cur_end_status = TRACE_REFIT;					\
    }									\
    \
    if (cur_end_status == TRACE_REFIT) {				\
      printf("Refit required.\n");					\
      LOCSEG_CHAIN_REFIT(current_locseg, list_end, end_index);		\
      printf("current_locseg refit done. label it by refit flag.\n");	\
      Trace_Record_Set_Refit(&tr, 1);					\
      if (nrefit > i + 1) {						\
        printf("too much refit: %d.\n", nrefit);			\
        cur_end_status = TRACE_OVER_REFIT;				\
      }									\
    }									\
    Locseg_Node *current_node = NULL;					\
    switch (cur_end_status) {						\
      case TRACE_NORMAL:	/* good trace */				\
                                                  printf("current_locseg is good, add it with index %d\n", i);	\
      /*Print_Local_Neuroseg(current_locseg);*/				\
      current_node = Make_Locseg_Node(current_locseg, Copy_Trace_Record(&tr)); \
      Locseg_Chain_Add_Node(chain, current_node, list_end);		\
      i++;								\
      current_ends[end_index] = current_node;				\
      hit_count[end_index] = 0;						\
      break;								\
      case TRACE_HIT_MARK:	/*hit traced region*/			\
      printf("current_locseg hits traced or marked region.\n");		\
      if (hit_count[end_index] == -1) {	/* allow multiple hits */				\
        printf("First-time hit, add current_locseg to the chain.\n");	\
        current_node = Make_Locseg_Node(current_locseg, Copy_Trace_Record(&tr)); \
        Locseg_Chain_Add_Node(chain, current_node, list_end);		\
        i++;								\
        current_ends[end_index] = current_node;				\
        hit_count[end_index]++;						\
      } else {								\
        if (hit_count[end_index] <= 0 && tw->add_hit) {				\
          printf("Add current_locseg to the chain.\n");			\
          current_node = Make_Locseg_Node(current_locseg, Copy_Trace_Record(&tr)); \
          Locseg_Chain_Add_Node(chain, current_node, list_end);		\
          i++;								\
          current_ends[end_index] = current_node;			\
        } else {							\
          Delete_Local_Neuroseg(current_locseg);			\
        }								\
        /*Locseg_Chain_Add(chain, current_locseg, NULL, list_end);*/	\
        printf("Stop tracing at this direction.\n");			\
        tw->trace_status[end_index] = cur_end_status;			\
        current_locseg = NULL;						\
      }									\
      break;								\
      case TRACE_OUT_OF_BOUND:	/* out of bound */			\
                                                        printf("Trace out of bound. Add current_locseg to the chain and stop traceing at this direction.\n"); \
      current_node = Make_Locseg_Node(current_locseg, Copy_Trace_Record(&tr)); \
      Locseg_Chain_Add_Node(chain, current_node, list_end);		\
      i++;								\
      current_ends[end_index] = current_node;				\
      tw->trace_status[end_index] = cur_end_status;			\
      current_locseg = NULL;						\
      break;								\
      default:								\
      if (0) { /* for testing */					\
        printf("Last seg added\n");					\
        Print_Local_Neuroseg(current_locseg);				\
        current_node = Make_Locseg_Node(current_locseg, Copy_Trace_Record(&tr)); \
        Locseg_Chain_Add_Node(chain, current_node, list_end);		\
      } else {								\
        printf("Stop tracing at this direction.\n");			\
        Delete_Local_Neuroseg(current_locseg);				\
      }									\
      current_locseg = NULL;						\
      tw->trace_status[end_index] = cur_end_status;			\
    }									\
  }						   

void Trace_Locseg(const Stack *stack, double z_scale, Locseg_Chain *chain, 
		 Trace_Workspace *tw)
{
  if ((stack == NULL) || (chain == NULL) || (chain->list == NULL) || 
      (tw == NULL)) {
    TZ_WARN(ERROR_POINTER_NULL);
    return;
  }

  Locseg_Fit_Workspace dfit_ws;
  Locseg_Fit_Workspace dfit_ws_h;

  /* Temporary workspace */
  Locseg_Fit_Workspace *fit_ws = &dfit_ws;
  Locseg_Fit_Workspace *fit_ws_h = &dfit_ws_h;

  if (tw->fit_workspace != NULL) {
    Locseg_Fit_Workspace_Copy(fit_ws, (const Receptor_Fit_Workspace*)tw->fit_workspace);
    Locseg_Fit_Workspace_Copy(fit_ws_h, (const Receptor_Fit_Workspace*)tw->fit_workspace);
  } else {
    fit_ws = New_Locseg_Fit_Workspace();
    fit_ws_h = New_Locseg_Fit_Workspace();
  }

  /* Initialize variable set for normal fitting */
#if 0
  int var_index[LOCAL_NEUROSEG_NPARAM];
  int nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
				     //NEUROSEG_VAR_MASK_HEIGHT |
				     NEUROSEG_VAR_MASK_ORIENTATION |
				     NEUROSEG_VAR_MASK_SCALE/* |
							       NEUROSEG_VAR_MASK_ALPHA*/,
				     NEUROSEG_VAR_MASK_NONE, var_index);
#endif

  /* variable set for height fitting */
  /*
  int var_index_h[LOCAL_NEUROSEG_NPARAM];
  int nvar_h = Local_Neuroseg_Var_Mask_To_Index(//NEUROSEG_VAR_MASK_R |
						NEUROSEG_VAR_MASK_HEIGHT, //|
						//NEUROSEG_VAR_MASK_ORIENTATION |
						//NEUROSEG_VAR_MASK_SCALE,
						NEUROSEG_VAR_MASK_NONE, 
						var_index_h);
  */

  fit_ws_h->nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_HEIGHT, 
				     NEUROSEG_VAR_MASK_NONE, 
				     fit_ws_h->var_index);

  /*
  int var_index_o[LOCAL_NEUROSEG_PLANE_NPARAM];
  int nvar_o = Local_Neuroseg_Var_Mask_To_Index(
						NEUROSEG_VAR_MASK_ORIENTATION,
						NEUROSEG_VAR_MASK_NONE, 
						var_index_o);
  */
  /*
#ifdef _CONE_SEG_
  int *var_link = NULL;
#else             
  int var_link[LOCAL_NEUROSEG_NPARAM];
  int k;
  for (k = 0; k < LOCAL_NEUROSEG_NPARAM; k++) {
    var_link[k] = 0;
  }
  Variable_Set_Add_Link(var_link, 0, 1);  
#endif
  */
  //Local_Neuroseg *locseg = chain->list->data->locseg;

  //int *var_link = NULL;

  /* initialize trace record */
  Trace_Record tr;
  tr.mask = ZERO_BIT_MASK;
  tr.fs.n = 2;
  tr.fs.options[0] = 0;
  tr.fs.options[1] = tw->tscore_option;
  
  fit_ws->sws->fs.options[fit_ws->sws->fs.n++] = tw->tscore_option;
  if (fit_ws->sws != fit_ws_h->sws) {
    fit_ws_h->sws->fs.options[fit_ws_h->sws->fs.n++] = tw->tscore_option;
  }

  Dlist_Direction_e trace_direction = (Dlist_Direction_e)DL_UNDEFINED;

  /* make parameters for validity testing */
  void *argv[15];
  argv[0] = NULL; /* segment */
  argv[1] = chain; /* chain */
  argv[2] = tw;
  argv[3] = &tr;
  argv[4] = &z_scale;
  argv[5] = &(tw->dyvar[0]); /* max radius */
  argv[6] = &(trace_direction);
  argv[7] = &(tw->dyvar[1]); /* min radius */
  argv[8] = NULL; /* previous segment */
  argv[9] = (Stack*) stack;
  argv[10] = NULL; /* reserved */

  /* Score threshold for height fit. When the fitting score is lower than the
   * threshold, the segment will adjust its height. */
  double fit_height_threshold = tw->dyvar[2]; 

  Locseg_Node *current_ends[2]; /* two ends: 0 head 1 tail*/
  current_ends[0] = Locseg_Chain_Head(chain);
  current_ends[1] = Locseg_Chain_Tail(chain);

  Local_Neuroseg *forward_locseg = NULL;
  Local_Neuroseg *backward_locseg = NULL;

  if ((tw->trace_status[1] == TRACE_NORMAL) || 
      (tw->trace_status[1] == TRACE_REFIT)) { /* tail, go forward */
    forward_locseg = current_ends[1]->locseg;
  }

  if ((tw->trace_status[0] == TRACE_NORMAL) ||
      (tw->trace_status[0] == TRACE_REFIT)) { /* head, go backward */
    backward_locseg = current_ends[0]->locseg;
  }

  if (backward_locseg == forward_locseg) {
    Trace_Record_Set_Direction(current_ends[0]->tr, DL_BOTHDIR);
  }

  int nrefit = 0;
  if (tw->fit_first == TRUE) {
    if (tw->trace_status[1] == TRACE_NORMAL) {
      printf("Fit the forward seed segment\n");
      //Fit_Local_Neuroseg_P(forward_locseg, stack, var_index, nvar, var_link, z_scale, &(tr.fs));
      LOCSEG_CHAIN_FIT(forward_locseg);

      argv[0] = forward_locseg;
      trace_direction = DL_FORWARD;
      tw->trace_status[1] = tw->test_func(argv);
    }
    
    if (tw->trace_status[0] == TRACE_NORMAL) {
      printf("Fit the backward seed segment.\n");
      Flip_Local_Neuroseg(backward_locseg);
      //Fit_Local_Neuroseg_P(backward_locseg, stack, var_index, nvar, var_link, z_scale, &(tr.fs));
      LOCSEG_CHAIN_FIT(backward_locseg);

      Flip_Local_Neuroseg(backward_locseg);
      argv[0] = backward_locseg;
      trace_direction = DL_BACKWARD;
      tw->trace_status[0] = tw->test_func(argv);
    }
  }

  argv[1] = chain;
  Neuroseg_Fit_Score ort_fs;
  ort_fs.n = 1;		
  ort_fs.options[0] = STACK_FIT_CORRCOEF;

  double default_step = tw->trace_step;
  double step[2];
  step[0] = default_step;
  step[1] = default_step;
  
  int hit_count[2];
  hit_count[0] = 0;
  hit_count[1] = 0;

  /* Test if the ends hit the mask or regions out of bound.
   * This step is skipped if there is only one segment in the chain.
   * Such a case will be handled in future versions. */
  int chain_length = Locseg_Chain_Length(chain);
  if (chain_length > 1) {
    double pos[3];
    if ((tw->trace_status[0] == TRACE_NORMAL) || 
        (tw->trace_status[0] == TRACE_REFIT)) {
      Local_Neuroseg_Bottom(backward_locseg, pos);
      if (Trace_Workspace_Point_In_Bound_Z(tw, pos, z_scale)
        == FALSE) {
        tw->trace_status[0] = TRACE_OUT_OF_BOUND;
      } else if (Trace_Workspace_Mask_Value_Z(tw, pos, z_scale) > 0) {
        tw->trace_status[0] = TRACE_HIT_MARK;
      }
    }

    if ((tw->trace_status[1] == TRACE_NORMAL) || 
        (tw->trace_status[1] == TRACE_REFIT)) {
      Local_Neuroseg_Top(forward_locseg, pos);
      if (Trace_Workspace_Point_In_Bound_Z(tw, pos, z_scale)
          == FALSE) {
        tw->trace_status[1] = TRACE_OUT_OF_BOUND;
      } else if (Trace_Workspace_Mask_Value_Z(tw, pos, z_scale) > 0) {
        tw->trace_status[1] = TRACE_HIT_MARK;
      }
    }
    
    /*
    argv[0] = backward_locseg;
    int status = tw->test_func(argv);
    if ((status == TRACE_HIT_MARK) || (status == TRACE_OUT_OF_BOUND)) {
      tw->trace_status[0] = status;
    }

    argv[0] = forward_locseg;
    status = tw->test_func(argv);
    if ((status == TRACE_HIT_MARK) || (status == TRACE_OUT_OF_BOUND)) {
      tw->trace_status[1] = status;
    }
    */
  }

  /* The status of the current end. Using this with other conditions to
     determine the tracing status.*/
  int cur_end_status = TRACE_NORMAL;
  Local_Neuroseg *prev_locseg = NULL;
  int i = chain_length;

#define TRACING_BACKWARD_POSSIBLE \
  ((tw->trace_status[0] == TRACE_NORMAL) || (tw->trace_status[0] == TRACE_REFIT))

#define TRACING_FORWARD_POSSIBLE \
  ((tw->trace_status[1] == TRACE_NORMAL) || (tw->trace_status[1] == TRACE_REFIT))

  while ((i < tw->length) && 
      (TRACING_BACKWARD_POSSIBLE || TRACING_FORWARD_POSSIBLE)) {
    //printf("segment %d\n", i);

    /* Backward tracing */
    if (TRACING_BACKWARD_POSSIBLE) {
      printf("Trace backward...\n");
      Reset_Trace_Record(&tr);
      Trace_Record_Set_Direction(&tr, DL_BACKWARD);
      Trace_Record_Set_Fix_Point(&tr, 1.0);
      TRACE_LOCSEG(backward_locseg, step[0], DL_HEAD, 0);
    }

    /* Forward tracing */
    if (TRACING_FORWARD_POSSIBLE) {
      printf("Trace forward...\n");
      Reset_Trace_Record(&tr);
      Trace_Record_Set_Direction(&tr, DL_FORWARD);
      Trace_Record_Set_Fix_Point(&tr, 0.0);
      TRACE_LOCSEG(forward_locseg, step[1], DL_TAIL, 1);
    }
  }

  if (tw->tune_end == TRUE) {
    if (Locseg_Chain_Length(chain) >= 2) {
      Local_Neuroseg *locseg = NULL;
      if ((tw->trace_status[0] != TRACE_HIT_MARK) && 
          (tw->trace_status[0] != TRACE_NOT_ASSIGNED)) {
        locseg = Locseg_Chain_Head_Seg(chain);
        Flip_Local_Neuroseg(locseg);
        //Local_Neuroseg_Height_Search_P(locseg, stack, z_scale);
        Local_Neuroseg_Height_Search_W(locseg, stack, z_scale, fit_ws->sws);
        Flip_Local_Neuroseg(locseg);
      }

      if ((tw->trace_status[1] != TRACE_HIT_MARK) && 
          (tw->trace_status[1] != TRACE_NOT_ASSIGNED)) {
        locseg = Locseg_Chain_Tail_Seg(chain);
        //Local_Neuroseg_Height_Search_P(locseg, stack, z_scale);
        Local_Neuroseg_Height_Search_W(locseg, stack, z_scale, fit_ws->sws);
      }

      Locseg_Chain_Remove_Overlap_Ends(chain);
    }
  }

  if (tw->refit == TRUE) {
    printf("refit times: %d\n", nrefit);
  }

  fit_ws->sws->fs.n--;

  if (tw->fit_workspace == NULL) {
    Kill_Locseg_Fit_Workspace(fit_ws);
    Kill_Locseg_Fit_Workspace(fit_ws_h);
  }
}

void Locseg_Chain_Trace_Np(const Stack *stack, double z_scale, 
			   Locseg_Chain *chain, Trace_Workspace *tw)
{
  Local_Neuroseg *locseg = NULL; 
  double hr = 0.0;
  switch (tw->trace_direction) {
  case DL_FORWARD:
    locseg = Locseg_Chain_Tail_Seg(chain);
    hr = 1.0;
    break;
  case DL_BACKWARD:
    locseg = Locseg_Chain_Head_Seg(chain);
    hr = 0.0;
    break;
  case DL_BOTHDIR:
    tw->trace_direction = DL_FORWARD;
    Locseg_Chain_Trace_Np(stack, z_scale, chain, tw);
    tw->trace_direction = DL_BACKWARD;
    Locseg_Chain_Trace_Np(stack, z_scale, chain, tw);
    tw->trace_direction = DL_BOTHDIR;
    return;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  Local_Neuroseg_Plane *locnp = 
    Local_Neuroseg_To_Plane(locseg, locseg->seg.h * hr, NULL);


  Locnp_Chain *locnp_chain = 
    Locnp_Chain_Trace_Init(stack, z_scale, locnp, NULL);

  Trace_Locnp(stack, 1.0, locnp_chain, tw);

  int end = DL_TAIL;
  if (tw->trace_direction == DL_BACKWARD) {
    end = DL_HEAD;
  }
   
  Print_Locnp_Chain(locnp_chain);
  if (end == DL_HEAD) {
    if (tw->trace_status[0] == TRACE_HIT_MARK) {
      Locseg_Chain_Append_Locnp(chain, locnp_chain, (Dlist_End_e)end);
    }
  }

  if (end == DL_TAIL) {
    if (tw->trace_status[1] == TRACE_HIT_MARK) {
      Locseg_Chain_Append_Locnp(chain, locnp_chain, (Dlist_End_e)end);
    }
  }

  Kill_Locnp_Chain(locnp_chain);
}

/*
static double 
locseg_chain_dist_upper_bound(Locseg_Chain *chain, double z_scale,
			      Local_Neuroseg *head,
			      Local_Neuroseg *tail)
{
  double source1[3];
  Local_Neuroseg_Center(head, source1);
  double source2[3];
  Local_Neuroseg_Center(tail, source2);

  double target[3];
  double dist;
  double min_dist;
  
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg2 = New_Local_Neuroseg();

  Local_Neuroseg_Copy(locseg2, Locseg_Chain_Next_Seg(chain));

  Local_Neuroseg_Scale_Z(locseg2, z_scale);

  Local_Neuroseg_Center(locseg2, target);
  min_dist = dmin2(Geo3d_Dist(source1[0], source1[1], source1[2], 
			      target[0], target[1], target[2]),
		   Geo3d_Dist(source2[0], source2[1], source2[2], 
			      target[0], target[1], target[2]));

  Local_Neuroseg *locseg = NULL;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Copy(locseg2, locseg);
    Local_Neuroseg_Scale_Z(locseg2, z_scale);

    Local_Neuroseg_Center(locseg2, target);
    dist = dmin2(Geo3d_Dist(source1[0], source1[1], source1[2], 
			    target[0], target[1], target[2]),
		 Geo3d_Dist(source2[0], source2[1], source2[2], 
			    target[0], target[1], target[2]));

    if (dist < min_dist) {
      min_dist = dist;
    }
  }

  Delete_Local_Neuroseg(locseg2);

  return min_dist;
}
*/

static double 
locseg_chain_dist_upper_bound(Locseg_Chain *chain, double z_scale,
			      Local_Neuroseg *testseg)
{
  double source[3];
  Local_Neuroseg_Center(testseg, source);

  double target[3];
  double dist;
  double min_dist;
  
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg2 = New_Local_Neuroseg();

  Local_Neuroseg_Copy(locseg2, Locseg_Chain_Next_Seg(chain));

  Local_Neuroseg_Scale_Z(locseg2, z_scale);

  Local_Neuroseg_Center(locseg2, target);
  min_dist = Geo3d_Dist(source[0], source[1], source[2], 
			target[0], target[1], target[2]);

  Local_Neuroseg *locseg = NULL;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Copy(locseg2, locseg);
    Local_Neuroseg_Scale_Z(locseg2, z_scale);

    Local_Neuroseg_Center(locseg2, target);
    dist = Geo3d_Dist(source[0], source[1], source[2], 
		      target[0], target[1], target[2]);

    if (dist < min_dist) {
      min_dist = dist;
    }
  }

  Delete_Local_Neuroseg(locseg2);

  return min_dist;
}

BOOL Locseg_Chain_Connection_Test(Locseg_Chain *chain1, Locseg_Chain *chain2,
				  const Stack *stack, double z_scale, 
				  Neurocomp_Conn *conn, 
				  Connection_Test_Workspace *ctw)
{
  TZ_ASSERT(ctw != NULL, "Null workspace");

  /* No connection if either of the chains is empty. */
  if (Locseg_Chain_Is_Empty(chain1) || Locseg_Chain_Is_Empty(chain2)) {
    conn->mode = NEUROCOMP_CONN_NONE;
    return FALSE;
  }

  /* Initialize resolution */
  double res[3] = {1.0, 1.0, 1.0};
  darraycpy(res, ctw->resolution, 0, 3);

  double xz_ratio = 1.0;
  
  if (res[0] != res[2]) {
    xz_ratio = res[0] / res[2];
  }

  /* Get head and tail of the hook */
  Local_Neuroseg *shead = Locseg_Chain_Head_Seg(chain1);
  Local_Neuroseg *stail = Locseg_Chain_Tail_Seg(chain1);

  /* alloc <head> */
  Local_Neuroseg *head = Copy_Local_Neuroseg(shead);

  /* alloc <tail> */
  Local_Neuroseg *tail = Copy_Local_Neuroseg(stail);

  /* Adjust head and tail */
  Flip_Local_Neuroseg(tail);

  if (Locseg_Chain_Length(chain1) >= 1) {
    head->seg.h = 2.0;
    tail->seg.h = 2.0;
  }

  Local_Neuroseg_Scale_Z(head, xz_ratio);
  Local_Neuroseg_Scale_Z(tail, xz_ratio);
  
  /* get upper bound */
  int index = 0;

  Geo3d_Ball range1, range2;
  Local_Neuroseg *locseg;

  double mindist = 0.0;

  if (ctw->hook_spot == 0) {
    mindist = locseg_chain_dist_upper_bound(chain2, xz_ratio, head);
  }

  if (ctw->hook_spot == 1) {
    mindist = locseg_chain_dist_upper_bound(chain2, xz_ratio, tail);
  }
  
  if (ctw->hook_spot == -1) {
    mindist = dmin2(locseg_chain_dist_upper_bound(chain2, xz_ratio, head),
		    locseg_chain_dist_upper_bound(chain2, xz_ratio, tail));
  }

  Locseg_Chain_Iterator_Start(chain2, DL_HEAD);

  //Local_Neuroseg *source_seg = NULL;
  //Local_Neuroseg *target_seg = NULL;

  double tmp_pos[3];

  conn->mode = NEUROCOMP_CONN_HL;
  Local_Neuroseg *locseg2 = New_Local_Neuroseg();

  /* Calculate the distance from the hook end(s) to the loop chain surface. */
  while ((locseg = Locseg_Chain_Next_Seg(chain2)) != NULL) {
    Local_Neuroseg_Copy(locseg2, locseg);
    Local_Neuroseg_Scale_Z(locseg2, xz_ratio);    
    Local_Neuroseg_Ball_Bound(locseg2, &(range1));

    /* if the head is required to be tested */
    if ((ctw->hook_spot == 0) || (ctw->hook_spot == -1)){
      Local_Neuroseg_Ball_Bound(head, &(range2));
      /* Ignore it if the minimal possible distance is not less than 
       * the current minial distnace */
      if (Geo3d_Dist(range1.center[0], range1.center[1], range1.center[2], 
		     range2.center[0], range2.center[1], range2.center[2])
	  - range1.r - range2.r < mindist) {	
	double dist = Local_Neuroseg_Dist2(head, locseg2, tmp_pos);
	BOOL update = FALSE;
	if (dist < mindist) {
	  mindist = dist;
	  conn->pdist = Local_Neuroseg_Planar_Dist_L(head, locseg2);
	  update = TRUE;
	} else if (dist == mindist) {
	  double pdist = Local_Neuroseg_Planar_Dist_L(head, locseg2);
	  if (conn->pdist > pdist) {	    
	    conn->pdist = pdist;
	    update = TRUE;
	  }
	}

	if (update) {
	  conn->info[0] = 0;
	  conn->info[1] = index;
	  conn->pos[0] = tmp_pos[0];
	  conn->pos[1] = tmp_pos[1];
	  conn->pos[2] = tmp_pos[2];
	  //source_seg = shead;
	  //target_seg = locseg;
	}
      }
    }

    /* if the tail is required to be tested */
    if ((ctw->hook_spot == 1) || (ctw->hook_spot == -1)){
      Local_Neuroseg_Ball_Bound(tail, &(range2));
      /* Ignore it if the minimal possible distance is not less than 
       * the current minial distnace */      
      if (Geo3d_Dist(range1.center[0], range1.center[1], range1.center[2], 
		     range2.center[0], range2.center[1], range2.center[2])
	  - range1.r - range2.r < mindist) {
	double dist = Local_Neuroseg_Dist2(tail, locseg2, tmp_pos);
	if (dist < mindist) {
	  mindist = dist;
	  conn->info[0] = 1;
	  conn->info[1] = index;
	  conn->pos[0] = tmp_pos[0];
	  conn->pos[1] = tmp_pos[1];
	  conn->pos[2] = tmp_pos[2];
	  conn->pdist = Local_Neuroseg_Planar_Dist_L(tail, locseg2);
	  //source_seg = stail;
	  //target_seg = locseg;
	} else if (dist == mindist) {
	  double pdist = Local_Neuroseg_Planar_Dist_L(tail, locseg2);
	  if (conn->pdist > pdist) {
	    conn->info[0] = 1;
	    conn->info[1] = index;
	    conn->pos[0] = tmp_pos[0];
	    conn->pos[1] = tmp_pos[1];
	    conn->pos[2] = tmp_pos[2];
	    conn->pdist = pdist;
	    //source_seg = stail;
	    //target_seg = locseg;
	  }
	}
      }
    }

    index++;
  }

  Delete_Local_Neuroseg(locseg2);

  /* scale position back to the chain space */
  conn->pos[2] *= xz_ratio;

  /* free <head> */
  Delete_Local_Neuroseg(head);  
  /* free <tail> */
  Delete_Local_Neuroseg(tail);  

  /*
  double dist_thre = 20.0;
  double big_euc = 15.0;
  double big_planar = 10.0;

  double dist_thre = ctw->dist_thre;
  double big_euc = ctw->big_euc;
  double big_planar = ctw->big_planar;
  */

  conn->sdist = mindist;

#ifdef _DEBUG_2
  if ((Geo3d_Dist(conn->pos[0], conn->pos[1], conn->pos[2], 370, 163, 20)
       < 5.0) && (conn->sdist < 5.0)) {
    printf("Debug here.\n");
  }
#endif

  if ((conn->sdist > ctw->dist_thre) /*||
				  ((conn->sdist > big_euc) && (conn->pdist > big_planar))*/) {
    conn->mode = NEUROCOMP_CONN_NONE;
    conn->cost = 10.0;
    return FALSE;
  } else {
    Local_Neuroseg *locseg1 = NULL;
    double test_pos[3];

    if (conn->info[0] == 0) {
      locseg1 = Locseg_Chain_Head_Seg(chain1);
      Local_Neuroseg_Top(locseg1, test_pos);
    } else {
      locseg1 = Locseg_Chain_Tail_Seg(chain1);
      Local_Neuroseg_Bottom(locseg1, test_pos);
    }

    Geo3d_Orientation_Normal(locseg1->seg.theta, locseg1->seg.psi,
			     conn->ort, conn->ort + 1, conn->ort + 2);

    /*    
    if ((conn->sdist > 5.0) && (conn->pos != NULL)) {
      double test_vec[3];
      if (conn->info[0] == 0) {
	test_vec[0] = -conn->pos[0] + test_pos[0];
	test_vec[1] = -conn->pos[1] + test_pos[1];
	test_vec[2] = -conn->pos[2] + test_pos[2];
      } else {
	test_vec[0] = conn->pos[0] - test_pos[0];
	test_vec[1] = conn->pos[1] - test_pos[1];
	test_vec[2] = conn->pos[2] - test_pos[2];	
      }
      if (Geo3d_Dot_Product(test_vec[0], test_vec[1], test_vec[2],
			    conn->ort[0], conn->ort[1], conn->ort[2]) < 0.0) {
	conn->mode = NEUROCOMP_CONN_NONE;
	conn->cost = 10.0;
	return FALSE;	
      }
    }
        */

    //locseg2 = Locseg_Chain_Peek_Seg_At(chain2, conn->info[1]);
    
    //int n;
    //double *feat = Locseg_Conn_Feature(locseg1, locseg2, NULL, res, NULL, &n);

    /*
    conn->cost = 
      1.0 / (1.0 + exp(0.546599 * feat[0] + -1.45673 * feat[7] +
		       6.48994 * fabs((feat[4] - feat[5])/(feat[4] + feat[5])) +
		       0.433958 * fabs(feat[6])));     
    */
    /*
    conn->cost = 
      1.0 / (1.0 + exp(0.592935 * feat[0] + -1.27852 * feat[7] +
		       0.766847 * fabs(feat[6])));
    */

    if ((conn->sdist > 2.0) && (ctw->sp_test == TRUE) && (stack != NULL)){
      double gdist = 0.0;
      Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
      sgw->conn = 26;
      sgw->wf = Stack_Voxel_Weight_S;
      sgw->resolution[0] = ctw->resolution[0];
      sgw->resolution[1] = ctw->resolution[1];
      sgw->resolution[2] = ctw->resolution[2];
      //sgw->argv[3] = Locseg_Chain_Min_Seg_Signal(chain1, stack, z_scale);
      //		   Locseg_Chain_Min_Seg_Signal(chain2, stack, z_scale));

      sgw->signal_mask = ctw->mask;
      Int_Arraylist *path = Locseg_Chain_Shortest_Path(chain1, chain2,
						       stack, z_scale, sgw);
      sgw->signal_mask = NULL;

      if (path != NULL) {
	gdist = sgw->value;
	double path_dist = path_length(path, stack->width, stack->height,
	    ctw->resolution[2] / ctw->resolution[1]);
	UNUSED_PARAMETER(path_dist);
	int coord[3];
	int hit_index;

	if (path->length >= 5) {
	  int k;
	  int dark_count = 0;
	  int bright_count = 0;
	  hit_index = 0;
	  for (k = 0; k < path->length; k++) {
	    if (hit_index < 3) {
	      Stack_Util_Coord(path->array[k], stack->width, 
			       stack->height, coord, coord + 1,
			       coord + 2);
	      if (conn->info[0] == 0) {
		hit_index = Locseg_Chain_Hit_Test(chain1, DL_FORWARD,
						  coord[0], coord[1], coord[2]);
	      } else {
		hit_index = Locseg_Chain_Hit_Test(chain1, DL_BACKWARD,
						  coord[0], coord[1], coord[2]);
	      }
	    }
	    
	    BOOL count = TRUE;
	    if (ctw->mask != NULL) {
	      if (Stack_Array_Value(ctw->mask, path->array[k]) < 0.5) {
		count = FALSE;
	      }
	    }
	    
	    if (count) {
	      if ((Stack_Array_Value(stack, path->array[k]) < 
		   sgw->argv[3] - sgw->argv[4]) || 
		  (Stack_Array_Value(stack, path->array[k]) == 0)){
		dark_count++;
	      } else {
		bright_count++;
	      }
	    }
	  }
	  
	  /* ~w~ */
	  if (((dark_count >= 2) && (dark_count >= bright_count)) || 
	      (dark_count >= 5) || (hit_index >= 3) /*|| 
	      (path_dist > dist_thre * 3.0)*/) {
	    conn->mode = NEUROCOMP_CONN_NONE;
	  } else {
	    if (dark_count + bright_count >= 2) {
	      int prev_pos[3];
	      int pos[3];
	      Stack_Util_Coord(path->array[path->length-2], stack->width, 
			       stack->height, prev_pos, prev_pos + 1,
			       prev_pos + 2);
	      int count = 0;
	      int i;
	      conn->ort[0] = 0.0;
	      conn->ort[1] = 0.0;
	      conn->ort[2] = 0.0;
	      for (i = path->length - 3; i >= 0; i--) {
		Stack_Util_Coord(path->array[i], stack->width, 
				 stack->height, pos, pos + 1, pos + 2);
		conn->ort[0] += prev_pos[0] - pos[0];
		conn->ort[1] += prev_pos[1] - pos[1];
		conn->ort[2] += prev_pos[2] - pos[2];
		prev_pos[0] = pos[0];
		prev_pos[1] = pos[1];
		prev_pos[2] = pos[2];
		count++;
		if (count >= 5) {
		  break;
		}
	      }

	      Coordinate_3d_Unitize(conn->ort);
	    }
	  }
	}

	if (sgw->resolution != NULL) {
	  gdist /= sgw->resolution[0];
	}
      } else {
	conn->mode = NEUROCOMP_CONN_NONE;
	gdist = 0.0;
      }
      
      Kill_Stack_Graph_Workspace(sgw);

      /*
      Path_Test_Workspace ptw;
      default_path_test_workspace(&ptw);
      ptw.z_res = ctw->resolution[2] / ctw->resolution[0];

      if (!is_path_valid(stack, path->array, path->length, &ptw)) {
	conn->mode = NEUROCOMP_CONN_NONE;
      }
      */

#ifdef _DEBUG_
      printf("%g, %g, %g\n", conn->pos[0], conn->pos[1], conn->pos[2]);
      printf("%g, %g, %g\n", conn->ort[0], conn->ort[1], conn->ort[2]);
#endif
      
      if (path != NULL) {
	Kill_Int_Arraylist(path);
      }
      conn->cost = 1.0 / (1.0 + exp(-(conn->sdist + gdist)/100.0));
      //conn->cost = 1.0 / (1.0 + exp(-conn->sdist));
    } else {
      conn->cost = 1.0 / (1.0 + exp(-conn->sdist / 100.0));
    }

    //conn->cost = 1.0 / (1.0 + exp(-feat[7]));

    /*
    conn->cost = 
      1.0 / (1.0 + exp(0.546474 * feat[0] -1.45891 * feat[7] +
		       6.50761 * fabs((feat[4] - feat[5])/(feat[4] + feat[5])) +
		       0.433692 * fabs(feat[6])));
    */

    /*
    conn->cost = exp(-0.559668 * feat[0] + 1.55163 * feat[7] +
		     -7.01974 * fabs((feat[4] - feat[5])/(feat[4] + feat[5])) +
		     -0.470092 * fabs(feat[6]));
    */

    /*
    conn->cost = exp(-1.02326 * feat[0] + 1.0818 * feat[7] +
		     0.0695292 * fabs(feat[0] * feat[7]));
    */
    /*
    conn->cost = exp(-0.7429 * feat[0] + 0.405817 * feat[7] +
		     0.173818 * feat[0] * feat[7]);
    */

    /*
    conn->cost = exp(0.0189185 * feat[0] + 0.880418 * feat[7] +
		     -6.06658 * fabs((feat[4] - feat[5])/(feat[4] + feat[5])) +
		     -0.42791 * fabs(feat[6]));
    */


    //conn->cost = exp(-0.276669 * feat[0] + 0.621916 * feat[7]);

    /*
    conn->cost = exp(-0.605281 * feat[0] + 1.37511 * feat[7] +
		     -0.838392 * fabs(feat[6]));
    */

    //conn->cost = feat[7];
    
    /*
    conn->cost = exp(-(0.6053 * feat[0] -1.3751 * feat[7] + 
		       0.8384 * fabs(feat[6])));
    */

    /*
    conn->cost = exp(-(1.1812 * feat[0] - 2.5651 * feat[7] + 
		       0.6728 * fabs(feat[6])));
    */

    //conn->cost = feat[0] + feat[6];

    //conn->cost = feat[2] + feat[7];

    //conn->cost = feat[3] + feat[10];
    //free(feat);

    /*
    conn->cost = mindist * res[0] + conn->cost;
    */
  }

  if (conn->mode == NEUROCOMP_CONN_NONE) {
    return FALSE;
  } else {
    if (ctw->interpolate == TRUE) {
      if (conn->mode == NEUROCOMP_CONN_HL) {
	int index = Locseg_Chain_Interpolate_L(chain2, conn->pos, conn->ort, 
					       conn->pos);
	if (index >= 0) {
	  conn->info[1] = index;
	} else {
	  if (conn->info[1] == 0) {
	    conn->mode = NEUROCOMP_CONN_LINK;
	    conn->info[1] = 0;
	  } else if (conn->info[1] == Locseg_Chain_Length(chain2) - 1) {
	    conn->mode = NEUROCOMP_CONN_LINK;
	    conn->info[1] = 1;	  
	  }
	}
      }
    }
  }

  return TRUE;
}


int Locseg_Chain_Hit_Test(Locseg_Chain *chain, Dlist_Direction_e d,
			  double x, double y, double z)
{
  int i = 1;
  Local_Neuroseg *locseg = NULL;
  Locseg_Node_Dlist *old_iter = NULL;

  if (d == DL_FORWARD) {
    old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    locseg = Locseg_Chain_Next_Seg(chain);
  } else {
    old_iter = Locseg_Chain_Iterator_Start(chain, DL_TAIL);
    locseg = Locseg_Chain_Prev_Seg(chain);
  }

  while (locseg != NULL) {
    if (Local_Neuroseg_Hit_Test(locseg, x, y, z) == TRUE) {
      return i;
    }

    if (d == DL_FORWARD) {
      locseg = Locseg_Chain_Next_Seg(chain);
    } else {
      locseg = Locseg_Chain_Prev_Seg(chain);
    }
    i++;
  }

  chain->iterator = old_iter;

  return 0;
}

int Locseg_Chain_Hit_Test_Seg(Locseg_Chain *chain, const Local_Neuroseg *locseg)
{
  int i = 0;
  double pos[3];
  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);
  double z;

  Local_Neuroseg *chain_locseg = NULL;
  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);

  int hit = 0;

  for (chain_locseg = Locseg_Chain_Next_Seg(chain); chain_locseg != NULL;
       chain_locseg = Locseg_Chain_Next_Seg(chain)) {
    i++;
    
    double center1[3];
    double center2[3];
    Local_Neuroseg_Center(chain_locseg, center1);
    Local_Neuroseg_Center(locseg, center2);

    double cdist_sqr = Geo3d_Dist_Sqr(center1[0], center1[1], center1[2],
				      center2[0], center2[1], center2[2]);

    double max_dist = 
      dmax3(chain_locseg->seg.h / 2.0, chain_locseg->seg.r1, 
	    NEUROSEG_R2(&(chain_locseg->seg))) +
	    dmax3(locseg->seg.h / 2.0, locseg->seg.r1, 
		  NEUROSEG_R2(&(locseg->seg)));
    if (cdist_sqr > max_dist * max_dist) { /* too far away */
      continue;
    }
       
    for (z = 0.0; z <= locseg->seg.h; z += 2.0) {
      Neuroseg_Axis_Offset(&(locseg->seg), z, pos);
      pos[0] += bottom[0];
      pos[1] += bottom[1];
      pos[2] += bottom[2];
      
      if (Local_Neuroseg_Hit_Test(chain_locseg, pos[0], pos[1], pos[2])
	  == TRUE) {
	/* test if the next segment is closer */
	if (Locseg_Chain_Peek_Next(chain) != NULL) { 
	  double center3[3];
	  Local_Neuroseg_Center(Locseg_Chain_Peek_Next(chain)->locseg, 
				center3);
	  double cdist_sqr_next = 
	    Geo3d_Dist_Sqr(center3[0], center3[1], center3[2],
			   center2[0], center2[1], center2[2]);
	  if (cdist_sqr_next < cdist_sqr) {
	    i++;
	  }
	}
	hit = i;
	break;
      }
    }
  }

  chain->iterator = old_iter;

  return hit;
}


void Locseg_Chain_Append_Locnp(Locseg_Chain *chain1, Locnp_Chain *chain2,
			       Dlist_End_e end)
{ 
  Locnp_Chain_Iterator_Start(chain2, (Dlist_End_e)(-end));
  Local_Neuroseg_Plane *locnp = NULL;
  BOOL add = FALSE;
  int offset = 0;
  Local_Neuroseg *locseg = NULL;
  double center[3];
  if (end == DL_TAIL) {
    while ((locnp = Locnp_Chain_Next_Seg(chain2)) != NULL) {
      locseg = Local_Neuroseg_From_Plane(NULL, locnp);
      Local_Neuroseg_Center(locseg, center);
      locseg->seg.h = 10;
      Set_Neuroseg_Position(locseg, center, NEUROSEG_CENTER);

      add = FALSE;
      offset++;
      if (offset > 5) {
	offset = 0;
	add = TRUE;
      }
      if (add == TRUE) {
	Locseg_Chain_Add(chain1, locseg, NULL, end);
      }
    }
    if (add == FALSE) {
      Locseg_Chain_Add(chain1, locseg, NULL, end);
    }
  } else {
    while ((locnp = Locnp_Chain_Prev_Seg(chain2)) != NULL) {
      locseg = Local_Neuroseg_From_Plane(NULL, locnp);
      Local_Neuroseg_Center(locseg, center);
      locseg->seg.h = 1.0;
      Set_Neuroseg_Position(locseg, center, NEUROSEG_CENTER);

      add = FALSE;
      offset++;
      if (offset > 5) {
	offset = 0;
	add = TRUE;
      }
      if (add == TRUE) {
	Locseg_Chain_Add(chain1, locseg, NULL, end);
      }
    }
    if (add == FALSE) {
      Locseg_Chain_Add(chain1, locseg, NULL, end);
    }
  }
}

void Locseg_Chain_Draw_Stack(Locseg_Chain *chain, Stack *stack, 
			     const Stack_Draw_Workspace *ws)
{
  Locseg_Chain_Label(chain, stack, ws->z_scale);
}

void Locseg_Chain_Label(Locseg_Chain *chain, Stack *stack, double z_scale)
{
  int color = 2;
  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = NULL;

  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
#if _DEBUG_2
    printf("Labeling: ");
    Print_Local_Neuroseg(locseg);
#endif
    if (Locseg_Chain_Peek(chain) == NULL) {
      color = 2;
    }

    Local_Neuroseg_Label(locseg, stack, color, z_scale);
    if (color == 0) {
      color = 1;
    } else {
      color = 0;
    }
  }
  
  chain->iterator = old_iter;
}

void Locseg_Chain_Label_E(Stack *stack, Locseg_Chain *chain, double z_scale,
			  int begin, int end, double ratio, double diff)
{
  int color = 0;
  Local_Neuroseg *tmp_seg = New_Local_Neuroseg();

  int i = 0;

  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = NULL;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    if ((i >= begin) && (i <= end)) {
      Local_Neuroseg_Copy(tmp_seg, locseg);
      Neuroseg_Change_Thickness(&(tmp_seg->seg), ratio, diff);
      Local_Neuroseg_Label(tmp_seg, stack, color, z_scale);
      if (color == 0) {
	color = 1;
      } else {
	color = 0;
      }	 
    }
    i++;
  }

  Delete_Local_Neuroseg(tmp_seg);

  chain->iterator = old_iter;
}

void Locseg_Chain_Label_G(Locseg_Chain *chain, Stack *stack, double z_scale,
			  int begin, int end, double ratio, double diff, 
			  int flag, int value)
{
  Local_Neuroseg *tmp_seg = New_Local_Neuroseg();
  Local_Neuroseg *locseg = NULL;

  int i = 0;

  if (end < 0) {
    end += Locseg_Chain_Length(chain);
  }

  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    if ((i >= begin) && (i <= end)) {
      Local_Neuroseg_Copy(tmp_seg, locseg);
      Neuroseg_Swell(&(tmp_seg->seg), ratio, diff, 3.0);
      Local_Neuroseg_Label_G(tmp_seg, stack, flag, value, z_scale);
    }
    i++;
  }

  Delete_Local_Neuroseg(tmp_seg);

  chain->iterator = old_iter;
}

BOOL Locseg_Chain_Has_Stack_Value(Locseg_Chain *chain, Stack *stack,
				  double z_scale, double value)
{
  Local_Neuroseg *locseg = NULL;

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    if (Local_Neuroseg_Has_Stack_Value(locseg, stack, z_scale, value)) {
      return TRUE;
    }
  }

  return FALSE;
}

void Locseg_Chain_Label_W(Locseg_Chain *chain, Stack *stack, double z_scale,
			  int begin, int end, Locseg_Label_Workspace *ws)
{
  if ((ws->option == 6) || (ws->option == 7)) {
    /* Initialize the buffer mask */
    if (ws->buffer_mask != NULL) {
      if (Stack_Voxel_Number(ws->buffer_mask) != Stack_Voxel_Number(stack)) {
        Kill_Stack(ws->buffer_mask);
        ws->buffer_mask = NULL;
      } else {
        ws->buffer_mask->width = stack->width;
        ws->buffer_mask->height = stack->height;
        ws->buffer_mask->depth = stack->depth;
      }
    }
    if (ws->buffer_mask == NULL) {
      ws->buffer_mask = Make_Stack(GREY, stack->width, stack->height,
          stack->depth);
    }

    Zero_Stack(ws->buffer_mask);

    Locseg_Label_Workspace tmp_ws = *ws;
    tmp_ws.option = 1;
    tmp_ws.value = 1;
    tmp_ws.flag = 0;
    int i;
    for (i = 0; i < 6; i++) {
      tmp_ws.range[i] = -1;
    }
    Locseg_Chain_Label_W(chain, ws->buffer_mask, z_scale, begin, end,
        &tmp_ws);
    for (i = 0; i < 3; i++) {
      if (tmp_ws.range[i] < 0) {
        tmp_ws.range[i] = 0;
      }
    }

    if ((tmp_ws.range[3] < 0) || (tmp_ws.range[3] >= stack->width)) {
      tmp_ws.range[3] = stack->width - 1;
    }
    if ((tmp_ws.range[4] < 0) || (tmp_ws.range[4] >= stack->height)) {
      tmp_ws.range[4] = stack->height - 1;
    }
    if ((tmp_ws.range[5] < 0) || (tmp_ws.range[5] >= stack->depth)) {
      tmp_ws.range[5] = stack->depth - 1;
    }

    if (ws->option == 6) {
      Stack_Add_R(stack, ws->buffer_mask, tmp_ws.range, stack);
    } else if (ws->option == 7) {
      Stack_Sub_R(stack, ws->buffer_mask, tmp_ws.range, stack);
    }

    /* clean buffer mask */
    tmp_ws.option = 1;
    tmp_ws.value = 0;
    tmp_ws.flag = 1;
    Locseg_Chain_Label_W(chain, ws->buffer_mask, z_scale, begin, end,
        &tmp_ws);
  } else {
    int i = 0;

    Local_Neuroseg *locseg = NULL;
    Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
      if ((i >= begin) && (i <= end)) {
        Local_Neuroseg_Label_W(locseg, stack, z_scale, ws);
      }
      i++;
    }

    chain->iterator = old_iter;
  }
}

void Locseg_Chain_Erase(Locseg_Chain *chain, Stack *stack, double z_scale)
{
  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = NULL;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Label(locseg, stack, -1, z_scale);
  }

  chain->iterator = old_iter;
}

void Locseg_Chain_Erase_E(Stack *stack, Locseg_Chain *chain, double z_scale,
			  int begin, int end, double ratio, double diff)
{
  Local_Neuroseg *tmp_seg = New_Local_Neuroseg();

  int i = 0;

  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = NULL;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    if ((i >= begin) && (i <= end)) {
      Local_Neuroseg_Copy(tmp_seg, locseg);
      Neuroseg_Change_Thickness(&(tmp_seg->seg), ratio, diff);
      Local_Neuroseg_Label(tmp_seg, stack, -1, z_scale);
    }
    i++;
  }

  Delete_Local_Neuroseg(tmp_seg);

  chain->iterator = old_iter;
}


Locseg_Chain *Locseg_Chain_From_Locnp_Chain(Locnp_Chain *chain)
{
  Locseg_Chain *locseg_chain = New_Locseg_Chain();

  Locnp_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg_Plane *locnp = NULL;
  while ((locnp = Locnp_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg *locseg = Local_Neuroseg_From_Plane(NULL, locnp);
    Locseg_Chain_Add(locseg_chain, locseg, NULL, DL_TAIL);
  }

  return locseg_chain;
}

int Locseg_Chain_Remove_Overlap_Ends(Locseg_Chain *chain)
{
  int nremove = 0;

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);

  Local_Neuroseg *head = Locseg_Chain_Next_Seg(chain);
  Local_Neuroseg *runner_up = Locseg_Chain_Next_Seg(chain);

  if (runner_up == NULL) {
    return nremove;
  }

  double top[3];
  double bottom[3];
  Local_Neuroseg_Top(head, top);
  Local_Neuroseg_Bottom(head, bottom);
 
  if ((Local_Neuroseg_Hit_Test(runner_up, top[0], top[1], top[2]) == TRUE) &&
      (Local_Neuroseg_Hit_Test(runner_up, bottom[0], bottom[1], bottom[2]) 
       == TRUE)) {
    Locseg_Chain_Remove_End(chain, DL_HEAD);
    nremove++;
  }

  if (Locseg_Chain_Length(chain) < 2) {
    return nremove;
  }

  Locseg_Chain_Iterator_Start(chain, DL_TAIL);

  Local_Neuroseg *tail = Locseg_Chain_Prev_Seg(chain);

  Local_Neuroseg_Top(tail, top);
  Local_Neuroseg_Bottom(tail, bottom);
 
  runner_up = Locseg_Chain_Prev_Seg(chain);

  if ((Local_Neuroseg_Hit_Test(runner_up, top[0], top[1], top[2]) == TRUE) &&
      (Local_Neuroseg_Hit_Test(runner_up, bottom[0], bottom[1], bottom[2]) 
       == TRUE)) {
    Locseg_Chain_Remove_End(chain, DL_TAIL);
    nremove++;
  }
  
  return nremove;
}

void Locseg_Chain_Remove_Turn_Ends(Locseg_Chain *chain, double max_angle)
{
  if (Locseg_Chain_Length(chain) < 2) {
    return;
  }

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);

  Local_Neuroseg *head = Locseg_Chain_Next_Seg(chain);
  Local_Neuroseg *runner_up = Locseg_Chain_Next_Seg(chain);
  
  double angle = Neuroseg_Angle_Between(&(head->seg), &(runner_up->seg));
  if (angle > TZ_PI) {
    angle = TZ_2PI - angle;
  }

  if (angle > max_angle) {
    Locseg_Chain_Remove_End(chain, DL_HEAD);
  }

  if (Locseg_Chain_Length(chain) < 2) {
    return;
  }

  Locseg_Chain_Iterator_Start(chain, DL_TAIL);

  Local_Neuroseg *tail = Locseg_Chain_Prev_Seg(chain);
  runner_up = Locseg_Chain_Prev_Seg(chain);
  
  angle = Neuroseg_Angle_Between(&(tail->seg), &(runner_up->seg));
  if (angle > TZ_PI) {
    angle = TZ_2PI - angle;
  }

  if (angle > max_angle) {
    Locseg_Chain_Remove_End(chain, DL_TAIL);
  }

}

/*
void Set_Locseg_Trace_Record(Local_Neuroseg *locseg, const Trace_Record *tr)
{
  if (locseg->info != NULL) { 	
    free(locseg->info);
  }
  locseg->info = Copy_Trace_Record(tr);
}
*/


int Locseg_Chain_Knot_Number(Locseg_Chain *chain) 
{
  /* alloc <ka> */
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  int length = Locseg_Chain_Knot_Array_Length(ka);
  /* free <ka> */
  Kill_Locseg_Chain_Knot_Array(ka);

  return length;
}

#define LOCSEG_CHAIN_TO_KNOT_ARRAY_EPS 1e-3

Locseg_Chain_Knot_Array* 
Locseg_Chain_To_Knot_Array(Locseg_Chain *chain, Locseg_Chain_Knot_Array *ka)
{
  int length = Locseg_Chain_Length(chain);
  if (length == 0) {
    return NULL;
  }


  if (ka == NULL) {
    ka = New_Locseg_Chain_Knot_Array();
  } else {
    Clean_Locseg_Chain_Knot_Array(ka);
  }

  ka->chain = chain;

  Locseg_Chain_Knot_Array_Append(ka, Make_Locseg_Chain_Knot(0, 0.0));

  if (length == 1) {
    //Locseg_Chain_Knot_Array_Append(ka, Make_Locseg_Chain_Knot(0, 0.5));
    Locseg_Chain_Knot_Array_Append(ka, Make_Locseg_Chain_Knot(0, 1.0));
    return ka;
  }

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Node *node = NULL;
  int index = 0;
  Locseg_Node *prev_node = NULL;
  Locseg_Node *last_node = NULL;
  double prev_pos[3], pos[3];
  
  while ((node = Locseg_Chain_Next(chain)) != NULL) {
    double offset = -1.0;
    if (Trace_Record_Direction(node->tr) != DL_BOTHDIR) { /* not a seed */
      Dlist_Direction_e trace_direction = DL_FORWARD;

      if (node->tr != NULL) {
	offset = Trace_Record_Fix_Point(node->tr);
	trace_direction = Trace_Record_Direction(node->tr);
      }

      if (offset < 0.0) {
	switch (trace_direction) {
	case DL_FORWARD:
	  offset = 0.0;
	  if (prev_node != NULL) {
	    if (Trace_Record_Direction(prev_node->tr) == DL_BACKWARD) {
	      offset = -1.0;
	    }
	  }
	  break;
	case DL_BACKWARD:
	  offset = 1.0;
	  break;
	default:
	  offset = 0.0;
	  break;
	}
      }
    }

    if (offset >= 0.0) {
      Local_Neuroseg_Axis_Coord_N(node->locseg, offset, pos);
      double dist = 10.0;
      if (last_node != NULL) {
	Local_Neuroseg_Axis_Coord_N(last_node->locseg, 
				    Locseg_Chain_Knot_Array_Last(ka)->offset,
				    prev_pos);
	dist = Geo3d_Dist(prev_pos[0], prev_pos[1], prev_pos[2],
			  pos[0], pos[1], pos[2]);
      }
      
      if (dist > LOCSEG_CHAIN_TO_KNOT_ARRAY_EPS) {
	Locseg_Chain_Knot_Array_Append_U(ka, 
					 Make_Locseg_Chain_Knot(index, 
								offset));
	last_node = node;
      }
    }
    /*
  } else {
    Locseg_Node *next_node = Locseg_Chain_Peek(chain);
    if (prev_node == NULL) {
      if (next_node != NULL) {
	int cmp = Local_Neuroseg_Tangent_Compare(next_node->locseg,
						 node->locseg);
	if ((cmp == -11) || (cmp == -12)) {
	  Flip_Local_Neuroseg(node->locseg);
	}
      }
    } else if (next_node == NULL) {
      int cmp = Local_Neuroseg_Tangent_Compare(node->locseg,
					       prev_node->locseg);
      if ((cmp == -11) || (cmp == -12)) {
	Flip_Local_Neuroseg(node->locseg);
      }
    }
  }
    */
    
    prev_node = node;
    index++;
  }

  Locseg_Chain_Knot_Array_Append_U(ka, 
				   Make_Locseg_Chain_Knot(length - 1, 1.0));

  return ka;
}

Geo3d_Circle* Locseg_Chain_To_Geo3d_Circle_Array_Z(Locseg_Chain *chain,
						   double z_scale,
						   Geo3d_Circle *circle, 
						   int *n)
{
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  
  circle = Locseg_Chain_Knot_Array_To_Circle_Z(ka, z_scale, circle);
  *n = Locseg_Chain_Knot_Array_Length(ka);

  Kill_Locseg_Chain_Knot_Array(ka);

  return circle;
}

Geo3d_Circle* Locseg_Chain_To_Geo3d_Circle_Array(Locseg_Chain *chain,
						 Geo3d_Circle *circle, int *n)
{
  return Locseg_Chain_To_Geo3d_Circle_Array_Z(chain, 1.0, circle, n);
}

Locne_Chain* Locseg_Chain_To_Locne_Chain(Locseg_Chain *chain)
{
  Local_Neuroseg *locseg = NULL;
  
  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);

  locseg = Locseg_Chain_Next_Seg(chain);

  Local_Neuroseg_Ellipse *locne =  
    Local_Neuroseg_To_Ellipse_T(locseg, 0.0, NULL);

  Trace_Record *tr = New_Trace_Record();
  
  int index[2];
  index[0] = 0;
  tr->index = index[0];
  tr->fs.scores[0] = 0.0;

  int cur_index = index[0];
  double cur_t = 0.0;

  Locne_Chain *locne_chain = New_Locne_Chain();
  Locne_Chain_Add(locne_chain, locne, tr, DL_TAIL);
  
  Local_Neuroseg *next_seg = Locseg_Chain_Next_Seg(chain);
  index[1] = 1;

  double d, t1, t2;
  while (next_seg != NULL) {
    d = Local_Neuroseg_Intersect(locseg, next_seg, &t1, &t2);

    if (t1 < -0.01) {
      TZ_WARN(ERROR_DATA_VALUE);
      t1 = 0.0;
    }

    if (t1 > 1.01) {
      TZ_WARN(ERROR_DATA_VALUE);
      t1 = 1.0;
    }

    if (t2 < -0.01) {
      TZ_WARN(ERROR_DATA_VALUE);
      t2 = 0.0;
    }

    if (t2 > 1.01) {
      TZ_WARN(ERROR_DATA_VALUE);
      t2 = 1.0;
    }

    /* to avoid going back */
    if ((index[0] > cur_index) || 
	((index[0] == cur_index) && (t1 > cur_t + 0.01))) {
      locne = Local_Neuroseg_To_Ellipse_T(locseg, t1, NULL);

      tr->index = index[0];
      tr->fs.scores[0] = t1;
#ifdef _DEBUG_2
      printf("%d, %g, %g\n", index[0], t1, t2);
#endif
      Locne_Chain_Add(locne_chain, locne, tr, DL_TAIL);

      cur_index = index[1];
      cur_t = t2;
    }

    if (d > 0.1) {
      locne = Local_Neuroseg_To_Ellipse_T(next_seg, t2, NULL);
      tr->index = index[1];
      tr->fs.scores[0] = t2;
#ifdef _DEBUG_2
      printf("%d, %g\n", index[1], t2);
#endif
      Locne_Chain_Add(locne_chain, locne, tr, DL_TAIL);

      cur_index = index[1];
      cur_t = t2;
    }
    
    index[0] = index[1];
    locseg = next_seg;
    next_seg = Locseg_Chain_Next_Seg(chain);
    index[1]++;
  }
  
  tr->index = index[0];
  tr->fs.scores[0] = 1.0;
#ifdef _DEBUG_2
    printf("%d, %g\n", index[0], 1.0);
#endif
  locne = Local_Neuroseg_To_Ellipse_T(locseg, 1.0, NULL);  
  Locne_Chain_Add(locne_chain, locne, tr, DL_TAIL);

  chain->iterator = old_iter;

  return locne_chain;
}

Geo3d_Ellipse* 
Locseg_Chain_To_Ellipse_Array_Z(Locseg_Chain *chain, double z_scale,
				Geo3d_Ellipse *locne, int *n)
{
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  locne = Locseg_Chain_Knot_Array_To_Ellipse_Z(ka, z_scale, locne);
  *n = Locseg_Chain_Knot_Array_Length(ka);

  Kill_Locseg_Chain_Knot_Array(ka);

  return locne;

#if 0
  *n = Locseg_Chain_Length(chain);

  if (*n == 0) {
    return NULL;
  }
  
  *n += 2;

  if (locne == NULL) {
    locne = Make_Local_Neuroseg_Ellipse_Array(*n);
  }

  Local_Neuroseg_Ellipse *locne_head = locne;
  Local_Neuroseg *locseg = NULL;
  
  Local_Neuroseg locseg2;

  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);

  locseg = Locseg_Chain_Peek_Seg(chain);
  Local_Neuroseg_Copy(&locseg2, locseg);  
  Local_Neuroseg_Scale_Z(&locseg2, z_scale);

  Local_Neuroseg_To_Ellipse(&locseg2, locne, NEUROSEG_BOTTOM);
  locne++;
  
  do {
    locseg = Locseg_Chain_Next_Seg(chain);
    Local_Neuroseg_Copy(&locseg2, locseg);  
    Local_Neuroseg_Scale_Z(&locseg2, z_scale);

    Local_Neuroseg_To_Ellipse(&locseg2, locne, NEUROSEG_CENTER);
    locne++;
  } while (Locseg_Chain_Peek(chain) != NULL);
  
  Local_Neuroseg_Copy(&locseg2, locseg);  
  Local_Neuroseg_Scale_Z(&locseg2, z_scale);
  Local_Neuroseg_To_Ellipse(&locseg2, locne, NEUROSEG_TOP);
  
  chain->iterator = old_iter;

  return locne_head;
#endif
}

void Locseg_Chain_Vrml_Fprint_Z(FILE *fp, Locseg_Chain *chain, double z_scale,
				const Vrml_Material *material, int indent)
{
  int ncircle;

  Geo3d_Ellipse *circle =  
    Locseg_Chain_To_Ellipse_Array_Z(chain, z_scale, NULL, &ncircle);
  int nsample = 20;
  coordinate_3d_t *pts = Geo3d_Ellipse_Array_Sampling(circle, ncircle,
						      nsample, NULL);

  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  if (material == NULL) {
    Vrml_Material *material2 = New_Vrml_Material();
    Bitmask_Set_Bit(DIFFUSE_COLOR, FALSE, &(material2->default_mask));
    Vrml_Material_Fprint(fp, material2, indent + 4);
    Delete_Vrml_Material(material2);
  } else {
    Vrml_Material_Fprint(fp, material, indent + 4);
  }
  Vrml_Node_End_Fprint(fp, "Appearance", indent + 2);
  Vrml_Pm_Rectangle_Fprint(fp, (const coordinate_3d_t *) pts, nsample, 
			   ncircle, indent + 2);
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  
  free(circle);
  free(pts);
}

void Locseg_Chain_Vrml_Fprint(FILE *fp, Locseg_Chain *chain,
			      const Vrml_Material *material, int indent)
{
  Locseg_Chain_Vrml_Fprint_Z(fp, chain, 1.0, material, indent);
}

void Locseg_Chain_To_Vrml_File(Locseg_Chain *chain, const char *file_path)
{
  FILE *fp = fopen(file_path, "w");
  Vrml_Head_Fprint(fp, "V2.0", "utf8");
  Locseg_Chain_Vrml_Fprint(fp, chain, NULL, 0);
  fclose(fp);
}

Graph* Locseg_Chain_Graph(Locseg_Chain *chain, int n, Int_Arraylist *hit_spots)
{
  Graph *graph = Make_Graph(n, n, FALSE);

  int i, j;
  
  int hit = 0;

  PROGRESS_BEGIN("Build neuron chain graph");
  for (i = 0; i < n; i++) {
    PROGRESS_STATUS(i * 100 / n);

    Local_Neuroseg *head_seg = Locseg_Chain_Head_Seg(chain + i);
    Local_Neuroseg *tail_seg = Locseg_Chain_Tail_Seg(chain + i);
    for (j = 0; j < n; j++) {
      if (i != j) {
	hit = Locseg_Chain_Hit_Test_Seg(&(chain[j]), head_seg);

	if ((hit > 1) && (hit < Locseg_Chain_Length(&(chain[j])))) {
	  Graph_Add_Edge(graph, i, j);
	  if (hit_spots != NULL) {
	    Int_Arraylist_Add(hit_spots, -hit);
	  }
	  //continue;
	} else {	 
	  hit = Locseg_Chain_Hit_Test_Seg(&(chain[j]), tail_seg);
	  if ((hit > 1) && (hit < Locseg_Chain_Length(&(chain[j])))) {
	    Graph_Add_Edge(graph, i, j);
	    if (hit_spots != NULL) {
	      Int_Arraylist_Add(hit_spots, hit);
	    }
	  }
	}
      }
    }
    PROGRESS_REFRESH
  }

  PROGRESS_END("done");

  return graph;
}

int Locseg_Chain_End_Points(Locseg_Chain *chain, int n, coordinate_3d_t pts[])
{
  ASSERT(n != 0, "0 points");

  Locseg_Node_Dlist *old_iter = chain->iterator;

  int i;
  if (n > 0) {
    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    Local_Neuroseg *head = Locseg_Chain_Next_Seg(chain);
    
    Local_Neuroseg_Bottom(head, pts[0]);
          
    for (i = 1; i < n; i++) {
      Local_Neuroseg_Center(head, pts[i]);
      head = Locseg_Chain_Next_Seg(chain);
      if (head == NULL) {
	break;
      }
    }
  } else {
    Locseg_Chain_Iterator_Start(chain, DL_TAIL);
    Local_Neuroseg *tail = Locseg_Chain_Prev_Seg(chain);

    n = -n;
    
    Local_Neuroseg_Top(tail, pts[0]);
    
    for (i = 1; i < n; i++) {
      Local_Neuroseg_Center(tail, pts[i]);      
      tail = Locseg_Chain_Prev_Seg(chain);
      if (tail == NULL) {
	break;
      }
    }
  }

  chain->iterator = old_iter;

  return i;
}

int Locseg_Chain_End_Points_Z(Locseg_Chain *chain, int n, coordinate_3d_t pts[],
			      double z_scale)
{
  ASSERT(n != 0, "0 points");

  Locseg_Node_Dlist *old_iter = chain->iterator;

  int i;
  if (n > 0) {
    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    Local_Neuroseg *head = Locseg_Chain_Next_Seg(chain);
    
    Local_Neuroseg_Bottom(head, pts[0]);
    pts[0][2] /= z_scale;
          
    for (i = 1; i < n; i++) {
      Local_Neuroseg_Center(head, pts[i]);
      pts[i][2] /= z_scale;
      head = Locseg_Chain_Next_Seg(chain);
      if (head == NULL) {
	break;
      }
    }
  } else {
    Locseg_Chain_Iterator_Start(chain, DL_TAIL);
    Local_Neuroseg *tail = Locseg_Chain_Prev_Seg(chain);

    n = -n;
    
    Local_Neuroseg_Top(tail, pts[0]);
    pts[0][2] /= z_scale;
    
    for (i = 1; i < n; i++) {
      Local_Neuroseg_Center(tail, pts[i]);      
      pts[i][2] /= z_scale;
      tail = Locseg_Chain_Prev_Seg(chain);
      if (tail == NULL) {
	break;
      }
    }
  }

  chain->iterator = old_iter;

  return i;
}

void Locseg_Chain_Hit_Stack(Locseg_Chain *chain, Stack *region, int hit[])
{  
  coordinate_3d_t pos[5];
  int i;
  int x, y, z;
  hit[0] = 0;
  hit[1] = 0;

  int nsample = imin2(Locseg_Chain_Length(chain) / 2,  5);
  if (nsample == 0) {
    nsample = 1;
  }

  int n = Locseg_Chain_End_Points(chain, nsample, pos);
  for (i = 0; i < n; i++) {
    x = (int) (pos[i][0] + 0.5);
    y = (int) (pos[i][1] + 0.5);
    z = (int) (pos[i][2] + 0.5);
    if ((x < 0) || (y < 0) || (z < 0) || (x >= region->width) ||
	(y >= region->height) || (z >= region->depth)) {
      hit[0] = -1;
    } else {
      int hit_value = (int) Get_Stack_Pixel(region, x, y, z, 0);
      if (hit_value > 0) {
	hit[0] = imax2(hit[0], hit_value);
      }
    }
  }

  n = Locseg_Chain_End_Points(chain, -nsample, pos); 

  for (i = 0; i < n; i++) {
    x = (int) (pos[i][0] + 0.5);
    y = (int) (pos[i][1] + 0.5);
    z = (int) (pos[i][2] + 0.5);
    if ((x < 0) || (y < 0) || (z < 0) || (x >= region->width) ||
	(y >= region->height) || (z >= region->depth)) {
      hit[1] = -1;
    } else {
      int hit_value = (int) Get_Stack_Pixel(region, x, y, z, 0);
      if ((hit_value > 0) && (hit_value != hit[0])) {
	hit[1] = imax2(hit[1], hit_value);
      }
    }
  }

#ifdef _DEBUG_2
  if ((hit[0] == 20) || (hit[1] == 20)) {
    printf("testing point\n");
  }
#endif
}

Neuron_Structure* Locseg_Chain_Neurostruct(Locseg_Chain *chain, int n, 
					   const Stack *stack, double z_scale,
					   void *ws)
{
  Neuron_Structure *ns = Make_Neuron_Structure(n);

  int i, j;

  PROGRESS_BEGIN("Build neuron chain graph");
  for (i = 0; i < n; i++) {
    PROGRESS_STATUS(i * 100 / n);

    for (j = 0; j < n; j++) {
      if (i != j) {
	Neurocomp_Conn conn;
	conn.mode = NEUROCOMP_CONN_HL;

	if (Locseg_Chain_Connection_Test(chain + i, chain + j, stack, 
    z_scale, &conn, (Connection_Test_Workspace*)ws) == TRUE) {
	  Neurocomp_Conn_Translate_Mode(Locseg_Chain_Length(chain + j), 
					&conn);
	  Neuron_Structure_Add_Conn(ns, i, j, &conn);
	}
      }
    }
    Set_Neuron_Component(Neuron_Structure_Get_Component(ns, i),
			 NEUROCOMP_TYPE_LOCSEG_CHAIN, chain + i);

    PROGRESS_REFRESH
  }


  PROGRESS_END("done");

  return ns;
}

void Locseg_Chain_Reach_Conn(Locseg_Chain *hook, Locseg_Chain *loop,
			     const Neurocomp_Conn *conn)
{
  if (conn->mode != NEUROCOMP_CONN_HL) {
    return;
  }

  double pt[3];
  if (conn->info[0] == 0) {
    Local_Neuroseg_Bottom(Locseg_Chain_Head_Seg(hook), pt);
  } else {
    Local_Neuroseg_Top(Locseg_Chain_Tail_Seg(hook), pt);
  }

  double line_start[3], line_end[3];
  Local_Neuroseg_Bottom(Locseg_Chain_Peek_Seg_At(loop, 
						 conn->info[1]), line_start);
  Local_Neuroseg_Top(Locseg_Chain_Peek_Seg_At(loop, 
					      conn->info[1]), line_end);
 
  double lambda;
  Geo3d_Point_Lineseg_Dist(pt, line_start, line_end, &lambda);

#ifdef _DEBUG_2
  printf("lambda: %g\n", lambda);
#endif

  double center[3];
  Geo3d_Lineseg_Break(line_start, line_end, lambda, center);

  if (conn->info[0] == 0) {
    Local_Neuroseg_Change_Bottom(Locseg_Chain_Head_Seg(hook), center);
  } else {
    Local_Neuroseg_Change_Top(Locseg_Chain_Tail_Seg(hook), center);
  }
}

int Locseg_Chain_Swc_Fprint_Z(FILE *fp, Locseg_Chain *chain, int type,
			      int start_id, int parent_id, 
			      Dlist_Direction_e direction, double z_scale)
{
  int n;
  Geo3d_Circle *circles = Locseg_Chain_To_Geo3d_Circle_Array_Z(chain, z_scale,
							       NULL, &n);
  int i;                                                                            
  int start;
  int end;
  int step;

  if (direction == DL_FORWARD) {
    start = 0;
    end = n;
    step = 1;
  } else {
    start = n - 1;
    end = -1;
    step = -1;
  }

  for (i = start; i != end; i += step) {
    Swc_Node cell;
    cell.id = start_id + i;
    cell.type = type;
    cell.d = circles[i].radius;
    cell.x = circles[i].center[0];
    cell.y = circles[i].center[1];
    cell.z = circles[i].center[2];
    cell.parent_id = cell.id - step;
    if (i == start) {
      cell.parent_id = parent_id;
    }
    Swc_Node_Fprint(fp, &cell);

    //Print_Geo3d_Circle(circles+i);
  }

  free(circles);

  return start_id + i;
}

int Locseg_Chain_Swc_Fprint_T(FILE *fp, Locseg_Chain *chain, int type,
			      int start_id, int parent_id, 
			      Dlist_Direction_e direction, double z_scale, 
			      const double *offset)
{
  int n;
  Geo3d_Circle *circles = Locseg_Chain_To_Geo3d_Circle_Array_Z(chain, z_scale,
							       NULL, &n);
  int i;                                                                            
  int start;
  int end;
  int step;

  if (direction == DL_FORWARD) {
    start = 0;
    end = n;
    step = 1;
  } else {
    start = n - 1;
    end = -1;
    step = -1;
  }

  for (i = start; i != end; i += step) {
    Swc_Node cell;
    cell.id = start_id + i;
    cell.type = type;
    cell.d = circles[i].radius;
    cell.x = circles[i].center[0];
    cell.y = circles[i].center[1];
    cell.z = circles[i].center[2];
    if (offset != NULL) {
      cell.x += offset[0];
      cell.y += offset[1];
      cell.z += offset[2];
    }
    cell.parent_id = cell.id - step;
    if (i == start) {
      cell.parent_id = parent_id;
    }
    Swc_Node_Fprint(fp, &cell);

    //Print_Geo3d_Circle(circles+i);
  }

  free(circles);

  return n;
}

void Locseg_Chain_To_Swc_File(Locseg_Chain *chain, const char *filepath)
{
  FILE *fp = Guarded_Fopen((char*) filepath, "w", "Locseg_Chain_To_Swc_File");
  Locseg_Chain_Swc_Fprint_Z(fp, chain, 2, 1, -1, DL_FORWARD, 1.0);
  fclose(fp);
}

int Locseg_Chain_Swc_Fprint(FILE *fp, Locseg_Chain *chain, int type,
			     int start_id,
			     int parent_id, Dlist_Direction_e direction)
{
  int n;
  Geo3d_Circle *circles = Locseg_Chain_To_Geo3d_Circle_Array(chain, NULL, &n);
  int i;                                                                            
  int start;
  int end;
  int step;

  if (direction == DL_FORWARD) {
    start = 0;
    end = n;
    step = 1;
  } else {
    start = n - 1;
    end = -1;
    step = -1;
  }

  for (i = start; i != end; i += step) {
    Swc_Node cell;
    cell.id = start_id + i;
    cell.type = type;
    cell.d = circles[i].radius;
    cell.x = circles[i].center[0];
    cell.y = circles[i].center[1];
    cell.z = circles[i].center[2];
    cell.parent_id = cell.id - step;
    if (i == start) {
      cell.parent_id = parent_id;
    }
    Swc_Node_Fprint(fp, &cell);

    //Print_Geo3d_Circle(circles+i);
  }

  free(circles);

  return n;
}

Neuron_Component* Dir_Locseg_Chain_Nc(const char *dir_name, const char *pattern,
				      int *n, int *file_num)
{
  *n = dir_fnum_p(dir_name, pattern);
  if (*n == 0) {
    return NULL;
  }

  Neuron_Component *chain_array =
    (Neuron_Component*) Guarded_Malloc(sizeof(Neuron_Component) * (*n), 
				       "Dir_Neurochain");
  
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *full_path = (char *) Guarded_Malloc(strlen(dir_name) + 100, 
					    "Dir_Neurochain");
  strcpy(full_path, dir_name);  
  strcat(full_path, "/");
	
  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int path_length = strlen(dir_name) + strlen(ent->d_name) + 1;
      if (path_length > strlen(full_path)) {
	full_path = (char *) Guarded_Realloc(full_path, path_length + 1, 
					     "Dir_Neurochain");
      }
      strcpy(full_path + strlen(dir_name) + 1, ent->d_name);
      printf("%d: %s\n", index, full_path);

      Locseg_Chain *chain = Read_Locseg_Chain(full_path);
      if (Locseg_Chain_Length(chain) > 0) {
	if (file_num != NULL) {
	  file_num[index] = String_Last_Integer(ent->d_name);
	}

	Set_Neuron_Component(chain_array + index, NEUROCOMP_TYPE_LOCSEG_CHAIN,
			     Read_Locseg_Chain(full_path));
	index++;
      }
    }
    ent = readdir(dir);
  }
  *n = index;
  free(full_path);
  closedir(dir);

  regfree(&preg);

  return chain_array;
}

Neuron_Structure* 
Locseg_Chain_Comp_Neurostruct(Neuron_Component *comp, int n, 
			      const Stack *stack, double z_scale,
			      void *ws)
{
  Neuron_Structure *ns = New_Neuron_Structure();
  Neuron_Structure_Set_Component_Array(ns, comp, n);

  int i, j;

  Graph_Workspace *gw = New_Graph_Workspace();

  PROGRESS_BEGIN("Build neuron chain graph");

  for (i = 0; i < n; i++) {
    PROGRESS_STATUS(i * 100 / n);

    for (j = 0; j < n; j++) {
      if (i != j) {
	Neurocomp_Conn conn;
	conn.mode = NEUROCOMP_CONN_HL;
	Locseg_Chain *chain_i = NEUROCOMP_LOCSEG_CHAIN(comp+ i);
	Locseg_Chain *chain_j = NEUROCOMP_LOCSEG_CHAIN(comp + j);

	if (Locseg_Chain_Connection_Test(chain_i, chain_j, stack, 
					 z_scale, &conn, (Connection_Test_Workspace*)ws) == TRUE) {
	  Neurocomp_Conn_Translate_Mode(Locseg_Chain_Length(chain_j), 
					&conn);
	  BOOL conn_existed = FALSE;
	  if (i > j) {
	    if (ns->graph->nedge > 0) {
	      int edge_idx = Graph_Edge_Index(j, i, gw);
	      if (edge_idx >= 0) {
		if (conn.mode == NEUROCOMP_CONN_LINK) {
		  if (ns->conn[edge_idx].info[0] == conn.info[1]) {
		    conn_existed = TRUE;
		  }
		} else if (ns->conn[edge_idx].mode == NEUROCOMP_CONN_LINK) {
		  if (ns->conn[edge_idx].info[1] == conn.info[0]) {
		    conn_existed = TRUE;
		  }
		}
		if (conn_existed == TRUE) {
		  if (ns->conn[edge_idx].cost > conn.cost) {
		    /*
		    ns->conn[edge_idx].mode = conn.mode;
		    ns->conn[edge_idx].info[0] = conn.info[0];
		    ns->conn[edge_idx].info[1] = conn.info[1];
		    ns->conn[edge_idx].cost = conn.cost;
		    */
		    Neurocomp_Conn_Copy(ns->conn + edge_idx, &conn);
		    ns->graph->edges[edge_idx][0] = i;
		    ns->graph->edges[edge_idx][1] = j;
		    Graph_Update_Edge_Table(ns->graph, gw);
		  }
		}
	      }
	    }
	  }
	  
	  if (conn_existed == FALSE) {
	    Neuron_Structure_Add_Conn(ns, i, j, &conn);
	    Graph_Expand_Edge_Table(i, j, ns->graph->nedge - 1, gw);
	  }
	}
      }
    }
    PROGRESS_REFRESH
  }

  PROGRESS_END("done");

  Kill_Graph_Workspace(gw);

  return ns;  
}

Neuron_Structure *
Locseg_Chain_Comp_Neurostruct_W(Neuron_Structure *ns, const Stack *stack,
			       double z_scale, Connection_Test_Workspace *ws)
{
  int i, j;

  Graph_Workspace *gw = New_Graph_Workspace();
  if (ns->graph != NULL) {
    if (ns->graph->nedge > 0) {
      Graph_Update_Edge_Table(ns->graph, gw);
    }
  }

  PROGRESS_BEGIN("Build neuron chain graph");
  
  int n = NEURON_STRUCTURE_COMPONENT_NUMBER(ns);

  for (i = 0; i < n; i++) {
    PROGRESS_STATUS(i * 100 / n);

    for (j = 0; j < n; j++) {
      if ((i != j) && (Graph_Edge_Index(i, j, gw) < 0)) {
	Neurocomp_Conn conn;
	conn.mode = NEUROCOMP_CONN_HL;
	Locseg_Chain *chain_i = NEUROCOMP_LOCSEG_CHAIN(ns->comp+ i);
	Locseg_Chain *chain_j = NEUROCOMP_LOCSEG_CHAIN(ns->comp + j);

#ifdef _DEBUG_
	if (i == 16 && j == 0) {
	  printf("debug here.\n");
	}
#endif

	if (Locseg_Chain_Connection_Test(chain_i, chain_j, stack, 
					 z_scale, &conn, ws) == TRUE) {
	  Neurocomp_Conn_Translate_Mode(Locseg_Chain_Length(chain_j), &conn);
	  BOOL conn_existed = FALSE;
	  if (i > j) { /* needs modification */
	    if (ns->graph->nedge > 0) {
	      int edge_idx = Graph_Edge_Index(j, i, gw);
	      if (edge_idx >= 0) {
		if (conn.mode == NEUROCOMP_CONN_LINK) {
		  if (ns->conn[edge_idx].info[0] == conn.info[1]) {
		    conn_existed = TRUE;
		  }
		} else if (ns->conn[edge_idx].mode == NEUROCOMP_CONN_LINK) {
		  if (ns->conn[edge_idx].info[1] == conn.info[0]) {
		    conn_existed = TRUE;
		  }
		}
		if (conn_existed == TRUE) {
		  if (ns->conn[edge_idx].cost > conn.cost) {
		    Neurocomp_Conn_Copy(ns->conn + edge_idx, &conn);
		    ns->graph->edges[edge_idx][0] = i;
		    ns->graph->edges[edge_idx][1] = j;
		    Graph_Update_Edge_Table(ns->graph, gw);
		  }
		}
	      }
	    }
	  }
	  
	  if (conn_existed == FALSE) {
	    Neuron_Structure_Add_Conn(ns, i, j, &conn);
	    Graph_Expand_Edge_Table(i, j, ns->graph->nedge - 1, gw);
	  }
	}
      }
    }
    PROGRESS_REFRESH
  }

  PROGRESS_END("done");

  Kill_Graph_Workspace(gw);

  return ns;    
}

Neuron_Component* 
Locseg_Chain_To_Neuron_Component(Locseg_Chain *chain, int type, 
				 Neuron_Component *nc, int *n)
{
  int i;
  switch (type) {
  case NEUROCOMP_TYPE_LOCSEG_CHAIN:
    *n = 1;
    if (nc == NULL) {
      nc = (Neuron_Component*) 
	Guarded_Malloc(sizeof(Neuron_Component) * (*n),
		       "Locseg_Chain_To_Neuron_Component");
    }
    Set_Neuron_Component(nc, type, chain);
    break;
  case NEUROCOMP_TYPE_LOCAL_NEUROSEG:
    *n = Locseg_Chain_Length(chain);
    if (nc == NULL) {
      nc = (Neuron_Component*) 
	Guarded_Malloc(sizeof(Neuron_Component) * (*n),
		       "Locseg_Chain_To_Neuron_Component");
    }

    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    for (i = 0; i < (*n); i++) {
      Set_Neuron_Component(nc + i, type,
			   Copy_Local_Neuroseg(Locseg_Chain_Next_Seg(chain)));
    }
    break;

  case NEUROCOMP_TYPE_GEO3D_CIRCLE: {
    /* turn the chain into a circle chain */
    /* bottom - center - center - ... - center - center - top*/
    *n = Locseg_Chain_Length(chain);

    if (*n == 0) {
      return NULL;
    }
  
    *n += 2;

    if (nc == NULL) {
      nc = (Neuron_Component*) 
	Guarded_Malloc(sizeof(Neuron_Component) * (*n),
		       "Locseg_Chain_To_Neuron_Component");
    }

    Local_Neuroseg *locseg = NULL;
    i = 0;
    
    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    locseg = Locseg_Chain_Peek_Seg(chain);
    Geo3d_Circle *circle = 
      Local_Neuroseg_To_Circle(locseg, NEUROSEG_BOTTOM, NEUROSEG_CIRCLE_RX, 
			       NULL);
    Set_Neuron_Component(nc + i, type, circle);

    do {
      i++;
      locseg = Locseg_Chain_Next_Seg(chain);
      circle =
	Local_Neuroseg_To_Circle(locseg, NEUROSEG_CENTER, NEUROSEG_CIRCLE_RX,
				 NULL);
      Set_Neuron_Component(nc + i, type, circle);
    } while (Locseg_Chain_Peek(chain) != NULL);
    
    circle = 
      Local_Neuroseg_To_Circle(locseg, NEUROSEG_TOP, NEUROSEG_CIRCLE_RX, NULL);
    i++;
    Set_Neuron_Component(nc + i, type, circle);
                                    }
    break;

    /*
    {
      Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
      *n = Locseg_Chain_Knot_Array_Length(ka);
      if (nc == NULL) {
	nc = (Neuron_Component*) 
	  Guarded_Malloc(sizeof(Neuron_Component) * (*n),
			 "Locseg_Chain_To_Neuron_Component");
      }
      Geo3d_Circle *circle = 
	Locseg_Chain_Knot_Array_To_Circle_Z(ka, 1.0, NULL);
      for (i = 0; i < *n; i++) {
	Set_Neuron_Component(nc + i, type, Copy_Geo3d_Circle(circle + i));
      }
      free(circle);
      ka->chain = NULL;
      Kill_Locseg_Chain_Knot_Array(ka);
    }
    */
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return nc;
}

Neuron_Component* 
Locseg_Chain_To_Neuron_Component_S(Locseg_Chain *chain, int type, 
				   Neuron_Component *nc, int *n,
				   double xy_scale, double z_scale)
{
  if (Locseg_Chain_Is_Empty(chain)) {
    *n = 0;
    return NULL;
  }

  switch (type) {
    case NEUROCOMP_TYPE_GEO3D_CIRCLE:
      /* turn the chain into a circle chain */
      {
        Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
        //*n = Locseg_Chain_Knot_Array_Length(ka) * 3 - 2;
        *n = Locseg_Chain_Knot_Array_Length(ka);

        if (nc == NULL) {
          nc = (Neuron_Component*) 
            Guarded_Malloc(sizeof(Neuron_Component) * (*n),
                "Locseg_Chain_To_Neuron_Component_S");
        }

        Local_Neuroseg *locseg = NULL;

        Locseg_Chain_Iterator_Start(ka->chain, DL_HEAD);
        int index = 0;
        int knot_index = 0;
        int circle_index = 0;

        while ((locseg = Locseg_Chain_Next_Seg(ka->chain)) != NULL) {
          Local_Neuroseg locseg2;
          locseg2 = *locseg;

          Local_Neuroseg_Scale(&locseg2, xy_scale, z_scale);

          Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
          while (knot != NULL) {
            if (knot->id == index) {
              Geo3d_Circle *circle = 
                Local_Neuroseg_To_Circle_T(&locseg2, knot->offset, 
                    NEUROSEG_CIRCLE_RX, NULL);
              Set_Neuron_Component(nc + circle_index, type, circle);
              //circle_index += 3;
              circle_index++;
              knot_index++;
              knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
            } else {
              break;
            }
          }

          index++;
        }

        /*
           int i;
           for (i = 0; i < *n - 1; i += 3) {
           Geo3d_Circle *circle = New_Geo3d_Circle();
           circle->radius = NEUROCOMP_GEO3D_CIRCLE(nc + i)->radius / 1.5 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->radius / 3.0;
           circle->center[0] = NEUROCOMP_GEO3D_CIRCLE(nc + i)->center[0] / 1.5 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->center[0] / 3.0;
           circle->center[1] = NEUROCOMP_GEO3D_CIRCLE(nc + i)->center[1] / 1.5 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->center[1] / 3.0;
           circle->center[2] = NEUROCOMP_GEO3D_CIRCLE(nc + i)->center[2] / 1.5 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->center[2] / 3.0;
           circle->orientation[0] = 
           NEUROCOMP_GEO3D_CIRCLE(nc + i)->orientation[0] / 1.5 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->orientation[0] / 3.0;
           circle->orientation[1] = 
           NEUROCOMP_GEO3D_CIRCLE(nc + i)->orientation[1] / 1.5 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->orientation[1] / 3.0;
           Set_Neuron_Component(nc + i + 1, type, circle);

           circle = New_Geo3d_Circle();
           circle->radius = NEUROCOMP_GEO3D_CIRCLE(nc + i)->radius / 3.0 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->radius / 1.5;
           circle->center[0] = NEUROCOMP_GEO3D_CIRCLE(nc + i)->center[0] / 3.0 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->center[0] / 1.5;
           circle->center[1] = NEUROCOMP_GEO3D_CIRCLE(nc + i)->center[1] / 3.0 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->center[1] / 1.5;
           circle->center[2] = NEUROCOMP_GEO3D_CIRCLE(nc + i)->center[2] / 3.0 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->center[2] / 1.5;
           circle->orientation[0] = 
           NEUROCOMP_GEO3D_CIRCLE(nc + i)->orientation[0] / 3.0 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->orientation[0] / 1.5;
           circle->orientation[1] = 
           NEUROCOMP_GEO3D_CIRCLE(nc + i)->orientation[1] / 3.0 +
           NEUROCOMP_GEO3D_CIRCLE(nc + i + 3)->orientation[1] / 1.5;
           Set_Neuron_Component(nc + i + 2, type, circle);
           }
           */

#ifdef _DEBUG_2
        for (i = 0; i < *n; i ++) {
          Print_Geo3d_Circle(NEUROCOMP_GEO3D_CIRCLE(nc + i));
        }
#endif

        Kill_Locseg_Chain_Knot_Array(ka);
      }

      break;

    default:
      TZ_ERROR(ERROR_DATA_TYPE);
  }

  return nc;
}

Locseg_Chain** Trace_Locseg_S(const Stack *signal, double z_scale, 
			      Local_Neuroseg *locseg, double *scores, 
			      int nseed, Trace_Workspace *tw, int *nchain)
{
  /*
  Locseg_Chain **chain = (Locseg_Chain**) 
    Guarded_Malloc(sizeof(Locseg_Chain*) * nseed, "Trace_Locseg_S");
  */

  Unipointer_List *chain_list = NULL; /* a list to record chains */
  Unipointer_List *chain_head = NULL; /* a positioning pointer */ 
  *nchain = 0;

  Locseg_Score_Workspace sws;
  sws.mask = NULL;

  Locseg_Fit_Workspace *fws = (Locseg_Fit_Workspace*) tw->fit_workspace;

  if (fws != NULL) {
    sws = *(fws->sws);
  } else {
    Default_Locseg_Score_Workspace(&sws);
  }

  /* Calculate seed sores and sort them */
  //Stack_Fit_Score fs;
  sws.fs.n = 1;
  sws.fs.options[0] = STACK_FIT_CORRCOEF;
  
  int i;

  BOOL null_score = (scores == NULL);
  if (null_score) {
    scores = darray_malloc(nseed);
    for (i = 0; i < nseed; i++) {
      //scores[i] = Local_Neuroseg_Score(locseg + i, signal, z_scale, &fs);
      scores[i] = Local_Neuroseg_Score_W(locseg + i, signal, z_scale, &sws);
    }
  }

  /* <alloc> indices */
  int *indices = iarray_malloc(nseed);
  darray_qsort(scores, indices, nseed);
  /*****************************/

  /* find max radius */
  if (tw->dyvar[0] < 0.0) {
    double max_r;
    max_r = Local_Neuroseg_Array_Maxr(locseg, nseed);
    max_r *= 3.0;
    tw->dyvar[0] = dmax2(max_r, 400.0);
  }
  /******************************************/


  /* create canvas */
  if (tw->canvas_updating == TRUE) {
    if (tw->canvas == NULL) {
      tw->canvas = Copy_Stack((Stack*) signal);
      Stretch_Stack_Value_Q(tw->canvas, 0.999);
      Translate_Stack(tw->canvas, COLOR, 1);
    }
  }
  /*****************************/

  /* create the tracing mask if necessary */
  if (tw->trace_mask_updating == TRUE) {
    if (tw->trace_mask == NULL) {
      tw->trace_mask = 
	Make_Stack(GREY16, signal->width, signal->height, signal->depth);
      Zero_Stack(tw->trace_mask);
    }
  }
  /*****************************/

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->signal = (Stack*) signal;

  tic();

  /* trace all seeds */
  for (i = nseed - 1; i >= 0; i--) {
    int index = indices[i];

    //chain[index] = NULL;
    printf("-------------------> seed: %d (%d) / %d\n", i, index, nseed);

#ifdef _DEBUG_2
    double center[3];
    Local_Neuroseg_Center(locseg + index, center);

    if (tw->trace_mask != NULL) {
      if ((center[0] >= 232) && (center[0] <= 248) && 
          (center[1] >= 226) && (center[1] <=244) && 
          (center[2] >=80) && (center[2] <= 95)) {
        printf("debug here\n");
        char tmpfile[100];
        sprintf(tmpfile, "../data/test/test%d.tb", index);
        Locseg_Chain *chain = New_Locseg_Chain();
        Locseg_Chain_Add(chain, locseg + index, NULL, DL_TAIL);
        Write_Locseg_Chain(tmpfile, chain);
      }
    }
#endif

    /* ignore low score seeds */
    if (Local_Neuroseg_Good_Score(locseg + index, scores[i],
          tw->min_score) == FALSE) {
      printf("low score: %g\n", scores[i]);
      continue;
    }
    /******************************/

    /* ignore too thick segments */
    double width = NEUROSEG_RB(&(locseg[index].seg));

    if (width > tw->dyvar[0]) {
      printf("too thick");
      continue;
    }
    /***********************************/

    /* test if the location has been traced */
    /*
       if (tw->trace_mask != NULL) {
       if (Local_Neuroseg_Hit_Mask(locseg + index, 
       tw->trace_mask, z_scale) > 0) {
       printf("traced \n");
       continue;
       }
       }
       */
    tw->trace_status[0] = TRACE_NORMAL;
    tw->trace_status[1] = TRACE_NORMAL;

    if (tw->trace_mask != NULL) {
      double pt[3];
      Local_Neuroseg_Axis_Position(locseg + index, pt, locseg[index].seg.h / 3.0);
      int tmpx = iround(pt[0]);
      int tmpy = iround(pt[1]);
      int tmpz = iround(pt[2] * z_scale);

      if (Stack_Pixel(tw->trace_mask, tmpx, tmpy, tmpz, 0) > 0.0) {
        tw->trace_status[0] = TRACE_HIT_MARK;
      }

      Local_Neuroseg_Axis_Position(locseg + index, pt, 
          locseg[index].seg.h * 2.0 / 3.0);
      tmpx = iround(pt[0]);
      tmpy = iround(pt[1]);
      tmpz = iround(pt[2] * z_scale);

      if (Stack_Pixel(tw->trace_mask, tmpx, tmpy, tmpz, 0) > 0.0) {
        tw->trace_status[1] = TRACE_HIT_MARK;
      }
    }

    if ((tw->trace_status[0] != TRACE_NORMAL) && 
        (tw->trace_status[1] != TRACE_NORMAL)) {
      printf("traced \n");
      continue;
    }

    /***********************************/

    /* The seed passes quals, let's start tracing */
    printf("Start: \n");

    if ((tw->trace_status[0] == TRACE_NORMAL) || 
        (tw->trace_status[1] == TRACE_NORMAL)) {
      /* Initialize a chain */
      Trace_Record *tr = New_Trace_Record();
      tr->mask = ZERO_BIT_MASK;
      Trace_Record_Set_Direction(tr, DL_BOTHDIR);
#if 0
      if (tw->trace_status[0] == TRACE_NORMAL) {
        if (tw->trace_status[1] != TRACE_NORMAL) { /* backward */
          Trace_Record_Set_Direction(tr, DL_BACKWARD);
        } else { /* both direction */
          Trace_Record_Set_Direction(tr, DL_BOTHDIR);
        }
      } else { /* forward */
        Trace_Record_Set_Direction(tr, DL_FORWARD);
      }
#endif
      Locseg_Node *p = 
        Make_Locseg_Node(Copy_Local_Neuroseg(locseg + index), tr);
      Locseg_Chain *locseg_chain = Make_Locseg_Chain(p);

      /* trace! */
      Trace_Locseg(signal, z_scale, locseg_chain, tw);
      /***********************/

      /* post processing */
      Locseg_Chain_Remove_Overlap_Ends(locseg_chain);
      Locseg_Chain_Remove_Turn_Ends(locseg_chain, 1.0);
      /***********************************/

      /**********************************************/
#define TRACE_LOCSEG_S_RECORD_CHAIN(new_chain)				\
      if (chain_list == NULL) {						\
        chain_head = Unipointer_Queue_En(&chain_list, new_chain);	\
      } else {								\
        Unipointer_Queue_En(&chain_list, new_chain);			\
      }									\
      (*nchain)++;							\
      if (strlen(tw->save_path) > 0) {					\
        char chain_file_path[MAX_PATH_LENGTH];				\
        sprintf(chain_file_path, "%s%d.tb", tw->save_path, tw->chain_id); \
        Write_Locseg_Chain(chain_file_path, new_chain);			\
        printf("%s saved\n", chain_file_path);				\
      }									\
      \
      if (tw->trace_mask_updating == TRUE) {				\
        ws->sratio = 1.5;						\
        ws->sdiff = 0.0;						\
        ws->option = 1;							\
        ws->value = tw->chain_id + 1;					\
        ws->flag = 0;							\
        Locseg_Chain_Label_W(new_chain, tw->trace_mask, z_scale,	\
            0, Locseg_Chain_Length(new_chain) - 1, ws); \
      }									\
      tw->chain_id++;							\
      \
      if (tw->canvas_updating == TRUE) {				\
        Locseg_Chain_Label(new_chain, tw->canvas, z_scale);		\
      }
      /**********************************************/

      /* record chains that are long enough */
      if ((Locseg_Chain_Geolen(locseg_chain) >= tw->min_chain_length) ||
          ((tw->trace_status[0] == TRACE_HIT_MARK) || 
           (tw->trace_status[1] == TRACE_HIT_MARK))) {
        int new_chain_number = 1;
        if (tw->break_refit == TRUE) {
          /*
             chain[index] = Locseg_Chain_Refit_Break(locseg_chain, chain[index], 
             &new_chain_number);
             */
          Locseg_Chain *subchain = Locseg_Chain_Refit_Break(locseg_chain, 
              NULL, 
              &new_chain_number);
          Delete_Locseg_Chain(locseg_chain);
          locseg_chain = NULL;
          int k;
          for (k = 0; k < new_chain_number; k++) {
            Locseg_Chain *tmp_chain = New_Locseg_Chain();
            tmp_chain->list = subchain[k].list;
            TRACE_LOCSEG_S_RECORD_CHAIN(tmp_chain);
          }
          free(subchain);
        } else {
          //chain[index] = locseg_chain;
          //Unipointer_Queue_En(&chain_list, locseg_chain);
          TRACE_LOCSEG_S_RECORD_CHAIN(locseg_chain);
        }
      }
    }
  }

  ws->signal = NULL;
  Kill_Locseg_Label_Workspace(ws);

  /* free <indices> */
  free(indices);

  if (null_score) {
    OBJECT_SAFE_FREE(scores, free);
  }

  printf("Time passed: %lld\n", toc());

  Locseg_Chain **chain = (Locseg_Chain**) 
    Guarded_Malloc(sizeof(Locseg_Chain*) * (*nchain), "Trace_Locseg_S");

  for (i = 0; i < *nchain; i++) {
    chain[i] = (Locseg_Chain*) Unipointer_Queue_De(&chain_head);
  }

  return chain;
}

double* Locseg_Chain_Intensity_Profile(Locseg_Chain *chain, const Stack *stack,
				       double z_scale, double *value)
{
  if (Locseg_Chain_Is_Empty(chain)) {
    return NULL;
  }

  int length = Locseg_Chain_Length(chain);
  if (value == NULL) {
    value = darray_malloc(length);
  }

  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  
  Local_Neuroseg *locseg = NULL;
  int i = 0;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    value[i++] = Local_Neuroseg_Average_Signal(locseg, stack, z_scale); 
  }
  
  chain->iterator = old_iter;

  return value;
}

int Locseg_Chain_Refit_Number(Locseg_Chain *chain)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Node *node = NULL;
  int n = 0;
  while((node = Locseg_Chain_Next(chain)) != NULL) {
    if (Trace_Record_Refit(node->tr) > 0) {
      n++;
    }
  }

  return n;
}

Locseg_Chain* Locseg_Chain_Refit_Break(Locseg_Chain *chain, 
				       Locseg_Chain *chain_array, int *n)
{
  *n = Locseg_Chain_Refit_Number(chain) + 1;
  if (chain_array == NULL) {
    chain_array = (Locseg_Chain*) 
      Guarded_Malloc(sizeof(Locseg_Chain) * ((*n) + 1), 
		     "Locseg_Chain_Refit_Break");
  }
  
  int i;
  for (i = 0; i <= *n; i++) {
    chain_array[i].list = NULL;
  }

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Node *node = NULL;
  int index = 0;
  int go_next = TRUE;

  chain_array[0].list = chain->iterator;

  Locseg_Chain_Next(chain);

  while ((node = Locseg_Chain_Peek(chain)) != NULL) {
    if (Trace_Record_Refit(node->tr) > 0) {
      if (Trace_Record_Direction(node->tr) == DL_BACKWARD) {
	Locseg_Chain_Next(chain);
	go_next = FALSE;
      }

      /* if the current start is not NULL and is not used by previous chain */
      if ((chain->iterator != NULL) && 
	  (chain->iterator != chain_array[index].list)) {
	index++;
	chain_array[index].list = chain->iterator;
	chain->iterator->prev->next = NULL;
	chain_array[index].list->prev = NULL;
      }
    }
    
    if (go_next == TRUE) {
      Locseg_Chain_Next(chain);
    } else {
      go_next = TRUE;
    }
  }

  *n = index + 1;

  return chain_array;
}

void Locseg_Chain_Scale_Z(Locseg_Chain *chain, double z_scale)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = NULL;

  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Scale_Z(locseg, z_scale);
  }
}

void Locseg_Chain_Scale_XY(Locseg_Chain *chain, double xy_scale)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = NULL;

  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Scale_XY(locseg, xy_scale);
  }
}

double Locseg_Chain_Connection_Test_P(Locseg_Chain *chain1,
				      Locseg_Chain *chain2,
				      Neurocomp_Conn *conn)
{
  Local_Neuroseg *head = Copy_Local_Neuroseg(Locseg_Chain_Head_Seg(chain1));
  Local_Neuroseg *tail = Copy_Local_Neuroseg(Locseg_Chain_Tail_Seg(chain1));

  double scale = 1.0;
  double offset = NEUROSEG_DEFAULT_H * 1.5;
  Local_Neuroseg_Stretch(head, scale, offset, -1);
  Local_Neuroseg_Stretch(tail, scale, offset, 1);
  
  Locseg_Chain_Iterator_Start(chain2, DL_HEAD);
  
  double mindist = Infinity;

  Local_Neuroseg *locseg;

  int index = 0;
  conn->mode = NEUROCOMP_CONN_HL;

  while ((locseg = Locseg_Chain_Next_Seg(chain2)) != NULL) {
    Local_Neuroseg *locseg2 = locseg;
    
    if (index == 0) { /* head */
      locseg2 = Copy_Local_Neuroseg(locseg);
      Local_Neuroseg_Stretch(locseg2, scale, offset, -1);
    }
    
    if (Locseg_Chain_Peek(chain2) == NULL) { /*tail*/
      if (locseg == locseg2) {
	locseg2 = Copy_Local_Neuroseg(locseg);
      }
      Local_Neuroseg_Stretch(locseg2, scale, offset, 1);      
    }

    double dist = Local_Neuroseg_Planar_Dist(head, locseg2);
    if (dist < mindist) {
      mindist = dist;
      conn->info[0] = 0;
      conn->info[1] = index;
    }

    dist = Local_Neuroseg_Planar_Dist(tail, locseg2);
    if (dist < mindist) {
      mindist = dist;
      conn->info[0] = 1;
      conn->info[1] = index;
    }

    if (locseg != locseg2) {
      Delete_Local_Neuroseg(locseg2);
    }

    index++;
  }

  Delete_Local_Neuroseg(head);  
  Delete_Local_Neuroseg(tail);  

  conn->cost = mindist;

  return mindist;
}

Locseg_Chain* Local_Neuroseg_Push(Local_Neuroseg *locseg, const Stack *stack, 
				  double z_scale, const Stack *mask,
				  int option, double *record, 
				  int *trial)
{
  Stack_Fit_Score fs;
  fs.n = 2;
  fs.options[0] = STACK_FIT_CORRCOEF;
  fs.options[1] = STACK_FIT_VALID_SIGNAL_RATIO;

  if (record != NULL) {
    fs.n = 3;
    fs.options[2] = option;
  }

  Local_Neuroseg_Score(locseg, stack, z_scale, &fs);
  double score0 = fs.scores[0];
  if (record != NULL) {
    record[0] = fs.scores[2];
  }
  int var_index[LOCAL_NEUROSEG_PLANE_NPARAM];
  int nvar = 
    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_ORIENTATION |
				     NEUROSEG_VAR_MASK_R1 | 
				     NEUROSEG_VAR_MASK_SCALE,
				     NEUROSEG_VAR_MASK_NONE, var_index);

  Locseg_Chain *chain = New_Locseg_Chain();

  Local_Neuroseg *locseg2 = locseg;

  int i;
  for (i = 1; i < 15; i++) {
    locseg2 = Next_Local_Neuroseg(locseg2, NULL, 0.2);
    //locseg2->seg.h = 5.0;
    if (mask == NULL) {
      Fit_Local_Neuroseg_P(locseg2, stack, var_index, nvar, NULL, z_scale, &fs);
    } else {
      Fit_Local_Neuroseg_Pm(locseg2, stack, var_index, nvar, NULL, z_scale, 
			    mask, &fs);
    }
    
    if ((Neuroseg_Angle_Between(&(locseg->seg), &(locseg2->seg)) 
	 > TZ_PI / 2.0) || (fs.scores[0] / score0 < 0.5)){
      i--;
      break;
    }

    Locseg_Chain_Add(chain, locseg2, NULL, DL_TAIL);
    if (record != NULL) {
      record[i] = fs.scores[2];
    }

    
    if (fs.scores[1] < 0.80) {
      break;
    }
    
  }

  if (trial != NULL) {
    *trial = i;
  }

  return chain;
}

/*
 * Features:
 *  1. anisotropic planar distance 
 *  2. isotropic planar distance 
 *  3. anisotropic euclidean distance 
 *  4. isotropic euclidean distance 
 *  5. thickness 1 
 *  6. thickness 2 
 *  7. angle 
 *  8. anisotropic surface distance 
 *  9. isotropic surface distance 
 *  10. geodesic distance
 *  11. maxmin distance
 */
double* Locseg_Chain_Conn_Feature(Locseg_Chain *chain1, Locseg_Chain *chain2,
				  const Stack *stack, const double *res, 
				  double *feat, int *n)
{
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  if (res != NULL) {
    ctw->resolution[0] = res[0];
    ctw->resolution[1] = res[1];
    ctw->resolution[2] = res[2];
  }

  Neurocomp_Conn nc_conn;
  Locseg_Chain_Connection_Test(chain1, chain2, NULL, 1.0, &nc_conn, ctw);
  Kill_Connection_Test_Workspace(ctw);

  Local_Neuroseg *locseg1 = NULL;

  if (nc_conn.info[0] == 0) {
    locseg1 = Locseg_Chain_Head_Seg(chain1);
  } else {
    locseg1 = Locseg_Chain_Tail_Seg(chain1);
  }
   
  Local_Neuroseg *locseg2 = Locseg_Chain_Peek_Seg_At(chain2, nc_conn.info[1]);

  Locseg_Conn_Feature(locseg1, locseg2, stack, res, feat, n);
  
  /* Calculate geodesic dist and maxmin dist. More efficient impementation
   * required.*/
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  if (res != NULL) {
    sgw->resolution[0] = res[0];
    sgw->resolution[1] = res[1];
    sgw->resolution[2] = res[2];
  }

  Int_Arraylist *path = Locseg_Chain_Shortest_Path(chain1, chain2, stack,
      1.0, sgw);
  if (path != NULL) {
    feat[(*n)++] = sgw->value;
    Kill_Int_Arraylist(path);
  } else {
    feat[(*n)++] = NaN;
  }
  Kill_Stack_Graph_Workspace(sgw);
  
  sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  if (res != NULL) {
    sgw->resolution[0] = res[0];
    sgw->resolution[1] = res[1];
    sgw->resolution[2] = res[2];
  }

  sgw->sp_option = 1;
  path = Locseg_Chain_Shortest_Path(chain1, chain2, stack,
      1.0, sgw);
  if (path != NULL) {
    feat[(*n)++] = -sgw->value;
    Kill_Int_Arraylist(path);
  } else {
    feat[(*n)++] = NaN;
  }
  Kill_Stack_Graph_Workspace(sgw);

  return feat;
}

#if 0
double *Locseg_Chain_Conn_Feature(Locseg_Chain *chain, 
				  const Local_Neuroseg *locseg1,
				  const Stack *stack, const double *res, 
				  double *feat, int *n)
{
  double z_scale = res[0] / res[2];

  Local_Neuroseg *zlocseg1 = Copy_Local_Neuroseg(locseg1);
  Local_Neuroseg_Scale_Z(zlocseg1, z_scale);  

  /* Find the segment in <chain> that is closest to <locseg1> */
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  
  Local_Neuroseg *locseg2 = Locseg_Chain_Next_Seg(chain);
  Local_Neuroseg *zlocseg2 = Locseg_Chain_Next_Seg(chain);

  zlocseg2 = Copy_Local_Neuroseg(locseg2);
  Local_Neuroseg_Scale_Z(zlocseg2, z_scale);
  
  double zbottom1[3], ztop1[3];
  Local_Neuroseg_Bottom(zlocseg1, zbottom1);
  Local_Neuroseg_Top(zlocseg1, ztop1);
  
  double mindist = 
    Local_Neuroseg_Lineseg_Dist(zlocseg2, zbottom1, ztop1, 1.0);

  Delete_Local_Neuroseg(zlocseg2);

  Local_Neuroseg *tmpseg = NULL;

  while ((tmpseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    zlocseg2 = Copy_Local_Neuroseg(tmpseg);
    Local_Neuroseg_Scale_Z(zlocseg2, z_scale);

    double dist = Local_Neuroseg_Lineseg_Dist(zlocseg2, zbottom1, ztop1, 1.0);
    if (dist < mindist) {
      locseg2 = tmpseg;
      mindist = dist;
    }
      
    Delete_Local_Neuroseg(zlocseg2);
  }

  /* Start calculating features*/

  *n = 9;
  if (feat == NULL) {
    feat = darray_malloc(*n);
  }
  
  double bottom1[3], top1[3];
  double bottom2[3], top2[3];

  Local_Neuroseg_Bottom(locseg1, bottom1);
  Local_Neuroseg_Top(locseg1, top1);
  Local_Neuroseg_Bottom(locseg2, bottom2);
  Local_Neuroseg_Top(locseg2, top2);
  
  zlocseg2 = Copy_Local_Neuroseg(locseg2);
 
  Local_Neuroseg_Scale_Z(zlocseg2, z_scale);

  double zbottom2[3], ztop2[3];
  Local_Neuroseg_Bottom(zlocseg2, zbottom2);
  Local_Neuroseg_Top(zlocseg2, ztop2);
  
  /* anisotropic planar distance */
  feat[0] = Geo3d_Line_Line_Dist(zbottom1, ztop1, zbottom2, ztop2);
  feat[0] *= res[0];

  /* isotropic planar distance */
  feat[1] = Geo3d_Line_Line_Dist(bottom1, top1, bottom2, top2);
  
  double intersect1, intersect2;
  int cond;

  /* anisotropic euclidean distance */
  feat[2] = Geo3d_Lineseg_Lineseg_Dist(zbottom1, ztop1, zbottom2, ztop2,
				       &intersect1, &intersect2, &cond);
  feat[2] *= res[0];

  /* isotropic euclidean distance */
  feat[3] = Geo3d_Lineseg_Lineseg_Dist(bottom1, top1, bottom2, top2,
				       &intersect1, &intersect2, &cond);

  /* thickness 1 */
  feat[4] = Neuroseg_Rx(&(locseg1->seg), NEUROSEG_CENTER) * res[0];

  /* thickness 2 */
  feat[5] = Neuroseg_Rx(&(locseg2->seg), NEUROSEG_CENTER) * res[0];

  /* angle */
  feat[6] = Neuroseg_Angle_Between(&(locseg1->seg), &(locseg2->seg));

  /* anisotropic surface distance */
  feat[7] = Local_Neuroseg_Lineseg_Dist(zlocseg2, zbottom1, ztop1, 1.0) 
    * res[0];

/* isotropic surface distance */
  feat[8] = Local_Neuroseg_Lineseg_Dist(locseg2, bottom1, top1, 1.0);

  return feat;
}
#endif

double Locseg_Chain_Average_Score(Locseg_Chain *chain, const Stack *stack,
				  double z_scale, int option)
{
  double score = 0.0;
  int n = 0;

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = option;

  Local_Neuroseg *locseg;
  while ( (locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    score += Local_Neuroseg_Score(locseg, stack, z_scale, &fs);
    n++;
  }

  return score / n;
}

double Locseg_Chain_Min_Seg_Score(Locseg_Chain *chain, const Stack *stack,
				  double z_scale, int option)
{
  double min_score = Infinity;
  double score;

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = option;

  Local_Neuroseg *locseg;
  while ( (locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    score = Local_Neuroseg_Score(locseg, stack, z_scale, &fs);
    if (score < min_score) {
      min_score = score;
    }
  }

  return min_score;
}

double Locseg_Chain_Median_Score(Locseg_Chain *chain, const Stack *stack,
				 double z_scale, int option)
{
  double score = 0.0;
  int n = Locseg_Chain_Length(chain);

  double *score_array = darray_malloc(n);

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = option;

  Local_Neuroseg *locseg;
  int i = 0;
  while ( (locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    score_array[i] = Local_Neuroseg_Score(locseg, stack, z_scale, &fs);
    i++;
  }

  score = score_array[n / 2];

  free(score_array);

  return score;
}

double Locseg_Chain_Average_Bottom_Radius(Locseg_Chain *chain)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  int n = 0;
  double r = 0.0;

  Local_Neuroseg *locseg;
  while ( (locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    r += NEUROSEG_CR1(&(locseg->seg));
    n++;
  }

  r /= n;

  return r;
}

int Locseg_Chain_File_Trace_Record_Size(const char *file)
{
  FILE *fp = fopen(file, "r");
  Local_Neuroseg locseg;
  Trace_Record tr;

  Local_Neuroseg_Fread(&locseg, fp);
  
  while (Trace_Record_Fread(&tr, fp) == NULL) {
    if (Local_Neuroseg_Fread(&locseg, fp) == NULL) {
      return sizeof(Trace_Record);
    }
  }

  double best_score = -1.0;
  int best_size = sizeof(Trace_Record);
  
  Local_Neuroseg locseg2;

  fseek(fp, -best_size, SEEK_CUR);

  int i;
  int largest_size = sizeof(Trace_Record) * 2;
  for (i = 1; i <= largest_size; i++) {
    fseek(fp, i, SEEK_CUR);
    if (Local_Neuroseg_Fread(&locseg2, fp) != NULL) {
      double score = 
	Geo3d_Dist_Sqr(locseg.pos[0], locseg.pos[1], locseg.pos[2],
		       locseg2.pos[0], locseg2.pos[1], locseg.pos[2]);
      if (best_score < 0) {
	best_score = score;
	best_size = i;
      } else {
	if (best_score > score) {
	  best_score = score;
	  best_size = i;
	}
      }
    } else {
      break;
    }
    fseek(fp, -i - sizeof(Local_Neuroseg), SEEK_CUR);
  }

  fclose(fp);

  return best_size;
}

void Locseg_Chain_Down_Sample(Locseg_Chain *chain)
{
  if (Locseg_Chain_Length(chain) >= 3) {
    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    double prev_pos[3];
    double pos[3];
    Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg(chain);
    Local_Neuroseg_Top(locseg, prev_pos);
    Locseg_Chain_Next_Seg(chain);
    locseg = Locseg_Chain_Peek_Seg(chain);

    while (Locseg_Chain_Peek_Next(chain) != NULL) {
      Local_Neuroseg_Top(locseg, pos);
#ifdef _DEBUG_2
      printf("%g\n", Geo3d_Dist_Sqr(prev_pos[0], prev_pos[1], prev_pos[2],
				   pos[0], pos[1], pos[2]));
#endif
      if (Geo3d_Dist_Sqr(prev_pos[0], prev_pos[1], prev_pos[2],
			 pos[0], pos[1], pos[2]) < 5.0) {
	Locseg_Chain_Remove_Current(chain);
	Locseg_Chain_Iterator_Start(chain, DL_HEAD);
	locseg = Locseg_Chain_Peek_Seg(chain);
	Local_Neuroseg_Top(locseg, prev_pos);
      } else {
	prev_pos[0] = pos[0];
	prev_pos[1] = pos[1];
	prev_pos[2] = pos[2];
      }
      Locseg_Chain_Next_Seg(chain);
      locseg = Locseg_Chain_Peek_Seg(chain);
    }
  }
}

void Locseg_Chain_Tune_End(Locseg_Chain *chain, Stack *stack, double z_scale,
			   Stack *mask, Dlist_End_e end)
{
  Local_Neuroseg *locseg = NULL;
  Local_Neuroseg *prev_locseg = NULL;
  double pos[3];
  BOOL end_removed = FALSE;

  Locseg_Fit_Workspace *fws = New_Locseg_Fit_Workspace();
  Locseg_Fit_Workspace_Set_Var(fws,
			       NEUROSEG_VAR_MASK_ORIENTATION,
			       NEUROSEG_VAR_MASK_NONE);
  
  Local_Neuroseg* (*step_func) (Locseg_Chain*);
  
  if (end == DL_HEAD) {
    step_func = Locseg_Chain_Next_Seg;
  } else {
    step_func = Locseg_Chain_Prev_Seg;
  }

  Locseg_Chain_Iterator_Start(chain, end);
  do {      
    locseg = step_func(chain);

    if (locseg == NULL) {
      break;
    }

    prev_locseg = Locseg_Chain_Peek_Seg(chain);
    
    end_removed = FALSE;
    if (end == DL_HEAD) {
      Flip_Local_Neuroseg(locseg);
    }
     
#ifdef _DEBUG_
    printf("before: ");
    Print_Local_Neuroseg(locseg);
#endif

    Local_Neuroseg_Break_Gap(locseg, stack, z_scale);
    
    Print_Local_Neuroseg(locseg);
    
    if (mask != NULL) {
      /* if the center hit marked region */
      if (Local_Neuroseg_Center_Sample(locseg, mask, z_scale) > 0.8) {
	Local_Neuroseg_Chop(locseg, 0.5);
      }

      /* if the top hit marked region */
      while (Local_Neuroseg_Top_Sample(locseg, mask, z_scale) > 0.8) {
	if (locseg->seg.h <= 1.0) {
	  Locseg_Chain_Remove_End(chain, end);
	  locseg = NULL;
	  end_removed = TRUE;
	} else if (prev_locseg != NULL) {
	  Local_Neuroseg_Top(locseg, pos);
	  if (Local_Neuroseg_Hit_Test(prev_locseg, pos[0], pos[1], pos[2])
	      == TRUE) {
	    Locseg_Chain_Remove_End(chain, end);
	    locseg = NULL;
	    end_removed = TRUE;
	  }
	}
	if (end_removed == FALSE) {
	  if (locseg->seg.h < 2.0) {
	    Locseg_Chain_Remove_End(chain, end);
	    locseg = NULL;
	    end_removed = TRUE;
	    break;
	  } else {
	    Local_Neuroseg_Change_Height(locseg, locseg->seg.h - 1.0);
	  }
	} else {
	  break;
	}
      }
      
#ifdef _DEBUG_
      Print_Local_Neuroseg(locseg);
#endif
    }

    if (end_removed == FALSE) {
      if (locseg->seg.h < 1.0) {
        Locseg_Chain_Remove_End(chain, end);
        locseg = NULL;
        end_removed = TRUE;
      }
    }

    if (stack != NULL) {
      if (end_removed == FALSE) {
	/* adjust the height of locseg */
	Local_Neuroseg_Height_Search_P(locseg, stack, z_scale);
	if (locseg->seg.h < 1.0) {
	  Locseg_Chain_Remove_End(chain, end);
	  locseg = NULL;
	  end_removed = TRUE;
	} else {
	  if (prev_locseg != NULL) {
	    Local_Neuroseg_Top(locseg, pos);
	    if (Local_Neuroseg_Hit_Test(prev_locseg, pos[0], pos[1], pos[2])
		== TRUE) {
	      Locseg_Chain_Remove_End(chain, end);
	      locseg = NULL;
	      end_removed = TRUE;
	    }
	  }
	}
      }
    }
  } while (end_removed == TRUE);
    
  if (locseg == NULL) {
    Locseg_Chain_Iterator_Start(chain, end);
    locseg = step_func(chain);
    if (end == DL_HEAD) {
      if (locseg != NULL) {
	Flip_Local_Neuroseg(locseg);
      }
    }
  }

  if (locseg != NULL) { /* fit the end */
    Print_Local_Neuroseg(locseg);
    //fws->sws->mask = mask;
    //Locseg_Fit_Workspace_Enable_Cone(fws);
    Fit_Local_Neuroseg_W(locseg, stack, z_scale, fws);
    fws->sws->mask = NULL;
    if (end == DL_HEAD) {
      Flip_Local_Neuroseg(locseg);
    }
  }

  Kill_Locseg_Fit_Workspace(fws);
}

void Locseg_Chain_Refine_End(Locseg_Chain *chain, Stack *signal, double z_scale,
    Trace_Workspace *tw)
{
  Trace_Workspace tmp_tw = *tw;
  tmp_tw.trace_step = 0.1;
  tmp_tw.refit = FALSE;
  tmp_tw.min_score -= 0.05;
  tmp_tw.trace_status[0] = TRACE_NORMAL;
  tmp_tw.trace_status[1] = TRACE_NORMAL;

  if (Locseg_Chain_Length(chain) == 1) {
    /* break the segment into two parts */
    Locseg_Chain_Break_Node(chain, 0, 0.5);
  } else {
    Locseg_Chain_Adjust_Seed(chain);
  }
  Trace_Locseg(signal, 1.0, chain, tw);
  Locseg_Chain_Down_Sample(chain);

  Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_HEAD);
  Locseg_Chain_Tune_End(chain, signal, z_scale, tw->trace_mask, DL_TAIL);
}

static double kappa3(double *p1, double *p2, double *p3)
{
  if ((p1 == NULL) || (p2 == NULL) || (p3 == NULL)) {
    return 0;
  }

  return sqrt(1 - Coordinate_3d_Cos3(p1, p2, p3));
}

int Locseg_Chain_Connect_Seg(Locseg_Chain *chain, Local_Neuroseg *locseg, 
			     Trace_Record *tr)
{
  int n = Locseg_Chain_Length(chain);

  double center[4][3];
  Local_Neuroseg *locseg_buffer[4];

  double new_center[3];
  Local_Neuroseg_Center(locseg, new_center);

  /* alloc <kappa> */
  double *kappa = darray_malloc(n + 2);
  
  int i, j;

  kappa[0] = 0;
  kappa[1] = 0;
  kappa[n-1] = 0;
  kappa[n] = 0;
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  locseg_buffer[0] = Locseg_Chain_Next_Seg(chain);
  locseg_buffer[1] = Locseg_Chain_Next_Seg(chain);

  for (i = 1; i < n - 1; i++) {
    locseg_buffer[2] = Locseg_Chain_Next_Seg(chain);

    for (j = 0; j < 3; j++) {
      Local_Neuroseg_Center(locseg_buffer[j], center[j]);
    }

    kappa[i] = kappa3(center[0], center[1], center[2]);
    locseg_buffer[0] = locseg_buffer[1];
    locseg_buffer[1] = locseg_buffer[2];
  }

  
  double min_cost = 5.0;
  int index = -1;
  double *p[4];

  for (i = 0; i < 4; i++) {
    p[i] = NULL;
  }

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  locseg_buffer[0] = NULL;
  locseg_buffer[1] = NULL;
  locseg_buffer[2] = Locseg_Chain_Next_Seg(chain);

  Local_Neuroseg_Center(locseg_buffer[2], center[0]);
  p[2] = center[0];

  j = 1;

  for (i = 0; i <= n; i++) {
    locseg_buffer[3] = Locseg_Chain_Next_Seg(chain);
    if (locseg_buffer[3] == NULL) {
      p[3] = NULL;
    } else {
      Local_Neuroseg_Center(locseg_buffer[3], center[j]);
      p[3] = center[j];
    }
    double cost = -kappa[i] - kappa[i+1] + kappa3(p[0], p[1], new_center) +
      kappa3(p[1], new_center, p[2]) + kappa3(new_center, p[2], p[3]);
    if (cost < min_cost) {
      min_cost = cost;
      index = i;
    }
    p[0] = p[1];
    p[1] = p[2];
    p[2] = p[3];
    j++;
    if (j > 3) {
      j = 0;
    }
  }

  /* free <kappa> */
  free(kappa);

  Xz_Orientation_t ort[3];
  ort[1][0] = locseg->seg.theta;
  ort[1][1] = locseg->seg.psi;

  double flow = 0.0;

  if (index == 0) {
    locseg_buffer[0] = Locseg_Chain_Head_Seg(chain);
    ort[0][0] = locseg_buffer[0]->seg.theta;
    ort[0][1] = locseg_buffer[0]->seg.psi;
    flow = Xz_Orientation_Dot(ort[0], ort[1]);
  } else if (index == n) {
    locseg_buffer[0] = Locseg_Chain_Tail_Seg(chain);
    ort[0][0] = locseg_buffer[0]->seg.theta;
    ort[0][1] = locseg_buffer[0]->seg.psi;
    flow = Xz_Orientation_Dot(ort[0], ort[1]);
  } else {
    Locseg_Chain_Iterator_Locate(chain, index - 1);
    locseg_buffer[0] = Locseg_Chain_Next_Seg(chain);
    ort[0][0] = locseg_buffer[0]->seg.theta;
    ort[0][1] = locseg_buffer[0]->seg.psi;
    locseg_buffer[0] = Locseg_Chain_Peek_Seg(chain);
    ort[2][0] = locseg_buffer[0]->seg.theta;
    ort[2][1] = locseg_buffer[0]->seg.psi;
    flow = Xz_Orientation_Dot(ort[0], ort[1]) + 
      Xz_Orientation_Dot(ort[1], ort[2]);
  }

  if (flow < 0) {
    Flip_Local_Neuroseg(locseg);
  }

  Locseg_Chain_Insert(chain, locseg, tr, index);

  return index;
}

/* See th M document for more details. */
void Locseg_Chain_Merge(Locseg_Chain *chain1, Locseg_Chain *chain2, 
			Dlist_End_e end1, Dlist_End_e end2)
{
  if (Locseg_Chain_Is_Empty(chain2)) {
    return;
  }

  /* head-head merge or tail-tail merge */
  if (end1 == end2) {
    Locseg_Chain_Flip(chain2);
  }

  /* Switch the chains if necessary. This is to make sure that the tail of 
   * input_chain[0] (master chain) is merged to the head of 
   * input_chain[1] (slave chain) */
  Locseg_Chain *input_chain[2];

  if (end1 == DL_TAIL) {
    input_chain[0] = chain1;
    input_chain[1] = chain2;
  } else {
    input_chain[1] = chain1;
    input_chain[0] = chain2;
  }

  Locseg_Node *node = NULL;
  BOOL changed = TRUE;
  while (((node = Locseg_Chain_Head(input_chain[1])) != NULL) &&
	 (changed == TRUE)){
    double top[3], bottom[3];
    Local_Neuroseg_Top(node->locseg, top);
    Local_Neuroseg_Bottom(node->locseg, bottom);
    changed = FALSE;
    if ((Locseg_Chain_Hit_Test(input_chain[0], DL_BACKWARD, 
			       top[0], top[1], top[2]) > 0) &&
	(Locseg_Chain_Hit_Test(input_chain[0], DL_BACKWARD, 
			       bottom[0], bottom[1], bottom[2]) > 0)) {
      /* Remove the head of slave chain if it is within the master chain. */
      Locseg_Chain_Remove_End(input_chain[1], DL_HEAD);
      changed = TRUE;
    } else {
      /* Remove the head of slave chain if the bottom of the secondary head is
	 within the master chain. */
      node = Locseg_Chain_Peek_At(input_chain[1], 1);
      if (node != NULL) {
	Local_Neuroseg_Bottom(node->locseg, bottom);
	if (Locseg_Chain_Hit_Test(input_chain[0], DL_BACKWARD, 
				  bottom[0], bottom[1], bottom[2]) > 0) {
	  Locseg_Chain_Remove_End(input_chain[1], DL_HEAD);
	  changed = TRUE;
	}
      }
    }    
  }

  double top[3], bottom[3];
  changed = TRUE;
  while (((node = Locseg_Chain_Tail(input_chain[0])) != NULL) &&
	 (changed == TRUE)){
    Local_Neuroseg_Top(node->locseg, top);
    Local_Neuroseg_Bottom(node->locseg, bottom);
    changed = FALSE;
    if ((Locseg_Chain_Hit_Test(input_chain[1], DL_FORWARD, 
			       top[0], top[1], top[2]) > 0) &&
	(Locseg_Chain_Hit_Test(input_chain[1], DL_FORWARD, 
			       bottom[0], bottom[1], bottom[2]) > 0)) {
	Locseg_Chain_Remove_End(input_chain[0], DL_TAIL);
	changed = TRUE;
    } else {
      Locseg_Chain_Iterator_Start(input_chain[0], DL_TAIL);
      Locseg_Chain_Prev(input_chain[0]);
      node = Locseg_Chain_Prev(input_chain[0]);
      if (node != NULL) {
	Local_Neuroseg_Bottom(node->locseg, top);
	if (Locseg_Chain_Hit_Test(input_chain[1], DL_BACKWARD, 
				  top[0], top[1], top[2]) > 0) {
	  Locseg_Chain_Remove_End(input_chain[0], DL_TAIL);
	  changed = TRUE;
	}
      }
    }    
  }

  if ((Locseg_Chain_Length(input_chain[0]) > 0) &&
      (Locseg_Chain_Length(input_chain[1]) > 0)) {
    node = Locseg_Chain_Tail(input_chain[0]);
    double pos[3];
    if (Locseg_Node_Fix_Pos(node, pos)) {
      if (Locseg_Chain_Hit_Test(input_chain[1], DL_BACKWARD, 
				pos[0], pos[1], pos[2]) > 0) {
	Locseg_Node_Set_Direction(node, DL_BOTHDIR);
      }
    }

    node = Locseg_Chain_Head(input_chain[1]);
    if (Locseg_Node_Fix_Pos(node, pos)) {
      if (Locseg_Chain_Hit_Test(input_chain[0], DL_FORWARD, 
				pos[0], pos[1], pos[2]) > 0) {
	Locseg_Node_Set_Direction(node, DL_BOTHDIR);
      }
    }

    if (Locseg_Chain_Length(input_chain[0]) == 1) {
      node = Locseg_Chain_Tail(input_chain[0]);
      if (Trace_Record_Direction(node->tr) == DL_BOTHDIR) {
	Local_Neuroseg_Bottom(node->locseg, bottom);
	Local_Neuroseg_Top(node->locseg, top);
	Local_Neuroseg *head = Locseg_Chain_Head_Seg(input_chain[1]);
	if (Local_Neuroseg_Point_Dist(head, 
				      bottom[0], bottom[1], bottom[2], 1.0) < 
	    Local_Neuroseg_Point_Dist(head, top[0], top[1], top[2], 1.0)) {
	  Flip_Local_Neuroseg(node->locseg);
	}
      }
    }

    if (Locseg_Chain_Length(input_chain[1]) == 1) {
      node = Locseg_Chain_Head(input_chain[1]);
      if (Trace_Record_Direction(node->tr) == DL_BOTHDIR) {
	Local_Neuroseg_Bottom(node->locseg, bottom);
	Local_Neuroseg_Top(node->locseg, top);
	Local_Neuroseg *tail = Locseg_Chain_Tail_Seg(input_chain[0]);
	if (Local_Neuroseg_Point_Dist(tail, 
				      bottom[0], bottom[1], bottom[2], 1.0) > 
	    Local_Neuroseg_Point_Dist(tail, top[0], top[1], top[2], 1.0)) {
	  Flip_Local_Neuroseg(node->locseg);
	}
      }
    }
  }

  if (Locseg_Chain_Is_Empty(input_chain[1]) == FALSE) {
    Locseg_Chain_Cat(input_chain[0], input_chain[1]);
  }
  
  if (end1 == DL_HEAD) {
    chain1->list = chain2->list;
  }

  chain2->list = NULL;
}


void Locseg_Chain_Flip(Locseg_Chain *chain)
{
  Locseg_Node_Dlist *tmp_list;

  if (chain != NULL) {
    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    while (chain->iterator != NULL) {
      Locseg_Node *node = Locseg_Chain_Peek(chain);
      Flip_Local_Neuroseg(node->locseg);
      if (node->tr != NULL) {
	if (node->tr->fix_point >= 0.0) {
	  node->tr->fix_point = 1.0 - node->tr->fix_point;
	}
	if (Trace_Record_Direction(node->tr) == DL_FORWARD) {
	  Trace_Record_Set_Direction(node->tr, DL_BACKWARD);
	} else if (Trace_Record_Direction(node->tr) == DL_BACKWARD) {
	  Trace_Record_Set_Direction(node->tr, DL_FORWARD);
	}
      }
      tmp_list = chain->iterator->prev;
      chain->iterator->prev = chain->iterator->next;
      chain->iterator->next = tmp_list;
      chain->iterator = chain->iterator->prev;
    }
  }
}

void Locseg_Chain_Conn_Fix(Locseg_Chain *chain1, Locseg_Chain *chain2,
			   Neurocomp_Conn *conn)
{
  Dlist_Direction_e test_direction = DL_FORWARD;
  if (conn->info[0] == 0) {
    test_direction = DL_BACKWARD;
  }

  if (conn->mode == NEUROCOMP_CONN_LINK) {
    if (conn->info[1] == 0) { /* shrink head */
      Local_Neuroseg *locseg = NULL;
      while ((locseg = Locseg_Chain_Head_Seg(chain2)) != NULL) {
	double center[3];
	Local_Neuroseg_Center(locseg, center);
	if (Locseg_Chain_Hit_Test(chain1, test_direction, 
				  center[0], center[1], center[2]) > 0) {
	  Locseg_Chain_Remove_End(chain2, DL_HEAD);
	} else {
	  break;
	}
      }
    } else { /* shrink tail */
      Local_Neuroseg *locseg = NULL;
      while ((locseg = Locseg_Chain_Tail_Seg(chain2)) != NULL) {
	double center[3];
	Local_Neuroseg_Center(locseg, center);
	if (Locseg_Chain_Hit_Test(chain1, test_direction, 
				  center[0], center[1], center[2]) > 0) {
	  Locseg_Chain_Remove_End(chain2, DL_TAIL);
	} else {
	  break;
	}
      }      
    }   
  }
}

/*
void Locseg_Chain_Remove_Redundant(Locseg_Chain *chain)
{
  if (Locseg_Chain_Length(chain) >= 2) {
    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    double pos[3];
    Local_Neuroseg *prev_locseg = NULL;
    Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg(chain);
    Local_Neuroseg_Top(locseg, prev_pos);
    Locseg_Chain_Next_Seg(chain);
    locseg = Locseg_Chain_Peek_Seg(chain);

    while (Locseg_Chain_Peek_Next(chain) != NULL) {
      Local_Neuroseg_Top(locseg, pos);
#ifdef _DEBUG_2
      printf("%g\n", Geo3d_Dist_Sqr(prev_pos[0], prev_pos[1], prev_pos[2],
				   pos[0], pos[1], pos[2]));
#endif
      if (Geo3d_Dist_Sqr(prev_pos[0], prev_pos[1], prev_pos[2],
			 pos[0], pos[1], pos[2]) < 5.0) {
	Locseg_Chain_Remove_Current(chain);
	Locseg_Chain_Iterator_Start(chain, DL_HEAD);
	locseg = Locseg_Chain_Peek_Seg(chain);
	Local_Neuroseg_Top(locseg, prev_pos);
      } else {
	prev_pos[0] = pos[0];
	prev_pos[1] = pos[1];
	prev_pos[2] = pos[2];
      }
      Locseg_Chain_Next_Seg(chain);
      locseg = Locseg_Chain_Peek_Seg(chain);
    }
  }  
}
*/

Locseg_Chain* Locseg_Chain_Bridge(const Local_Neuroseg *locseg1, 
				  const Local_Neuroseg *locseg2)
{
  double center1[3], center2[3];
  Local_Neuroseg_Center(locseg1, center1);
  Local_Neuroseg_Center(locseg2, center2);
  
  double vec[3];
  
  Geo3d_Coordinate_Offset(center1[0], center1[1], center1[2],
			  center2[0], center2[1], center2[2],
			  vec, vec + 1, vec + 2);
  
  Local_Neuroseg *start_locseg = Copy_Local_Neuroseg(locseg1);
  Local_Neuroseg *end_locseg = Copy_Local_Neuroseg(locseg2);
  
  coordinate_3d_t vec1, vec2;
  Geo3d_Orientation_Normal(locseg1->seg.theta, locseg1->seg.psi,
			   vec1, vec1 + 1, vec1 + 2);
  Geo3d_Orientation_Normal(locseg2->seg.theta, locseg2->seg.psi,
			   vec2, vec2 + 1, vec2 + 2);

  if (Geo3d_Dot_Product(vec1[0], vec1[1], vec1[2],
			vec[0], vec[1], vec[2]) < 0) {
    Flip_Local_Neuroseg(start_locseg);
    Geo3d_Orientation_Normal(start_locseg->seg.theta, start_locseg->seg.psi,
			     vec1, vec1 + 1, vec1 + 2);
  }

  if (Geo3d_Dot_Product(vec2[0], vec2[1], vec2[2],
			vec[0], vec[1], vec[2]) < 0) {
    Flip_Local_Neuroseg(end_locseg);
    Geo3d_Orientation_Normal(end_locseg->seg.theta, end_locseg->seg.psi,
			     vec2, vec2 + 1, vec2 + 2);
  }
  
  
  Locseg_Chain *chain = New_Locseg_Chain();

  Local_Neuroseg *locseg = New_Local_Neuroseg();

  double length = Geo3d_Orgdist(vec[0], vec[1], vec[2]);
  length -= locseg->seg.h;
  int nseg = 1;

  if (length > 0) {
    nseg += iround(length * 2.0 / (locseg->seg.h - 1.0));
  }

  double dx, dy, dz, dr, dc, ds;
  dx = vec[0] / (nseg + 1);
  dy = vec[1] / (nseg + 1);
  dz = vec[2] / (nseg + 1);
  dr = (end_locseg->seg.r1 - start_locseg->seg.r1) / (nseg + 1);
  dc = (end_locseg->seg.c - start_locseg->seg.c) / (nseg + 1);

  /*
  start_locseg->seg.theta = Normalize_Radian(start_locseg->seg.theta);
  start_locseg->seg.psi = Normalize_Radian(start_locseg->seg.psi);
  end_locseg->seg.theta = Normalize_Radian(end_locseg->seg.theta);
  end_locseg->seg.psi = Normalize_Radian(end_locseg->seg.psi);
  dtheta = (end_locseg->seg.theta - start_locseg->seg.theta) / (nseg + 1);
  dpsi = (end_locseg->seg.psi - start_locseg->seg.psi) / (nseg + 1);
  */

  int i;
  /*
  double dort[3];
  for (i =0; i < 3; i++) {
    dort[i] = (vec2[i] - vec1[i]) / (nseg + 1);
  }
  */
  ds = (end_locseg->seg.scale - start_locseg->seg.scale) / (nseg + 1);  

  Set_Neuroseg_Position(locseg, center1, NEUROSEG_BOTTOM);
  Coordinate_3d_Unitize(vec);
  Geo3d_Normal_Orientation(vec[0], vec[1], vec[2], &(locseg->seg.theta),
			   &(locseg->seg.psi));

  locseg->seg.r1 = start_locseg->seg.r1 + dr;
  locseg->seg.c = start_locseg->seg.c + dc;
  /*
  locseg->seg.theta = start_locseg->seg.theta + dtheta;
  locseg->seg.psi = start_locseg->seg.psi + dpsi;
  */
  locseg->seg.scale = start_locseg->seg.scale + ds;

  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);

  for (i = 1; i < nseg; i++) {
    locseg = Copy_Local_Neuroseg(locseg);
    locseg->pos[0] += dx;
    locseg->pos[1] += dy;
    locseg->pos[2] += dz;
    locseg->seg.r1 += dr;
    locseg->seg.c += dc;

    locseg->seg.scale += ds;
    Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
  }

  Local_Neuroseg_Center(locseg, center1);
  locseg = Copy_Local_Neuroseg(locseg);
  Set_Neuroseg_Position(locseg, center1, NEUROSEG_BOTTOM);
  Local_Neuroseg_Change_Top(locseg, center2);
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);

  Delete_Local_Neuroseg(start_locseg);
  Delete_Local_Neuroseg(end_locseg);

  return chain;
}

Locseg_Chain* Locseg_Chain_Bridge_Sp(const Local_Neuroseg *source, 
				     Locseg_Chain *target, 
				     const Stack *signal, double z_scale,
				     Stack_Graph_Workspace *sgw,
				     Locseg_Fit_Workspace *fw)
{
  Int_Arraylist *path = Locseg_Chain_Shortest_Path_Seg(source, target, signal,
						       z_scale, sgw);

  if (path == NULL) {
    return NULL;
  }

  int coord[3];
  int i;
  double pos[3];
  double start_pos[3];
  double theta, psi;

  Local_Neuroseg_Center(source, start_pos);

  Locseg_Chain *bridge = New_Locseg_Chain();

  /* for average orientation of the local path */
  double vec[3];
  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;

  BOOL first = TRUE;
  Locseg_Fit_Workspace tmp_fw;
  const double dist2_thre = 
    (NEUROSEG_DEFAULT_H - 1.0) * (NEUROSEG_DEFAULT_H - 1.0);

  for (i = 1; i < path->length - 1; i++) {
    Stack_Util_Coord(path->array[i], signal->width, signal->height, 
		     coord, coord + 1, coord + 2);
    pos[0] = coord[0];
    pos[1] = coord[1];
    pos[2] = coord[2];
    if (Geo3d_Dist_Sqr(start_pos[0], start_pos[1], start_pos[2],
		       pos[0], pos[1], pos[2]) < dist2_thre) {
      vec[0] += pos[0] - start_pos[0];
      vec[1] += pos[1] - start_pos[1];
      vec[2] += pos[2] - start_pos[2];
    } else {
      Geo3d_Coord_Orientation(vec[0], vec[1], vec[2], &theta, &psi);
      Local_Neuroseg *new_seg = New_Local_Neuroseg();
      Set_Local_Neuroseg(new_seg, source->seg.r1, 0.0, NEUROSEG_DEFAULT_H, 
			 theta, psi,
			 0.0, 0.0, source->seg.scale, 0.0, 0.0, 0.0);
      Set_Neuroseg_Position(new_seg, start_pos, NEUROSEG_BOTTOM);
      if (fw != NULL) {
	if (first) {
          Locseg_Fit_Workspace_Copy(&tmp_fw, fw);
	  tmp_fw.nvar = 
	    Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
					     NEUROSEG_VAR_MASK_SCALE,
					     NEUROPOS_VAR_MASK_NONE,
					     tmp_fw.var_index);
	  first = FALSE;
	}
	Fit_Local_Neuroseg_W(new_seg, signal, z_scale, &tmp_fw);
      }
      Trace_Record *tr = New_Trace_Record();
      Trace_Record_Set_Direction(tr, DL_FORWARD);
      Trace_Record_Set_Fix_Point(tr, 0.0);
      Locseg_Chain_Add(bridge, new_seg, tr, DL_TAIL);
      Local_Neuroseg_Center(new_seg, start_pos);
      vec[0] = 0.0;
      vec[1] = 0.0;
      vec[2] = 0.0;
    }
  }

  Stack_Util_Coord(path->array[path->length - 2], 
		   signal->width, signal->height, 
		   coord, coord + 1, coord + 2);
  pos[0] = coord[0];
  pos[1] = coord[1];
  pos[2] = coord[2];

  /* last segment */
  if (Coordinate_3d_Distance(pos, start_pos) >= 2.0) {
    int seg_index;
    Locseg_Chain_Point_Dist(target, pos, &seg_index, NULL);
    
    Local_Neuroseg *last_seg = Locseg_Chain_Tail_Seg(bridge);
    
    Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(target, seg_index);
    if (last_seg == NULL) {
      last_seg = locseg;
    }
    
    //Local_Neuroseg_Center(locseg, pos);
    Local_Neuroseg *new_seg = New_Local_Neuroseg();
    Set_Local_Neuroseg(new_seg, last_seg->seg.r1, 0.0, NEUROSEG_DEFAULT_H, 
		       0.0, 0.0, 0.0, 0.0, last_seg->seg.scale, 0.0, 0.0, 0.0);
    Set_Neuroseg_Position(new_seg, start_pos, NEUROSEG_BOTTOM);
    //Locseg_Chain_Point_Dist_K(target, pos, start_pos);
    //Local_Neuroseg_Change_Top(new_seg, start_pos);
    Local_Neuroseg_Change_Top(new_seg, pos);
    
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_FORWARD);
    Trace_Record_Set_Fix_Point(tr, 0.0);
    Locseg_Chain_Add(bridge, new_seg, tr, DL_TAIL);
  }

  return bridge;
}


void Locseg_Chain_End_Pos(Locseg_Chain *chain, Dlist_End_e end, double *pos)
{
  if (end == DL_HEAD) {
    Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain);
    Local_Neuroseg_Bottom(locseg, pos);
  } else {
    Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(chain);
    Local_Neuroseg_Top(locseg, pos);
  }
}

double Locseg_Chain_Point_Dist(Locseg_Chain *chain, const double *pos, 
			       int *seg_index, double *skel_pos)
{
  double dist, min_dist;
  /* alloc <ka> */
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);

  /* alloc <ellipse> */
  Geo3d_Ellipse *ellipse = 
    Locseg_Chain_Knot_Array_To_Ellipse_Z(ka, 1.0, NULL);

  Geo3d_Ellipse cur_ellipse = ellipse[0];
  
  int i;
  int n = Locseg_Chain_Knot_Array_Length(ka);
  min_dist = Geo3d_Ellipse_Point_Distance(&cur_ellipse, pos);
  if (skel_pos != NULL) {
    Geo3d_Ellipse_Center(&cur_ellipse, skel_pos);
  }

  int min_index = 0;
  for (i = 0; i < n - 1; i++) {
    double interval = Coordinate_3d_Distance(ellipse[i].center, 
					     ellipse[i+1].center);
    if (interval > 1.5) {
      double dl = 1.0 / interval;
      double lambda = dl;
      for (lambda = dl; lambda < 1.0; lambda += dl) {
	Geo3d_Ellipse_Interpolate(ellipse + i, ellipse + i + 1, lambda, 
				  &cur_ellipse);
	dist = Geo3d_Ellipse_Point_Distance(&cur_ellipse, pos);
	if (dist < min_dist) {
	  min_dist = dist;
	  /*
	  if (min_dist < 1.0) {
	    min_index = i;
	  }
	  */
	  min_index = Locseg_Chain_Knot_Array_At(ka, i)->id;
	  if (skel_pos != NULL) {
	    Geo3d_Ellipse_Center(&cur_ellipse, skel_pos);
	  }
	}
      }
      dist = Geo3d_Ellipse_Point_Distance(ellipse + i + 1, pos);
      if (dist < min_dist) {
	min_dist = dist;
	min_index = Locseg_Chain_Knot_Array_At(ka, i + 1)->id;
	if (skel_pos != NULL) {
	  Geo3d_Ellipse_Center(ellipse + i + 1, skel_pos);
	}

	//min_index = i;
      }
    }
  }

  /* free <ka> */
  Kill_Locseg_Chain_Knot_Array(ka);

  /* free <ellipse> */
  free(ellipse);

  if (seg_index != NULL) {
    *seg_index = min_index;
  }

  return min_dist;  
}

double Locseg_Chain_Point_Dist_K(Locseg_Chain *chain, const double *pos, 
				 double *knot_pos)
{
  double dist, min_dist;
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  Geo3d_Ellipse *ellipse = 
    Locseg_Chain_Knot_Array_To_Ellipse_Z(ka, 1.0, NULL);

  Geo3d_Ellipse cur_ellipse = ellipse[0];
  
  int i;
  int n = Locseg_Chain_Knot_Array_Length(ka);
  min_dist = Geo3d_Ellipse_Point_Distance(&cur_ellipse, pos);
  int min_index = 0;
  for (i = 1; i < n - 1; i++) {
    dist = Geo3d_Ellipse_Point_Distance(ellipse + i, pos);
    if (dist < min_dist) {
      min_dist = dist;
      min_index = i;
    }
  }
  
  Kill_Locseg_Chain_Knot_Array(ka);
  
  if (knot_pos != NULL) {
    knot_pos[0] = ellipse[min_index].center[0];
    knot_pos[1] = ellipse[min_index].center[1];
    knot_pos[2] = ellipse[min_index].center[2];
  }

  free(ellipse);

  return min_dist;  
}

double Locseg_Chain_Dist(Locseg_Chain *chain1, Locseg_Chain *chain2, 
			 Dlist_End_e end)
{
  double pos[3];
  Locseg_Chain_End_Pos(chain1, end, pos);

  return Locseg_Chain_Point_Dist(chain2, pos, NULL, NULL);
}

double Locseg_Chain_Average_Signal(Locseg_Chain *chain, const Stack *signal,
				   double z_scale)
{
  return Locseg_Chain_Average_Score(chain, signal, z_scale,
				    STACK_FIT_MEAN_SIGNAL);
}

double Locseg_Chain_Min_Seg_Signal(Locseg_Chain *chain, const Stack *signal,
				   double z_scale)
{
  double score = 0.0;
  double min_score = Infinity;

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_MEAN_SIGNAL;

  Local_Neuroseg *locseg;
  while ( (locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    score = Local_Neuroseg_Score(locseg, signal, z_scale, &fs);
    if (score < min_score) {
      min_score = score;
    }
  }

  return min_score;
}

void Locseg_Chain_Bright_End(Locseg_Chain *chain, Dlist_End_e end,
			     const Stack *signal, double z_scale, 
			     double *pos)
{
  Local_Neuroseg *locseg = NULL;
  double max_value = 0.0;
  double last_offset = 0.0;
  double offset = -1.0;
  double start_offset = 0.0;
  double tmp_pos[3];
  double value = 0.0;
  double step = 0.0;

  switch (end) {
  case DL_HEAD:
    locseg = Locseg_Chain_Head_Seg(chain);
    step = 1.0;
    start_offset = 0.0;
    break;
  case DL_TAIL:
    locseg = Locseg_Chain_Tail_Seg(chain);
    step = -1.0;
    start_offset = locseg->seg.h - 1.0;
    break;
  case DL_BOTHDIR:
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  if (step != 0.0) { /* step has been assigned */
    last_offset = (locseg->seg.h - 1.0) / 2.0;
    if (last_offset < 0.0) {
      PRINT_EXCEPTION("Abnormal value", "Negative offset");
      last_offset = 0.0;
    }   
    Local_Neuroseg_Axis_Position(locseg, pos, start_offset);
    if (z_scale != 1.0) {
      pos[2] /= z_scale;
    }
    max_value = Stack_Point_Sampling(signal, pos[0], pos[1], pos[2]);
    offset = start_offset + step;

    while (((end == DL_HEAD) && (offset <= last_offset)) ||
	   ((end == DL_TAIL) && (offset >= last_offset))) {
      Local_Neuroseg_Axis_Position(locseg, tmp_pos, offset);
      if (z_scale != 1.0) {
	tmp_pos[2] /= z_scale;
      }
      value = Stack_Point_Sampling(signal, tmp_pos[0], tmp_pos[1], tmp_pos[2]);
      if (!tz_isnan(value)) {
	if ((value > max_value) || (tz_isnan(max_value))) {
	  max_value = value;
	  pos[0] = tmp_pos[0];
	  pos[1] = tmp_pos[1];
	  pos[2] = tmp_pos[2];
	}
      }
      offset += step;
    }    
  }
}

static void validate_value(int *v, double r)
{
  if (*v < 0) {
    *v = 0;
  } else if(*v >= r) {
    *v = r - 1;
  }  
}

static void validate_position(int *pos, int width, int height, int depth)
{
  validate_value(pos, width);
  validate_value(pos + 1, height);
  validate_value(pos + 2, depth);
}

Int_Arraylist *Locseg_Chain_Shortest_Path_Pt(double *pos,
					     Locseg_Chain *target,
					     int start_index,
					     int end_index,
					     const Stack *signal, 
					     double z_scale,
					     Stack_Graph_Workspace *sgw)
{
  if (start_index < 0) {
    start_index = 0;
  }
  int length = Locseg_Chain_Length(target);
  if (end_index >= length) {
    end_index = length - 1;
  }

  int start_pos[3];
  start_pos[0] = iround(pos[0]);
  start_pos[1] = iround(pos[1]);
  start_pos[2] = iround(pos[2]);

  int end_pos[3];

  int seg_index = (start_index + end_index) / 2;
  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(target, seg_index);
  //Print_Local_Neuroseg(locseg);
  Local_Neuroseg_Center(locseg, pos);

  if (z_scale> 1.0) {
    pos[2] /= z_scale;
  }

  end_pos[0] = iround(pos[0]);
  end_pos[1] = iround(pos[1]);
  end_pos[2] = iround(pos[2]);

  /* alloc <ws> */
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  if (sgw->sp_option != 1) {
    if (sgw->group_mask == NULL) {
      sgw->group_mask = Make_Stack(GREY, signal->width, signal->height, 
	  signal->depth);
    }
    Zero_Stack(sgw->group_mask);
    ws->flag = 0;
    ws->value = 1;
    Locseg_Chain_Label_W(target, sgw->group_mask, 1.0, start_index, 
	end_index, ws);
    Stack_Graph_Workspace_Set_Range(sgw, start_pos[0], ws->range[0], 
	start_pos[1], ws->range[1], 
	start_pos[2], ws->range[2]);
    Stack_Graph_Workspace_Update_Range(sgw, ws->range[3], ws->range[4], 
	ws->range[5]); 
  } else {
    Stack_Graph_Workspace_Set_Range(sgw, start_pos[0], end_pos[0], 
	start_pos[1], end_pos[1], start_pos[2], end_pos[2]);
  }

  Stack_Graph_Workspace_Expand_Range(sgw, 10, 10, 10, 10, 10, 10);
  Stack_Graph_Workspace_Validate_Range(sgw, signal->width, signal->height,
                                       signal->depth);

  validate_position(start_pos, signal->width, signal->height, signal->depth);
  validate_position(end_pos, signal->width, signal->height, signal->depth);

  /* alloc <offset_path> */
  Int_Arraylist *offset_path = NULL;
  if (fabs((end_pos[2] - start_pos[2]) * (end_pos[1] - start_pos[1]) * 
        (end_pos[0] - start_pos[0]) * 1.0) < 100000.0) {
    offset_path = Stack_Route(signal, start_pos, end_pos, sgw);
  } else {
    Stack *mask = Make_Stack(GREY, Stack_Width(signal), Stack_Height(signal),
        Stack_Depth(signal));
    Zero_Stack(mask);
    Sp_Grow_Workspace *tmpsgw = New_Sp_Grow_Workspace();
    tmpsgw->size = Stack_Voxel_Number(signal);

    Sp_Grow_Workspace_Set_Mask(tmpsgw, mask->array);
    tmpsgw->wf = Stack_Voxel_Weight_S;
    Stack_Sp_Grow_Infer_Parameter(tmpsgw, signal);

    Set_Stack_Pixel(mask, iround(start_pos[0]), iround(start_pos[1]), 
        iround(start_pos[2]), 0, 2);
    Set_Stack_Pixel(mask, iround(end_pos[0]), iround(end_pos[1]), 
        iround(end_pos[2]), 0, 1);
    offset_path = Stack_Sp_Grow(signal, NULL, 0, NULL, 0, tmpsgw);

    Kill_Sp_Grow_Workspace(tmpsgw);
    Kill_Stack(mask);
    /*
    Locseg_Chain_Label_G(source, &mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);
    Locseg_Chain_Label_G(target, &mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);	    
    */
  }

  /* free <ws> */
  Kill_Locseg_Label_Workspace(ws);

  if (offset_path == NULL) {
    return NULL;
  }

  Int_Arraylist *path = Make_Int_Arraylist(0, 1);  

  int i;
  int nvoxel = Stack_Voxel_Number(signal);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      Int_Arraylist_Add(path, index);
    }
  }

  /* free <offset_path> */
  Kill_Int_Arraylist(offset_path);

#ifdef _DEBUG_2
  if (sgw->signal_mask != NULL) {
    if (sgw->signal_mask->kind != GREY) {
      printf("here\n");
    }
  }
#endif

  return path;  
}

static void locseg_chain_point_range(Locseg_Chain *target, int index,
				     double *pos, double dist, 
				     int *start, int *end)
{
  Locseg_Chain_Iterator_Locate(target, index);
  Local_Neuroseg *locseg = Locseg_Chain_Prev_Seg(target);
  double current[3], extend[3];
  Coordinate_3d_Copy(current, pos);
  Local_Neuroseg_Bottom(locseg, extend);
  
  *start = index;
  double d = Coordinate_3d_Distance(current, extend);
  while (d < dist) {
    (*start)--;
    locseg = Locseg_Chain_Prev_Seg(target);
    if (locseg != NULL) {
      Coordinate_3d_Copy(current, pos);
      Local_Neuroseg_Bottom(locseg, extend);
      d += Coordinate_3d_Distance(current, extend);
    } else {
      break;
    }
  }

  Locseg_Chain_Iterator_Locate(target, index);
  locseg = Locseg_Chain_Next_Seg(target);
  Coordinate_3d_Copy(current, pos);
  Local_Neuroseg_Top(locseg, extend);
  
  *end = index;
  d = Coordinate_3d_Distance(current, extend);
  while (d < dist) {
    (*end)++;
    locseg = Locseg_Chain_Next_Seg(target);
    if (locseg != NULL) {
      Coordinate_3d_Copy(current, pos);
      Local_Neuroseg_Top(locseg, extend);
      d += Coordinate_3d_Distance(current, extend);
    } else {
      break;
    }
  }
  
}

void Locseg_Chain_Update_Stack_Graph_Workspace(const Local_Neuroseg *source,
					       Locseg_Chain *target,
					       const Stack *signal,
					       double z_scale,
					       Stack_Graph_Workspace *sgw)
{
  double pos[3];
  Local_Neuroseg_Center(source, pos);

  int seg_index;
  double skel_pos[3];
  Locseg_Chain_Point_Dist(target, pos, &seg_index, skel_pos);

  int start, end;
  locseg_chain_point_range(target, seg_index, skel_pos, 
			   NEUROSEG_DEFAULT_H * 2.5, 
			   &start, &end);

  //int start, end;
  //start = seg_index - 1;
  //end = seg_index + 2;
  
  if (start < 0) {
    start = 0;
  }
  int length = Locseg_Chain_Length(target);
  if (end >= length) {
    end = length - 1;
  }

  /* alloc <ws> */
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();

  if (sgw->sp_option != 1) {
    if (sgw->group_mask == NULL) {
      sgw->group_mask = Make_Stack(GREY, signal->width, signal->height, 
	  signal->depth);
    }
    Zero_Stack(sgw->group_mask);
    ws->flag = 0;
    ws->value = 1;
    Locseg_Chain_Label_W(target, sgw->group_mask, 1.0, start, end, ws);
  }

  Stack_Graph_Workspace_Set_Range(sgw, (int) pos[0], ws->range[0], 
				  (int) pos[1], ws->range[1], 
				  (int) pos[2], ws->range[2]);
  Stack_Graph_Workspace_Update_Range(sgw, ws->range[3], ws->range[4], 
				     ws->range[5]);   
  sgw->wf = Stack_Voxel_Weight_S;

  if (tz_isnan(sgw->argv[3]) || tz_isnan(sgw->argv[4])) {
    int option = 0;
    if (!isnan(sgw->argv[5])) { /* option for how to find threshold */
      option = iround(sgw->argv[5]);
    }

    switch (option) {
    case 0: 
      {
	Stack *substack = Crop_Stack(signal, sgw->range[0], sgw->range[2], 
				     sgw->range[4], 
				     sgw->range[1] - sgw->range[0] + 1, 
				     sgw->range[3] - sgw->range[2] + 1,
				     sgw->range[5] - sgw->range[4] + 1, NULL);
	int *hist = NULL;
	if (sgw->signal_mask != NULL) {
	  Stack *submask = 
	    Crop_Stack(sgw->signal_mask, 
		       sgw->range[0], sgw->range[2], sgw->range[4], 
		       sgw->range[1] - sgw->range[0] + 1, 
		       sgw->range[3] - sgw->range[2] + 1,
		       sgw->range[5] - sgw->range[4] + 1, NULL);
	  hist = Stack_Hist_M(substack, submask);
	  Kill_Stack(submask);
	} else {
	   hist = Stack_Hist(substack);
	}

	double c1, c2;
	int thre = Hist_Rcthre_R(hist, Int_Histogram_Min(hist), 
				 Int_Histogram_Max(hist), &c1, &c2);
	free(hist);
	sgw->argv[3] = thre;
	sgw->argv[4] = c2 - c1;
	if (sgw->argv[4] < 1.0) {
	  sgw->argv[4] = 1.0;
	}
	sgw->argv[4] /= 9.2;
	Kill_Stack(substack);
      }
      break;

    case 1:
      {
	Stack_Fit_Score fs;
	fs.n = 1;
	fs.options[0] = STACK_FIT_MEAN_SIGNAL;
	double inner =Local_Neuroseg_Score(source, signal, z_scale, &fs);
	fs.options[0] = STACK_FIT_OUTER_SIGNAL;
	double outer =Local_Neuroseg_Score(source, signal, z_scale, &fs);
      
	if (tz_isnan(sgw->argv[3])) {
	  sgw->argv[3] = inner * 0.1 + outer * 0.9;
	}
      
	if (tz_isnan(sgw->argv[4])) {
	  sgw->argv[4] = (inner - outer) / 4.6 * 1.8;
	}
      }
      break;

    default:
      break;
    }
  }

  /* free <ws> */
  Kill_Locseg_Label_Workspace(ws);
}

Int_Arraylist *Locseg_Chain_Shortest_Path_Seg(const Local_Neuroseg *source,
					      Locseg_Chain *target,
					      const Stack *signal, 
					      double z_scale,
					      Stack_Graph_Workspace *sgw)
{
  double pos[3];
  Local_Neuroseg_Center(source, pos);
  
  int seg_index;
  double skel_pos[3];
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index, skel_pos);
  if (dist > 200.0) {
    return NULL;
  }
  
  int start_index = 0;
  int end_index = 0;
  locseg_chain_point_range(target, seg_index, skel_pos, 
			   NEUROSEG_DEFAULT_H * 2.5, 
			   &start_index, &end_index);

  Int_Arraylist *path = 
    Locseg_Chain_Shortest_Path_Pt(pos, target, start_index, end_index, 
				  signal, z_scale, sgw);

  return path;
}

Int_Arraylist *Locseg_Chain_Shortest_Path(Locseg_Chain *source, 
					  Locseg_Chain *target,
					  const Stack *signal, double z_scale,
					  Stack_Graph_Workspace *sgw)
{
  double pos[3];
  Locseg_Chain_Bright_End(source, DL_HEAD, signal, 1.0, pos);
  if (z_scale != 1.0) {
    pos[2] /= z_scale;
  }

  int seg_index;
  double skel_pos[3];
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index, skel_pos);

  double tmp_pos[3];
  int tmp_seg_index;
  
  Locseg_Chain_Bright_End(source, DL_TAIL, signal, 1.0, tmp_pos);
  if (z_scale != 1.0) {
    tmp_pos[2] /= z_scale;
  }
  double tmp_skel_pos[3];
  double tmp_dist = Locseg_Chain_Point_Dist(target, tmp_pos, &tmp_seg_index, 
					    tmp_skel_pos);

  Local_Neuroseg *source_seg = Locseg_Chain_Head_Seg(source);

  if (tmp_dist < dist) {
    dist = tmp_dist;
    seg_index = tmp_seg_index;
    pos[0] = tmp_pos[0];
    pos[1] = tmp_pos[1];
    pos[2] = tmp_pos[2];

    skel_pos[0] = tmp_skel_pos[0];
    skel_pos[1] = tmp_skel_pos[1];
    skel_pos[2] = tmp_skel_pos[2];
    
    source_seg = Locseg_Chain_Tail_Seg(source);
  }

  if (!IS_IN_CLOSE_RANGE3(pos[0], pos[1], pos[2], 
			  0, Stack_Width(signal) - 1, 
			  0, Stack_Height(signal) - 1, 
			  0, Stack_Depth(signal) - 1)) {
    return NULL;
  }

  /*
  if (dist > 200.0) {
    return NULL;
  }
  */
  
#ifdef _DEBUG_
  if (sgw->signal_mask != NULL) {
    if (sgw->signal_mask->kind != GREY) {
      printf("here\n");
    }
  }
#endif

  Locseg_Chain_Update_Stack_Graph_Workspace(source_seg, target, signal,
  					    1.0, sgw);
  if (tz_isnan(sgw->argv[3])) {
    double tmpc;
    double c2 = Locseg_Chain_Min_Seg_Score(source, signal, z_scale,
	STACK_FIT_MEAN_SIGNAL);
    tmpc = Locseg_Chain_Min_Seg_Score(target, signal, z_scale,
	STACK_FIT_MEAN_SIGNAL);
    if (tmpc < c2) {
     c2 = tmpc;
    } 
    double c1 = Locseg_Chain_Min_Seg_Score(source, signal, z_scale,
	STACK_FIT_OUTER_SIGNAL);
    tmpc = Locseg_Chain_Min_Seg_Score(target, signal, z_scale,
	STACK_FIT_OUTER_SIGNAL);
    if (tmpc < c1) {
     c1 = tmpc;
    } 
    sgw->argv[3] = (c1 + c2) / 2.0;
    if (tz_isnan(sgw->argv[4])) {
      sgw->argv[4] = c2 - c1;
      if (sgw->argv[4] < 1.0) {
	sgw->argv[4] = 1.0;
      }
      sgw->argv[4] /= 9.2;
    }
  }

  int start_index = 0;
  int end_index = 0;
  locseg_chain_point_range(target, seg_index, skel_pos, 
			   NEUROSEG_DEFAULT_H * 2.5, 
			   &start_index, &end_index);

  return Locseg_Chain_Shortest_Path_Pt(pos, target, start_index,
				       end_index, signal, z_scale,
				       sgw);
}

Int_Arraylist* Locseg_Chain_Skel_Shortest_Path_Pt(double *pos,
						  Locseg_Chain *target,
						  int start_index,
						  int end_index,
						  const Stack *signal, 
						  double z_scale,
						  Stack_Graph_Workspace *sgw)
{
  /* alloc <ws> */
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  
  if (sgw->group_mask == NULL) {
    sgw->group_mask = Make_Stack(GREY, signal->width, signal->height, 
				 signal->depth);
  }
  Zero_Stack(sgw->group_mask);

  int start, end;
  start = start_index;
  end = end_index;
  
  if (start < 0) {
    start = 0;
  }
  int length = Locseg_Chain_Length(target);
  if (end >= length) {
    end = length - 1;
  }

  ws->flag = 0;
  ws->value = 1;
  ws->option = 10;
  Locseg_Chain_Label_W(target, sgw->group_mask, 1.0, start, end, ws);

  Stack_Graph_Workspace_Set_Range(sgw, (int) pos[0], ws->range[0], 
				  (int) pos[1], ws->range[1], 
				  (int) pos[2], ws->range[2]);
  Stack_Graph_Workspace_Update_Range(sgw, ws->range[3], ws->range[4], 
				     ws->range[5]); 
  
  int start_pos[3];
  int end_pos[3];
  
  start_pos[0] = (int) pos[0];
  start_pos[1] = (int) pos[1];
  start_pos[2] = (int) pos[2];
  
  int seg_index = (start_index + end_index) / 2;
  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(target, seg_index);
  //Print_Local_Neuroseg(locseg);
  Local_Neuroseg_Center(locseg, pos);

  if (z_scale> 1.0) {
    pos[2] /= z_scale;
  }

  end_pos[0] = iround(pos[0]);
  end_pos[1] = iround(pos[1]);
  end_pos[2] = iround(pos[2]);

  /* alloc <offset_path> */
  Int_Arraylist *offset_path = 
    Stack_Route(signal, start_pos, end_pos, sgw);

  Int_Arraylist *path = Make_Int_Arraylist(0, 1);  

  int i;
  int nvoxel = Stack_Voxel_Number(signal);

  for (i = 0; i < offset_path->length; i++) {
    int index = offset_path->array[i];
    if ((index < nvoxel) && (index >= 0)) {
      Int_Arraylist_Add(path, index);
    }
  }

  /* free <offset_path> */
  Kill_Int_Arraylist(offset_path);

  /* free <ws> */
  Kill_Locseg_Label_Workspace(ws);

  return path;  
}

Int_Arraylist*
Locseg_Chain_Skel_Shortest_Path_Seg(const Local_Neuroseg *source,
				    Locseg_Chain *target,
				    const Stack *signal, 
				    double z_scale,
				    Stack_Graph_Workspace *sgw)
{
  double pos[3];
  Local_Neuroseg_Center(source, pos);
  
  int seg_index;
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index, NULL);
  if (dist > 200.0) {
    return NULL;
  }

  Int_Arraylist *path = 
    Locseg_Chain_Skel_Shortest_Path_Pt(pos, target, seg_index - 1, 
				       seg_index + 2, signal, z_scale, sgw);
  
  return path;  
}

Int_Arraylist *Locseg_Chain_Skel_Shortest_Path(Locseg_Chain *source, 
					       Locseg_Chain *target,
					       const Stack *signal, 
					       double z_scale,
					       Stack_Graph_Workspace *sgw)
{
  double pos[3];
  Locseg_Chain_Bright_End(source, DL_HEAD, signal, 1.0, pos);
  
  if (z_scale != 1.0) {
    pos[2] /= z_scale;
  }

  int seg_index;
  double dist = Locseg_Chain_Point_Dist(target, pos, &seg_index, NULL);

  double tmp_pos[3];
  int tmp_seg_index;
  
  Locseg_Chain_Bright_End(source, DL_TAIL, signal, 1.0, tmp_pos);
  if (z_scale != 1.0) {
    tmp_pos[2] /= z_scale;
  }
  double tmp_dist = Locseg_Chain_Point_Dist(target, tmp_pos, &tmp_seg_index,
					    NULL);

  Local_Neuroseg *source_seg = Locseg_Chain_Head_Seg(source);

  if (tmp_dist < dist) {
    dist = tmp_dist;
    seg_index = tmp_seg_index;
    pos[0] = tmp_pos[0];
    pos[1] = tmp_pos[1];
    pos[2] = tmp_pos[2];
    //source_seg = Locseg_Chain_Tail_Seg(source);
  }

  if (!IS_IN_CLOSE_RANGE3(pos[0], pos[1], pos[2], 
			  0, Stack_Width(signal) - 1, 
			  0, Stack_Height(signal) - 1, 
			  0, Stack_Depth(signal) - 1)) {
    return NULL;
  }

  if (dist > 200.0) {
    return NULL;
  }
  
  Locseg_Chain_Update_Stack_Graph_Workspace(source_seg, target, signal,
  					    1.0, sgw);
  if (tz_isnan(sgw->argv[3])) {
    sgw->argv[3] = Locseg_Chain_Min_Seg_Signal(source, signal, z_scale);
  }

  return Locseg_Chain_Skel_Shortest_Path_Pt(pos, target, seg_index - 1,
					    seg_index + 2, signal, z_scale,
					    sgw);
}


int Locseg_Chain_Interpolate_L(Locseg_Chain *chain, const double *pt, 
			       const double *ort, double *new_pos)
{
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);

  int n = Locseg_Chain_Knot_Array_Length(ka);

  if (n == 1) {
    return 0;
  }

  double dist;
  double min_dist = Infinity;
  int min_index;
  int i;
  int index = -1;

  double start_pos[3], end_pos[3];
  double insertion[3];
  double lambda1, lambda2;
  double lambda = 0.0;

  if (ort == NULL) { /* no orientation information */
    min_index = 0;
    Locseg_Chain_Knot_Pos(ka, 0, start_pos);
    Locseg_Chain_Knot_Pos(ka, 1, end_pos);

    min_dist = Geo3d_Point_Lineseg_Dist(pt, start_pos, end_pos, &lambda1);
    lambda = lambda1;

    for (i = 2; i < n; i++) {
      start_pos[0] = end_pos[0];
      start_pos[1] = end_pos[1];
      start_pos[2] = end_pos[2];
      Locseg_Chain_Knot_Pos(ka, i, end_pos);
      dist = Geo3d_Point_Lineseg_Dist(pt, start_pos, end_pos, &lambda1);

      if (dist < min_dist) {
	min_dist = dist;
	min_index = i - 1;
	lambda = lambda1;
      }
    }
  } else {
    double start[3], end[3];

    for (i = 0; i < 3; i++) {
      start[i] = pt[i] - ort[i] * 5.0;
      end[i] = pt[i] + ort[i] * 5.0;
    }

    min_index = 0;

    Locseg_Chain_Knot_Pos(ka, 0, start_pos);
    Locseg_Chain_Knot_Pos(ka, 1, end_pos);

    int cond;
    min_dist = Geo3d_Lineseg_Lineseg_Dist(start_pos, end_pos, 
					  start, end, &lambda1, &lambda2, 
					  &cond);
    
    lambda = lambda1;

    for (i = 2; i < n; i++) {
      start_pos[0] = end_pos[0];
      start_pos[1] = end_pos[1];
      start_pos[2] = end_pos[2];
      Locseg_Chain_Knot_Pos(ka, i, end_pos);
      dist = Geo3d_Lineseg_Lineseg_Dist(start_pos, end_pos, 
					start, end, &lambda1, &lambda2, &cond);

      if (dist < min_dist) {
	min_dist = dist;
	min_index = i - 1;
	lambda = lambda1;
      }
    }  
  }

  if ((lambda > 0.0) && (lambda < 1.0)) {
    Locseg_Chain_Knot_Pos(ka, min_index, start_pos);
    Locseg_Chain_Knot_Pos(ka, min_index + 1, end_pos);
    double len = Geo3d_Dist(start_pos[0], start_pos[1], start_pos[2],
			    end_pos[0], end_pos[1], end_pos[2]);
    if ((len * lambda < 1.0) && (lambda <= 0.5)) {
      lambda = 0.0;
    } else if ((len * (1.0 - lambda) < 1.0) && (lambda >= 0.5)) {
      lambda = 1.0;
    } else {
      if (min_index == 0) {
	if (len * lambda < 3.0) {
	  lambda = 0.0;
	}
      } else if (min_index == Locseg_Chain_Knot_Array_Length(ka) - 2) {
	if (len * (1.0 - lambda) < 3.0) {
	  lambda = 1.0;
	}
      }
    }
  }  

  if ((lambda > 0.0) && (lambda < 1.0)) {
    Geo3d_Lineseg_Break(start_pos, end_pos, lambda, insertion);
    Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, min_index);
    Locseg_Chain_Knot *next_knot = 
      Locseg_Chain_Knot_Array_At(ka, min_index + 1);

    Local_Neuroseg *locseg = NULL;
    if (next_knot->id == knot->id) { /* break in the same locseg*/
      TZ_ASSERT((knot->offset == 0.0) || (next_knot->offset == 1.0), 
		"Invalid knots");
      Local_Neuroseg *prev_locseg = Locseg_Chain_Peek_Seg_At(chain, knot->id);
      double h = prev_locseg->seg.h;
      if (h > 1.0) {
	double alpha = 
	  knot->offset * (1.0 - lambda) + next_knot->offset * lambda;
	locseg = Copy_Local_Neuroseg(prev_locseg);
	Local_Neuroseg_Chop(prev_locseg, alpha);
	Local_Neuroseg_Chop(locseg, -alpha);

	Locseg_Node *node = Locseg_Chain_Peek_At(chain, knot->id);
	if (node->tr == NULL) {
	  node->tr = New_Trace_Record();
	}
	Trace_Record *tr = New_Trace_Record();
	if (knot->offset == 0.0) {	  
	  Trace_Record_Set_Fix_Point(node->tr, 1.0);
	  Trace_Record_Set_Fix_Point(tr, next_knot->offset * (1.0 - alpha) / 
				     (1.0 - alpha * next_knot->offset));
	} else if (next_knot->offset == 1.0) {
	  Trace_Record_Set_Fix_Point(node->tr, knot->offset / 
				     (knot->offset + 
				      alpha * (1.0 - knot->offset)));
	  Trace_Record_Set_Fix_Point(tr, 0.0);
	} else {
	  TZ_ERROR(ERROR_DATA_VALUE);
	}
	  
	Locseg_Chain_Insert(chain, locseg, tr, knot->id + 1);
	index = knot->id + 1;
      }
    } else { /* break in different segments */      
      Locseg_Node *node1 = Locseg_Chain_Peek_At(chain, knot->id);
      Locseg_Node *node2 = Locseg_Chain_Peek_At(chain, next_knot->id);

      double r1 = Neuroseg_Ry_T(&(node1->locseg->seg), knot->offset);
      double r2 = Neuroseg_Ry_T(&(node2->locseg->seg), next_knot->offset);

      //locseg = Copy_Local_Neuroseg(Locseg_Chain_Peek_Seg_At(chain, knot->id));
      locseg = Copy_Local_Neuroseg(node1->locseg);
      Set_Neuroseg_Position(locseg, insertion, NEUROSEG_BOTTOM);
      locseg->seg.h = 1.0;
      if (r1 != r2) {
	locseg->seg.r1 = r1 * (1.0 - lambda) + r2 * lambda;
      }

      //if (next_knot->id - knot->id >= 1) { /* adjacent segments */
	//Locseg_Node *tmp_node = Locseg_Chain_Peek_At(chain, next_knot->id);
	Locseg_Node_Set_Fix_Point(node2, next_knot->offset);
	//}

      Trace_Record *tr = New_Trace_Record();
      Trace_Record_Set_Fix_Point(tr, 0.0);
      Locseg_Chain_Insert(chain, locseg, tr, next_knot->id);
      index = next_knot->id;
    }

    if (new_pos != NULL) {
      Local_Neuroseg_Bottom(locseg, new_pos);
    }
  } else { /* break on knot, update new_pos */
    if (new_pos != NULL) {
      if (lambda == 0.0) {
	 Locseg_Chain_Knot_Pos(ka, min_index, new_pos);
      } else {
	 Locseg_Chain_Knot_Pos(ka, min_index + 1, new_pos);
      }
    }
  }

  Kill_Locseg_Chain_Knot_Array(ka);

  return index;
}

void Locseg_Chain_Translate(Locseg_Chain *chain, const double *offset)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *locseg = NULL;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Translate(locseg, offset);
  }
}

BOOL Locseg_Chain_Break_Node(Locseg_Chain *chain, int index, double t)
{
  Locseg_Node *node = Locseg_Chain_Peek_At(chain, index);
  
  if ((node->locseg->seg.h <= 1.5) || (t <= 0.0) || (t >= 1.0)) {
    return FALSE;
  }

  Local_Neuroseg *locseg = Copy_Local_Neuroseg(node->locseg);
  Local_Neuroseg_Chop(locseg, t);
  Local_Neuroseg_Chop(node->locseg, t - 1.0);

  Trace_Record *tr = NULL;

  if (node->tr != NULL) {
    New_Trace_Record();
    Trace_Record_Set_Direction(tr, Trace_Record_Direction(node->tr));

    if (Trace_Record_Direction(node->tr) != DL_BOTHDIR) {
      double fix_point = Trace_Record_Fix_Point(node->tr);
      if (fix_point >= 0) {
	if (fix_point <= t) {
	  fix_point /= t;
	  Trace_Record_Set_Fix_Point(tr, fix_point);
	} else {
	  fix_point -= t;
	  fix_point /= 1.0 - t;
	  Trace_Record_Set_Fix_Point(node->tr, fix_point);
	}
      }
    }
  }
  
  Locseg_Chain_Insert(chain, locseg, tr, index);

  return TRUE;
}

int Locseg_Chain_Remove_Seed(Locseg_Chain *chain)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Node *node = NULL;
  
  int removed = 0;

  while ((node = Locseg_Chain_Next(chain)) != NULL) {
    if (Trace_Record_Direction(node->tr) == DL_BOTHDIR) {
      Locseg_Chain_Prev(chain);
      Locseg_Chain_Remove_Current(chain);
      removed++;
    }
  }

  return removed;
}

/* This routine is mainly designed for pushing. */
void Locseg_Chain_Adjust_Seed(Locseg_Chain *chain)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Node *node = NULL;
  
  Locseg_Node *prev_node = NULL;

  while ((node = Locseg_Chain_Next(chain)) != NULL) {
    if (Trace_Record_Direction(node->tr) == DL_BOTHDIR) {
      Locseg_Node *next_node = Locseg_Chain_Peek(chain);
      double pos[3];
      if (next_node == NULL) { /* the seed is the tail */
        if (prev_node != NULL) {
          Local_Neuroseg_Top(prev_node->locseg, pos);
          Local_Neuroseg_Change_Bottom(node->locseg, pos);
        }
      } else if (prev_node == NULL) {
        Local_Neuroseg_Bottom(next_node->locseg, pos);
	Local_Neuroseg_Change_Top(node->locseg, pos);
      }
    }
    prev_node = node;
  }  
}

BOOL Locseg_Chain_Form_Loop(Locseg_Chain *chain, Local_Neuroseg *locseg,
			    Dlist_Direction_e direction) 
{
  double center[3];
  Local_Neuroseg_Center(locseg, center);
  if (direction == DL_FORWARD) {
    Locseg_Chain_Iterator_Start(chain, DL_TAIL);
    Locseg_Node *node = Locseg_Chain_Prev(chain);
    while ((node = Locseg_Chain_Prev(chain)) != NULL) {
      if (Local_Neuroseg_Hit_Test(node->locseg, 
				  center[0], center[1], center[2]) > 0) {
	double bottom[3];
	Local_Neuroseg_Bottom(locseg, bottom);
	if (Local_Neuroseg_Hit_Test(node->locseg, 
				    bottom[0], bottom[1], bottom[2]) == 0) {
	  return TRUE;
	} else if (Neuroseg_Angle_Between(&(locseg->seg), 
					  &(node->locseg->seg))
		   > TZ_PI_2) {
	  return TRUE;
	}
      }
    }
  } else if (direction == DL_BACKWARD) {
    Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    Locseg_Node *node = Locseg_Chain_Next(chain);
    while ((node = Locseg_Chain_Next(chain)) != NULL) {
      if (Local_Neuroseg_Hit_Test(node->locseg, 
				  center[0], center[1], center[2]) > 0) {
	double top[3];
	Local_Neuroseg_Top(locseg, top);
	if (Local_Neuroseg_Hit_Test(node->locseg, 
				    top[0], top[1], top[2]) == 0) {
	  return TRUE;
	} else if (Neuroseg_Angle_Between(&(locseg->seg), 
					  &(node->locseg->seg))
		   > TZ_PI_2) {
	  return TRUE;
	}
      }
    }
  }

  return FALSE;
}

Locseg_Chain* Locseg_Chain_From_Skel(int *skel, int n, double sr, double ss,
				     const Stack *signal, double z_scale,
				     Locseg_Fit_Workspace *fw)
{
  if (n <= 0) {
    return NULL;
  }

  int coord[3];
  double pos[3];
  double start_pos[3];
  double theta, psi;
  BOOL first = TRUE;
  Locseg_Fit_Workspace tmp_fw;
  const double dist2_thre = 
    (NEUROSEG_DEFAULT_H - 1.0) * (NEUROSEG_DEFAULT_H - 1.0);

  double dist2 = 0.0;

  Stack_Util_Coord(skel[0], signal->width, signal->height, coord, coord + 1,
		   coord + 2);
  Set_Coordinate_3d(start_pos, coord[0], coord[1], coord[2]);

  /* for average orientation of the local path */
  double vec[3];
  Set_Coordinate_3d(vec, 0.0, 0.0, 0.0);

  Locseg_Chain *bridge = New_Locseg_Chain();
  int i;
  for (i = 0; i < n; i++) {
    Stack_Util_Coord(skel[i], signal->width, signal->height, 
		     coord, coord + 1, coord + 2);
    Set_Coordinate_3d(pos, coord[0], coord[1], coord[2]);
    dist2 = Geo3d_Dist_Sqr(start_pos[0], start_pos[1], start_pos[2],
			   pos[0], pos[1], pos[2]);
    if (dist2 < dist2_thre) {
      vec[0] += pos[0] - start_pos[0];
      vec[1] += pos[1] - start_pos[1];
      vec[2] += pos[2] - start_pos[2];
    } else {
#ifdef _DEBUG_
      if ((vec[0] == 0.0) && (vec[1] == 0.0) && (vec[2] == 0.0)) {
	printf("stop here: 0 vec.\n");
      }
#endif
      
      vec[0] += pos[0] - start_pos[0];
      vec[1] += pos[1] - start_pos[1];
      vec[2] += pos[2] - start_pos[2];

      Geo3d_Coord_Orientation(vec[0], vec[1], vec[2], &theta, &psi);
      Local_Neuroseg *new_seg = New_Local_Neuroseg();
      Set_Local_Neuroseg(new_seg, sr, 0.0, sqrt(dist2) + 1.0, 
			 theta, psi, 0.0, 0.0, ss, 0.0, 0.0, 0.0);
      dist2 = 0.0;
      Set_Neuroseg_Position(new_seg, start_pos, NEUROSEG_BOTTOM);

      if ((sr > 0.0) && (ss > 0.0)) {
	if (fw != NULL) {
	  if (first) {
	    Locseg_Fit_Workspace_Copy(&tmp_fw, fw);
	    tmp_fw.nvar = 
	      Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
					       NEUROSEG_VAR_MASK_SCALE,
					       NEUROPOS_VAR_MASK_NONE,
					       tmp_fw.var_index);
	    first = FALSE;
	  }
	  Fit_Local_Neuroseg_W(new_seg, signal, z_scale, &tmp_fw);
	  if (new_seg->seg.r1 * sqrt(new_seg->seg.scale) + 1.0 >
	      1.5 * (sr * sqrt(ss) + 1.0)) { /* reject it if it is too big */
	    new_seg->seg.r1 = sr;
	    new_seg->seg.scale = ss;
	  } else {
	    sr = new_seg->seg.r1;
	    ss = new_seg->seg.scale;
	  }
	}
      }

      /* Add new segment */
      Trace_Record *tr = New_Trace_Record();
      Trace_Record_Set_Direction(tr, DL_FORWARD);
      Trace_Record_Set_Fix_Point(tr, 0.0);
      Locseg_Chain_Add(bridge, new_seg, tr, DL_TAIL);
      
      /* step forward */
      Local_Neuroseg_Center(new_seg, start_pos);
      Set_Coordinate_3d(vec, 0.0, 0.0, 0.0);
    }
  }

  /* last segment */
  Stack_Util_Coord(skel[n-1], signal->width, signal->height, 
		   coord, coord + 1, coord + 2);
  Set_Coordinate_3d(pos, coord[0], coord[1], coord[2]);
  
  Local_Neuroseg *last_seg = Locseg_Chain_Tail_Seg(bridge);
  Local_Neuroseg *new_seg = New_Local_Neuroseg();
  Set_Local_Neuroseg(new_seg, sr, 0.0, 1.0, 
		     0.0, 0.0, 0.0, 0.0, ss, 0.0, 0.0, 0.0);
  if (last_seg != NULL) {
    new_seg->seg.r1 = new_seg->seg.r1;
    new_seg->seg.scale = new_seg->seg.scale;
  }

  Set_Neuroseg_Position(new_seg, start_pos, NEUROSEG_BOTTOM);
  Local_Neuroseg_Change_Top(new_seg, pos);
 
  if ((sr > 0) && (new_seg->seg.h >= 5.0)) {
    if (fw != NULL) {
      if (first) {
	Locseg_Fit_Workspace_Copy(&tmp_fw, fw);
	tmp_fw.nvar = 
	  Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
					   NEUROSEG_VAR_MASK_SCALE,
					   NEUROPOS_VAR_MASK_NONE,
					   tmp_fw.var_index);
	first = FALSE;
      }
      Fit_Local_Neuroseg_W(new_seg, signal, z_scale, &tmp_fw);
    }
  } else {
    new_seg->seg.r1 = 0.0;
  }
   
  Trace_Record *tr = New_Trace_Record();
  Trace_Record_Set_Direction(tr, DL_FORWARD);
  Trace_Record_Set_Fix_Point(tr, 0.0);
  Locseg_Chain_Add(bridge, new_seg, tr, DL_TAIL);

  return bridge;  
}

Locseg_Chain* Locseg_Chain_From_Ball_Array(Geo3d_Ball *skel, int n,
					   const Stack *signal, double z_scale,
					   Locseg_Fit_Workspace *fw)
{
  if (n <= 1) {
    return NULL;
  }

  double theta, psi;
  BOOL first = TRUE;
  Locseg_Fit_Workspace tmp_fw;

  /* for average orientation of the local path */
  double vec[3];
  Set_Coordinate_3d(vec, 0.0, 0.0, 0.0);

  Locseg_Chain *bridge = New_Locseg_Chain();
  int i;
  double ss = 1.0;
  double sr = skel[0].r;
  for (i = 0; i < n - 1; i++) {
    vec[0] += skel[i+1].center[0] - skel[i].center[0];
    vec[1] += skel[i+1].center[1] - skel[i].center[1];
    vec[2] += skel[i+1].center[2] - skel[i].center[2];

    Geo3d_Coord_Orientation(vec[0], vec[1], vec[2], &theta, &psi);
    Local_Neuroseg *new_seg = New_Local_Neuroseg();
    double h;
    double tmpvec[3];
    tmpvec[0] = skel[i+1].center[0] - skel[i].center[0];
    tmpvec[1] = skel[i+1].center[1] - skel[i].center[1];
    tmpvec[2] = skel[i+1].center[2] - skel[i].center[2];
    h = Coordinate_3d_Norm(tmpvec);
    Set_Local_Neuroseg(new_seg, skel[i].r, 0.0, h, 
		       theta, psi, 0.0, 0.0, ss, 0.0, 0.0, 0.0);
    Set_Neuroseg_Position(new_seg, skel[i].center, NEUROSEG_BOTTOM);

    if ((fw != NULL) && (signal != NULL)) {
      if (first) {
	Locseg_Fit_Workspace_Copy(&tmp_fw, fw);
	tmp_fw.nvar = 
	  Local_Neuroseg_Var_Mask_To_Index(NEUROSEG_VAR_MASK_R |
					   NEUROSEG_VAR_MASK_SCALE,
					   NEUROPOS_VAR_MASK_NONE,
					   tmp_fw.var_index);
	first = FALSE;
      }
      Fit_Local_Neuroseg_W(new_seg, signal, z_scale, &tmp_fw);
      if (new_seg->seg.r1 * sqrt(new_seg->seg.scale) + 1.0 >
	  1.5 * (sr * sqrt(ss) + 1.0)) { /* reject it if it is too big */
	new_seg->seg.r1 = sr;
	new_seg->seg.scale = ss;
      } else {
	sr = new_seg->seg.r1;
	ss = new_seg->seg.scale;
      }
    }

    /* Add new segment */
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_FORWARD);
    Trace_Record_Set_Fix_Point(tr, 0.0);
    Locseg_Chain_Add(bridge, new_seg, tr, DL_TAIL);
    
    /* step forward */
    Set_Coordinate_3d(vec, 0.0, 0.0, 0.0);
  }

  return bridge;  
}

Locseg_Chain* Locseg_Chain_From_Sp_Grow(Stack *stack, double z_scale,
					Sp_Grow_Workspace *sgw)
{
  Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
  
  Locseg_Fit_Workspace *fw = New_Locseg_Fit_Workspace();
  Locseg_Chain *chain = Locseg_Chain_From_Skel(path->array, path->length,
					       3.0, 1.0, stack, z_scale,
					       fw);

  Kill_Locseg_Fit_Workspace(fw);
  Kill_Int_Arraylist(path);
  
  return chain;
}

Locseg_Chain* Locseg_Chain_Bridge_Sp_G(Locseg_Chain *source, 
				       Locseg_Chain *target,
				       Stack *stack, double z_scale,
				       Sp_Grow_Workspace *sgw)
{
  Stack mask;
  mask.kind = GREY;
  mask.width = stack->width;
  mask.height = stack->height;
  mask.depth = stack->depth;
  mask.array = sgw->mask;
  
  Locseg_Chain_Label_G(source, &mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);
  Locseg_Chain_Label_G(target, &mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);				   
  Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);

  int x, y, z;
  Stack_Util_Coord(path->array[0], stack->width, stack->height, &x, &y, &z);
  double pos[3];
  Set_Coordinate_3d(pos, x, y, z);

  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_Nearby(source, pos);
  double sr = locseg->seg.r1;
  double ss = locseg->seg.scale;
  
  Locseg_Fit_Workspace *fw = New_Locseg_Fit_Workspace();
  Locseg_Chain *chain = Locseg_Chain_From_Skel(path->array, path->length,
					       sr, ss, stack, z_scale,
					       fw);

  Kill_Locseg_Fit_Workspace(fw);
  Kill_Int_Arraylist(path);
  
  return chain;
}

Local_Neuroseg *Locseg_Chain_Peek_Seg_Nearby(Locseg_Chain *chain, 
					     const double *pos)
{
  int seg_index;
  Locseg_Chain_Point_Dist(chain, pos, &seg_index, NULL);

  return Locseg_Chain_Peek_Seg_At(chain, seg_index);
}

Neuron_Structure*
Locseg_Chain_Sp_Grow_Reconstruct(Locseg_Chain **chain_array,
				 int n, Stack *stack, double z_scale,
				 Sp_Grow_Workspace *sgw)
{
  Stack mask;
  mask.kind = GREY;
  mask.width = stack->width;
  mask.height = stack->height;
  mask.depth = stack->depth;
  mask.array = sgw->mask;
  
  /* alloc <id_stack> */
  Stack *id_stack = Make_Stack(GREY16, stack->width, stack->height, 
			       stack->depth);
  Zero_Stack(id_stack);

  int i;
  for (i = 0; i < n; i++) {
    Locseg_Chain_Label_G(chain_array[i], id_stack, 
			 1.0, 0, -1, 1.0, 0.0, 0, i + 1);
  }

  Locseg_Chain_Label_G(chain_array[0], &mask, 1.0, 0, -1, 1.0, 0.0, 0, 2);

  for (i = 1; i < n; i++) {
    Locseg_Chain_Label_G(chain_array[i], &mask, 1.0, 0, -1, 1.0, 0.0, 0, 1);
  }

  int ntarget = n - 1;
  /* alloc ns */
  Neuron_Structure *ns = Make_Neuron_Structure(n + n - 1);
  
  Neuron_Component nc;
  int tmpn;
  for (i = 0; i < n; i++) {
    Locseg_Chain_To_Neuron_Component(chain_array[i], 
				     NEUROCOMP_TYPE_LOCSEG_CHAIN, 
				     &nc, &tmpn);
    Neuron_Structure_Set_Component(ns, i, &nc);
  }
    
  /* alloc <fw> */
  Locseg_Fit_Workspace *fw = New_Locseg_Fit_Workspace();

  int last_id = n;
  Neurocomp_Conn conn;
  Default_Neurocomp_Conn(&conn);
  Locseg_Chain *source_chain = NULL;
  //Locseg_Chain *target_chain = NULL;
  BOOL bridge;

  while (ntarget > 0) {
    bridge = TRUE;

    /* alloc <path> */
    Int_Arraylist *path = Stack_Sp_Grow(stack, NULL, 0, NULL, 0, sgw);
#ifdef _DEBUG_
    if (path->length == 1) {
      printf("stop here: one-voxel path\n");
    }
#endif

    if (path->length > 0) {
      if (path->length > 20) { /* need modification */
	int k;
	int fg_count = 0;
	for (k = 0; k < path->length; k++) {
	  if (Stack_Array_Value(stack, path->array[k]) > 0) {
	    fg_count++;
	  }
	}
	
#ifdef _DEBUG_
	printf("%g\n", (double) fg_count / path->length);
#endif

	if ((double)path->length * sgw->fgratio > fg_count) {
	  bridge = FALSE;
	}
      }
  
      int source_id = Stack_Array_Value(id_stack, path->array[0]) - 1;
      int target_id = 
	Stack_Array_Value(id_stack, path->array[path->length-1]) - 1;

#ifdef _DEBUG_  
      if (source_id == target_id) {
	printf("stop here\n");
      }
#endif

      if (bridge) {
	int x, y, z;
	Stack_Util_Coord(path->array[0], stack->width, stack->height, 
			 &x, &y, &z);
	double source_pos[3];
	Set_Coordinate_3d(source_pos, x, y, z);
	int source_seg_index;
	source_chain = 
	  NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, source_id));
	Locseg_Chain_Point_Dist(source_chain, source_pos,
				&source_seg_index, NULL);
	
	Stack_Util_Coord(path->array[path->length-1], 
			 stack->width, stack->height, &x, &y, &z);
	double target_pos[3];
	Set_Coordinate_3d(target_pos, x, y, z);
	int target_seg_index;
	Locseg_Chain_Point_Dist(chain_array[target_id], target_pos, 
				&target_seg_index, NULL);
	
	Local_Neuroseg *locseg = 
	  Locseg_Chain_Peek_Seg_Nearby(source_chain, target_pos);
	double sr = locseg->seg.r1;
	double ss = locseg->seg.scale;
	
	Locseg_Chain *new_chain = NULL;
	
	new_chain = Locseg_Chain_From_Skel(path->array, path->length,
					   sr, ss, stack, z_scale, fw);
	
	if (new_chain != NULL) {
	  Locseg_Chain_To_Neuron_Component(new_chain, 
					   NEUROCOMP_TYPE_LOCSEG_CHAIN, 
					   &nc, &tmpn);  
	  Neuron_Structure_Set_Component(ns, last_id, &nc);
	  
	  conn.info[0] = 0;
	  conn.info[1] = source_seg_index;
	  conn.cost = sgw->value;
	  Set_Coordinate_3d(conn.pos, source_pos[0], source_pos[1], source_pos[2]);
	  conn.mode = NEUROCOMP_CONN_HL;
	  Neurocomp_Conn_Translate_Mode(Locseg_Chain_Length(source_chain), &conn);
	  Neuron_Structure_Add_Conn(ns, last_id, source_id, &conn);
	  
	  conn.info[0] = 1;
	  conn.info[1] = target_seg_index;
	  conn.mode = NEUROCOMP_CONN_HL;
	  conn.cost = sgw->value;
	  Set_Coordinate_3d(conn.pos, target_pos[0], target_pos[1], target_pos[2]);
	  Neurocomp_Conn_Translate_Mode(Locseg_Chain_Length(source_chain), &conn);
	  Neuron_Structure_Add_Conn(ns, last_id, target_id, &conn);
	
	  Locseg_Chain_Label_G(new_chain, id_stack, 1.0, 0, -1, 1.0, 0.0, -1, 
			       last_id + 1);
	  Locseg_Chain_Label_G(new_chain, &mask, 1.0, 0, -1, 1.0, 0.0, -1, 2);
	}
      }

      Locseg_Chain_Label_G(chain_array[target_id], id_stack, 1.0, 0, -1, 1.0,
			   0.0, -1, target_id + 1);
      Locseg_Chain_Label_G(chain_array[target_id], &mask, 
			   1.0, 0, -1, 1.0, 0.0, 1, 2);  
    }

    Kill_Int_Arraylist(path);
  
    ntarget--;
    last_id++;
    
    printf("%d\n", ntarget);
  }

  /* free <fw> */
  Kill_Locseg_Fit_Workspace(fw);

  return ns;
}

void Locseg_Chain_Regularize(Locseg_Chain *chain)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  
  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg(chain);
  while (locseg != NULL) {
    if (locseg->seg.r1 * locseg->seg.scale <= 0.0) {
      Locseg_Chain_Remove_Current(chain);
    } else {
      Locseg_Chain_Next(chain);
    }
    locseg = Locseg_Chain_Peek_Seg(chain);
  }
}

Locseg_Chain** Locseg_Chain_Import_List(const char *file_path, int *n)
{
  FILE *fp = GUARDED_FOPEN(file_path, "r");
  String_Workspace *sw = New_String_Workspace();

  *n = 0;
  while (Read_Line(fp, sw) != NULL) {
    (*n)++;
  }

  Locseg_Chain **chain_array;
  GUARDED_MALLOC_ARRAY(chain_array, *n, Locseg_Chain*);

  *n = 0;
  fseek(fp, 0, SEEK_SET);
  
  char *chain_file = NULL;
  while ((chain_file = Read_Line(fp, sw)) != NULL) {
    if (fhasext(chain_file, "tb")) {
      chain_array[(*n)++] = Read_Locseg_Chain(chain_file);
    }
  }

  Kill_String_Workspace(sw);

  fclose(fp);

  return chain_array;
}

Locseg_Chain* Locseg_Chain_Break_At(Locseg_Chain *chain, int index)
{
  Locseg_Chain_Iterator_Locate(chain, index);
  Locseg_Node_Dlist *list = chain->iterator;

  Locseg_Chain *new_chain = NULL;

  if (list != NULL) {
    if (list->next != NULL) {
      new_chain = New_Locseg_Chain();
      new_chain->list = list->next;
      list->next->prev = NULL;
      list->next = NULL;
    }
  }

  return new_chain;
}

static int locseg_chain_maximum_turn(Locseg_Chain *chain, 
				     int index1, int index2)
{
  Locseg_Chain_Iterator_Locate(chain, index1 + 1);
  double maxangle = 0.0;
  int maxindex = index1 + 1;
  int count = index1 + 2;
  while (count < index2) {
    Local_Neuroseg *locseg1 = Locseg_Chain_Next_Seg(chain);
    Local_Neuroseg *locseg2 = Locseg_Chain_Peek_Seg(chain);
    
    double angle = Neuroseg_Angle_Between(&(locseg1->seg), &(locseg2->seg));
    if (angle > maxangle) {
      maxangle = angle;
      maxindex = count - 1;
    }
    count++;
  }
  
  return maxindex;
}

static void locseg_chain_switch_list(Locseg_Chain *chain1, Locseg_Chain *chain2)
{
  Locseg_Node_Dlist *tmplist;
  SWAP2(chain1->list, chain2->list, tmplist);
}

Locseg_Chain* Locseg_Chain_Break_Between(Locseg_Chain *chain, 
					 int index1, int index2)
{
  Locseg_Chain *new_chain = NULL;;

  BOOL switching = FALSE;
  if (index1 > index2) {
    int tmp;
    SWAP2(index1, index2, tmp);
    switching = TRUE;
  }

  if (index1 == index2) {
    if (index1 > 0) {
      index1--;
    } else {
      index2++;
    }
  } else if (index2 - index1 == 1) {
    new_chain = Locseg_Chain_Break_At(chain, index1);
  } else if (index2 - index1 == 2) {
    Locseg_Chain_Iterator_Locate(chain, index1 + 1);
    Locseg_Chain_Remove_Current(chain);
    new_chain = Locseg_Chain_Break_At(chain, index1);
  } else if (index2 - index1 == 3) {
      Locseg_Chain_Iterator_Locate(chain, index1 + 1);
      Locseg_Chain_Remove_Current(chain);
      Locseg_Chain_Remove_Current(chain);
      new_chain = Locseg_Chain_Break_At(chain, index1);
  } else {
    int index = locseg_chain_maximum_turn(chain, index1, index2);
    Locseg_Chain_Iterator_Locate(chain, index);
    Locseg_Chain_Remove_Current(chain);
    Locseg_Chain_Remove_Current(chain);
    new_chain = Locseg_Chain_Break_At(chain, index - 1);
  }

  if (switching) {
    locseg_chain_switch_list(new_chain, chain);
  }

  return new_chain;
}

double Locseg_Chain_Hit_Ratio(Locseg_Chain *source, Locseg_Chain *target)
{
  if ((source == NULL) || (target == NULL)) {
    return 0.0;
  }

  if (source == target) {
    return 1.0;
  }

  int n = 0;
  int total = 0;
  Locseg_Chain_Iterator_Start(source, DL_HEAD);
  
  Local_Neuroseg *locseg = Locseg_Chain_Next_Seg(source);
  if (locseg == NULL) {
    return 0.0;
  }

  double pos[3];
  Local_Neuroseg_Bottom(locseg, pos);
  if (Locseg_Chain_Hit_Test(target, DL_FORWARD, pos[0], pos[1], pos[2])) {
    n++;
  }
  Local_Neuroseg_Center(locseg, pos);
  if (Locseg_Chain_Hit_Test(target, DL_FORWARD, pos[0], pos[1], pos[2])) {
    n++;
  }

  total += 2;
  
  Local_Neuroseg *locseg2 = locseg;
  while ((locseg = Locseg_Chain_Next_Seg(source)) != NULL) {
    Local_Neuroseg_Center(locseg, pos);
    if (Locseg_Chain_Hit_Test(target, DL_FORWARD, pos[0], pos[1], pos[2])) {
      n++;
    }
    locseg2 = locseg;
    total++;
  }

  Local_Neuroseg_Top(locseg2, pos);
  if (Locseg_Chain_Hit_Test(target, DL_FORWARD, pos[0], pos[1], pos[2])) {
    n++;
  }  
  total++;

  return (double) n / total;
}

double Locseg_Chain_Hit_Ratio_Swc(Locseg_Chain *source, Swc_Tree *target)
{
  if ((source == NULL) || (target == NULL)) {
    return 0.0;
  }

  Swc_Tree_Iterator_Start(target, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  int n = 0;
  int total = 0;
  Locseg_Chain_Iterator_Start(source, DL_HEAD);
  
  Local_Neuroseg *locseg = Locseg_Chain_Next_Seg(source);
  if (locseg == NULL) {
    return 0.0;
  }

  double pos[3];
  Local_Neuroseg_Bottom(locseg, pos);
  if (Swc_Tree_Hit_Test(target, SWC_TREE_ITERATOR_NO_UPDATE, pos[0], pos[1], pos[2])) {
    n++;
  }
  Local_Neuroseg_Center(locseg, pos);
  if (Swc_Tree_Hit_Test(target, SWC_TREE_ITERATOR_NO_UPDATE, pos[0], pos[1], pos[2])) {
    n++;
  }

  total += 2;
  
  Local_Neuroseg *locseg2 = locseg;
  while ((locseg = Locseg_Chain_Next_Seg(source)) != NULL) {
    Local_Neuroseg_Center(locseg, pos);
    if (Swc_Tree_Hit_Test(target, SWC_TREE_ITERATOR_NO_UPDATE,
			  pos[0], pos[1], pos[2])) {
      n++;
    }
    locseg2 = locseg;
    total++;
  }

  Local_Neuroseg_Top(locseg2, pos);
  if (Swc_Tree_Hit_Test(target, SWC_TREE_ITERATOR_NO_UPDATE, 
			pos[0], pos[1], pos[2])) {
    n++;
  }  
  total++;

  return (double) n / total;
}

void Locseg_Chain_Extend(Locseg_Chain *chain, Dlist_End_e end, 
    const double *pos)
{
  TZ_ASSERT((end == DL_TAIL) || (end == DL_HEAD), "Invalid end.");

  const double *bottom, *top;
  double lastpos[3];
  Local_Neuroseg *locseg = NULL;
  Dlist_Direction_e direction = DL_UNKNOWN;
  Local_Neuroseg *lastseg = NULL;
  double fix_point = 0.0;

  if (Locseg_Chain_Length(chain) == 1) {
    Locseg_Node *node = Locseg_Chain_Tail(chain);
    lastseg = node->locseg;
    double tmpbot[3], tmptop[3];
    Local_Neuroseg_Bottom(lastseg, tmpbot);
    Local_Neuroseg_Top(lastseg, tmptop);

    /* flip the segment if the bottom is closer */
    if (Coordinate_3d_Distance(pos, tmpbot) < 
        Coordinate_3d_Distance(pos, tmptop)) {
      end = DL_HEAD;
    } else {
      end = DL_TAIL;
    }
  }

  if (end == DL_TAIL) {
    lastseg = Locseg_Chain_Tail_Seg(chain);
    Local_Neuroseg_Top(lastseg, lastpos);
    bottom = lastpos;
    top = pos;
    direction = DL_FORWARD;
  } else {
    lastseg = Locseg_Chain_Head_Seg(chain);
    Local_Neuroseg_Bottom(lastseg, lastpos);
    bottom = pos;
    top = lastpos;
    direction = DL_BACKWARD;
    fix_point = 1.0;
  }

  locseg = Copy_Local_Neuroseg(lastseg);
  Local_Neuroseg_Set_Bottom_Top(locseg, bottom, top);
  Trace_Record *tr = New_Trace_Record();
  Trace_Record_Set_Direction(tr, direction);
  Trace_Record_Set_Fix_Point(tr, fix_point);
  Locseg_Chain_Add(chain, locseg, tr, end);
}

static double local_neuroseg_zscore(const Local_Neuroseg *locseg)
{
  double pos[3];
  Local_Neuroseg_Bottom(locseg, pos);
  double zbottom = pos[2];
  Local_Neuroseg_Top(locseg, pos);
  double ztop = pos[2];

  return fabs(zbottom - ztop) / locseg->seg.h;
}

void Locseg_Chain_Array_Screen_Z(Locseg_Chain **chain, int n, double thre)
{
  int i;
  for (i = 0; i < n; i++) {
    if (!Locseg_Chain_Is_Empty(chain[i])) {
      if (Locseg_Chain_Length(chain[i]) == 1) {
	Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain[i]);
	if (local_neuroseg_zscore(locseg) > thre) {
	  Clean_Locseg_Chain(chain[i]);
	}
      }
    }
  }
}

static Neuron_Structure* locseg_chain_neurostruct(Locseg_Chain **chain, int n, 
    Connection_Test_Workspace *ws)
{
  Neuron_Structure *ns = Make_Neuron_Structure(n);

  int i, j;
  for (i = 0; i < n; i++) {
    for (j = i + 1; j < n; j++) {
      if ((Locseg_Chain_Is_Empty(chain[i]) == FALSE) &&
	  (Locseg_Chain_Is_Empty(chain[j]) == FALSE)) {
	Neurocomp_Conn conn;
	conn.mode = NEUROCOMP_CONN_HL;
	if (Locseg_Chain_Connection_Test(chain[i], chain[j], NULL, 
	      1.0, &conn, ws) == TRUE) {
	  Neurocomp_Conn conn2;
	  if (Locseg_Chain_Connection_Test(chain[i], chain[j], NULL, 
		1.0, &conn2, ws) == TRUE) {
	    if (conn.cost > conn2.cost) {
	      conn = conn2;
	    }
	  }
	  Neurocomp_Conn_Translate_Mode(Locseg_Chain_Length(chain[j]), 
	      &conn);
	  Neuron_Structure_Add_Conn(ns, i, j, &conn);
	}
      }
    }
    Set_Neuron_Component(Neuron_Structure_Get_Component(ns, i),
	NEUROCOMP_TYPE_LOCSEG_CHAIN, chain + i);
  }

  return ns;
}

static double turning_angle(coordinate_3d_t *pts)
{
  coordinate_3d_t vec1;
  coordinate_3d_t vec2;
  coordinate_3d_t vec3;

  int i;
  for (i = 0; i < 3; i++) {
    vec1[i] = pts[1][i] - pts[0][i];
    vec2[i] = pts[2][i] - pts[1][i];
    vec3[i] = pts[3][i] - pts[2][i];
  }

  return Geo3d_Angle2(vec1[0], vec1[1], vec1[2], vec2[0], vec2[1], vec2[2]) + 
    Geo3d_Angle2(vec2[0], vec2[1], vec2[2], vec3[0], vec3[1], vec3[2]);
}

double Locseg_Chain_Turning_Angle(Locseg_Chain *chain1, 
    Locseg_Chain *chain2, int end1, int end2, double *res)
{
  coordinate_3d_t pts[4];
  
  if (end1 == 0) {
    Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain1);
    Local_Neuroseg_Center(locseg, pts[1]);
    Local_Neuroseg_Top(locseg, pts[0]);
  } else {
    Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(chain1);
    Local_Neuroseg_Center(locseg, pts[1]);
    Local_Neuroseg_Bottom(locseg, pts[0]);
  }
  if (end2 == 0) {
    Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain2);
    Local_Neuroseg_Top(locseg, pts[3]);
    Local_Neuroseg_Center(locseg, pts[2]);
  } else {
    Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(chain2);
    Local_Neuroseg_Bottom(locseg, pts[3]);
    Local_Neuroseg_Center(locseg, pts[2]);
  }
  if (res != NULL) {
    int i;
    for (i = 0; i < 4; i++) {
      int j;
      for (j = 0; j < 3; j++) {
	pts[i][j] *= res[j];
      }
    }
  }
  return turning_angle(pts);
}

static int link_node(int index, int end)
{
  return index * 2 + end;
}

#define MERGE_LIST_ALT(row) (((row)%2) ? ((row)-1) : ((row)+1))

static int construct_merge_list(BOOL **conn, int nvertex, int *merge_list)
{
  int length = 0;
  int search_start = -1;
  int cur_row = -1;
  int i, j;
  /* search for the end */
  for (i = 0; i < nvertex; i++) {
    for (j = 0; j < nvertex; j++) {
      if (conn[i][j] == TRUE) {
	search_start = i;
	cur_row = MERGE_LIST_ALT(j);
	break;
      }
    }
    if (search_start >= 0) {
      break;
    }
  }

  if (search_start >= 0) {
    BOOL found = TRUE;
    while (found) {
      int k;
      found = FALSE;
      for (k = 0; k < nvertex; k++) {
	if (cur_row >= nvertex) {
	  break;
	}
	if (conn[cur_row][k] == TRUE) {
	  cur_row = MERGE_LIST_ALT(k);
	  found = TRUE;
	  break;
	}
      }
      if (cur_row == MERGE_LIST_ALT(search_start)) {
	/* break the loop */
	printf("Merging loop detected.\n");
	for (k = 0; k < nvertex; k++) {
	  conn[cur_row][k] = FALSE;
	  conn[k][cur_row] = FALSE;
	}
	break;
      }
    }

    merge_list[length++] = cur_row;
    cur_row = MERGE_LIST_ALT(cur_row);
    found = TRUE;
    while (found) {
      int k;
      found = FALSE;
      for (k = 0; k < nvertex; k++) {
	if (cur_row >= nvertex) {
	  break;
	}
	if (conn[cur_row][k] == TRUE) {
	  merge_list[length++] = k;
	  cur_row = MERGE_LIST_ALT(k);
	  found = TRUE;
	  break;
	}
      }
    }

  }

  return length;
} 

#define CONN_ID_TO_CHAIN_ID(cid) ((cid) / 2)

#define CONN_INDEX_TO_CONN_ID(idx, id1, id2)  \
{ \
  id1 = idx / nvertex; \
  id2 = idx % nvertex; \
}

#define CONN_INDEX_TO_CHAIN_ID(idx, id1, id2)  \
{ \
  id1 = (CONN_ID_TO_CHAIN_ID(idx/nvertex))); \
  id2 = (CONN_ID_TO_CHAIN_ID(idx%nvertex))); \
}

void Locseg_Chain_Array_Force_Merge(Locseg_Chain **chain, int n, 
    const Connection_Test_Workspace *ws)
{
  Connection_Test_Workspace tmpws = *ws;
  tmpws.interpolate = FALSE;
  Neuron_Structure *ns = locseg_chain_neurostruct(chain, n, &tmpws);
  Graph *graph = Make_Graph(0, 1, TRUE);

  int i;
  int nconn = Neuron_Structure_Link_Number(ns);
  for (i = 0; i < nconn; i++) {
    if (ns->conn[i].mode == NEUROCOMP_CONN_LINK) {
      int id1 = ns->graph->edges[i][0];
      int id2 = ns->graph->edges[i][1];
      int end1 = ns->conn[i].info[0];
      int end2 = ns->conn[i].info[1];

      double res[3];
      res[0] = tmpws.resolution[0];
      res[1] = tmpws.resolution[1];
      res[2] = tmpws.resolution[2];
      double w =Locseg_Chain_Turning_Angle(chain[id1], chain[id2], end1, end2,
	  res); 
      if ((w < TZ_PI / 3.0) || ((w < TZ_PI_2) && (ns->conn[i].sdist < 5.0))) {
	w =Locseg_Chain_Turning_Angle(chain[id1], chain[id2], end1, end2, NULL);
	Graph_Add_Weighted_Edge(graph, link_node(id1, end1), 
	    link_node(id2, end2), w); 
      }
    }
  }

  Graph_Workspace *gw = New_Graph_Workspace();
  BOOL **match = Graph_Hungarian_Match(graph, gw);
  /* make the match matrix symmetric */
  int j;
  int nvertex = graph->nvertex;
  for (i = 0; i < nvertex; i++) {
    for (j = i + 1; j < nvertex; j++) {
      match[i][j] |= match[j][i];
      match[j][i] = match[i][j];
    }
  }

  int *merge_list = iarray_malloc(nvertex * 2);
  int length = construct_merge_list(match, nvertex, merge_list);
  while (length >= 1) {
    int id1, id2;
    id1 = MERGE_LIST_ALT(merge_list[0]);
    id2 = merge_list[1];
    Locseg_Chain *master_chain = chain[CONN_ID_TO_CHAIN_ID(id1)];
    if (id1 % 2 == 0) {
      Locseg_Chain_Flip(master_chain);
    }
    for (i = 1; i < length; i++) {
      id1 = MERGE_LIST_ALT(merge_list[i-1]);
      id2 = merge_list[i];
      Locseg_Chain *slave_chain = chain[CONN_ID_TO_CHAIN_ID(id2)];
      Dlist_End_e slave_end = DL_HEAD;
      if (id2 % 2 == 1) {
	slave_end = DL_TAIL;
      }
      Locseg_Chain_Merge(master_chain, slave_chain, DL_TAIL, slave_end);
      match[id1][id2] = FALSE;
      match[id2][id1] = FALSE;
    }
    length = construct_merge_list(match, nvertex, merge_list);
  }

  free(ns->comp);
  ns->comp = NULL;
  Kill_Neuron_Structure(ns);
}

static double locseg_chain_point_dist_c(Locseg_Chain *chain, double *pos, 
    int *seg_index)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  *seg_index = 0;
  Local_Neuroseg *locseg = NULL;
  double min_dist = Infinity;
  int i = 0;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    double center[3];
    Local_Neuroseg_Center(locseg, center);
    double dist = Coordinate_3d_Distance(pos, center);
    if (dist < min_dist) {
      min_dist = dist;
      *seg_index = i;
    }
    i++;
  }

  return min_dist;
}

BOOL locseg_chain_index_is_internal(int chain_length, int seg_index)
{
  return (seg_index > 1) && (seg_index < chain_length - 2);
}

Locseg_Chain** Locseg_Chain_Array_Break_Jump(Locseg_Chain **chain, int n, 
    double dist_thre, int *n2)
{
  *n2 = 0;
  Locseg_Chain **chain2;
  GUARDED_MALLOC_ARRAY(chain2, n*2, Locseg_Chain*);
  int i, j;
  int break_point[1000];
  double tip[3];
  for (i = 0; i < n; i++) {
#ifdef _DEBUG_2
    if (i == 190) {
      printf("debug here\n");
    }
#endif
    int ntouch = 0;
    int chain_length = Locseg_Chain_Length(chain[i]);
    if (chain_length > 5) {
      for (j = 0; j < n; j++) {
        if (i != j) {
          if (Locseg_Chain_Length(chain[j]) > 5) {
            int seg_index1, seg_index2;
            Locseg_Chain_End_Pos(chain[j], DL_HEAD, tip);
            double dist1 = 
              locseg_chain_point_dist_c(chain[i], tip, &seg_index1);
            Locseg_Chain_End_Pos(chain[j], DL_TAIL, tip);
            double dist2 = 
              locseg_chain_point_dist_c(chain[i], tip, &seg_index2);
            if (dist1 > dist2) {
              dist1 = dist2;
              seg_index1 = seg_index2;
            }
            if (dist1 < dist_thre) {
              if (locseg_chain_index_is_internal(chain_length, seg_index1)) {
                break_point[ntouch++] = seg_index1;
              }
            }
          }
        }
      }
    }
    chain2[(*n2)++] = Copy_Locseg_Chain(chain[i]);
    if (ntouch > 1) {
      iarray_qsort(break_point, NULL, ntouch);
      int k;
      int min_interval = 10;
      int index1, index2;
      for (k = 0; k < ntouch - 1; k++) {
        if (break_point[k+1] - break_point[k] < min_interval) {
          min_interval = break_point[k+1] - break_point[k];
          index1 = break_point[k];
          index2 = break_point[k+1];
        }
      }
      if (min_interval < 5) {
	if (min_interval < 3) {
	  index1--;
	  index2++;
	}
        chain2[(*n2)] = 
          Locseg_Chain_Break_Between(chain2[(*n2)-1], index1, index2);
        (*n2)++;
      }
    }
  }

  return chain2;
}

void Locseg_Chain_Correct_Ends(Locseg_Chain *chain)
{
  int len = Locseg_Chain_Length(chain);
  if (len > 2) {
    Local_Neuroseg *locseg1 = Locseg_Chain_Head_Seg(chain);
    Local_Neuroseg *locseg2 = Locseg_Chain_Peek_Seg_At(chain, 1);
    double pos1[3], pos2[3];
    Local_Neuroseg_Bottom(locseg1, pos1);
    Local_Neuroseg_Bottom(locseg2, pos2);
    if (Coordinate_3d_Distance(pos1, pos2) < 2.0) {
      Locseg_Chain_Remove_End(chain, DL_HEAD);
      Local_Neuroseg_Change_Bottom(locseg2, pos1);
      len--;
    }

    if (len > 2) {
      Local_Neuroseg *locseg1 = Locseg_Chain_Tail_Seg(chain);
      Local_Neuroseg *locseg2 = Locseg_Chain_Peek_Seg_At(chain, len - 2);
      double pos1[3], pos2[3];
      Local_Neuroseg_Top(locseg1, pos1);
      Local_Neuroseg_Top(locseg2, pos2);
      if (Coordinate_3d_Distance(pos1, pos2) < 2.0) {
	Locseg_Chain_Remove_End(chain, DL_TAIL);
	Local_Neuroseg_Change_Top(locseg2, pos1);
	len--;
      }
    }
  }
}
