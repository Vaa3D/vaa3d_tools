/**@file tz_r2_rect.h
 * @author Ting Zhao
 * @date 06-Nov-2010
 */

#ifndef _TZ_R2_RECT_H_
#define _TZ_R2_RECT_H_

#include "tz_cdefs.h"
#include "tz_zobject.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_image_lib_defs.h"
#include "tz_receptor_transform.h"
#include "tz_workspace.h"
#include "tz_local_neuroseg.h"

__BEGIN_DECLS

typedef struct _R2_Rect {
  int z;
} R2_Rect;

typedef R2_Rect* R2_Rect_P;

Geo3d_Scalar_Field* R2_Rect_Field(const R2_Rect *seg,
    Geo3d_Scalar_Field *field);

typedef struct _Local_R2_Rect {
  R2_Rect base;
  Receptor_Transform_Rect_2d transform;
} Local_R2_Rect;

typedef Local_R2_Rect* Local_R2_Rect_P;

DECLARE_ZOBJECT_INTERFACE(Local_R2_Rect)

Local_R2_Rect* Copy_Local_R2_Rect(Local_R2_Rect *rect);
void Local_R2_Rect_Fwrite(Local_R2_Rect *rect, FILE *fp);
Local_R2_Rect* Local_R2_Rect_Fread(Local_R2_Rect *rect, FILE *fp);
void Local_R2_Rect_Fwrite_V(const Local_R2_Rect *rect, FILE *stream, 
    double version);
  
void Set_Local_R2_Rect(Local_R2_Rect *rect, double x, double y, double z,
    double h, double theta, double r);

#define R2_RECT_BASE_NPARAM 1
//#define LOCAL_R2_RECT_NPARAM (RECEPTOR_TRANSFORM_RECT_2D_NPARAM+1)
const static int LOCAL_R2_RECT_NPARAM = RECEPTOR_TRANSFORM_RECT_2D_NPARAM + R2_RECT_BASE_NPARAM;

//#define DECLARE_LOCAL_R2_RECT_VAR_MIN DECLARE_RECEPTOR_TRANSFORM_RECT_2D_VAR_MIN
//#define DECLARE_LOCAL_R2_RECT_VAR_MAX DECLARE_RECEPTOR_TRANSFORM_RECT_2D_VAR_MAX

#define DECLARE_LOCAL_R2_RECT_VAR_MIN(var_min)		\
  double var_min[] = {0.5, -Infinity, 5.0, -Infinity, -Infinity, -Infinity};

#define DECLARE_LOCAL_R2_RECT_VAR_MAX(var_max)		\
  double var_max[] = {10.0, Infinity, 10.0, Infinity, Infinity, Infinity};

int Local_R2_Rect_Var_Mask_To_Index(Bitmask_t mask, int *index);

void Local_R2_Rect_Set_Var(Local_R2_Rect *seg, int var_index, 
        double value);
void Local_R2_Rect_Validate(double *var, const double *var_min, 
        const double *var_max, const void *param);

Geo3d_Scalar_Field* Local_R2_Rect_Field(const Local_R2_Rect *locseg,
        Geo3d_Scalar_Field *field);

double Local_R2_Rect_Score_W(const Local_R2_Rect *locseg, 
        const Stack *stack, double z_scale, Receptor_Score_Workspace *ws);

double Local_R2_Rect_Score_G(const double *var, const void *param);
double Local_R2_Rect_Score_R(const double *var, const void *param);
double Local_R2_Rect_Score_P(const Local_R2_Rect *locseg, 
    const Stack *stack, double z_scale, Stack_Fit_Score *fs);
double Fit_Local_R2_Rect_Pr(Local_R2_Rect *locseg, 
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs);
double Fit_Local_R2_Rect_W(Local_R2_Rect *locseg, const Stack *stack,
    double z_scale, Receptor_Fit_Workspace *ws);
double Local_R2_Rect_Optimize_W(Local_R2_Rect *locseg,
    const Stack *stack, double z_scale, int option, 
    Receptor_Fit_Workspace *ws);

Local_Neuroseg* Local_R2_Rect_To_Local_Neuroseg(const Local_R2_Rect *rseg,
    Local_Neuroseg *locseg);

void Local_R2_Rect_Center(const Local_R2_Rect *locseg, double *center);
void Local_R2_Rect_Set_Center(Local_R2_Rect *locseg, const double *center);
BOOL Local_R2_Rect_Hit_Test(const Local_R2_Rect *rect, double x, double y,
    double z);

void Local_R2_Rect_Bottom(const Local_R2_Rect *rect, double pos[]);
void Local_R2_Rect_Top(const Local_R2_Rect *rect, double pos[]);

__END_DECLS

#endif
