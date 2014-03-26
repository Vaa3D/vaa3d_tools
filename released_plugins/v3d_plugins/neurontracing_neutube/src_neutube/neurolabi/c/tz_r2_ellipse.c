#include <string.h>
#include "tz_r2_ellipse.h"
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_perceptor.h"
#include "tz_geoangle_utils.h"
#include "tz_coordinate_3d.h"
#include "tz_3dgeom.h"

#define R2_ELLIPSE_SLICE_FIELD_LENGTH NEUROSEG_SLICE_FIELD_LENGTH

#define DELTA_ALPHA 0.015
#define DELTA_LOFFSET 0.5
#define DELTA_ROFFSET 0.5
#define DELTA_TOFFSET 0.5
#define DELTA_BOFFSET 0.5

static const double Delta[] = {
  DELTA_LOFFSET, DELTA_ROFFSET, DELTA_TOFFSET, DELTA_BOFFSET, DELTA_ALPHA };


void R2_Ellipse_Slice_Field(const R2_Ellipse *seg, 
    coordinate_3d_t *pcoord, double *value, int *length)
{
  UNUSED_PARAMETER(seg);

  Neuroseg_Slice_Field(pcoord, value, length, NULL);
}

Geo3d_Scalar_Field* R2_Ellipse_Field(const R2_Ellipse *seg, 
    Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    Construct_Geo3d_Scalar_Field(field, R2_ELLIPSE_SLICE_FIELD_LENGTH);
  }

  int length;
  
  coordinate_3d_t *points = field->points;
  double *values = field->values;

  R2_Ellipse_Slice_Field(seg, points, values, &length);
  field->size = length;

  int i;
  for (i = 0; i < length; i++) {
    field->points[i][2] = seg->z;
  }

  return field;
}

void Default_Local_R2_Ellipse(Local_R2_Ellipse *ellipse)
{
  ellipse->transform.loffset = 0.0;
  ellipse->transform.roffset = 0.0;
  ellipse->transform.boffset = 0.0;
  ellipse->transform.toffset = 0.0;
  ellipse->transform.alpha = 0.0;
  ellipse->base.z = 0.0;
}

void Clean_Local_R2_Ellipse(Local_R2_Ellipse *ellipse)
{
  Default_Local_R2_Ellipse(ellipse);
}

void Print_Local_R2_Ellipse(Local_R2_Ellipse *ellipse)
{
  printf("alpha: %g; loffset: %g; roffset: %g; toffset: %g; boffset: %g\n", 
      ellipse->transform.alpha,
      ellipse->transform.loffset, ellipse->transform.roffset,
      ellipse->transform.toffset, ellipse->transform.boffset);
}

DEFINE_ZOBJECT_INTERFACE(Local_R2_Ellipse)

Local_R2_Ellipse* Copy_Local_R2_Ellipse(Local_R2_Ellipse *ellipse)
{
  Local_R2_Ellipse *ellipse2 = New_Local_R2_Ellipse();
  *ellipse2 = *ellipse;

  return ellipse2;
}

void Set_Local_R2_Ellipse(Local_R2_Ellipse *ellipse, double z, double alpha, 
    double loffset, double roffset, double toffset, double boffset)
{
  ellipse->base.z = z;
  ellipse->transform.alpha = alpha;
  ellipse->transform.loffset = loffset;
  ellipse->transform.roffset = roffset;
  ellipse->transform.toffset = toffset;
  ellipse->transform.boffset = boffset;
}

int Local_R2_Ellipse_Var_Mask_To_Index(Bitmask_t mask, int *index)
{
  return Receptor_Transform_Ellipse_2d_Var_Mask_To_Index(mask, index);
}

int Local_R2_Ellipse_Var(const Local_R2_Ellipse *ellipse, double *var[])
{
  var[0] = (double*) &(ellipse->transform.loffset);
  var[1] = (double*) &(ellipse->transform.roffset);
  var[2] = (double*) &(ellipse->transform.toffset);
  var[3] = (double*) &(ellipse->transform.boffset);
  var[4] = (double*) &(ellipse->transform.alpha);
  var[5] = (double*) &(ellipse->base.z);

  return LOCAL_R2_ELLIPSE_NPARAM;
}

void Local_R2_Ellipse_Set_Var(Local_R2_Ellipse *locseg, int var_index, 
        double value)
{
  double *var[LOCAL_R2_ELLIPSE_NPARAM];
  Local_R2_Ellipse_Var(locseg, var);
  *(var[var_index]) = value;
}

int Local_R2_Ellipse_Param_Array(const Local_R2_Ellipse *locseg,
    double z_scale, double *param)
{
  int i;
  double *var[LOCAL_R2_ELLIPSE_NPARAM];
  Local_R2_Ellipse_Var(locseg, var);
  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    param[i] = *(var[i]);
  }
  param[LOCAL_R2_ELLIPSE_NPARAM] = z_scale;

  return LOCAL_R2_ELLIPSE_NPARAM;
}

void Local_R2_Ellipse_Validate(double *var, const double *var_min,
    const double *var_max, const void *param)
{ 
  int i;
  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    if (var[i] < var_min[i]) {
      var[i] = var_min[i];
    } else if (var[i] > var_max[i]) {
      var[i] = var_max[i];
    }
  }
}

Geo3d_Scalar_Field* Local_R2_Ellipse_Field(const Local_R2_Ellipse *locseg,
        Geo3d_Scalar_Field *field)
{
  field = R2_Ellipse_Field(&(locseg->base), field);
  Receptor_Transform_Ellipse_2d_Apply(&(locseg->transform), field);

#ifdef _DEBUG_2
  Print_Geo3d_Scalar_Field(field);
#endif

  return field;
}

Geo3d_Scalar_Field* Local_R2_Ellipse_Field_C(const Local_R2_Ellipse *locseg,
        Geo3d_Scalar_Field *field)
{
  field = R2_Ellipse_Field(&(locseg->base), field);
  Receptor_Transform_Ellipse_2d_Apply_C(&(locseg->transform), field);

  return field;
}

double Local_R2_Ellipse_Score_W(const Local_R2_Ellipse *locseg, 
        const Stack *stack, double z_scale, Receptor_Score_Workspace *ws)
{
  Geo3d_Scalar_Field *field = Local_R2_Ellipse_Field(locseg, NULL);
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

double Local_R2_Ellipse_Score_Wc(const Local_R2_Ellipse *locseg, 
        const Stack *stack, double z_scale, Receptor_Score_Workspace *ws)
{
  Geo3d_Scalar_Field *field = Local_R2_Ellipse_Field_C(locseg, NULL);
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

double Local_R2_Ellipse_Score_R(const double *var, const void *param)
{
  void **param_array = (void**) param;

  Stack *stack = (Stack *) param_array[0];
  Receptor_Score_Workspace *ws =
    (Receptor_Score_Workspace *) param_array[1];

  Local_R2_Ellipse seg;
  int i;
  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    Local_R2_Ellipse_Set_Var(&seg, i, var[i]);
  }

  double z_scale = var[LOCAL_R2_ELLIPSE_NPARAM];

  double score = Local_R2_Ellipse_Score_W(&seg, stack, z_scale, ws);

  return score;
}

double Local_R2_Ellipse_Score_Rc(const double *var, const void *param)
{
  void **param_array = (void**) param;

  Stack *stack = (Stack *) param_array[0];
  Receptor_Score_Workspace *ws =
    (Receptor_Score_Workspace *) param_array[1];

  Local_R2_Ellipse seg;
  int i;
  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    Local_R2_Ellipse_Set_Var(&seg, i, var[i]);
  }

  double z_scale = var[LOCAL_R2_ELLIPSE_NPARAM];

  double score = Local_R2_Ellipse_Score_Wc(&seg, stack, z_scale, ws);

  return score;
}

double Local_R2_Ellipse_Score_P(const Local_R2_Ellipse *locseg,
    const Stack *stack, double z_scale, Stack_Fit_Score *fs)
{
  double score = 0.0;

  Geo3d_Scalar_Field *field = Local_R2_Ellipse_Field(locseg, NULL);
  score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

  Kill_Geo3d_Scalar_Field(field);

  return score;
}

double Local_R2_Ellipse_Score_Pc(const Local_R2_Ellipse *locseg,
    const Stack *stack, double z_scale, Stack_Fit_Score *fs)
{
  double score = 0.0;

  Geo3d_Scalar_Field *field = Local_R2_Ellipse_Field_C(locseg, NULL);
  score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

  Kill_Geo3d_Scalar_Field(field);

  return score;
}

double Fit_Local_R2_Ellipse_Pr(Local_R2_Ellipse *locseg,
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs)
{
  if ((stack == NULL) || (locseg == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM);

  double *var = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM + 1);
  var[LOCAL_R2_ELLIPSE_NPARAM] = 1.0;
  Local_R2_Ellipse_Param_Array(locseg, z_scale, var);

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
    Make_Continuous_Function(Local_R2_Ellipse_Score_R,
        Local_R2_Ellipse_Validate, var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    Local_R2_Ellipse_Set_Var(locseg, i, perceptor.vs->var[i]);
  }
  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  locseg->transform.alpha = Normalize_Radian(locseg->transform.alpha);

  Kill_Receptor_Score_Workspace((Receptor_Score_Workspace*)perceptor.arg);

  return Local_R2_Ellipse_Score_P(locseg, stack, z_scale, fs);
}

double Fit_Local_R2_Ellipse_Pr_C(Local_R2_Ellipse *locseg,
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs)
{
  if ((stack == NULL) || (locseg == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM);

  double *var = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM + 1);
  var[LOCAL_R2_ELLIPSE_NPARAM] = 1.0;
  Local_R2_Ellipse_Param_Array(locseg, z_scale, var);

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
    Make_Continuous_Function(Local_R2_Ellipse_Score_Rc,
        Local_R2_Ellipse_Validate, var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    Local_R2_Ellipse_Set_Var(locseg, i, perceptor.vs->var[i]);
  }
  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  locseg->transform.alpha = Normalize_Radian(locseg->transform.alpha);

  Kill_Receptor_Score_Workspace((Receptor_Score_Workspace*)perceptor.arg);

  return Local_R2_Ellipse_Score_Pc(locseg, stack, z_scale, fs);
}

double Fit_Local_R2_Ellipse_W(Local_R2_Ellipse *locseg, const Stack *stack,
        double z_scale, Receptor_Fit_Workspace *ws)
{
  if ((stack == NULL) || (locseg == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM);

  double *var = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM + 1);

  Local_R2_Ellipse_Param_Array(locseg, z_scale, var);

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
    Make_Continuous_Function(Local_R2_Ellipse_Score_R,
        Local_R2_Ellipse_Validate, ws->var_min, ws->var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    Local_R2_Ellipse_Set_Var(locseg, i, perceptor.vs->var[i]);
  }
  Delete_Variable_Set(perceptor.vs);
  free(var);
  free(weight);
  Free_Continuous_Function(perceptor.s);

  locseg->transform.alpha = Normalize_Radian(locseg->transform.alpha);

  return Local_R2_Ellipse_Score_W(locseg, stack, z_scale, ws->sws);
}

double Fit_Local_R2_Ellipse_Wc(Local_R2_Ellipse *locseg, const Stack *stack,
        double z_scale, Receptor_Fit_Workspace *ws)
{
  if ((stack == NULL) || (locseg == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM);

  double *var = darray_malloc(LOCAL_R2_ELLIPSE_NPARAM + 1);

  Local_R2_Ellipse_Param_Array(locseg, z_scale, var);

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
    Make_Continuous_Function(Local_R2_Ellipse_Score_Rc,
        Local_R2_Ellipse_Validate, ws->var_min, ws->var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_R2_ELLIPSE_NPARAM; i++) {
    Local_R2_Ellipse_Set_Var(locseg, i, perceptor.vs->var[i]);
  }
  Delete_Variable_Set(perceptor.vs);
  free(var);
  free(weight);
  Free_Continuous_Function(perceptor.s);

  locseg->transform.alpha = Normalize_Radian(locseg->transform.alpha);

  return Local_R2_Ellipse_Score_Wc(locseg, stack, z_scale, ws->sws);
}

void Local_R2_Ellipse_Set_Center(Local_R2_Ellipse *locseg, 
    const double *center)
{
  double old_center[3];
  Local_R2_Ellipse_Center(locseg, old_center);

  double dx = center[0] - old_center[0];
  double dy = center[1] - old_center[1];

  locseg->transform.roffset += dx;
  locseg->transform.loffset -= dx;
  locseg->transform.toffset += dy;
  locseg->transform.boffset -= dy;
  locseg->base.z = center[2];
}

void Local_R2_Ellipse_Position_Adjust(Local_R2_Ellipse *locseg,
    const Stack *stack, double z_scale)
{   
  /* alloc <field> */
  Geo3d_Scalar_Field *field = Local_R2_Ellipse_Field(locseg, NULL);

  Geo3d_Scalar_Field_Stack_Sampling(field, stack, z_scale, 
      field->values);

  coordinate_3d_t center;
  Geo3d_Scalar_Field_Centroid(field, center);
#ifdef _DEBUG_
  printf("Centroid: %g, %g, %g\n", center[0], center[1], center[2]);
#endif

  Local_R2_Ellipse_Set_Center(locseg, center);

  /* free <field> */
  Kill_Geo3d_Scalar_Field(field);
} 


Geo3d_Scalar_Field* Local_R2_Ellipse_Field_Sp(Local_R2_Ellipse *locseg, 
    Geo3d_Scalar_Field *field)
{
  Geo3d_Scalar_Field *result = Local_R2_Ellipse_Field(locseg, field);

  int i;
  for (i = 0; i < result->size; i++) {
    if (result->values[i] < 0.0) {
      result->values[i] = 0.0;
    }
  }

  return result;
}

void Local_R2_Ellipse_Center(const Local_R2_Ellipse *ellipse, double *center)
{
  center[0] = (ellipse->transform.roffset - ellipse->transform.loffset) / 2.0;
  center[1] = (ellipse->transform.toffset - ellipse->transform.boffset) / 2.0;
  center[2] = ellipse->base.z;
}

double Local_R2_Ellipse_Optimize_W(Local_R2_Ellipse *locseg, const Stack *stack,
                                double z_scale, int option,
                                Receptor_Fit_Workspace *ws)
{
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_CORRCOEF;

  int i;
  for (i = 0; i < ws->pos_adjust; i++) {
    Local_R2_Ellipse_Position_Adjust(locseg, stack, z_scale);
  }

  double score = 0.0;
  score = Fit_Local_R2_Ellipse_W(locseg, stack, z_scale, ws);

  //score = Fit_Local_R2_Ellipse_Wc(locseg, stack, z_scale, ws);

  return score;
}

Local_Neuroseg* Local_R2_Ellipse_To_Local_Neuroseg(const Local_R2_Ellipse *rseg,
    Local_Neuroseg *locseg)
{
  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
  }

  locseg->seg.theta = 0;
  locseg->seg.psi = 0;
  locseg->seg.h = 11;

  double center[3];
  Local_R2_Ellipse_Center(rseg, center);
  Set_Neuroseg_Position(locseg, center, NEUROSEG_CENTER);

  locseg->seg.r1 = (rseg->transform.toffset + rseg->transform.boffset) / 2.0 + 1.0;
  double rx = (rseg->transform.loffset + rseg->transform.roffset) / 2.0 + 1.0;

  locseg->seg.scale = rx / locseg->seg.r1;
  locseg->seg.alpha = rseg->transform.alpha;

  return locseg;
}

void Local_R2_Ellipse_Set_Size(Local_R2_Ellipse *ellipse, double rx, double ry)
{
  //todo
  ellipse->transform.loffset += rx - 1.0;
  ellipse->transform.roffset += rx - 1.0;
  ellipse->transform.toffset += ry - 1.0;
  ellipse->transform.boffset += ry - 1.0;
}

void Local_R2_Ellipse_Post_Update(double *var)
{

}
