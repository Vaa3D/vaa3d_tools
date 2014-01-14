/* tz_locne_chain.c
 *
 * 06-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_vrml_io.h"
#include "tz_geo3d_point_array.h"
#include "tz_geo3d_utils.h"
#include "tz_locne_chain.h"
#include "tz_trace_utils.h"

#define LOCAL_NEUROSEG_ELLIPSE_RADIUS(locne) ((locne->np.rx + locne->np.ry)/2.0)

int Locne_Chain_Trace_Test(void *argv[])
{
  Local_Neuroseg_Ellipse *locne = (Local_Neuroseg_Ellipse *) argv[0];
  Locne_Chain *chain = (Locne_Chain *) argv[1];
  Trace_Workspace *tw = (Trace_Workspace *) argv[2];
  Trace_Record *tr = (Trace_Record *) argv[3];
  double z_scale = *((double *) argv[4]);
  double max_r = tw->dyvar[0];
  double max_ecc = tw->dyvar[1];

  ASSERT(chain != NULL, "Invalid tracing result");

  double center[3];
  Local_Neuroseg_Ellipse_Center(locne, center);

  if (tr->fs.scores[1] < tw->min_score) {
    printf("low score\n");
    return TRACE_LOW_SCORE;
  }

  if (LOCAL_NEUROSEG_ELLIPSE_RADIUS(locne) > max_r) {
    printf("too large\n");
    return TRACE_TOO_LARGE;
  }

  Local_Neuroseg_Ellipse *last_locne = Locne_Chain_Tail_Seg(chain);
  double center2[3];
  double npr = dmax2(LOCAL_NEUROSEG_ELLIPSE_RADIUS(last_locne), 
		     LOCAL_NEUROSEG_ELLIPSE_RADIUS(locne)) * 2.0 + 2.0;
  Local_Neuroseg_Ellipse_Center(last_locne, center2);
  if (Geo3d_Dist_Sqr(locne->np.offset_x, locne->np.offset_y, 0, 
		     last_locne->np.offset_x, last_locne->np.offset_y, 0) 
      > npr * npr) {
    printf("too much shift\n");
    return TRACE_INVALID_SHAPE;
  }

  
  double ecc = locne->np.ry / locne->np.rx;
  if (ecc < 1.0) {
    ecc = 1.0 / ecc;
  }
  if (ecc > max_ecc) {
    printf("invalid shape: %g\n", ecc);
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


Locne_Chain* Locne_Chain_Trace_Init(const Stack *stack, double z_scale, 
				    Local_Neuroseg_Ellipse *locne,
				    void *ws)
{
  Stack_Fit_Score *fs = (Stack_Fit_Score*) ws;

  int var_index[LOCAL_NEUROSEG_ELLIPSE_NPARAM];
  int nvar = 
    Local_Neuroseg_Ellipse_Var_Mask_To_Index(NEUROSEG_ELLIPSE_VAR_MASK_R | 
					     NEUROSEG_ELLIPSE_VAR_MASK_OFFSET |
					     NEUROSEG_ELLIPSE_VAR_MASK_ALPHA,
					     NEUROPOS_VAR_MASK_NONE,
					     var_index);
  Fit_Local_Neuroseg_Ellipse(locne, stack, var_index, nvar, z_scale, fs);

  Trace_Record *tr = New_Trace_Record();
  tr->mask = ZERO_BIT_MASK;

  Locne_Node *p = Make_Locne_Node(locne, tr);
  Locne_Chain *chain = Make_Locne_Chain(p);
  
  return chain;
}


#define TRACE_LOCNE(current_locne, step, list_end, end_index)		\
  if (current_locne != NULL) {						\
    current_locne = Next_Neuroseg_Ellipse(current_locne, step);		\
    Fit_Local_Neuroseg_Ellipse(current_locne, stack, var_index, nvar, z_scale, &(tr.fs)); \
    argv[0] = current_locne;						\
    ASSERT(tw->test_func != NULL, "Null testing function.");		\
    cur_end_status = tw->test_func(argv);				\
    Locne_Node *current_node = NULL;					\
    if (cur_end_status == TRACE_NORMAL) {				\
      current_node = Make_Locne_Node(current_locne, Copy_Trace_Record(&tr)); \
      Locne_Chain_Add_Node(chain, current_node, list_end);		\
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
	current_node = Make_Locne_Node(current_locne,			\
				       Copy_Trace_Record(&tr));		\
	Locne_Chain_Add_Node(chain, current_node, list_end);		\
      } else {								\
	Delete_Local_Neuroseg_Ellipse(current_locne);			\
      }									\
      current_locne = NULL;						\
    }									\
  }

void Trace_Locne(const Stack *stack, double z_scale, Locne_Chain *chain, 
		 Trace_Workspace *tw)
{
  if ((stack == NULL) || (chain == NULL) || (chain->list == NULL)) {
    return;
  }

  int var_index[LOCAL_NEUROSEG_ELLIPSE_NPARAM];
  int nvar = 
    Local_Neuroseg_Ellipse_Var_Mask_To_Index(NEUROSEG_ELLIPSE_VAR_MASK_R | 
					     NEUROSEG_ELLIPSE_VAR_MASK_OFFSET |
					     NEUROSEG_ELLIPSE_VAR_MASK_ALPHA,
					     NEUROPOS_VAR_MASK_NONE,
					     var_index);



  Trace_Record tr;
  tr.mask = ZERO_BIT_MASK;
  tr.fs.n = 2;
  tr.fs.options[0] = STACK_FIT_DOT;
  tr.fs.options[1] = tw->tscore_option;

  Dlist_Direction_e trace_direction = (Dlist_Direction_e)DL_UNDEFINED;

  /* make parameters for validity testing */
  void *argv[5];
  argv[0] = NULL;
  argv[1] = chain;
  argv[2] = tw;
  argv[3] = &tr;
  argv[4] = &z_scale;

  Locne_Node *current_ends[2]; /* two ends: 0 head 1 tail*/
  current_ends[0] = Locne_Chain_Head(chain);
  current_ends[1] = Locne_Chain_Tail(chain);

  Local_Neuroseg_Ellipse *forward_locne = NULL;
  Local_Neuroseg_Ellipse *backward_locne = NULL;

  if (tw->trace_status[1] == TRACE_NORMAL) { /* tail, go forward */
    forward_locne = current_ends[1]->locseg;
  }

  if (tw->trace_status[0] == TRACE_NORMAL) { /* head, go backward */
    backward_locne = current_ends[0]->locseg;
  }

  if (tw->fit_first == TRUE) {
    if (tw->trace_status[1] == TRACE_NORMAL) {
      Fit_Local_Neuroseg_Ellipse(forward_locne, stack, var_index, nvar, 
				 z_scale, &(tr.fs));
      argv[0] = forward_locne;
      trace_direction = DL_FORWARD;
      tw->trace_status[1] = tw->test_func(argv);
    } else if (tw->trace_status[0] == TRACE_NORMAL) {
      Fit_Local_Neuroseg_Ellipse(backward_locne, stack, var_index, nvar, 
				 z_scale, &(tr.fs));
      argv[0] = backward_locne;
      trace_direction = DL_BACKWARD;
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
      TRACE_LOCNE(backward_locne, -step, DL_HEAD, end_index);
    }
    end_index = 1;
    if (tw->trace_status[end_index] == TRACE_NORMAL) {
      Reset_Trace_Record(&tr);
      TRACE_LOCNE(forward_locne, step, DL_TAIL, end_index);
    }
  }
}


void Locne_Chain_Subchain(Locne_Chain *chain, int start, int end)
{
  printf("%d, %d\n", start, end);

  int length = Locne_Chain_Length(chain);
  int i;
  for (i = 0; i < start; i++) {
    if (chain->list == NULL) {
      return;
    }
    Locne_Chain_Remove_End(chain, DL_HEAD);
  }

  for (i = length - 1; i > end; i--) {
    if (chain->list == NULL) {
      return;
    }
    Locne_Chain_Remove_End(chain, DL_TAIL);
  }
}

double* Locne_Chain_Area_Array(Locne_Chain *chain, double *a, int *length)
{
  *length = Locne_Chain_Length(chain);
  if (a == NULL) {
    a = darray_malloc(*length);
  }
  
  Locne_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg_Ellipse *locne;
  int i = 0;
  while ((locne = Locne_Chain_Next_Seg(chain)) != NULL) {
    a[i++] = locne->np.rx * locne->np.ry;
  }
  
  ASSERT(i == *length, "Unmatched length");

  return a;
}

BOOL Locne_Chain_Hittest(Locne_Chain *chain, const coordinate_3d_t point)
{
  BOOL hit;

  int length = Locne_Chain_Length(chain);
  coordinate_3d_t *pos = (coordinate_3d_t *)
    Guarded_Malloc(sizeof(coordinate_3d_t) * length, "Locne_Chain_Hitpoint");

  Locne_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg_Ellipse *locne = NULL;

  int i = 0;
  while ((locne = Locne_Chain_Next_Seg(chain)) != NULL) {
    Coordinate_3d_Copy(pos[i++], locne->pos);
  }

  locne = Locne_Chain_Head_Seg(chain);
  coordinate_3d_t ort;
  Geo3d_Orientation_Normal(locne->np.theta, locne->np.psi, 
			   ort, ort + 1, ort + 2);
  
  coordinate_3d_t registered_point;
  registered_point[0] = point[0] - pos[0][0];
  registered_point[1] = point[1] - pos[0][1];
  registered_point[2] = point[2] - pos[0][2];

  double point_proj = Coordinate_3d_Dot(ort, registered_point);
  
  double span = Coordinate_3d_Distance(pos[0], pos[length - 1]);
  
  if ((point_proj < 0) || (point_proj > span)) {
    hit = FALSE;
  } else {
    double range[2];
    Local_Neuroseg_Ellipse *checking_locne[2];
    
    range[0] = 0.0;
    Locne_Chain_Iterator_Start(chain, DL_HEAD);
    checking_locne[0] = Locne_Chain_Next_Seg(chain);
    
    for (i = 1; i < length; i++) {
      range[1] = Coordinate_3d_Distance(pos[0], pos[i]);
      checking_locne[1] = Locne_Chain_Next_Seg(chain);
      if ((point_proj >= range[0]) && (point_proj <= range[1])) {
	double w = point_proj - range[0];
	double rx = (1.0 - w) * checking_locne[0]->np.rx + 
	  w * checking_locne[1]->np.rx;
	double ry = (1.0 - w) * checking_locne[0]->np.ry + 
	  w * checking_locne[1]->np.ry;
	double offset_x = (1.0 - w) * checking_locne[0]->np.offset_x + 
	  w * checking_locne[1]->np.offset_x;
	double offset_y = (1.0 - w) * checking_locne[0]->np.offset_y + 
	  w * checking_locne[1]->np.offset_y;
	coordinate_3d_t center;
	center[0] = (1.0 - w) * checking_locne[0]->pos[0] + 
	  w * checking_locne[1]->pos[0];
	center[1] = (1.0 - w) * checking_locne[0]->pos[1] + 
	  w * checking_locne[1]->pos[1];
	center[2] = (1.0 - w) * checking_locne[0]->pos[2] + 
	  w * checking_locne[1]->pos[2];
	registered_point[0] = point[0] - center[0];
	registered_point[1] = point[1] - center[1];
	registered_point[2] = point[2] - center[2];
	Geo3d_Rotate_Coordinate(registered_point, registered_point + 1,
				registered_point + 2, 
				checking_locne[0]->np.theta,
				checking_locne[0]->np.psi, 1);
	double r = dsqr(registered_point[0] - offset_x) / rx / rx +
	  dsqr(registered_point[1] - offset_y) / ry / ry;
	hit = (r <= 1.0);
	break;
      }
      range[0] = range[1];
      checking_locne[0] = checking_locne[1];
    }
  }

  free(pos);

  return hit;
}

double Locne_Chain_Span(Locne_Chain *chain)
{
  Local_Neuroseg_Ellipse *head = Locne_Chain_Head_Seg(chain);
  Local_Neuroseg_Ellipse *tail = Locne_Chain_Tail_Seg(chain);

  return Coordinate_3d_Distance(head->pos, tail->pos);
}

void Locne_Chain_Vrml_Fprint(FILE *fp, Locne_Chain *chain, 
			     const Vrml_Material *material, int indent)
{
  const int nsample = 36;

  coordinate_3d_t *top = (coordinate_3d_t *)
    Guarded_Malloc(sizeof(coordinate_3d_t) * nsample, 
		   "Locne_Chain_Vrml_Fprint");
  coordinate_3d_t *bottom = (coordinate_3d_t *)
    Guarded_Malloc(sizeof(coordinate_3d_t) * nsample, 
		   "Locne_Chain_Vrml_Fprint");

  Locne_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg_Ellipse *locne = Locne_Chain_Next_Seg(chain);
  Local_Neuroseg_Ellipse_Points(locne, nsample, 0.0, top);

  while ((locne = Locne_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Ellipse_Points(locne, nsample, 0.0, bottom);
    Vrml_Platform_Fprint(fp, Coordinate_3d_Double_Array(top),
			 Coordinate_3d_Double_Array(bottom), nsample, 
			 NULL, indent);
    Geo3d_Point_Array_Copy(top, (const coordinate_3d_t*) bottom, nsample);
  }

  free(top);
  free(bottom);
}

int Locne_Chain_Swc_Fprint(FILE *fp, Locne_Chain *chain, int type,
			   int start_id, int parent_id, 
			   Dlist_Direction_e direction)
{
  int n = Locne_Chain_Length(chain);
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

  Locne_Chain_Iterator_Start(chain, DL_HEAD);

  for (i = start; i != end; i += step) {
    Local_Neuroseg_Ellipse *locne = Locne_Chain_Next_Seg(chain);

    Swc_Node cell;
    cell.id = start_id + i;
    cell.type = type;
    cell.d = sqrt(locne->np.rx * locne->np.ry);
    cell.x = locne->pos[0];
    cell.y = locne->pos[1];
    cell.z = locne->pos[2];
    cell.parent_id = cell.id - step;
    if (i == start) {
      cell.parent_id = parent_id;
    }
    Swc_Node_Fprint(fp, &cell);

    //Print_Geo3d_Circle(circles+i);
  }

  return start_id + i;
}

Trace_Workspace *Make_Locne_Trace_Workspace(Dlist_Direction_e trace_direction,
					    int max_length, double min_score,
					    double max_r, double max_ecc)
{
  Trace_Workspace *tw = New_Trace_Workspace();
  switch(trace_direction) {
  case DL_FORWARD:
    tw->trace_status[0] = TRACE_NORMAL;
    tw->trace_status[1] = TRACE_NOT_ASSIGNED;
    break;
  case DL_BACKWARD:
    tw->trace_status[1] = TRACE_NORMAL;
    tw->trace_status[0] = TRACE_NOT_ASSIGNED;
    break;
  case DL_BOTHDIR:
    tw->trace_status[0] = TRACE_NORMAL;
    tw->trace_status[1] = TRACE_NORMAL;
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  tw->length = max_length;
  tw->min_score = min_score;
  tw->dyvar[0] = max_r;
  tw->dyvar[1] = max_ecc;
  tw->test_func = Locne_Chain_Trace_Test;

  return tw;
}
