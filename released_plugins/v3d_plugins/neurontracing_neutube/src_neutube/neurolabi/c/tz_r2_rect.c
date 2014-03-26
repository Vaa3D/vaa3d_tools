#include <string.h>
#include "tz_r2_rect.h"
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_perceptor.h"
#include "tz_geoangle_utils.h"
#include "tz_coordinate_3d.h"
#include "tz_3dgeom.h"

#define R2_RECT_SLICE_FIELD_LENGTH 33

#define DELTA_R 0.1
#define DELTA_THETA 0.015
#define DELTA_H 1.0
#define DELTA_X 0.5
#define DELTA_Y 0.5
#define DELTA_Z 0.5

static const double Delta[] = {
    DELTA_R, DELTA_THETA, DELTA_H, DELTA_X, DELTA_Y, DELTA_Z };

void R2_Rect_Slice_Field(const R2_Rect *seg, 
    coordinate_3d_t *pcoord, double *value, int *length)
{
  UNUSED_PARAMETER(seg);

  *length = R2_RECT_SLICE_FIELD_LENGTH;
  int i;
  pcoord[0][0] = -1.6;
  pcoord[0][1] = 0;
  pcoord[0][2] = seg->z;
  double x2 = pcoord[0][0] * pcoord[0][0];
  value[0] = (1 - x2) * exp(-x2);
  
  for (i = 1; i < *length; i++) {
    pcoord[i][0] = pcoord[i-1][0] + 0.1;
    pcoord[i][1] = 0;
    pcoord[i][2] = seg->z;
    x2 = pcoord[i][0] * pcoord[i][0];
    value[i] = (1 - x2) * exp(-x2);
  }
}

Geo3d_Scalar_Field* R2_Rect_Field(const R2_Rect *seg, Geo3d_Scalar_Field *field)
{
  int nslice = 11;
  if (field == NULL) {
    field = New_Geo3d_Scalar_Field();
    Construct_Geo3d_Scalar_Field(field, R2_RECT_SLICE_FIELD_LENGTH * 11);
  }

  int length;
  
  coordinate_3d_t *points = field->points;
  double *values = field->values;
  double y = 0.0;
  double y_step = 0.1;

  R2_Rect_Slice_Field(seg, points, values, &length);
  field->size = length;

  int i, j;
  for (j = 1; j < nslice; j++) {
    y += y_step;
    points += length;
    values += length;
    field->size += length;

    memcpy(points, field->points, sizeof(coordinate_3d_t) * length);
    memcpy(values, field->values, sizeof(double) * length);
    for (i = 0; i < length; i++) {
      points[i][1] = y;
    }
  }

  return field;
}

void Default_Local_R2_Rect(Local_R2_Rect *locseg)
{
  locseg->transform.r = 1.0;
  locseg->transform.theta = 0.0;
  locseg->transform.h = 10.0;
  locseg->transform.x = 0.0;
  locseg->transform.y = 0.0;
  locseg->base.z = 0.0;
}

void Clean_Local_R2_Rect(Local_R2_Rect *locseg)
{
  Default_Local_R2_Rect(locseg);
}

void Print_Local_R2_Rect(Local_R2_Rect *locseg)
{
  printf("r: %g; theta: %g; h: %g; ", locseg->transform.r, 
      locseg->transform.theta, locseg->transform.h);
  printf("Position: (%g, %g, %d)\n", locseg->transform.x, locseg->transform.y,
      locseg->base.z);
}

DEFINE_ZOBJECT_INTERFACE(Local_R2_Rect)

Local_R2_Rect* Copy_Local_R2_Rect(Local_R2_Rect *rect)
{
  Local_R2_Rect *rect2 = New_Local_R2_Rect();
  *rect2 = *rect;

  return rect2;
}

void Set_Local_R2_Rect(Local_R2_Rect *rect, double x, double y, double z,
    double h, double theta, double r)
{
  rect->base.z = z;
  rect->transform.x = x;
  rect->transform.y = y;
  rect->transform.h = h;
  rect->transform.r = r;
  rect->transform.theta = theta;
}

int Local_R2_Rect_Var_Mask_To_Index(Bitmask_t mask, int *index)
{
  return Receptor_Transform_Rect_2d_Var_Mask_To_Index(mask, index);
}

int Local_R2_Rect_Var(const Local_R2_Rect *locseg, double *var[])
{
  var[0] = (double*) &(locseg->transform.r);
  var[1] = (double*) &(locseg->transform.theta);
  var[2] = (double*) &(locseg->transform.h);
  var[3] = (double*) &(locseg->transform.x);
  var[4] = (double*) &(locseg->transform.y);
  var[5] = (double*) &(locseg->base.z);

  return LOCAL_R2_RECT_NPARAM;
}

void Local_R2_Rect_Set_Var(Local_R2_Rect *locseg, int var_index, 
        double value)
{
  double *var[LOCAL_R2_RECT_NPARAM];
  Local_R2_Rect_Var(locseg, var);
  *(var[var_index]) = value;
}

int Local_R2_Rect_Param_Array(const Local_R2_Rect *locseg,
    double z_scale, double *param)
{
  int i;
  double *var[LOCAL_R2_RECT_NPARAM];
  Local_R2_Rect_Var(locseg, var);
  for (i = 0; i < LOCAL_R2_RECT_NPARAM; i++) {
    param[i] = *(var[i]);
  }
  param[LOCAL_R2_RECT_NPARAM] = z_scale;

  return LOCAL_R2_RECT_NPARAM;
}

void Local_R2_Rect_Validate(double *var, const double *var_min,
    const double *var_max, const void *param)
{ 
  int i;
  for (i = 0; i < LOCAL_R2_RECT_NPARAM; i++) {
    if (var[i] < var_min[i]) {
      var[i] = var_min[i];
    } else if (var[i] > var_max[i]) {
      var[i] = var_max[i];
    }
  }
}

Geo3d_Scalar_Field* Local_R2_Rect_Field(const Local_R2_Rect *locseg,
        Geo3d_Scalar_Field *field)
{
  field = R2_Rect_Field(&(locseg->base), field);
  Receptor_Transform_Rect_2d_Apply(&(locseg->transform), field);

#ifdef _DEBUG_2
  Print_Geo3d_Scalar_Field(field);
#endif

  return field;
}

double Local_R2_Rect_Score_W(const Local_R2_Rect *locseg, 
        const Stack *stack, double z_scale, Receptor_Score_Workspace *ws)
{
  Geo3d_Scalar_Field *field = Local_R2_Rect_Field(locseg, NULL);
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

double Local_R2_Rect_Score_R(const double *var, const void *param)
{
  void **param_array = (void**) param;

  Stack *stack = (Stack *) param_array[0];
  Receptor_Score_Workspace *ws =
    (Receptor_Score_Workspace *) param_array[1];

  Local_R2_Rect seg;
  int i;
  for (i = 0; i < LOCAL_R2_RECT_NPARAM; i++) {
    Local_R2_Rect_Set_Var(&seg, i, var[i]);
  }

  double z_scale = var[LOCAL_R2_RECT_NPARAM];

  double score = Local_R2_Rect_Score_W(&seg, stack, z_scale, ws);

  return score;
}

double Local_R2_Rect_Score_P(const Local_R2_Rect *locseg,
    const Stack *stack, double z_scale, Stack_Fit_Score *fs)
{
  double score = 0.0;

  Geo3d_Scalar_Field *field = Local_R2_Rect_Field(locseg, NULL);
  score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

  Kill_Geo3d_Scalar_Field(field);

  return score;
}

double Fit_Local_R2_Rect_Pr(Local_R2_Rect *locseg,
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs)
{
  if ((stack == NULL) || (locseg == NULL)) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_R2_RECT_NPARAM);

  double *var = darray_malloc(LOCAL_R2_RECT_NPARAM + 1);
  var[LOCAL_R2_RECT_NPARAM] = 1.0;
  Local_R2_Rect_Param_Array(locseg, z_scale, var);

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
    Make_Continuous_Function(Local_R2_Rect_Score_R,
        Local_R2_Rect_Validate, var_min, var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_R2_RECT_NPARAM; i++) {
    Local_R2_Rect_Set_Var(locseg, i, perceptor.vs->var[i]);
  }
  Delete_Variable_Set(perceptor.vs);
  free(var);
  Free_Continuous_Function(perceptor.s);

  locseg->transform.theta = Normalize_Radian(locseg->transform.theta);

  Kill_Receptor_Score_Workspace((Receptor_Score_Workspace*)perceptor.arg);

  return Local_R2_Rect_Score_P(locseg, stack, z_scale, fs);
}

double Fit_Local_R2_Rect_W(Local_R2_Rect *locseg, const Stack *stack,
        double z_scale, Receptor_Fit_Workspace *ws)
{
  if ((stack == NULL) || (locseg == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  double *weight = darray_malloc(LOCAL_R2_RECT_NPARAM);

  double *var = darray_malloc(LOCAL_R2_RECT_NPARAM + 1);

  Local_R2_Rect_Param_Array(locseg, z_scale, var);

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
    Make_Continuous_Function(Local_R2_Rect_Score_R,
        Local_R2_Rect_Validate, ws->var_min, ws->var_max);

  Fit_Perceptor(&perceptor, stack);

  for (i = 0; i < LOCAL_R2_RECT_NPARAM; i++) {
    Local_R2_Rect_Set_Var(locseg, i, perceptor.vs->var[i]);
  }
  Delete_Variable_Set(perceptor.vs);
  free(var);
  free(weight);
  Free_Continuous_Function(perceptor.s);

  locseg->transform.theta = Normalize_Radian(locseg->transform.theta);

  return Local_R2_Rect_Score_W(locseg, stack, z_scale, ws->sws);
}

void Local_R2_Rect_Position_Adjust(Local_R2_Rect *locseg,
    const Stack *stack, double z_scale)
{   
  /* alloc <field> */
  Geo3d_Scalar_Field *field = Local_R2_Rect_Field(locseg, NULL);

  Geo3d_Scalar_Field_Stack_Sampling(field, stack, z_scale, 
      field->values);

  coordinate_3d_t center;
  Geo3d_Scalar_Field_Centroid(field, center);
#ifdef _DEBUG_2
  printf("%g, %g, %g\n", center[0], center[1], center[2]);
#endif

  locseg->transform.x = center[0];
  locseg->transform.y = center[1];

  /* free <field> */
  Kill_Geo3d_Scalar_Field(field);
} 

double Local_R2_Rect_R_Scale_Search(Local_R2_Rect *locseg, 
    const Stack *stack, double z_scale, double r_start, double r_end,
    double r_step, Receptor_Score_Workspace *fs)
{
  double best_score;
  double score;

  best_score = Local_R2_Rect_Score_W(locseg, stack, z_scale, fs);

  double best_r = locseg->transform.r;

  double r;
  for (r = r_start; r <= r_end; r += r_step) {
    locseg->transform.r = r;

    score = Local_R2_Rect_Score_W(locseg, stack, z_scale, fs);

    if (score > best_score) {
      best_score = score;
      best_r = r;
    }
  }

  locseg->transform.r = best_r;

  return best_score;
}

Geo3d_Scalar_Field* Local_R2_Rect_Field_Sp(Local_R2_Rect *locseg, 
    Geo3d_Scalar_Field *field)
{
  Geo3d_Scalar_Field *result = Local_R2_Rect_Field(locseg, field);

  int i;
  for (i = 0; i < result->size; i++) {
    if (result->values[i] < 0.0) {
      result->values[i] = 0.0;
    }
  }

  return result;
}

void Local_R2_Rect_Center(const Local_R2_Rect *locseg, double *center)
{
  center[0] = locseg->transform.x - 0.5 * locseg->transform.h 
    * sin(locseg->transform.theta);
  center[1] = locseg->transform.y + 0.5 * locseg->transform.h 
    * cos(locseg->transform.theta);
  center[2] = locseg->base.z;
}

void Local_R2_Rect_Set_Center(Local_R2_Rect *locseg, const double *center)
{
  locseg->transform.x = center[0] + 0.5 * locseg->transform.h 
    * sin(locseg->transform.theta);
  locseg->transform.y = center[1] - 0.5 * locseg->transform.h 
    * cos(locseg->transform.theta);
  locseg->base.z = center[2];
}

double Local_R2_Rect_Orientation_Search_C(Local_R2_Rect *locseg,
    const Stack *stack, double z_scale, Stack_Fit_Score *fs)
{
  double best_theta;
  double best_score;
  double theta;
  double score;

  double center[3];
  Local_R2_Rect_Center(locseg, center);

  Geo3d_Scalar_Field *field = Local_R2_Rect_Field_Sp(locseg, NULL);

  best_score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);
  best_theta = locseg->transform.theta;

  Local_R2_Rect tmp_locseg = *locseg;

  for (theta = 0.0; theta < TZ_2PI; theta += 0.2) {
    tmp_locseg.transform.theta = theta;

    Local_R2_Rect_Set_Center(&tmp_locseg, center);
    Local_R2_Rect_Field_Sp(&tmp_locseg, field);

    score = Geo3d_Scalar_Field_Stack_Score(field, stack, z_scale, fs);

    if (score > best_score) {
      best_theta = tmp_locseg.transform.theta;
      best_score = score;
    }
  }

  locseg->transform.theta = best_theta;
  Local_R2_Rect_Set_Center(locseg, center);

  Kill_Geo3d_Scalar_Field(field);

  return best_score;
}


double Local_R2_Rect_Optimize_W(Local_R2_Rect *locseg, const Stack *stack,
                                double z_scale, int option,
                                Receptor_Fit_Workspace *ws)
{
  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_CORRCOEF;

  int i;
  for (i = 0; i < ws->pos_adjust; i++) {
    Local_R2_Rect_Position_Adjust(locseg, stack, z_scale);
  }

  Local_R2_Rect_Orientation_Search_C(locseg, stack, z_scale, &fs);

  if (option <= 1) {
    for (i = 0; i < 3; i++) {
      Local_R2_Rect_Position_Adjust(locseg, stack, z_scale);
    }
  }

  if ((option == 1) || (option == 2)){
    Local_R2_Rect_R_Scale_Search(locseg, stack, z_scale, 1.0, 10.0, 1.0, ws->sws);
  }

  double score = Fit_Local_R2_Rect_W(locseg, stack, z_scale, ws);

  return score;
}

Local_Neuroseg* Local_R2_Rect_To_Local_Neuroseg(const Local_R2_Rect *rseg,
    Local_Neuroseg *locseg)
{
  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
  }

  locseg->seg.theta = -TZ_PI_2;
  locseg->seg.psi = rseg->transform.theta;
  locseg->seg.h = rseg->transform.h;

  locseg->pos[0] = rseg->transform.x;
  locseg->pos[1] = rseg->transform.y;
  locseg->pos[2] = rseg->base.z;

  locseg->seg.r1 = rseg->transform.r;
  locseg->seg.scale = 1.0;
  

  return locseg;
}

BOOL Local_R2_Rect_Hit_Test(const Local_R2_Rect *rect, double x, double y,
    double z)
{
  if (z == rect->base.z) {
    coordinate_3d_t pt;
    x -= rect->transform.x;
    y -= rect->transform.y;
    Set_Coordinate_3d(pt, x, y, z);
    Scale_X_Rotate_Z(pt, pt, 1 , rect->transform.r, rect->transform.theta, 1);
    if ((fabs(pt[0]) <= 1.0) && 
        ((pt[1] >= -0.5) && (pt[1] <= rect->transform.h - 0.5))) {
      return TRUE;
    }
  }

  return FALSE;
}

void Local_R2_Rect_Bottom(const Local_R2_Rect *rect, double pos[])
{
  pos[0] = rect->transform.x;
  pos[1] = rect->transform.y;
  pos[2] = rect->base.z;
}

void Local_R2_Rect_Top(const Local_R2_Rect *rect, double pos[])
{
  /* Note that the start vector is (0, 1) */
  pos[0] = rect->transform.x - rect->transform.h 
    * sin(rect->transform.theta);
  pos[1] = rect->transform.y + rect->transform.h 
    * cos(rect->transform.theta);
  pos[2] = rect->base.z;
}

Local_R2_Rect* Local_R2_Rect_Fread(Local_R2_Rect *rect, FILE *fp)
{
  TZ_ERROR(ERROR_PART_FUNC);
  UNUSED_PARAMETER(rect);
  UNUSED_PARAMETER(fp);

  return NULL;
}

void Local_R2_Rect_Fwrite(Local_R2_Rect *rect, FILE *fp)
{
  TZ_ERROR(ERROR_PART_FUNC);
  UNUSED_PARAMETER(rect);
  UNUSED_PARAMETER(fp);
}
