/**@file tz_local_neuroseg_ellipse.h
 * @brief local neuroseg ellipse
 * @author Ting Zhao
 * @date 20-Apr-2008
 */

#ifndef _TZ_LOCAL_NEUROSEG_ELLIPSE_H_
#define _TZ_LOCAL_NEUROSEG_ELLIPSE_H_

#include "tz_cdefs.h"
#include "tz_geo3d_circle.h"
#include "tz_neuroseg_ellipse.h"
#include "tz_neuropos.h"
#include "tz_geo3d_circle.h"
#include "tz_workspace.h"
#include "tz_geo3d_ellipse.h"

__BEGIN_DECLS

/**@addtogroup local_neuroseg_ellipse_ Local Neuron Ellipse (tz_local_neuroseg_ellipse.h)
 * @{
 */

//#define LOCAL_NEUROSEG_ELLIPSE_NPARAM (NEUROSEG_ELLIPSE_NPARAM + NEUROPOS_NPARAM)

static const int LOCAL_NEUROSEG_ELLIPSE_NPARAM = NEUROSEG_ELLIPSE_NPARAM + NEUROPOS_NPARAM;


#define DECLARE_LOCAL_NEUROSEG_ELLIPSE_VAR_NAME				\
  const static char *Local_Neuroseg_Ellipse_Var_Name[] = {		\
    "rx", "ry", "theta", "psi", "x offset",				\
    "y offset", "alpha", "x", "y", "z"					\
  };

#define DECLARE_LOCAL_NEUROSEG_ELLIPSE_VAR_MIN(var_min)			\
  double var_min[] = {1.0, 1.0, -Infinity, -Infinity, -Infinity, -Infinity, \
		      -Infinity, -Infinity, -Infinity, -Infinity}

#define DECLARE_LOCAL_NEUROSEG_ELLIPSE_VAR_MAX(var_max)			\
  double var_max[] = {50.0, 50.0, Infinity, Infinity, Infinity, Infinity, \
		      Infinity, Infinity, Infinity, Infinity}

/**@struct _Local_Neuroseg_Ellipse tz_local_neuroseg_ellipse.h
 *
 * Local ellipse.
 */
typedef struct _Local_Neuroseg_Ellipse {
  Neuroseg_Ellipse np; /**< ellipse geometry */
  neuropos_t pos;      /**< position */
} Local_Neuroseg_Ellipse;

typedef Local_Neuroseg_Ellipse* Local_Neuroseg_Ellipse_P;

Local_Neuroseg_Ellipse* New_Local_Neuroseg_Ellipse();
void Delete_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locne);

void Set_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locne, 
				double rx, double ry, double theta, double psi,
				double alpha,
				double offset_x, double offset_y,
				double x, double y, double z);
void Default_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locne);

void Print_Local_Neuroseg_Ellipse(const Local_Neuroseg_Ellipse *locne);

Local_Neuroseg_Ellipse* 
Copy_Local_Neuroseg_Ellipse(const Local_Neuroseg_Ellipse *locne);

void Local_Neuroseg_Ellipse_Fwrite(const Local_Neuroseg_Ellipse *locne,
				     FILE *fp);
Local_Neuroseg_Ellipse* 
Local_Neuroseg_Ellipse_Fread(Local_Neuroseg_Ellipse *locne, FILE *fp);

void Local_Neuroseg_Ellipse_Swc_Fprint_T(FILE *fp, 
					 const Local_Neuroseg_Ellipse *locne,
					 int start_id, int parent_id, int type);

void Local_Neuroseg_Ellipse_Normalize_Position(Local_Neuroseg_Ellipse *locne);

void Local_Neuroseg_Ellipse_Scale_Z(Local_Neuroseg_Ellipse *locne,
				    double z_scale);

double Local_Neuroseg_Ellipse_Point_Dist(const Local_Neuroseg_Ellipse *locne,
					 coordinate_3d_t pt);

Geo3d_Circle* 
Local_Neuroseg_Ellipse_To_Geo3d_Circle(const Local_Neuroseg_Ellipse *locne,
				       Geo3d_Circle *circle);

Geo3d_Scalar_Field* 
Local_Neuroseg_Ellipse_Field(const Local_Neuroseg_Ellipse *locne, double step,
			     Geo3d_Scalar_Field *field);

/* variable operation routines */
int Local_Neuroseg_Ellipse_Var(const Local_Neuroseg_Ellipse *locne, 
			       double *var[]);
void Local_Neuroseg_Ellipse_Set_Var(Local_Neuroseg_Ellipse *locne, 
				    int var_index, double value);
int Local_Neuroseg_Ellipse_Param_Array(const Local_Neuroseg_Ellipse *locne, 
				     double z_scale, double *param);
int Local_Neuroseg_Ellipse_Var_Mask_To_Index(Bitmask_t plane_mask, 
					   Bitmask_t neuropos_mask, int *index);
void Local_Neuroseg_Ellipse_Validate(double *var, const double *var_min, 
				     const double *var_max, const void *param);
/**********************************************************************/

double Local_Neuroseg_Ellipse_Score(const Local_Neuroseg_Ellipse *locne, 
				    const Stack *stack, 
				    double z_scale, Stack_Fit_Score *fs);
double Local_Neuroseg_Ellipse_Score_G(const double *var, const Stack *stack);
double Local_Neuroseg_Ellipse_Score_Gv(const double *var, const void *param);

double 
Fit_Local_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *locne, const Stack *stack, 
			   const int *var_index, int nvar, double z_scale,
			   Stack_Fit_Score *fs);


double 
Fit_Local_Neuroseg_Ellipse_W(Local_Neuroseg_Ellipse *locne, const Stack *stack, 
			     double z_scale, Locseg_Fit_Workspace *ws);

double 
Local_Neuroseg_Ellipse_Optimize_W(Local_Neuroseg_Ellipse *locne, 
    const Stack *stack, double z_scale, Locseg_Fit_Workspace *ws);

void Local_Neuroseg_Ellipse_Center(const Local_Neuroseg_Ellipse *locne,
				   double *center);

Local_Neuroseg_Ellipse*
Next_Neuroseg_Ellipse(Local_Neuroseg_Ellipse *np, double step);

double Local_Neuroseg_Ellipse_Point_Angle(const Local_Neuroseg_Ellipse *locne,
					   const coordinate_3d_t coord);

void Local_Neuroseg_Ellipse_Points(const Local_Neuroseg_Ellipse *locne,
				   int npt, double start, 
				   coordinate_3d_t coord[]);

void Local_Neuroseg_Ellipse_Stack_Centroid(Local_Neuroseg_Ellipse *locne, 
    const Stack *stack, const Stack *mask, double *centroid);

void Local_Neuroseg_Ellipse_Reset_Center(Local_Neuroseg_Ellipse *locne,
    const double *center);

coordinate_3d_t* 
Local_Neuroseg_Ellipse_Array_Points(const Local_Neuroseg_Ellipse *locne,
				    int nobj, int nsample, 
				    coordinate_3d_t *pts);

Local_Neuroseg_Ellipse* Make_Local_Neuroseg_Ellipse_Array(int n);

Geo3d_Ellipse* 
Local_Neuroseg_Ellipse_To_Geo3d_Ellipse(const Local_Neuroseg_Ellipse *locne,
   Geo3d_Ellipse *ellipse);

void Local_Neuroseg_Ellipse_Draw_Stack(const Local_Neuroseg_Ellipse *locne,
    Stack *stack, uint8_t r, uint8_t g, uint8_t b);

void Local_Neuroseg_Ellipse_Label(const Local_Neuroseg_Ellipse *locne,
    Stack *stack, double z_scale, int flag, int value); 
/**@}*/

__END_DECLS

#endif
