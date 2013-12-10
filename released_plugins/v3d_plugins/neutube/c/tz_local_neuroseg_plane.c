/* tz_local_neuroseg_plane.c
 *
 * 20-Apr-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utilities.h>
#include <math.h>
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_perceptor.h"
#include "tz_geo3d_point_array.h"
#include "tz_3dgeom.h"
#include "tz_geo3d_utils.h"
#include "tz_local_neuroseg_plane.h"

Local_Neuroseg_Plane* New_Local_Neuroseg_Plane()
{
  Local_Neuroseg_Plane *locnp = (Local_Neuroseg_Plane *) 
    Guarded_Malloc(sizeof(Local_Neuroseg_Plane), "New_Local_Neuroseg_Plane");

  Reset_Local_Neuroseg_Plane(locnp);
  
  return locnp;
}

void Delete_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp)
{
  free(locnp);
}

void Set_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp, 
			      double r, double theta, double psi,
			      double offset_x, double offset_y,
			      double x, double y, double z)
{
  Set_Neuroseg_Plane(&(locnp->np), r, theta, psi, offset_x, offset_y);
  Set_Neuropos(locnp->pos, x, y, z);
}

void Reset_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp)
{
  Set_Local_Neuroseg_Plane(locnp, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

void Print_Local_Neuroseg_Plane(const Local_Neuroseg_Plane *locnp)
{
  printf("Local neuroseg plane: \n");
  printf("r: %g, theta: %g, psi: %g, offset: [%g, %g], position(%g, %g, %g)\n",
	 locnp->np.r, locnp->np.theta, locnp->np.psi, 
	 locnp->np.offset_x, locnp->np.offset_y,
	 locnp->pos[0], locnp->pos[1], locnp->pos[2]);
}

void Local_Neuroseg_Plane_Fwrite(const Local_Neuroseg_Plane *locseg, FILE *fp)
{
  Neuroseg_Plane_Fwrite(&(locseg->np), fp);
  Neuropos_Fwrite(locseg->pos, fp);
}

Local_Neuroseg_Plane* Local_Neuroseg_Plane_Fread(Local_Neuroseg_Plane *locnp, 
						 FILE *fp)
{
  if (feof(fp)) {
    return NULL;
  }

  BOOL is_local_alloc = FALSE;
  if (locnp == NULL) {
    locnp = New_Local_Neuroseg_Plane();
    is_local_alloc = TRUE;
  }

  if (Neuroseg_Plane_Fread(&(locnp->np), fp) == NULL) {
    if (is_local_alloc == TRUE) {
      Delete_Local_Neuroseg_Plane(locnp);
    }
    locnp = NULL;
  } else {
    if (Neuropos_Fread(locnp->pos, fp) == NULL) {
      if (is_local_alloc == TRUE) {
	Delete_Local_Neuroseg_Plane(locnp);
      }
      locnp = NULL;
    }
  }

  return locnp;
}


Local_Neuroseg_Plane* 
Copy_Local_Neuroseg_Plane(const Local_Neuroseg_Plane *locnp)
{
  Local_Neuroseg_Plane *locnp_copy = New_Local_Neuroseg_Plane();
  memcpy(locnp_copy, locnp, sizeof(Local_Neuroseg_Plane));

  return locnp_copy;
}

double Local_Neuroseg_Plane_Score(const Local_Neuroseg_Plane *locnp, 
				  const Stack *stack, 
				  double z_scale, Stack_Fit_Score *fs)
{
  double sample_step = 1.0;
  Geo3d_Scalar_Field *field = Local_Neuroseg_Plane_Field(locnp, sample_step,
							 NULL);
  double score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

  Kill_Geo3d_Scalar_Field(field);
  
  return score;
}

int Local_Neuroseg_Plane_Var(const Local_Neuroseg_Plane *locnp, 
			     double *var[])
{
  int nvar = Neuroseg_Plane_Var(&(locnp->np), var);
  Neuropos_Var(locnp->pos, var + nvar);

  return LOCAL_NEUROSEG_PLANE_NPARAM;
}

void Local_Neuroseg_Plane_Set_Var(Local_Neuroseg_Plane *locnp, 
				  int var_index, double value)
{
  double *var[LOCAL_NEUROSEG_PLANE_NPARAM];
  Local_Neuroseg_Plane_Var(locnp, var);
  *(var[var_index]) = value;
}

Geo3d_Scalar_Field* 
Local_Neuroseg_Plane_Field(const Local_Neuroseg_Plane *locnp, double step,
			   Geo3d_Scalar_Field *field)
{
  field = Neuroseg_Plane_Field_S(&(locnp->np), step, field);
  Geo3d_Point_Array_Translate(field->points, field->size, locnp->pos[0],
			      locnp->pos[1], locnp->pos[2]);
  return field;
}

int Local_Neuroseg_Plane_Param_Array(const Local_Neuroseg_Plane *locnp, 
				     double z_scale, double *param)
{
  int i;
  double *var[LOCAL_NEUROSEG_PLANE_NPARAM];
  Local_Neuroseg_Plane_Var(locnp, var);
  for (i = 0; i < LOCAL_NEUROSEG_PLANE_NPARAM; i++) {
    param[i] = *(var[i]);
  }

  param[LOCAL_NEUROSEG_PLANE_NPARAM] = z_scale;
  
  return LOCAL_NEUROSEG_PLANE_NPARAM + 1;
}

int Local_Neuroseg_Plane_Var_Mask_To_Index(Bitmask_t plane_mask, 
					   Bitmask_t neuropos_mask, int *index)
{
  int nvar1 = Neuroseg_Plane_Var_Mask_To_Index(plane_mask, index);
  int nvar2 = Neuropos_Var_Mask_To_Index(neuropos_mask, index + nvar1);
  
  int i;
  for (i = 0; i < nvar2; i++) {
    index[nvar1 + i] += NEUROSEG_PLANE_NPARAM;
  }

  return nvar1 + nvar2;
}

void Local_Neuroseg_Plane_Validate(double *var, const double *var_min, 
				   const double *var_max, const void *param)
{
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_PLANE_NPARAM; i++) {
    if (var[i] < var_min[i]) {
      var[i] = var_min[i];
    } else if (var[i] > var_max[i]) {
      var[i] = var_max[i];
    }
  }
}

double Local_Neuroseg_Plane_Score_G(const double *var, const Stack *stack)
{
  Local_Neuroseg_Plane *locnp = New_Local_Neuroseg_Plane();
  int i;
  for (i = 0; i < LOCAL_NEUROSEG_PLANE_NPARAM; i++) {
    Local_Neuroseg_Plane_Set_Var(locnp, i, var[i]);
  }

  double z_scale = var[LOCAL_NEUROSEG_PLANE_NPARAM];

  double score = Local_Neuroseg_Plane_Score(locnp, stack, z_scale, NULL);

  Delete_Local_Neuroseg_Plane(locnp);

  return score;
}

double Local_Neuroseg_Plane_Score_Gv(const double *var, const void *param)
{
  return Local_Neuroseg_Plane_Score_G(var, (const Stack *) param);
}

double Fit_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp, const Stack *stack,
				const int *var_index, int nvar, double z_scale,
				Stack_Fit_Score *fs)
{
  ASSERT(stack != NULL, "Null stack.");
  ASSERT(locnp != NULL, "Null segment plane.");

  double *var = darray_malloc(LOCAL_NEUROSEG_PLANE_NPARAM + 1);
  Local_Neuroseg_Plane_Param_Array(locnp, z_scale, var);


  Perceptor perceptor;
  perceptor.vs = New_Variable_Set();
  perceptor.vs->nvar = nvar;
  perceptor.vs->var_index = (int *) var_index;
  perceptor.vs->var = var;
  perceptor.min_gradient = 1e-5;

  double delta[] = {0.5, 0.015, 0.015, 0.5, 0.5, 0.5, 0.5, 0.5, 0.1};
  perceptor.delta = delta;
  perceptor.weight = delta;

  double var_min[] = {1.0, -Infinity, -Infinity, -Infinity, -Infinity, 
		      -Infinity, -Infinity, -Infinity, 0.5};
  double var_max[] = {30.0, Infinity, Infinity, Infinity, Infinity, 
		      Infinity, Infinity, Infinity, 6.0};
  perceptor.s = 
    Make_Continuous_Function(Local_Neuroseg_Plane_Score_Gv, 
			     Local_Neuroseg_Plane_Validate,
			     var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  int i;
  for (i = 0; i < LOCAL_NEUROSEG_PLANE_NPARAM; i++) {
    Local_Neuroseg_Plane_Set_Var(locnp, i, perceptor.vs->var[i]);
  }

  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  return Local_Neuroseg_Plane_Score(locnp, stack, z_scale, fs);  
}

void Local_Neuroseg_Plane_Center(const Local_Neuroseg_Plane *locnp,
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

Geo3d_Circle*
Local_Neuroseg_Plane_To_Geo3d_Circle(const Local_Neuroseg_Plane *locnp,
				     Geo3d_Circle *circle)
{
  if (circle == NULL) {
    circle = New_Geo3d_Circle();
  }

  circle->radius = locnp->np.r;
  circle->orientation[0] = locnp->np.theta;
  circle->orientation[1] = locnp->np.psi;

  Local_Neuroseg_Plane_Center(locnp, circle->center);
  /*  
  circle->center[0] = locnp->np.offset_x;
  circle->center[1] = locnp->np.offset_y;
  circle->center[2] = 0;
  
  Rotate_XZ(circle->center, circle->center, 1, 
	    locnp->np.theta, locnp->np.psi, 0);

  circle->center[0] += locnp->pos[0];
  circle->center[1] += locnp->pos[1];
  circle->center[2] += locnp->pos[2];
  */
  return circle;
}

Local_Neuroseg_Plane*
Next_Neuroseg_Plane(Local_Neuroseg_Plane *locnp, double step)
{
  Local_Neuroseg_Plane *next = Copy_Local_Neuroseg_Plane(locnp);

  double dx, dy, dz;
  Geo3d_Orientation_Normal(locnp->np.theta, locnp->np.psi, &dx, &dy, &dz);
  dx *= step;
  dy *= step;
  dz *= step;
  Neuropos_Translate(next->pos, dx, dy, dz);

  return next;
}
