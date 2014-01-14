/* tz_local_bifold_neuroseg.c
 *
 * 20-Apr-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <utilities.h>
#include <math.h>
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_perceptor.h"
#include "tz_geo3d_point_array.h"
#include "tz_local_bifold_neuroseg.h"

Local_Bifold_Neuroseg* New_Local_Bifold_Neuroseg()
{
  Local_Bifold_Neuroseg *locbn = (Local_Bifold_Neuroseg *) 
    Guarded_Malloc(sizeof(Local_Bifold_Neuroseg), "New_Local_Bifold_Neuroseg");

  Reset_Local_Bifold_Neuroseg(locbn);
  
  return locbn;
}

void Delete_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn)
{
  free(locbn);
}

void Set_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn, 
			       double r1, double r2,
			       double r3, double r4, double h, double knot,
			       double theta2, double psi2,
			       double theta, double psi,
			       double x, double y, double z)
{
  Set_Bifold_Neuroseg(&(locbn->bn), r1, r2, r3, r4, h, knot, theta2, psi2,
		      theta, psi);
  Set_Neuropos(locbn->pos, x, y, z);
}

void Reset_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn)
{
  Reset_Bifold_Neuroseg(&(locbn->bn));
  Set_Neuropos(locbn->pos, 0.0, 0.0, 0.0);
}

void Print_Local_Bifold_Neuroseg(const Local_Bifold_Neuroseg *locbn)
{
  printf("Local bifold neuroseg: \n");
  printf("Bifold neuroseg: r: (%g, %g, %g, %g); h: (%g, %g), orientation 2: (%g, %g), overall orientation: (%g, %g); position (%g, %g, %g)\n",
	 locbn->bn.r1, locbn->bn.r2, locbn->bn.r3, locbn->bn.r4, 
	 locbn->bn.h, locbn->bn.knot, 
	 locbn->bn.theta2, locbn->bn.psi2, locbn->bn.theta, locbn->bn.psi,
	 locbn->pos[0], locbn->pos[1], locbn->pos[2]);
}

double Local_Bifold_Neuroseg_Score(const Local_Bifold_Neuroseg *locbn, 
				  const Stack *stack, 
				  double z_scale, Stack_Fit_Score *fs)
{
  Geo3d_Scalar_Field *field = Local_Bifold_Neuroseg_Field(locbn,
							 NULL);
  double score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

  Kill_Geo3d_Scalar_Field(field);
  
  return score;
}

int Local_Bifold_Neuroseg_Var(const Local_Bifold_Neuroseg *locbn, 
			     double *var[])
{
  int nvar = Bifold_Neuroseg_Var(&(locbn->bn), var);
  Neuropos_Var(locbn->pos, var + nvar);

  return LOCAL_BIFOLD_NEUROSEG_NPARAM;
}

void Local_Bifold_Neuroseg_Set_Var(Local_Bifold_Neuroseg *locbn, 
				  int var_index, double value)
{
  double *var[LOCAL_BIFOLD_NEUROSEG_NPARAM];
  Local_Bifold_Neuroseg_Var(locbn, var);
  *(var[var_index]) = value;
}

Geo3d_Scalar_Field* 
Local_Bifold_Neuroseg_Field(const Local_Bifold_Neuroseg *locbn,
			   Geo3d_Scalar_Field *field)
{
  field = Bifold_Neuroseg_Field(&(locbn->bn), field);
  Geo3d_Point_Array_Translate(field->points, field->size, locbn->pos[0],
			      locbn->pos[1], locbn->pos[2]);
  return field;
}

int Local_Bifold_Neuroseg_Param_Array(const Local_Bifold_Neuroseg *locbn, 
				     double z_scale, double *param)
{
  int i;
  double *var[LOCAL_BIFOLD_NEUROSEG_NPARAM];
  Local_Bifold_Neuroseg_Var(locbn, var);
  for (i = 0; i < LOCAL_BIFOLD_NEUROSEG_NPARAM; i++) {
    param[i] = *(var[i]);
  }

  param[LOCAL_BIFOLD_NEUROSEG_NPARAM] = z_scale;
  
  return LOCAL_BIFOLD_NEUROSEG_NPARAM + 1;
}

int Local_Bifold_Neuroseg_Var_Mask_To_Index(Bitmask_t plane_mask, 
					   Bitmask_t neuropos_mask, int *index)
{
  int nvar1 = Bifold_Neuroseg_Var_Mask_To_Index(plane_mask, index);
  int nvar2 = Neuropos_Var_Mask_To_Index(neuropos_mask, index + nvar1);
  
  int i;
  for (i = 0; i < nvar2; i++) {
    index[nvar1 + i] += BIFOLD_NEUROSEG_NPARAM;
  }

  return nvar1 + nvar2;
}

void Local_Bifold_Neuroseg_Validate(double *var, const double *var_min, 
				    const double *var_max, const void *param)
{
  darray_threshold(var, LOCAL_BIFOLD_NEUROSEG_NPARAM, var_min, var_max);
}

double Local_Bifold_Neuroseg_Score_G(const double *var, const Stack *stack)
{
  Local_Bifold_Neuroseg *locbn = New_Local_Bifold_Neuroseg();
  int i;
  for (i = 0; i < LOCAL_BIFOLD_NEUROSEG_NPARAM; i++) {
    Local_Bifold_Neuroseg_Set_Var(locbn, i, var[i]);
  }

  double z_scale = var[LOCAL_BIFOLD_NEUROSEG_NPARAM];

  double score = Local_Bifold_Neuroseg_Score(locbn, stack, z_scale, NULL);

  Delete_Local_Bifold_Neuroseg(locbn);

  return score;
}

double Local_Bifold_Neuroseg_Score_Gv(const double *var, const void *param)
{
  return Local_Bifold_Neuroseg_Score_G(var, (const Stack *) param);
}

double Fit_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn, 
				 const Stack *stack, 
				 const int *var_index, int nvar, double z_scale,
				 Stack_Fit_Score *fs)
{
  ASSERT(stack != NULL, "Null stack.");
  ASSERT(locbn != NULL, "Null segment plane.");

  double *var = darray_malloc(LOCAL_BIFOLD_NEUROSEG_NPARAM + 1);
  Local_Bifold_Neuroseg_Param_Array(locbn, z_scale, var);


  Perceptor perceptor;
  perceptor.vs = New_Variable_Set();
  perceptor.vs->nvar = nvar;
  perceptor.vs->var_index = (int *) var_index;
  perceptor.vs->var = var;
  perceptor.min_gradient = 1e-5;

  double delta[] = {0.5, 0.5, 0.5, 0.5, 1.0, 0.05, 0.015, 0.015, 0.015, 0.015, 
		    0.5, 0.5, 0.5, 0.1};
  perceptor.delta = delta;
  perceptor.weight = delta;

  double var_min[] = {1.0, 1.0, 1.0, 1.0, 1.0, 0.1, 
		      -Infinity, -Infinity, -Infinity, -Infinity, 
		      -Infinity, -Infinity, -Infinity, 0.5};
  double var_max[] = {10.0, 10.0, 10.0, 10.0, 0.9, 
		      Infinity, Infinity, Infinity, Infinity, 
		      Infinity, Infinity, Infinity, 6.0};
  perceptor.s = 
    Make_Continuous_Function(Local_Bifold_Neuroseg_Score_Gv, 
			     Local_Bifold_Neuroseg_Validate,
			     var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  int i;
  for (i = 0; i < LOCAL_BIFOLD_NEUROSEG_NPARAM; i++) {
    Local_Bifold_Neuroseg_Set_Var(locbn, i, perceptor.vs->var[i]);
  }

  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  return Local_Bifold_Neuroseg_Score(locbn, stack, z_scale, fs);  
}
