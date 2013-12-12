/* tz_trace_utils.c
 *
 * 02-JUL-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_trace_utils.h"
#include "tz_darray.h"
#include "tz_stack_utils.h"
#include "tz_math.h"
#include "tz_workspace.h"
#include "tz_stack_io.h"
#include "tz_stack_sampling.h"

Trace_Workspace* New_Trace_Workspace()
{
  Trace_Workspace *tw = (Trace_Workspace *) 
    Guarded_Malloc(sizeof(Trace_Workspace), "New_Trace_Workspace");
  Default_Trace_Workspace(tw);
  return tw;
}

void Default_Trace_Workspace(Trace_Workspace *tw)
{
  tw->length = 5000;
  tw->fit_first = 0;
  tw->refit = TRUE;
  tw->break_refit = FALSE;
  tw->tune_end = FALSE;
  tw->tscore_option = STACK_FIT_CORRCOEF;
  tw->trace_step = 0.5;
  tw->seg_length = NEUROSEG_DEFAULT_H;
  tw->min_score = LOCAL_NEUROSEG_MIN_CORRCOEF;
  tw->trace_direction = DL_BOTHDIR;
  tw->stop_reason[0] = 0; /* obsolete */
  tw->stop_reason[1] = 0; /* obsolete */
  tw->trace_status[0] = TRACE_NORMAL;
  tw->trace_status[1] = TRACE_NORMAL;
  tw->min_chain_length = NEUROSEG_DEFAULT_H * 2.5;
  tw->chain_id = 0;
  int i;
  for (i = 0; i < 6; i++) {
    tw->trace_range[i] = -1.0;
  }
  for (i = 0; i < 5; i++) {
    tw->dyvar[i] = -1.0;
  }
  tw->sup_stack = NULL;
  tw->trace_mask = NULL;
  tw->swc_mask = NULL;
  tw->trace_mask_updating = TRUE;
  tw->canvas_updating = FALSE;
  tw->canvas = NULL;
  tw->test_func = NULL;
  tw->save_path[0] = '\0';
  strcpy(tw->save_prefix, "chain");
  tw->resolution[0] = -1.0;
  tw->resolution[1] = -1.0;
  tw->resolution[2] = -1.0;
  tw->fit_workspace = NULL;
  tw->add_hit = TRUE;
}

void Clean_Trace_Workspace(Trace_Workspace *tw) 
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

  if (tw->swc_mask != NULL) {
    Kill_Stack(tw->swc_mask);
    tw->swc_mask = NULL;
  }
}

void Kill_Trace_Workspace(Trace_Workspace *tw) 
{
  Clean_Trace_Workspace(tw);
  free(tw);
}

void Print_Trace_Workspace(const Trace_Workspace *tw) 
{
  printf("Maximum length: %d\n", tw->length);
  printf("Fit first? ");
  if (tw->fit_first == TRUE) {
    printf("Yes.\n");
  } else {
    printf("No.\n");
  }
  printf("Refit? ");
  if (tw->refit == TRUE) {
    printf("Yes.\n");
  } else {
    printf("No.\n");
  }
  printf("Break refit? ");
  if (tw->break_refit == TRUE) {
    printf("Yes.\n");
  } else {
    printf("No.\n");
  }
  printf("Tune ends? ");
  if (tw->tune_end == TRUE) {
    printf("Yes.\n");
  } else {
    printf("No.\n");
  }
  printf("Thresholding score option: %d; Threshold %g\n", 
	 tw->tscore_option, tw->min_score);
  printf("Minimal chain length: %g\n", tw->min_chain_length);
  printf("Trace step: %g\n", tw->trace_step);
  printf("Chain id: %d\n", tw->chain_id);
  printf("Tracing status: forward - %d, backward - %d\n",
	 tw->trace_status[0], tw->trace_status[1]);
  printf("Tracing range: [%g %g] [%g %g] [%g %g]\n",
	 tw->trace_range[0], tw->trace_range[3], 
	 tw->trace_range[1], tw->trace_range[4],
	 tw->trace_range[2], tw->trace_range[5]);
  printf("Supervising stack: ");
  Print_Stack_Info(tw->sup_stack);
  printf("Tracing mask: ");
  Print_Stack_Info(tw->trace_mask);
  printf("Update tracing mask? ");
  if (tw->trace_mask_updating == TRUE) {
    printf("Yes.\n");
  } else {
    printf("No.\n");
  }
  printf("Canvas stack: ");
  Print_Stack_Info(tw->canvas);
  printf("Dynamic variables: %g %g %g %g %g\n", 
	 tw->dyvar[0], tw->dyvar[1], tw->dyvar[2], tw->dyvar[3], tw->dyvar[4]);
  printf("Resolution: %g um x %g um x %g um\n", 
	 tw->resolution[0], tw->resolution[1], tw->resolution[2]);
  printf("Saving path: %s\n", tw->save_path);
}

void Trace_Workspace_Set_Trace_Status(Trace_Workspace *tw, int forward_status, 
				      int backward_status)
{
  tw->trace_status[0] = forward_status;
  tw->trace_status[1] = backward_status;
}

int Trace_Workspace_Mask_Value(const Trace_Workspace *tw, 
			       const double pos[3])
{
  int hit_label = 0;

  if (tw->trace_mask != NULL) {
    if (pos[0] >= 0) {
      hit_label = Get_Stack_Pixel(tw->trace_mask, iround(pos[0]),
				  iround(pos[1]), iround(pos[2]), 0);
    }
  }

  return hit_label;
}

int Trace_Workspace_Mask_Value_Z(const Trace_Workspace *tw, 
				 double pos[3], double z_scale)
{
  pos[2] *= z_scale;
  return Trace_Workspace_Mask_Value(tw, pos);
}

BOOL Trace_Workspace_Point_In_Bound(const Trace_Workspace *tw, 
				    const double pos[3])
{
  if (pos[0] >= 0) {
    int i;
    for (i = 0; i < 3; i++) {
      if (tw->trace_range[i] >= 0.0) {
	if (pos[i] < tw->trace_range[i]) {
	  return FALSE;
	}
      }
	
      if (tw->trace_range[i + 3] >= 0.0) {
	if (pos[i] > tw->trace_range[i + 3]) {
	  return FALSE;
	}
      }
    }
  }

  return TRUE;
}

BOOL Trace_Workspace_Point_In_Bound_Z(const Trace_Workspace *tw, 
				      double pos[3], double z_scale)
{
  pos[2] *= z_scale;
  return Trace_Workspace_Point_In_Bound(tw, pos);
}

Trace_Record* New_Trace_Record()
{
  Trace_Record *tr = (Trace_Record *) 
    Guarded_Malloc(sizeof(Trace_Record), "New_Trace_Record");
  Reset_Trace_Record(tr);

  return tr;
}

void Trace_Workspace_Set_Fit_Mask(Trace_Workspace *tw, Stack *mask)
{
  Locseg_Fit_Workspace *fw = (Locseg_Fit_Workspace*) tw->fit_workspace;
  if (fw != NULL) {
    if (fw->sws != NULL) {
      fw->sws->mask = mask;
    }
  }
}

BOOL Trace_Workspace_Is_Masked(const Trace_Workspace *tw)
{
  Locseg_Fit_Workspace *fw = (Locseg_Fit_Workspace*) tw->fit_workspace;
  if (fw != NULL) {
    if (fw->sws != NULL) {
      if (fw->sws->mask != NULL) {
	return TRUE;
      }
    }
  }

  return FALSE;
}

void Print_Trace_Status(int status)
{
  switch (status) {
  case TRACE_NORMAL:
    printf("Trace going on normally.\n");
    break;
  case TRACE_HIT_MARK:
    printf("Hit marked region.\n");
    break;
  case TRACE_LOW_SCORE:
    printf("Low score.\n");
    break;
  case TRACE_TOO_LARGE:
    printf("Too large.\n");
    break;
  case TRACE_TOO_SMALL:
    printf("Too small.\n");
    break;
  case TRACE_INVALID_SHAPE:
    printf("Invalid shape.\n");
    break;
  case TRACE_LOOP_FORMED:
    printf("Loop formed.\n");
    break;
  case TRACE_OUT_OF_BOUND: 
    printf("Trace out of bound.\n");
    break;
  case TRACE_REPEATED:
    printf("Trace repeated.\n");
    break;
  case TRACE_OVER_REFIT:
    printf("Too much refit.\n");
    break;
  case TRACE_SEED_OUT_OF_BOUND:
    printf("Seed out of bound.\n");
    break;
  case TRACE_SIZE_CHANGE:
    printf("Size changed too much.\n");
    break;
  case TRACE_SIGNAL_CHANGE:
    printf("Signal changed too much.\n");
    break;
  case TRACE_OVERLAP:
    printf("Segment overlap.\n");
    break;
  case TRACE_NOT_ASSIGNED:
    printf("Trace not assigned.\n");
    break;
  default:
    printf("Unknown tracing status.\n");
  }
}

void Reset_Trace_Record(Trace_Record *tr)
{
  tr->mask = ZERO_BIT_MASK;
  tr->fs.n = 0;
  tr->hit_region = -1;
  tr->index = 0;
  tr->refit = 0;
  tr->direction = DL_UNKNOWN;
  tr->fix_point = -1.0;
  tr->fit_height[0] = 0;
  tr->fit_height[1] = 0;
}

void Delete_Trace_Record(Trace_Record *tr)
{
  free(tr);
}

Trace_Record* Copy_Trace_Record(Trace_Record *tr)
{
  if (tr == NULL) {
    return NULL;
  }

  Trace_Record *newtr = New_Trace_Record();
  Trace_Record_Copy(newtr, tr);

  return newtr;
}

void Trace_Record_Copy(Trace_Record *des, const Trace_Record *src)
{
  memcpy(des, src, sizeof(Trace_Record));
}

void Fprint_Trace_Record(FILE *fp, const Trace_Record *tr)
{
  if (tr == NULL) {
    fprintf(fp, "Null tracing history\n");
  } else if (tr->mask == ZERO_BIT_MASK) {
    fprintf(fp, "Tracing history: no information recorded.\n");
  } else {
    fprintf(fp, "Tracing history:\n");
    if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_FIT_SCORE_MASK) == TRUE) {
      fprintf(fp, "  ");
      Fprint_Stack_Fit_Score(fp, &(tr->fs));
    }
    if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_HIT_REGION_MASK) == TRUE) {
      fprintf(fp, "  Hit region: %d\n", tr->hit_region);
    }
    if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_INDEX_MASK) == TRUE) {
      fprintf(fp, "  Chain index: %d\n", tr->index);
    }
    if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_REFIT_MASK) == TRUE) {
      fprintf(fp, "  refit: %d\n", tr->refit);
    }
    if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_FIT_HEIGHT_MASK) == TRUE) {
      if (tr->fit_height[0] == 1) {
	fprintf(fp, "  height adjusted: backward\n");
      }
      if (tr->fit_height[1] == 1) {
	fprintf(fp, "  height adjusted: forward\n");
      }
    }
    if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_DIRECTION_MASK) == TRUE) {
      fprintf(fp, "  Tracing direction: ");
      switch (tr->direction) {
      case DL_FORWARD:
	fprintf(fp, "FORWARD\n");
	break;
      case DL_BACKWARD:
	fprintf(fp, "BACKWARD\n");
	break;
      case DL_BOTHDIR:
	fprintf(fp, "BOTH DIRECTION\n");
	break;
      default:
	fprintf(fp, "UNKNOWN DIRECTION\n");
      }
    }
  }  
}

void Print_Trace_Record(const Trace_Record *tr)
{
  Fprint_Trace_Record(stdout, tr);
}


void Trace_Record_Fwrite(const Trace_Record *tr, FILE *stream)
{
  Bitmask_t mask = ZERO_BIT_MASK;
  if (tr != NULL) {
    mask = tr->mask;
  }

  fwrite(&mask, sizeof(Bitmask_t), 1, stream);

  if (mask != ZERO_BIT_MASK) {
    /*fwrite(tr, sizeof(Trace_Record), 1, stream);*/
    Stack_Fit_Score_Fwrite(&(tr->fs), stream);
    fwrite(&(tr->hit_region), sizeof(tr->hit_region), 1, stream);
    fwrite(&(tr->index), sizeof(tr->index), 1, stream);
    fwrite(&(tr->refit), sizeof(tr->refit), 1, stream);
    fwrite(tr->fit_height, sizeof(tr->fit_height[0]), 2, stream);
    fwrite(&(tr->direction), sizeof(tr->direction), 1, stream);
  }
}

void Trace_Record_Fwrite_V(const Trace_Record *tr, FILE *stream, double version)
{
  if (version == 0.0) {
    Bitmask_t mask = ZERO_BIT_MASK;
    if (tr != NULL) {
      mask = tr->mask;
    }

    if (mask != ZERO_BIT_MASK) {
      fwrite(tr, sizeof(Trace_Record) - sizeof(tr->fix_point), 1, stream);
    } else {
      fwrite(&mask, sizeof(Bitmask_t), 1, stream); 
    }
  } else if (version == 1.0) {
    Trace_Record_Fwrite(tr, stream);
  } else if ((version == 2.0) || (version == 3.0)){
    Trace_Record_Fwrite(tr, stream);
    if (tr != NULL){
      if (tr->mask != ZERO_BIT_MASK) {
	fwrite(&(tr->fix_point), sizeof(tr->fix_point), 1, stream);
      }
    }
  } else {
    TZ_ERROR(ERROR_IO_FORMAT);
  }
}

Trace_Record* Trace_Record_Fread(Trace_Record *tr, FILE *stream)
{
  ASSERT(stream != NULL, "Invalid file pointer");
  if (feof(stream)) {
    return NULL;
  }

  Bitmask_t mask = ZERO_BIT_MASK;
  if (fread(&mask, sizeof(Bitmask_t), 1, stream) != 1) {
    return NULL;
  }
  
  BOOL is_local_alloc = FALSE;

  if (tr == NULL) {
    if (mask == ZERO_BIT_MASK) {
      return NULL;
    } else {
      tr = New_Trace_Record();
      is_local_alloc = TRUE;
    }
  }

  tr->mask = mask;
  if (mask != ZERO_BIT_MASK) {
    /*
    if (fread(((void *) tr) + sizeof(Bitmask_t), 
	      sizeof(Trace_Record) - sizeof(Bitmask_t), 1, stream) != 1) {
      if (is_local_alloc == TRUE) {
	Delete_Trace_Record(tr);
      }
      tr = NULL;
    }
    */
    Stack_Fit_Score_Fread(&(tr->fs), stream);
    fread(&(tr->hit_region), sizeof(tr->hit_region), 1, stream);
    fread(&(tr->index), sizeof(tr->index), 1, stream);
    fread(&(tr->refit), sizeof(tr->refit), 1, stream);
    fread(tr->fit_height, sizeof(tr->fit_height[0]), 2, stream);
    fread(&(tr->direction), sizeof(tr->direction), 1, stream);
  }

  return tr;
}

Trace_Record* Trace_Record_Fread_V(Trace_Record *tr, FILE *stream, 
				   double version)
{
  if (version == 0.0) {
    ASSERT(stream != NULL, "Invalid file pointer");
    if (feof(stream)) {
      return NULL;
    }

    Bitmask_t mask = ZERO_BIT_MASK;
    if (fread(&mask, sizeof(Bitmask_t), 1, stream) != 1) {
      return NULL;
    }
  
    BOOL is_local_alloc = FALSE;

    if (tr == NULL) {
      if (mask == ZERO_BIT_MASK) {
	return NULL;
      } else {
	tr = New_Trace_Record();
	is_local_alloc = TRUE;
      }
    }
    
    tr->mask = mask;
    if (mask != ZERO_BIT_MASK) {
      if (fread(BYTE_ARRAY(tr) + sizeof(Bitmask_t), 
                sizeof(Trace_Record) - sizeof(Bitmask_t) -
                sizeof(tr->fix_point), 1, stream) != 1) {
        if (is_local_alloc == TRUE) {
          Delete_Trace_Record(tr);
        }
        tr = NULL;
      }
    }
  } else if (version == 1.0) {
    tr = Trace_Record_Fread(tr, stream);
  } else if (version == 2.0) {
    tr = Trace_Record_Fread(tr, stream);
    if (tr != NULL) {
      int fix_point;
      fread(&fix_point, sizeof(fix_point), 1, stream);
      tr->fix_point = fix_point;
    }
  } else if (version == 3.0) {
    tr = Trace_Record_Fread(tr, stream);
    if (tr != NULL) {
      fread(&(tr->fix_point), sizeof(tr->fix_point), 1, stream);
    }
  } else {
    TZ_ERROR(ERROR_IO_FORMAT);
  }

  return tr;
}

void Trace_Record_Set_Score(Trace_Record *tr, const Stack_Fit_Score *fs)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_FIT_SCORE_MASK, TRUE, &(tr->mask));
    Stack_Fit_Score_Copy(&(tr->fs), fs);
  }
}

void Trace_Record_Set_Hit_Region(Trace_Record *tr, int hit_region)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_HIT_REGION_MASK, TRUE, &(tr->mask));
    tr->hit_region = hit_region;
  }
}

void Trace_Record_Set_Index(Trace_Record *tr, int index)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_INDEX_MASK, TRUE, &(tr->mask));
    tr->index = index;
  }
}

void Trace_Record_Set_Refit(Trace_Record *tr, int refit)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_REFIT_MASK, TRUE, &(tr->mask));
    tr->refit = refit;
  }
}

void Trace_Record_Set_Fit_Height(Trace_Record *tr, int index, int value)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_FIT_HEIGHT_MASK, TRUE, &(tr->mask));
    tr->fit_height[index] = value;
  }
}

void Trace_Record_Set_Direction(Trace_Record *tr, Dlist_Direction_e direction)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_DIRECTION_MASK, TRUE, &(tr->mask));
    tr->direction = direction;
  }
}

void Trace_Record_Set_Fix_Point(Trace_Record *tr, double value)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_FIX_POINT_MASK, TRUE, &(tr->mask));
    tr->fix_point = value;
  }
}

void Trace_Record_Disable_Fix_Point(Trace_Record *tr)
{
  if (tr != NULL) {
    Bitmask_Set_Bit(TRACE_RECORD_FIX_POINT_MASK, FALSE, &(tr->mask));
  }
}

int Trace_Record_Index(const Trace_Record *tr)
{
  if (tr == NULL) {
    return -1;
  }

  if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_INDEX_MASK) == 0) {
    return -1;
  }

  return tr->index;
}

int Trace_Record_Refit(const Trace_Record *tr)
{
  if (tr == NULL) {
    return -1;
  }

  if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_REFIT_MASK) == 0) {
    return 0;
  } else {
    return tr->refit;
  }
}

int Trace_Record_Fit_Height(const Trace_Record *tr, int index)
{
  if (tr == NULL) {
    return -1;
  }

  if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_FIT_HEIGHT_MASK) == 0) {
    return 0;
  } else {
    return tr->fit_height[index];
  }
}

Dlist_Direction_e Trace_Record_Direction(const Trace_Record *tr)
{
  if (tr == NULL) {
    return DL_UNKNOWN;
  }

  if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_DIRECTION_MASK) == 0) {
    return DL_UNKNOWN;
  }

  return tr->direction;
}

double Trace_Record_Fix_Point(const Trace_Record *tr)
{
  if (tr == NULL) {
    return -1.0;
  }

  if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_FIX_POINT_MASK) == 0) {
    return -1.0;
  }

  return tr->fix_point;
}

BOOL Trace_Record_Has_Fix_Point(Trace_Record *tr)
{
  if (tr == NULL) {
    return FALSE;
  }

  if (Bitmask_Get_Bit(tr->mask, TRACE_RECORD_FIX_POINT_MASK) == 0) {
    return FALSE;
  }

  return TRUE;
}

Trace_Evaluate_Seed_Workspace* New_Trace_Evaluate_Seed_Workspace()
{
  Trace_Evaluate_Seed_Workspace *ws = (Trace_Evaluate_Seed_Workspace*) 
    Guarded_Malloc(sizeof(Trace_Evaluate_Seed_Workspace),
		   "New_Trace_Evaluate_Seed_Workspace");

  Default_Trace_Evaluate_Seed_Workspace(ws);

  return ws;
}

void Default_Trace_Evaluate_Seed_Workspace(Trace_Evaluate_Seed_Workspace *ws)
{
  ws->score_option = 1;
  ws->fit_option = 0;
  ws->zshift = FALSE;
  ws->min_score = LOCAL_NEUROSEG_MIN_CORRCOEF;
  ws->nseed = 0;
  ws->base_mask = NULL;
  ws->trace_mask = NULL;
  ws->score = NULL;
  ws->locseg = NULL;
  ws->fws = New_Locseg_Fit_Workspace();
  ws->fws->sws->fs.n = 2;
  ws->fws->sws->fs.options[0] = STACK_FIT_DOT;
  ws->fws->sws->fs.options[1] = STACK_FIT_CORRCOEF;
}

void Clean_Trace_Evaluate_Seed_Workspace(Trace_Evaluate_Seed_Workspace *ws)
{
  OBJECT_SAFE_FREE(ws->base_mask, Kill_Stack);
  OBJECT_SAFE_FREE(ws->trace_mask, Kill_Stack);
  OBJECT_SAFE_FREE(ws->score, free);
  OBJECT_SAFE_FREE(ws->locseg, free);
  OBJECT_SAFE_FREE(ws->fws, Kill_Locseg_Fit_Workspace);
}

void Kill_Trace_Evaluate_Seed_Workspace(Trace_Evaluate_Seed_Workspace *ws)
{
  Clean_Trace_Evaluate_Seed_Workspace(ws);
  free(ws);
}

PRIVATE void stack_adjust_zpos(const Stack *stack, int x, int y, int *z)
{
  int old_z = *z;
  int new_z = *z;
  double value[5];
  double points[15];
  int i;
  for (i = 0; i <= 5; i++) {
    points[i*3] = x;
    points[i*3+1] = y;
    points[i*3+2] = *z + i - 2;
  }
 
  do {
    old_z = new_z;
    Stack_Points_Sampling(stack, points, 5, value);
    if (!(isnan(value[0]) || isnan(value[1]) || isnan(value[2]) ||
	  isnan(value[3]) || isnan(value[4]))) {
      if ((dmax2(value[0], value[1]) < value[2]) && (value[2] < dmin2(value[3],
	      value[4]))) {
	new_z++;
      } else if ((dmin2(value[0], value[1]) > value[2]) && 
	  (value[2] > dmax2(value[3], value[4]))) {
	new_z--;
      } 
    }

    if (new_z == old_z) {
      double w = 0.0;
      double zw = 0.0;
      for (i = 0; i < 5; i++) {
	if (!isnan(value[i])) {
	  w += value[i];
	  zw += value[i] * points[3*i+2];
	}
      }

      if (w == 0.0) {
	break;
      } else {
	new_z = iround(zw / w);
      }
    }

    for (i = 0; i < 5; i++) {
      points[i*3+2] += new_z - old_z;
    }
  } while (old_z != new_z);

  *z = new_z;
}


void Trace_Evaluate_Seed(const Geo3d_Scalar_Field *seed, 
			 const Stack *signal, double z_scale, 
			 Trace_Evaluate_Seed_Workspace *ws)
{
  OBJECT_SAFE_FREE(ws->score, free);
  ws->score = darray_malloc(seed->size);

  int i;
  OBJECT_SAFE_FREE(ws->locseg, free);  
  ws->locseg = (Local_Neuroseg *) malloc(seed->size * sizeof(Local_Neuroseg));

  ws->nseed = seed->size;

  int index = 0;

  if (ws->base_mask == NULL) {
    ws->base_mask = Make_Stack(GREY, signal->width, signal->height, 
			       signal->depth);
    Zero_Stack(ws->base_mask);
  }

  for (i = 0; i < seed->size; i++) {
    printf("-----------------------------> seed: %d / %d\n", i, seed->size);

    index = i;
    int x = (int) seed->points[index][0];
    int y = (int) seed->points[index][1];
    int z = (int) seed->points[index][2];

    if (ws->zshift) {
      stack_adjust_zpos(signal, x, y, &z);
      if (ws->trace_mask != NULL) {
	if (Stack_Pixel(ws->trace_mask, x, y, z, 0) > 0.0) {
	  printf("traced**\n");
	  ws->score[i] = 0.0;
	  continue;
	}
      }
    }

    double width = seed->values[index];

    int seed_offset = Stack_Util_Offset(x, y, z, signal->width, signal->height,
					signal->depth);

    if (width < 3.0) {
      width += 0.5;
    }
    Set_Neuroseg(&(ws->locseg[i].seg), width, 0.0, NEUROSEG_DEFAULT_H, 
		 0.0, 0.0, 0.0, 0.0, 1.0);

    double cpos[3];
    cpos[0] = x;
    cpos[1] = y;
    cpos[2] = z;
    cpos[2] /= z_scale;
    
    Set_Neuroseg_Position(&(ws->locseg[i]), cpos, NEUROSEG_CENTER);

    if (ws->base_mask->array[seed_offset] > 0) {
      printf("labeled\n");
      ws->score[i] = 0.0;
      continue;
    }

    { /* for faster evaluation*/
      Local_Neuroseg *locseg = ws->locseg + i;
      Stack_Fit_Score fs;
      fs.n = 1;
      fs.options[0] = STACK_FIT_CORRCOEF;

      Locseg_Fit_Workspace *fw = (Locseg_Fit_Workspace*) ws->fws;
      
      int k;
      for (k = 0; k < fw->pos_adjust; k++) {
	Local_Neuroseg_Position_Adjust(locseg, signal, z_scale);
      }
      
      Local_Neuroseg_Orientation_Search_C(locseg, signal, z_scale, &fs); 
  
      if (ws->fit_option <= 1) {
	for (k = 0; k < 3; k++) {
	  Local_Neuroseg_Position_Adjust(locseg, signal, z_scale);
	}
      }

      double bpos[3];
      double tpos[3];
      Local_Neuroseg_Bottom(locseg, bpos);
      Local_Neuroseg_Center(locseg, cpos);
      Local_Neuroseg_Top(locseg, tpos);
      if (ws->trace_mask != NULL) {
	if ((Stack_Pixel(ws->trace_mask, bpos[0], bpos[1], bpos[2], 0) > 0) &&
	    (Stack_Pixel(ws->trace_mask, cpos[0], cpos[1], cpos[2], 0) > 0) &&
	    (Stack_Pixel(ws->trace_mask, tpos[0], tpos[1], tpos[2], 0) > 0)) {
	  printf("traced*\n");
	  ws->score[i] = 0.0;
	  continue;
	}
      }

      if ((ws->fit_option == 1) || (ws->fit_option == 2)){
	Local_Neuroseg_R_Scale_Search(locseg, signal, z_scale, 1.0, 10.0, 1.0,
				      0.5, 5.0, 0.5, NULL);
      }

      Fit_Local_Neuroseg_W(locseg, signal, z_scale, fw);
    }

    if (ws->trace_mask != NULL) {
      if (Local_Neuroseg_Hit_Mask(ws->locseg + i, 
				  ws->trace_mask, z_scale) > 0) {
	printf("traced\n");
	ws->score[i] = 0.0;
	continue;
      }
    }

    //ws->score[i] = Local_Neuroseg_Score(ws->locseg + i, signal, z_scale, &fs);
    ws->score[i] = ws->fws->sws->fs.scores[1];
    
    printf("%g\n", ws->score[i]);

    if (Local_Neuroseg_Good_Score(ws->locseg + i, ws->score[i], ws->min_score) 
	== TRUE) {
      Local_Neuroseg_Label_G(ws->locseg + i, ws->base_mask, -1, 2, z_scale);
    } else {
      Local_Neuroseg_Label_G(ws->locseg + i, ws->base_mask, -1, 1, z_scale);
    }
  }
}

Connection_Test_Workspace* New_Connection_Test_Workspace()
{
  Connection_Test_Workspace *ctw = (Connection_Test_Workspace*)
    Guarded_Malloc(sizeof(Connection_Test_Workspace), 
		   "New_Connection_Test_Workspace");

  Default_Connection_Test_Workspace(ctw);

  return ctw;
}

void Delete_Connection_Test_Workspace(Connection_Test_Workspace *ctw)
{
  free(ctw);
}

void Kill_Connection_Test_Workspace(Connection_Test_Workspace *ctw)
{
  Delete_Connection_Test_Workspace(ctw);
}

void Default_Connection_Test_Workspace(Connection_Test_Workspace *ctw)
{
  ctw->hook_spot = -1;
  ctw->dist = NEUROSEG_DEFAULT_H * 10.0;
  ctw->cos1 = 0.0;
  ctw->cos2 = 0.0;
  //ctw->dist_thre = NEUROSEG_DEFAULT_H / 2.0;
  ctw->dist_thre = NEUROSEG_DEFAULT_H;
  ctw->good_dist = FALSE;
  ctw->resolution[0] = 1.0;
  ctw->resolution[1] = 1.0;
  ctw->resolution[2] = 1.0;
  ctw->unit = 'p';
  ctw->big_euc = 15.0;
  ctw->big_planar = 10.0;
  ctw->sp_test = TRUE;
  ctw->interpolate = TRUE;
  ctw->crossover_test = FALSE;
  //ctw->big_euc = 2.0;
  //ctw->big_planar = 2.5;
  ctw->mask = NULL;
}

void Connection_Test_Workspace_Read_Resolution(Connection_Test_Workspace *ctw,
					       const char *filepath)
{
  FILE *fp = fopen(filepath, "r");
  if (fp != NULL) {
    if (darray_fscanf(fp, ctw->resolution, 3) != 3) {
      fprintf(stderr, "Faield to load %s\n", filepath);
      ctw->resolution[0] = 1.0;
      ctw->resolution[1] = 1.0;
      ctw->resolution[2] = 1.0;
    }
    fclose(fp);
  }

  if (ctw->mask != NULL) {
    Kill_Stack(ctw->mask);
    ctw->mask = NULL;
  }
}
