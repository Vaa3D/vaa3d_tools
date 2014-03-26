#include <stdio.h>
#include "tz_local_rpi_neuroseg.h"
#include "tz_geo3d_point_array.h"
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_perceptor.h"
#include "tz_geoangle_utils.h"
#include "tz_3dgeom.h"

#define DELTA_THETA 0.015
#define DELTA_PSI 0.015
#define DELTA_LEFT_OFFSET 0.5
#define DELTA_RIGHT_OFFSET 0.5
#define DELTA_BOTTOM_OFFSET 0.5
#define DELTA_TOP_OFFSET 0.5
#define DELTA_LENGTH 1.0 

static const double Delta[] = {
  DELTA_THETA, DELTA_PSI, DELTA_LEFT_OFFSET, DELTA_RIGHT_OFFSET,
  DELTA_BOTTOM_OFFSET, DELTA_TOP_OFFSET, DELTA_LENGTH };

void Default_Local_Rpi_Neuroseg(Local_Rpi_Neuroseg *locseg)
{
  Default_Rpi_Neuroseg(&locseg->seg);
  locseg->pos[0] = 0.0;
  locseg->pos[1] = 0.0;
  locseg->pos[2] = 0.0;
}

void Clean_Local_Rpi_Neuroseg(Local_Rpi_Neuroseg *locseg)
{
  Default_Local_Rpi_Neuroseg(locseg);
}

void Print_Local_Rpi_Neuroseg(Local_Rpi_Neuroseg *locseg)
{
  Print_Rpi_Neuroseg(&locseg->seg);
  printf("Position: (%g, %g, %g)\n", locseg->pos[0], locseg->pos[1],
      locseg->pos[2]);
}

DEFINE_ZOBJECT_INTERFACE(Local_Rpi_Neuroseg)

int Local_Rpi_Neuroseg_Var_Mask_To_Index(Bitmask_t neuroseg_mask, 
    Bitmask_t neuropos_mask, int *index)
{
  int nvar1 = Rpi_Neuroseg_Var_Mask_To_Index(neuroseg_mask, index);
  int nvar2 = Neuropos_Var_Mask_To_Index(neuropos_mask, index + nvar1);
  
  int i;
  for (i = 0; i < nvar2; i++) {
    index[nvar1 + i] += NEUROSEG_NPARAM;
  }

  return nvar1 + nvar2;
}

int Local_Rpi_Neuroseg_Var(const Local_Rpi_Neuroseg *locseg, double *var[])
{  
  int nvar = Rpi_Neuroseg_Var(&(locseg->seg), var);
  Neuropos_Var(locseg->pos, var + nvar);

  return LOCAL_RPI_NEUROSEG_NPARAM;
}

void Local_Rpi_Neuroseg_Set_Var(Local_Rpi_Neuroseg *locseg, int var_index, 
    double value)
{
  double *var[LOCAL_RPI_NEUROSEG_NPARAM];
  Local_Rpi_Neuroseg_Var(locseg, var);
  *(var[var_index]) = value;
}

int Local_Rpi_Neuroseg_Param_Array(const Local_Rpi_Neuroseg *locseg,
    double z_scale, double *param)
{
  int i;
  double *var[LOCAL_RPI_NEUROSEG_NPARAM];
  Local_Rpi_Neuroseg_Var(locseg, var);
  for (i = 0; i < LOCAL_RPI_NEUROSEG_NPARAM; i++) {
    param[i] = *(var[i]);
  }
  
  return LOCAL_RPI_NEUROSEG_NPARAM;
}

void Local_Rpi_Neuroseg_Validate(double *var, const double *var_min, 
			     const double *var_max, const void *param)
{
  int i;
  for (i = 0; i < LOCAL_RPI_NEUROSEG_NPARAM; i++) {
    if (var[i] < var_min[i]) {
      var[i] = var_min[i];
    } else if (var[i] > var_max[i]) {
      var[i] = var_max[i];
    }
  }
}

Geo3d_Scalar_Field* Local_Rpi_Neuroseg_Field(const Local_Rpi_Neuroseg *locseg,
    Geo3d_Scalar_Field *field)
{
  field = Rpi_Neuroseg_Field(&(locseg->seg), field);

  Geo3d_Point_Array_Translate(field->points, field->size, locseg->pos[0],
			      locseg->pos[1], locseg->pos[2]);

  return field;
}

double Local_Rpi_Neuroseg_Score_W(const Local_Rpi_Neuroseg *locseg, 
    const Stack *stack, double z_scale, Receptor_Score_Workspace *ws)
{
  Geo3d_Scalar_Field *field = Local_Rpi_Neuroseg_Field(locseg, NULL);
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

double Local_Rpi_Neuroseg_Score_R(const double *var, const void *param)
{
  void **param_array = (void**) param;
  
  Stack *stack = (Stack *) param_array[0];
  Receptor_Score_Workspace *ws = 
    (Receptor_Score_Workspace *) param_array[1];
  
  Local_Rpi_Neuroseg seg;
  int i;
  for (i = 0; i < LOCAL_RPI_NEUROSEG_NPARAM; i++) {
    Local_Rpi_Neuroseg_Set_Var(&seg, i, var[i]);
  }
  
  double z_scale = var[LOCAL_RPI_NEUROSEG_NPARAM];
  
  double score = Local_Rpi_Neuroseg_Score_W(&seg, stack, z_scale, ws);
  
  return score;
}

double Local_Rpi_Neuroseg_Score_P(const Local_Rpi_Neuroseg *locseg, 
    const Stack *stack, double z_scale, Stack_Fit_Score *fs)
{
  double score = 0.0;

  Geo3d_Scalar_Field *field = Local_Rpi_Neuroseg_Field(locseg, NULL);
  score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

  Kill_Geo3d_Scalar_Field(field);
  
  return score;  
}

double Fit_Local_Rpi_Neuroseg_Pr(Local_Rpi_Neuroseg *locseg, 
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs)
{
  if ((stack == NULL) || (locseg == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_RPI_NEUROSEG_NPARAM);
  
  double *var = darray_malloc(LOCAL_RPI_NEUROSEG_NPARAM);
  Local_Rpi_Neuroseg_Param_Array(locseg, z_scale, var);

  Perceptor perceptor;
  perceptor.vs = New_Variable_Set();
  perceptor.vs->nvar = nvar;
  perceptor.vs->var_index = (int *) var_index;
  perceptor.vs->var = var;
  perceptor.vs->link = var_link;
  perceptor.min_gradient = 1e-3;

  perceptor.arg = New_Receptor_Score_Workspace();

  double *delta = (double *) Delta;
  perceptor.delta = delta;

  int i;
  for (i = 0; i < nvar; i++) {
    weight[i] = delta[var_index[i]];
  }
  double wl = darray_norm(weight, nvar);
  
  for (i = 0; i < nvar; i++) {
    weight[i] /= wl;
  }
  perceptor.weight = weight;

  
  perceptor.s = 
    Make_Continuous_Function(Local_Rpi_Neuroseg_Score_R, 
	Local_Rpi_Neuroseg_Validate, var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_RPI_NEUROSEG_NPARAM; i++) {
    Local_Rpi_Neuroseg_Set_Var(locseg, i, perceptor.vs->var[i]);
  }

  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  locseg->seg.theta = Normalize_Radian(locseg->seg.theta);
  locseg->seg.psi = Normalize_Radian(locseg->seg.psi);
  
  Kill_Receptor_Score_Workspace((Receptor_Score_Workspace*)perceptor.arg);

  return Local_Rpi_Neuroseg_Score_P(locseg, stack, z_scale, fs);
}

double Fit_Local_Rpi_Neuroseg_W(Local_Rpi_Neuroseg *locseg, const Stack *stack,
    double z_scale, Receptor_Fit_Workspace *ws)
{
  if ((stack == NULL) || (locseg == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_RPI_NEUROSEG_NPARAM);

  double *var = darray_malloc(LOCAL_RPI_NEUROSEG_NPARAM);

  Local_Rpi_Neuroseg_Param_Array(locseg, z_scale, var);

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
    Make_Continuous_Function(Local_Rpi_Neuroseg_Score_R,
       	Local_Rpi_Neuroseg_Validate, ws->var_min, ws->var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_RPI_NEUROSEG_NPARAM; i++) {
    Local_Rpi_Neuroseg_Set_Var(locseg, i, perceptor.vs->var[i]);
  }

  Delete_Variable_Set(perceptor.vs);
  free(var);
  free(weight);
  Free_Continuous_Function(perceptor.s);

  locseg->seg.theta = Normalize_Radian(locseg->seg.theta);
  locseg->seg.psi = Normalize_Radian(locseg->seg.psi);

  return Local_Rpi_Neuroseg_Score_W(locseg, stack, z_scale, ws->sws);
}

double Local_Rpi_Neuroseg_Optimize_W(Local_Rpi_Neuroseg *rpiseg, 
    const Stack *stack, double z_scale, int option, Locseg_Fit_Workspace *ws)
{
  Local_Neuroseg locseg;
  Local_Rpi_Neuroseg_To_Local_Neuroseg(rpiseg, &locseg);

  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_CORRCOEF;

  int i;
  for (i = 0; i < ws->pos_adjust; i++) {
    Local_Neuroseg_Position_Adjust(&locseg, stack, z_scale);
  }

  Local_Neuroseg_Orientation_Search_C(&locseg, stack, z_scale, &fs); 

  if (option <= 1) {
    for (i = 0; i < 3; i++) {
      Local_Neuroseg_Position_Adjust(&locseg, stack, z_scale);
    }
  }

  if ((option == 1) || (option == 2)){
    Local_Neuroseg_R_Scale_Search(&locseg, stack, z_scale, 1.0, 10.0, 1.0,
				  0.5, 5.0, 0.5, NULL);
  }

  Local_Rpi_Neuroseg_From_Local_Neuroseg(rpiseg, &locseg);

  double score = Fit_Local_Rpi_Neuroseg_W(rpiseg, stack, z_scale, ws);

  return score;
}

Local_Neuroseg* 
Local_Rpi_Neuroseg_To_Local_Neuroseg(const Local_Rpi_Neuroseg *rpiseg,
    Local_Neuroseg *locseg)
{
  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
  }

  locseg->seg.theta = rpiseg->seg.theta;
  locseg->seg.psi = rpiseg->seg.psi;
  locseg->seg.h = rpiseg->seg.length;

  double offset[3];
  offset[0] = (rpiseg->seg.left_offset + rpiseg->seg.right_offset) / 2.0;
  offset[1] = (rpiseg->seg.bottom_offset + rpiseg->seg.top_offset) / 2.0;
  offset[2] = 0;
  
  Rotate_XZ(offset, offset, 1, rpiseg->seg.theta, rpiseg->seg.psi, 0);

  locseg->pos[0] = rpiseg->pos[0] + offset[0];
  locseg->pos[1] = rpiseg->pos[1] + offset[1];
  locseg->pos[2] = rpiseg->pos[2] + offset[2];

  locseg->seg.r1 = (rpiseg->seg.top_offset - rpiseg->seg.bottom_offset) / 2.0;
  double r2 = (rpiseg->seg.right_offset - rpiseg->seg.left_offset) / 2.0;
  locseg->seg.scale = r2 / locseg->seg.r1;

  return locseg;
}

void Local_Rpi_Neuroseg_From_Local_Neuroseg(Local_Rpi_Neuroseg *rpiseg,
    const Local_Neuroseg *locseg)
{
  rpiseg->seg.theta = locseg->seg.theta;
  rpiseg->seg.psi = locseg->seg.psi;
  rpiseg->seg.length = locseg->seg.h;
  rpiseg->pos[0] = locseg->pos[0];
  rpiseg->pos[1] = locseg->pos[1];
  rpiseg->pos[2] = locseg->pos[2];

  rpiseg->seg.top_offset = locseg->seg.r1 / 2.0;
  rpiseg->seg.right_offset = rpiseg->seg.top_offset * locseg->seg.scale;
  rpiseg->seg.bottom_offset = -rpiseg->seg.top_offset;
  rpiseg->seg.left_offset = -rpiseg->seg.right_offset;
}
