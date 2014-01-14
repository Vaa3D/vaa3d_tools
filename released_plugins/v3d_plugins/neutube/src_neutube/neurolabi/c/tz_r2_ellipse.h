/**@file tz_r2_ellipse.h
 * @author Ting Zhao
 * @date 06-Nov-2010
 */

#ifndef _TZ_R2_ELLIPSE_H_
#define _TZ_R2_ELLIPSE_H_

#include "tz_cdefs.h"
#include "tz_zobject.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_image_lib_defs.h"
#include "tz_receptor_transform.h"
#include "tz_workspace.h"
#include "tz_local_neuroseg.h"

__BEGIN_DECLS

typedef struct _R2_Ellipse {
  int z;
} R2_Ellipse;

typedef R2_Ellipse* R2_Ellipse_P;

Geo3d_Scalar_Field* R2_Ellipse_Field(const R2_Ellipse *seg,
    Geo3d_Scalar_Field *field);

typedef struct _Local_R2_Ellipse {
  R2_Ellipse base;
  Receptor_Transform_Ellipse_2d transform;
} Local_R2_Ellipse;

typedef Local_R2_Ellipse* Local_R2_Ellipse_P;

DECLARE_ZOBJECT_INTERFACE(Local_R2_Ellipse)

Local_R2_Ellipse* Copy_Local_R2_Ellipse(Local_R2_Ellipse *ellipse);
void Local_R2_Ellipse_Fwrite(Local_R2_Ellipse *ellipse, FILE *fp);
Local_R2_Ellipse* Local_R2_Ellipse_Fread(Local_R2_Ellipse *ellipse, FILE *fp);
void Local_R2_Ellipse_Fwrite_V(const Local_R2_Ellipse *ellipse, FILE *stream, 
    double version);
  
void Set_Local_R2_Ellipse(Local_R2_Ellipse *ellipse, double z, double alpha, 
    double loffset, double roffset, double toffset, double boffset);

#define R2_ELLIPSE_BASE_NPARAM 1
//#define LOCAL_R2_ELLIPSE_NPARAM (RECEPTOR_TRANSFORM_ELLIPSE_2D_NPARAM+1)
const static int LOCAL_R2_ELLIPSE_NPARAM = RECEPTOR_TRANSFORM_ELLIPSE_2D_NPARAM + R2_ELLIPSE_BASE_NPARAM;

//#define DECLARE_LOCAL_R2_ELLIPSE_VAR_MIN DECLARE_RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MIN
//#define DECLARE_LOCAL_R2_ELLIPSE_VAR_MAX DECLARE_RECEPTOR_TRANSFORM_ELLIPSE_2D_VAR_MAX

#define DECLARE_LOCAL_R2_ELLIPSE_VAR_MIN(var_min)		\
  double var_min[] = {-Infinity, -Infinity, -Infinity, -Infinity, -Infinity, -Infinity};

#define DECLARE_LOCAL_R2_ELLIPSE_VAR_MAX(var_max)		\
  double var_max[] = {Infinity, Infinity, Infinity, Infinity, Infinity, Infinity};

int Local_R2_Ellipse_Var_Mask_To_Index(Bitmask_t mask, int *index);

void Local_R2_Ellipse_Set_Var(Local_R2_Ellipse *seg, int var_index, 
        double value);
void Local_R2_Ellipse_Validate(double *var, const double *var_min, 
        const double *var_max, const void *param);

Geo3d_Scalar_Field* Local_R2_Ellipse_Field(const Local_R2_Ellipse *locseg,
        Geo3d_Scalar_Field *field);
Geo3d_Scalar_Field* Local_R2_Ellipse_Field_C(const Local_R2_Ellipse *locseg,
        Geo3d_Scalar_Field *field);

double Local_R2_Ellipse_Score_W(const Local_R2_Ellipse *locseg, 
        const Stack *stack, double z_scale, Receptor_Score_Workspace *ws);
double Local_R2_Ellipse_Score_Wc(const Local_R2_Ellipse *locseg, 
        const Stack *stack, double z_scale, Receptor_Score_Workspace *ws);

double Local_R2_Ellipse_Score_R(const double *var, const void *param);
double Local_R2_Ellipse_Score_Rc(const double *var, const void *param);
double Local_R2_Ellipse_Score_P(const Local_R2_Ellipse *locseg, 
    const Stack *stack, double z_scale, Stack_Fit_Score *fs);
double Local_R2_Ellipse_Score_Pc(const Local_R2_Ellipse *locseg, 
    const Stack *stack, double z_scale, Stack_Fit_Score *fs);

double Fit_Local_R2_Ellipse_Pr(Local_R2_Ellipse *locseg, 
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs);
double Fit_Local_R2_Ellipse_Pr_C(Local_R2_Ellipse *locseg, 
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs);
double Fit_Local_R2_Ellipse_W(Local_R2_Ellipse *locseg, const Stack *stack,
    double z_scale, Receptor_Fit_Workspace *ws);
double Fit_Local_R2_Ellipse_Wc(Local_R2_Ellipse *locseg, const Stack *stack,
    double z_scale, Receptor_Fit_Workspace *ws);
double Local_R2_Ellipse_Optimize_W(Local_R2_Ellipse *locseg,
    const Stack *stack, double z_scale, int option, 
    Receptor_Fit_Workspace *ws);
void Local_R2_Ellipse_Position_Adjust(Local_R2_Ellipse *locseg,
    const Stack *stack, double z_scale);

Local_Neuroseg* Local_R2_Ellipse_To_Local_Neuroseg(const Local_R2_Ellipse *rseg,
    Local_Neuroseg *locseg);

void Local_R2_Ellipse_Center(const Local_R2_Ellipse *locseg, double *center);
void Local_R2_Ellipse_Set_Center(Local_R2_Ellipse *locseg, 
    const double *center);
void Local_R2_Ellipse_Set_Size(Local_R2_Ellipse *locseg, double rx, double ry);

void Local_R2_Ellipse_Post_Update(double *var);

__END_DECLS

#endif
