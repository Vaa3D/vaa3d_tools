/* tz_local_neuroseg_ellipse.c
 *
 * 20-Apr-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utilities.h>
#include <math.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_darray.h"
#include "tz_perceptor.h"
#include "tz_geo3d_point_array.h"
#include "tz_3dgeom.h"
#include "tz_geo3d_utils.h"
#include "tz_local_neuroseg_ellipse.h"
#include "tz_geoangle_utils.h"
#include "tz_stack_sampling.h"
#include "tz_stack_attribute.h"
#include "tz_stack_draw.h"
#include "tz_math.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_utils.h"

const double Delta[] = 
  {0.5, 0.5, 0.015, 0.015, 0.5, 0.5, 0.015, 0.5, 0.5, 0.5};

Local_Neuroseg_Ellipse* New_Local_Neuroseg_Ellipse()
{
  Local_Neuroseg_Ellipse *locnp = (Local_Neuroseg_Ellipse *) 
    Guarded_Malloc(sizeof(Local_Neuroseg_Ellipse), "New_Local_Neuroseg_Ellipse");

  Default_Local_Neuroseg_Ellipse(locnp);
  
  return locnp;
}

void Delete_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locnp)
{
  free(locnp);
}

void Set_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locnp, 
				double rx, double ry, double theta, double psi,
				double alpha,
				double offset_x, double offset_y,
				double x, double y, double z)
{
  Set_Neuroseg_Ellipse(&(locnp->np), rx, ry, theta, psi, alpha,
		       offset_x, offset_y);
  Set_Neuropos(locnp->pos, x, y, z);
}

void Default_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locnp)
{
  Set_Local_Neuroseg_Ellipse(locnp, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			     0.0, 0.0, 0.0);
}

void Print_Local_Neuroseg_Ellipse(const Local_Neuroseg_Ellipse *locnp)
{
  printf("Local neuroseg plane: \n");
  printf("r: (%g, %g), theta: %g, psi: %g, alpha: %g, offset: [%g, %g], position(%g, %g, %g)\n",
	 locnp->np.rx, locnp->np.ry, locnp->np.theta, locnp->np.psi, 
	 locnp->np.alpha,
	 locnp->np.offset_x, locnp->np.offset_y,
	 locnp->pos[0], locnp->pos[1], locnp->pos[2]);
}

Local_Neuroseg_Ellipse* 
Copy_Local_Neuroseg_Ellipse(const Local_Neuroseg_Ellipse *locnp)
{
  Local_Neuroseg_Ellipse *locnp_copy = New_Local_Neuroseg_Ellipse();
  memcpy(locnp_copy, locnp, sizeof(Local_Neuroseg_Ellipse));

  return locnp_copy;
}

void Local_Neuroseg_Ellipse_Fwrite(const Local_Neuroseg_Ellipse *locne,
				   FILE *fp)
{
  Neuroseg_Ellipse_Fwrite(&(locne->np), fp);
  Neuropos_Fwrite(locne->pos, fp);
}

Local_Neuroseg_Ellipse* Local_Neuroseg_Ellipse_Fread(Local_Neuroseg_Ellipse *locnp, FILE *fp)
{
  if (feof(fp)) {
    return NULL;
  }

  BOOL is_local_alloc = FALSE;
  if (locnp == NULL) {
    locnp = New_Local_Neuroseg_Ellipse();
    is_local_alloc = TRUE;
  }

  if (Neuroseg_Ellipse_Fread(&(locnp->np), fp) == NULL) {
    if (is_local_alloc == TRUE) {
      Delete_Local_Neuroseg_Ellipse(locnp);
    }
    locnp = NULL;
  } else {
    if (Neuropos_Fread(locnp->pos, fp) == NULL) {
      if (is_local_alloc == TRUE) {
	Delete_Local_Neuroseg_Ellipse(locnp);
      }
      locnp = NULL;
    }
  }

  return locnp;
}

void Local_Neuroseg_Ellipse_Swc_Fprint_T(FILE *fp, 
					 const Local_Neuroseg_Ellipse *locne,
					 int start_id, int parent_id, int type)
{
  TZ_ASSERT(fp != NULL, "Null file pointer.");

  Geo3d_Circle *circle = New_Geo3d_Circle();

  Local_Neuroseg_Ellipse_To_Geo3d_Circle(locne, circle);
  Geo3d_Circle_Swc_Fprint_T(fp, circle, start_id, parent_id, type, 1.0);

  Kill_Geo3d_Circle(circle);  
}

Geo3d_Circle* 
Local_Neuroseg_Ellipse_To_Geo3d_Circle(const Local_Neuroseg_Ellipse *locne,
				       Geo3d_Circle *circle)
{
  if (circle == NULL) {
    circle = New_Geo3d_Circle();
  }

  circle->radius = sqrt(locne->np.rx * locne->np.ry);
  circle->center[0] = locne->pos[0];
  circle->center[1] = locne->pos[1];
  circle->center[2] = locne->pos[2];
  circle->orientation[0] = locne->np.theta;
  circle->orientation[1] = locne->np.psi;

  return circle;
}

double Local_Neuroseg_Ellipse_Score(const Local_Neuroseg_Ellipse *locnp, 
				  const Stack *stack, 
				  double z_scale, Stack_Fit_Score *fs)
{
  double sample_step = 1.0;
  Geo3d_Scalar_Field *field = Local_Neuroseg_Ellipse_Field(locnp, sample_step,
							 NULL);
  double score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

  Kill_Geo3d_Scalar_Field(field);
  
  return score;
}

int Local_Neuroseg_Ellipse_Var(const Local_Neuroseg_Ellipse *locnp, 
			     double *var[])
{
  int nvar = Neuroseg_Ellipse_Var(&(locnp->np), var);
  Neuropos_Var(locnp->pos, var + nvar);

  return LOCAL_NEUROSEG_ELLIPSE_NPARAM;
}

void Local_Neuroseg_Ellipse_Set_Var(Local_Neuroseg_Ellipse *locnp, 
				  int var_index, double value)
{
  double *var[LOCAL_NEUROSEG_ELLIPSE_NPARAM];
  Local_Neuroseg_Ellipse_Var(locnp, var);
  *(var[var_index]) = value;
}

void Local_Neuroseg_Ellipse_Set_Var_S(Local_Neuroseg_Ellipse *locnp, 
				  int var_index, double value)
{
  double *var[LOCAL_NEUROSEG_ELLIPSE_NPARAM];
  Local_Neuroseg_Ellipse_Var(locnp, var);
  *(var[var_index]) = value * Delta[var_index];
}

Geo3d_Scalar_Field* 
Local_Neuroseg_Ellipse_Field(const Local_Neuroseg_Ellipse *locnp, double step,
			   Geo3d_Scalar_Field *field)
{
  field = Neuroseg_Ellipse_Field(&(locnp->np), step, field);
  Geo3d_Point_Array_Translate(field->points, field->size, locnp->pos[0],
			      locnp->pos[1], locnp->pos[2]);
  return field;
}

int Local_Neuroseg_Ellipse_Param_Array(const Local_Neuroseg_Ellipse *locnp, 
				     double z_scale, double *param)
{
  int i;
  double *var[LOCAL_NEUROSEG_ELLIPSE_NPARAM];
  Local_Neuroseg_Ellipse_Var(locnp, var);
  for (i = 0; i < LOCAL_NEUROSEG_ELLIPSE_NPARAM; i++) {
    param[i] = *(var[i]);
  }

  param[LOCAL_NEUROSEG_ELLIPSE_NPARAM] = z_scale;
  
  return LOCAL_NEUROSEG_ELLIPSE_NPARAM + 1;
}

int Local_Neuroseg_Ellipse_Param_Array_S(const Local_Neuroseg_Ellipse *locnp, 
				     double z_scale, double *param)
{
  int i;
  double *var[LOCAL_NEUROSEG_ELLIPSE_NPARAM];
  Local_Neuroseg_Ellipse_Var(locnp, var);
  for (i = 0; i < LOCAL_NEUROSEG_ELLIPSE_NPARAM; i++) {
    param[i] = *(var[i]) / Delta[i];
  }

  param[LOCAL_NEUROSEG_ELLIPSE_NPARAM] = z_scale;
  
  return LOCAL_NEUROSEG_ELLIPSE_NPARAM + 1;
}

int Local_Neuroseg_Ellipse_Var_Mask_To_Index(Bitmask_t plane_mask, 
					   Bitmask_t neuropos_mask, int *index)
{
  int nvar1 = Neuroseg_Ellipse_Var_Mask_To_Index(plane_mask, index);
  int nvar2 = Neuropos_Var_Mask_To_Index(neuropos_mask, index + nvar1);
  
  int i;
  for (i = 0; i < nvar2; i++) {
    index[nvar1 + i] += NEUROSEG_ELLIPSE_NPARAM;
  }

  return nvar1 + nvar2;
}

void Local_Neuroseg_Ellipse_Validate(double *var, const double *var_min, 
				     const double *var_max, const void *param)
{
  darray_threshold(var, LOCAL_NEUROSEG_ELLIPSE_NPARAM, var_min, var_max);
}

double Local_Neuroseg_Ellipse_Score_G(const double *var, const Stack *stack)
{
  Local_Neuroseg_Ellipse *locnp = New_Local_Neuroseg_Ellipse();
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_ELLIPSE_NPARAM; i++) {
    Local_Neuroseg_Ellipse_Set_Var(locnp, i, var[i]);
  }

  double z_scale = var[LOCAL_NEUROSEG_ELLIPSE_NPARAM];

  double score = Local_Neuroseg_Ellipse_Score(locnp, stack, z_scale, NULL);

  Delete_Local_Neuroseg_Ellipse(locnp);

  return score;
}

double Local_Neuroseg_Ellipse_Score_Gv(const double *var, const void *param)
{
  return Local_Neuroseg_Ellipse_Score_G(var, (const Stack *) param);
}

double Local_Neuroseg_Ellipse_Score_S(const double *var, const void *param)
{
  void **param_array = (void**) param;
  const Stack *stack = (const Stack*) param_array[0];

  Local_Neuroseg_Ellipse *locnp = New_Local_Neuroseg_Ellipse();
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_ELLIPSE_NPARAM; i++) {
    Local_Neuroseg_Ellipse_Set_Var(locnp, i, var[i]);
  }

  double z_scale = var[LOCAL_NEUROSEG_ELLIPSE_NPARAM];

  double score = Local_Neuroseg_Ellipse_Score(locnp, stack, z_scale, NULL);

  Delete_Local_Neuroseg_Ellipse(locnp);

  return score;
}

double Fit_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locnp, 
				  const Stack *stack, 
				  const int *var_index, int nvar, 
				  double z_scale,
				  Stack_Fit_Score *fs)
{
  ASSERT(stack != NULL, "Null stack.");
  ASSERT(locnp != NULL, "Null segment plane.");

  double *var = darray_malloc(LOCAL_NEUROSEG_ELLIPSE_NPARAM + 1);
  Local_Neuroseg_Ellipse_Param_Array_S(locnp, z_scale, var);


  Perceptor perceptor;
  perceptor.vs = New_Variable_Set();
  perceptor.vs->nvar = nvar;
  perceptor.vs->var_index = (int *) var_index;
  perceptor.vs->var = var;
  perceptor.min_gradient = 1e-5;

  //double delta[] = {0.5, 0.5, 0.015, 0.015, 0.5, 0.5, 0.5, 0.5, 0.5, 0.1};
  //double delta[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  perceptor.delta = (double *) Delta;
  perceptor.weight = NULL;

  double var_min[] = {1.0, 1.0, -Infinity, -Infinity, -Infinity, -Infinity, 
		      -Infinity, -Infinity, -Infinity, 0.5};
  double var_max[] = {50.0, 50.0, Infinity, Infinity, Infinity, Infinity, 
		      Infinity, Infinity, Infinity, 6.0};
  var_min[0] /= Delta[0];
  var_max[0] /= Delta[0];
  var_min[1] /= Delta[1];
  var_max[1] /= Delta[1];
  
  perceptor.s = 
    Make_Continuous_Function(Local_Neuroseg_Ellipse_Score_S, 
			     Local_Neuroseg_Ellipse_Validate,
			     var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  int i;
  for (i = 0; i < LOCAL_NEUROSEG_ELLIPSE_NPARAM; i++) {
    Local_Neuroseg_Ellipse_Set_Var_S(locnp, i, perceptor.vs->var[i]);
  }

  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  return Local_Neuroseg_Ellipse_Score(locnp, stack, z_scale, fs);  
}

void Local_Neuroseg_Ellipse_Stack_Centroid(Local_Neuroseg_Ellipse *locne, 
    const Stack *stack, const Stack *mask, double *centroid)
{
  centroid[0] = 0.0;
  centroid[1] = 0.0;
  centroid[2] = 0.0;
  double tw = 0.0;
  Geo3d_Scalar_Field *field = Local_Neuroseg_Ellipse_Field(locne, 1.0, NULL);
  int i;
  for (i = 0; i < field->size; i++) {
    if ((mask == NULL) || (Stack_Point_Sampling(mask, field->points[i][0],
	    field->points[i][1], field->points[i][2]) == 0.0)) {
      double w = Stack_Point_Sampling(stack, field->points[i][0],
	  field->points[i][1], field->points[i][2]);
      if (!isnan(w)) {
	tw += w;
	centroid[0] += field->points[i][0] * w;
	centroid[1] += field->points[i][1] * w;
	centroid[2] += field->points[i][2] * w;
      }
    }
  }

  if (tw != 0.0) {
    centroid[0] /= tw;
    centroid[1] /= tw;
    centroid[2] /= tw;
  }
  Kill_Geo3d_Scalar_Field(field);
}

static void local_neuroseg_ellipse_reset_center(Local_Neuroseg_Ellipse *locne,
    const double *center)
{
  locne->np.offset_x = 0.0;
  locne->np.offset_y = 0.0;
  locne->pos[0] = center[0];
  locne->pos[1] = center[1];
  locne->pos[2] = center[2];
}

void Local_Neuroseg_Ellipse_Reset_Center(Local_Neuroseg_Ellipse *locne,
    const double *center)
{
  local_neuroseg_ellipse_reset_center(locne, center);
}


double Local_Neuroseg_Ellipse_Optimize_W(Local_Neuroseg_Ellipse *locne, 
    const Stack *stack, double z_scale, Receptor_Fit_Workspace *ws)
{
  TZ_ASSERT(z_scale == 1.0, "Non-unit z-scale is not supported yet.");
  
  Fit_Local_Neuroseg_Ellipse_W(locne, stack, z_scale, ws);
  double center[3];
  int k;
  for (k = 0; k < 20; k++) {
    Local_Neuroseg_Ellipse_Stack_Centroid(locne, stack, ws->sws->mask, center);
    local_neuroseg_ellipse_reset_center(locne, center);
  }
  return Fit_Local_Neuroseg_Ellipse_W(locne, stack, z_scale, ws);
}

double Local_Neuroseg_Ellipse_Score_W(const Local_Neuroseg_Ellipse *locseg, 
				      const Stack *stack, 
				      double z_scale, 
				      Locseg_Ellipse_Score_Workspace *ws)
{
  double sample_step = 1.0;
  Geo3d_Scalar_Field *field = 
    Local_Neuroseg_Ellipse_Field(locseg, sample_step, NULL);

  double score = 0.0;
  if (ws->mask == NULL) {
    score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, &(ws->fs));
  } else {
    score = Geo3d_Scalar_Field_Stack_Score_M(field, stack, z_scale, ws->mask,
					     &(ws->fs));
  }

  Kill_Geo3d_Scalar_Field(field);
  
  return score;  
}

double 
Fit_Local_Neuroseg_Ellipse_W(Local_Neuroseg_Ellipse *locne, const Stack *stack, 
			     double z_scale, Locseg_Fit_Workspace *ws)
{
  if ((stack == NULL) || (locne == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_NEUROSEG_ELLIPSE_NPARAM + 1);
  
  double *var = darray_malloc(LOCAL_NEUROSEG_ELLIPSE_NPARAM + 1);

  Local_Neuroseg_Ellipse_Param_Array(locne, z_scale, var);

  Perceptor perceptor;
  perceptor.vs = New_Variable_Set();
  perceptor.vs->nvar = ws->nvar;
  perceptor.vs->var_index = (int *) ws->var_index;
  perceptor.vs->var = var;
  perceptor.vs->link = ws->var_link;
  perceptor.min_gradient = 1e-3;
    
  perceptor.arg = ws->sws;

  double *delta = (double *) Delta;
  perceptor.delta = delta;

  int i;
  for (i = 0; i < perceptor.vs->nvar; i++) {
    weight[i] = delta[perceptor.vs->var_index[i]];
  }
  double wl = darray_norm(weight, perceptor.vs->nvar);
  
  for (i = 0; i < perceptor.vs->nvar; i++) {
    weight[i] /= wl;
  }
  perceptor.weight = weight;
  
  perceptor.s = 
    Make_Continuous_Function(Local_Neuroseg_Ellipse_Score_S, 
			     Local_Neuroseg_Ellipse_Validate,
			     ws->var_min, ws->var_max);
  
  Fit_Perceptor(&perceptor, stack);
  
  for (i = 0; i < LOCAL_NEUROSEG_ELLIPSE_NPARAM; i++) {
    Local_Neuroseg_Ellipse_Set_Var(locne, i, perceptor.vs->var[i]);
  }
  
  Delete_Variable_Set(perceptor.vs);
  free(var);
  free(weight);
  Free_Continuous_Function(perceptor.s);
  
  locne->np.theta = Normalize_Radian(locne->np.theta);
  locne->np.psi = Normalize_Radian(locne->np.psi);
  locne->np.alpha = Normalize_Radian(locne->np.alpha);
  
  return Local_Neuroseg_Ellipse_Score_W(locne, stack, z_scale, ws->sws);
}

void Local_Neuroseg_Ellipse_Center(const Local_Neuroseg_Ellipse *locnp,
				 double *center)
{
  center[0] = locnp->np.offset_x;
  center[1] = locnp->np.offset_y;
  center[2] = 0;

  Rotate_XZ(center, center, 1, locnp->np.theta, locnp->np.psi, 0);

  center[0] += locnp->pos[0];
  center[1] += locnp->pos[1];
  center[2] += locnp->pos[2];
}

Local_Neuroseg_Ellipse*
Next_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locnp, double step)
{
  Local_Neuroseg_Ellipse *next = Copy_Local_Neuroseg_Ellipse(locnp);

  double dx, dy, dz;
  Geo3d_Orientation_Normal(locnp->np.theta, locnp->np.psi, &dx, &dy, &dz);
  dx *= step;
  dy *= step;
  dz *= step;
  Neuropos_Translate(next->pos, dx, dy, dz);

  return next;
}

void Local_Neuroseg_Ellipse_Points(const Local_Neuroseg_Ellipse *locne,
				   int npt, double start, 
				   coordinate_3d_t coord[])
{
  Neuroseg_Ellipse_Points(&(locne->np), npt, start, coord);
  Geo3d_Point_Array_Translate(coord, npt, locne->pos[0], locne->pos[1],
			      locne->pos[2]);
}

double Local_Neuroseg_Ellipse_Point_Angle(const Local_Neuroseg_Ellipse *locne,
					   const coordinate_3d_t coord)
{
  coordinate_3d_t coord2;
  Local_Neuroseg_Ellipse_Center(locne, coord2);

  coord2[0] = coord[0] - coord2[0];
  coord2[1] = coord[1] - coord2[1];
  coord2[2] = coord[2] - coord2[2];

  return Neuroseg_Ellipse_Vector_Angle(&(locne->np), coord2);
}

coordinate_3d_t* 
Local_Neuroseg_Ellipse_Array_Points(const Local_Neuroseg_Ellipse *locne, 
				    int nobj, int nsample, coordinate_3d_t *pts)
{
  if (pts == NULL) {
    pts = (coordinate_3d_t *) 
      Guarded_Malloc(sizeof(coordinate_3d_t) * nobj * nsample,
		     "Local_Neuroseg_Ellipse_Array_Points");
  }
  
  coordinate_3d_t *pts_head = pts;

  const Local_Neuroseg_Ellipse *bottom = &(locne[0]);
  const Local_Neuroseg_Ellipse *top = &(locne[1]);

  xz_orientation_t bottom_orientation;
  xz_orientation_t top_orientation;
  coordinate_3d_t plane_normal;
  Set_Xz_Orientation(bottom_orientation, bottom->np.theta, bottom->np.psi);
  Set_Xz_Orientation(top_orientation, top->np.theta, top->np.psi);
  Xz_Orientation_Cross(bottom_orientation, top_orientation, plane_normal);
  //  Coordinate_3d_Unitize(plane_normal);

  xz_orientation_t plane_ort;
  /* 0.001 is picked to test if two vectors are parallel */
  if (Coordinate_3d_Norm(plane_normal) < 0.001) {
    /* pick a plane that is not parallel to the circles */
    plane_ort[0] = bottom_orientation[0] + TZ_PI_2;
    plane_ort[1] = bottom_orientation[1];
    Geo3d_Orientation_Normal(plane_ort[0], plane_ort[1], plane_normal,
			     plane_normal + 1, plane_normal + 2);
  } else {
    Coordinate_3d_Unitize(plane_normal);
  }

  double start1 = 
    Neuroseg_Ellipse_Vector_Angle(&(bottom->np), plane_normal);
  Local_Neuroseg_Ellipse_Points(bottom, nsample, start1, pts);

  double start2 = 
    Neuroseg_Ellipse_Vector_Angle(&(top->np), plane_normal);
  pts += nsample;
  Local_Neuroseg_Ellipse_Points(top, nsample, start2, pts);

  
  int i;
  double prev_start = start2;
  for (i = 1; i < nobj - 1; i++) {
    bottom = &(locne[i]);
    top = &(locne[i + 1]);
    Set_Xz_Orientation(bottom_orientation, bottom->np.theta, bottom->np.psi);
    Set_Xz_Orientation(top_orientation, top->np.theta, top->np.psi);
    Xz_Orientation_Cross(bottom_orientation, top_orientation, plane_normal);
    
    if (Coordinate_3d_Norm(plane_normal) < 0.001) {
      /* pick a plane that is not parallel to the circles */
      plane_ort[0] = bottom_orientation[0] + TZ_PI_2;
      plane_ort[1] = bottom_orientation[1];
      Geo3d_Orientation_Normal(plane_ort[0], plane_ort[1], plane_normal,
			       plane_normal + 1, plane_normal + 2);
    } else {
      Coordinate_3d_Unitize(plane_normal);
    }
    
    double start1 = Neuroseg_Ellipse_Vector_Angle(&(bottom->np), plane_normal);
    double start2 = Neuroseg_Ellipse_Vector_Angle(&(top->np), plane_normal);
    
    start2 += prev_start - start1;

    pts += nsample;
    Local_Neuroseg_Ellipse_Points(top, nsample, start2, pts);

    prev_start = start2;
  }

  return pts_head;
}

Local_Neuroseg_Ellipse* Make_Local_Neuroseg_Ellipse_Array(int n)
{
  Local_Neuroseg_Ellipse *locne = (Local_Neuroseg_Ellipse *) 
    Guarded_Malloc(sizeof(Local_Neuroseg_Ellipse) * n, 
		   "Make_Geo3d_Circle_Array");
  int i;
  for (i = 0; i < n; i++) {
    Default_Local_Neuroseg_Ellipse(locne + i);
  }

  return locne;
}

void Local_Neuroseg_Ellipse_Normalize_Position(Local_Neuroseg_Ellipse *locne)
{
  if ((locne->np.offset_x != 0.0) || (locne->np.offset_y != 0.0)) {
    double pos[3];
    pos[0] = locne->np.offset_x;
    pos[1] = locne->np.offset_y;
    pos[2] = 0;
    Rotate_XZ(pos, pos, 1, locne->np.theta, locne->np.psi, 0);
    
    locne->pos[0] += pos[0];
    locne->pos[1] += pos[1];
    locne->pos[2] += pos[2];
    
    locne->np.offset_x = 0;
    locne->np.offset_y = 0;
  }
}

void Local_Neuroseg_Ellipse_Scale_Z(Local_Neuroseg_Ellipse *locne,
				    double z_scale)
{
  Local_Neuroseg_Ellipse_Normalize_Position(locne);

  locne->pos[2] /= z_scale;
  coordinate_3d_t normal;
  Geo3d_Orientation_Normal(locne->np.theta, locne->np.psi, 
			   normal, normal + 1, normal + 2);

  /* cross section stretch for z scale */
  ASSERT(locne->np.alpha == 0.0, "Alpha not allowed yet.");

  double factor = sqrt(dsqr(cos(locne->np.theta)) + 
		       dsqr(sin(locne->np.theta) / z_scale));

  locne->np.ry *=  factor;

  normal[2] /= z_scale;
  Coordinate_3d_Unitize(normal);
  Geo3d_Normal_Orientation(normal[0], normal[1], normal[2], 
			   &(locne->np.theta), &(locne->np.psi));  
}

Geo3d_Ellipse* 
Local_Neuroseg_Ellipse_To_Geo3d_Ellipse(const Local_Neuroseg_Ellipse *locne,
   Geo3d_Ellipse *ellipse)
{
  if (ellipse == NULL) {
    ellipse = New_Geo3d_Ellipse();
  }

  ellipse->radius = locne->np.ry;
  ellipse->scale = locne->np.rx / locne->np.ry;
  ellipse->alpha = locne->np.alpha;
  ellipse->orientation[0] = locne->np.theta;
  ellipse->orientation[1] = locne->np.psi;
  Local_Neuroseg_Ellipse_Center(locne, ellipse->center);

  return ellipse;
}

void Local_Neuroseg_Ellipse_Draw_Stack(const Local_Neuroseg_Ellipse *locne,
    Stack *stack, uint8_t r, uint8_t g, uint8_t b)
{
  TZ_ASSERT(Stack_Kind(stack) == COLOR, "Color stack required.");
  coordinate_3d_t pts[20];
  Local_Neuroseg_Ellipse_Points(locne, 20, 0, pts);
  int start[3], end[3];
  int i;
  int j;
  for (i = 0; i < 19; i++) {
    for (j = 0; j < 3; j++) {
      start[j] = iround(pts[i][j]);
      end[j] = iround(pts[i+1][j]);
    }
    Object_3d *line = Line_To_Object_3d(start, end);
    Stack_Draw_Object_C(stack, line, r, g, b);
    Kill_Object_3d(line);
  }
    
  for (j = 0; j < 3; j++) {
    start[j] = iround(pts[19][j]);
    end[j] = iround(pts[0][j]);
  }
  Object_3d *line = Line_To_Object_3d(start, end);
  Stack_Draw_Object_C(stack, line, r, g, b);
  Kill_Object_3d(line);
}

static BOOL is_on_ellipse(coordinate_3d_t pt, double rx, double ry, 
    coordinate_3d_t normvec, coordinate_3d_t normxvec, 
    coordinate_3d_t normyvec)
{
  static const double eps = 0.01;
  if (Coordinate_3d_Dot(pt, normvec) < eps) {
    double px = Coordinate_3d_Dot(pt, normxvec);
    double py = Coordinate_3d_Dot(pt, normyvec);
    if ((px <= rx) && (py <= ry)) {
      if (px*px/rx/rx + py*py/ry/ry <= 1.0) {
	return TRUE;
      }
    }
  } 

  return FALSE;
}

void Local_Neuroseg_Ellipse_Label(const Local_Neuroseg_Ellipse *locne,
    Stack *stack, double z_scale, int flag, int value) 
{
  TZ_ASSERT(stack->kind == GREY, "Only grey stack is supported");

  coordinate_3d_t normvec, xvec, yvec;

  Geo3d_Orientation_Normal(locne->np.theta, locne->np.psi, normvec, normvec+1,
      normvec+2);
  xvec[0] = locne->np.rx;
  xvec[1] = 0.0;
  xvec[2] = 0.0;

  yvec[0] = 0.0;
  yvec[1] = locne->np.ry;
  yvec[2] = 0.0;

  Rotate_Z(xvec, xvec, 1, locne->np.alpha, 0);
  Rotate_XZ(xvec, xvec, 1, locne->np.theta, locne->np.psi, 0);
  Rotate_Z(yvec, yvec, 1, locne->np.alpha, 0);
  Rotate_XZ(yvec, yvec, 1, locne->np.theta, locne->np.psi, 0);

  Geo3d_Scalar_Field corners;
  corners.size = 4;
  coordinate_3d_t points[4];
  corners.points = points;
  corners.values = NULL;

  Coordinate_3d_Add(xvec, yvec, corners.points[0]);
  Coordinate_3d_Sub(xvec, yvec, corners.points[1]);
  Coordinate_3d_Neg(corners.points[0], corners.points[2]);
  Coordinate_3d_Neg(corners.points[1], corners.points[3]);

  coordinate_3d_t boundbox[2];
  Geo3d_Scalar_Field_Boundbox(&corners, boundbox);

  coordinate_3d_t center;
  Local_Neuroseg_Ellipse_Center(locne, center);
  Coordinate_3d_Add(boundbox[0], center, boundbox[0]);
  Coordinate_3d_Add(boundbox[1], center, boundbox[1]);

  int i, j, k;
  int x0, y0, z0;
  int x, y, z;
  x = x0 = (int) floor(boundbox[0][0]);
  y = y0 = (int) floor(boundbox[0][1]);
  z = z0 = (int) floor(boundbox[0][2]);
  int subwidth = iround(boundbox[1][0]) - x + 1;
  int subheight = iround(boundbox[1][1]) - y + 1;
  int subdepth = iround(boundbox[1][2]) - z + 1;
  coordinate_3d_t pt;
  int width = Stack_Width(stack);
  int area = Stack_Width(stack) * Stack_Height(stack);
  Coordinate_3d_Unitize(xvec);
  Coordinate_3d_Unitize(yvec);

  for (k = 0; k < subdepth; k++) {
    for (j = 0; j < subheight; j++) {
      for (i = 0; i < subwidth; i++) {
	size_t offset = STACK_UTIL_OFFSET(x, y, z, width, area);
	if (offset > 0) {
	  if ((flag == -1) || (stack->array[offset] == flag)) {
	    pt[0] = x - center[0];
	    pt[1] = y - center[1];
	    pt[2] = z - center[2];
	    if (is_on_ellipse(pt, locne->np.rx, locne->np.ry, normvec, xvec, 
		  yvec)) {
	      stack->array[offset] = value;
	    }
	  }
	}	
	x++;
      }
      x = x0;
      y++;
    }
    x = x0;
    y = y0;
    z++;
  }
}

