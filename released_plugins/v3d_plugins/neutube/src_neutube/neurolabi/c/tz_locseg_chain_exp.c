/* tz_locseg_chain.c
 *
 * 06-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tz_trace_utils.h"
#include "tz_locseg_chain_exp.h"
#include "tz_math.h"


Pixel_Feature* Get_Pixel_Feature(double pos[3], const Stack *stack, const Trace_Workspace *tw)
{
  int x = iround(pos[0]);
  int y = iround(pos[1]);
  int z = iround(pos[2]);
  if ((!IS_IN_CLOSE_RANGE3(x, y, z, 0, stack->width-1, 0, stack->height-1, 0, stack->depth-1))
      || tw->Ixx == NULL) {
    return NULL;
  }
  Pixel_Feature *pf = (Pixel_Feature*)malloc(sizeof(Pixel_Feature));
  pf->loc[0] = x;
  pf->loc[1] = y;
  pf->loc[2] = z;
  pf->indice = (size_t)x + (size_t)y * stack->width + (size_t)z * stack->width * stack->height;
  uint16* array16 = (uint16*) (stack->array);
  float32* array32 = (float32*) (stack->array);
  float64* array64 = (float64*) (stack->array);

  switch(stack->kind) {
  case GREY:
    pf->value = stack->array[pf->indice];
    break;
  case GREY16:
    pf->value = array16[pf->indice];
    break;
  case FLOAT32:
    pf->value = array32[pf->indice];
    break;
  case FLOAT64:
    pf->value = array64[pf->indice];
    break;
  default:
    fprintf(stderr,"Unrecongnzied stack kind in Get_Pixel_Feature");
  }

  int res = Matrix_Eigen_Value_Cs(tw->Ixx->array[pf->indice], tw->Iyy->array[pf->indice], tw->Izz->array[pf->indice],
                                  tw->Ixy->array[pf->indice], tw->Ixz->array[pf->indice], tw->Iyz->array[pf->indice],
                                  pf->eigen_values);


  if (res == 1) {
    pf->eigen_vector1[0] = pf->eigen_vector1[1] = pf->eigen_vector1[2] = 0;
    pf->eigen_vector2[0] = pf->eigen_vector2[1] = pf->eigen_vector2[2] = 0;
    pf->eigen_vector3[0] = pf->eigen_vector3[1] = pf->eigen_vector3[2] = 0;
  } else {
    Matrix_Eigen_Vector_Cs(tw->Ixx->array[pf->indice], tw->Iyy->array[pf->indice], tw->Izz->array[pf->indice],
                           tw->Ixy->array[pf->indice], tw->Ixz->array[pf->indice], tw->Iyz->array[pf->indice],
                           pf->eigen_values[0], pf->eigen_vector1);
    Matrix_Eigen_Vector_Cs(tw->Ixx->array[pf->indice], tw->Iyy->array[pf->indice], tw->Izz->array[pf->indice],
                           tw->Ixy->array[pf->indice], tw->Ixz->array[pf->indice], tw->Iyz->array[pf->indice],
                           pf->eigen_values[1], pf->eigen_vector2);
    Matrix_Eigen_Vector_Cs(tw->Ixx->array[pf->indice], tw->Iyy->array[pf->indice], tw->Izz->array[pf->indice],
                           tw->Ixy->array[pf->indice], tw->Ixz->array[pf->indice], tw->Iyz->array[pf->indice],
                           pf->eigen_values[2], pf->eigen_vector3);
  }
  return pf;
}

void Local_Neuroseg_Pixel_Feature(Local_Neuroseg *ln, const Stack *stack, const Trace_Workspace *tw)
{
  double pos[3];
  Local_Neuroseg_Bottom(ln, pos);
  ln->pf1 = Get_Pixel_Feature(pos, stack, tw);
  Local_Neuroseg_Center(ln, pos);
  ln->pf2 = Get_Pixel_Feature(pos, stack, tw);
  Local_Neuroseg_Top(ln, pos);
  ln->pf3 = Get_Pixel_Feature(pos, stack, tw);
}

void Reset_Pixel_Feature(Pixel_Feature *pf)
{
  memset(pf, 0, sizeof(Pixel_Feature));
  pf->loc[0] = pf->loc[1] = pf->loc[2] = -1;
}

void Clean_Trace_Workspace_Exp(Trace_Workspace *tw)
{
  if (tw->sup_stack != NULL) {
    Kill_Stack(tw->sup_stack);
    tw->sup_stack = NULL;
  }

  if (tw->trace_mask != NULL) {
    Kill_Stack(tw->trace_mask);
    tw->trace_mask = NULL;
  }

  if (tw->canvas != NULL) {
    Kill_Stack(tw->canvas);
    tw->canvas = NULL;
  }
  if (tw->Ixx != NULL) {
    Kill_FMatrix(tw->Ixx);
    tw->Ixx = NULL;
  }
  if (tw->Iyy != NULL) {
    Kill_FMatrix(tw->Iyy);
    tw->Iyy = NULL;
  }
  if (tw->Izz != NULL) {
    Kill_FMatrix(tw->Izz);
    tw->Izz = NULL;
  }
  if (tw->Ixy != NULL) {
    Kill_FMatrix(tw->Ixy);
    tw->Ixy = NULL;
  }
  if (tw->Iyz != NULL) {
    Kill_FMatrix(tw->Iyz);
    tw->Iyz = NULL;
  }
  if (tw->Ixz != NULL) {
    Kill_FMatrix(tw->Ixz);
    tw->Ixz = NULL;
  }
}

void Kill_Trace_Workspace_Exp(Trace_Workspace *tw)
{
  Clean_Trace_Workspace_Exp(tw);
  free(tw);
}

Locseg_Chain* Locseg_Chain_Trace_Init_Exp(const Stack *stack, double z_scale,
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

int Locseg_Chain_Trace_Test_Exp(void *argv[])
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
  }

  return TRACE_NORMAL;
}

Trace_Workspace*
Locseg_Chain_Default_Trace_Workspace_Exp(Trace_Workspace *tw,
             const Stack *stack)
{
  if (tw == NULL) {
    tw = New_Trace_Workspace();
  } else {
    Clean_Trace_Workspace_Exp(tw);
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

  if (0 && stack != NULL && stack->array != NULL && stack->kind != 3) {
    double sigma[] = {1.0, 1.0, 1.0};
    FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
    FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);

    Correct_Filter_Stack_F(filter, f);

    Kill_FMatrix(filter);

    FMatrix *Ix = FMatrix_Partial_Diff(f, 0, NULL);
    tw->Ixx = FMatrix_Partial_Diff(Ix, 0, NULL);
    tw->Ixy = FMatrix_Partial_Diff(Ix, 1, NULL);
    tw->Ixz = FMatrix_Partial_Diff(Ix, 2, NULL);

    FMatrix *Iy = FMatrix_Partial_Diff(f, 1, Ix);
    tw->Iyy = FMatrix_Partial_Diff(Iy, 1, NULL);
    tw->Iyz = FMatrix_Partial_Diff(Iy, 2, NULL);

    FMatrix *Iz = FMatrix_Partial_Diff(f, 2, Iy);
    tw->Izz = FMatrix_Partial_Diff(Iz, 2, f);

    Kill_FMatrix(Ix);
  } else {
    tw->Ixx = tw->Ixy = tw->Ixz = tw->Iyy = tw->Iyz = tw->Izz = NULL;
  }

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
  tw->test_func = Locseg_Chain_Trace_Test_Exp;

  return tw;
}
