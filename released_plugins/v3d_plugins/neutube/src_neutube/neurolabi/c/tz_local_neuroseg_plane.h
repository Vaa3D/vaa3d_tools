/**@file tz_local_neuroseg_plane.h
 * @brief lcoal neuroseg plane
 * @author Ting Zhao
 * @date 20-Apr-2008
 */

#ifndef _TZ_LOCAL_NEUROSEG_PLANE_H_
#define _TZ_LOCAL_NEUROSEG_PLANE_H_

#include "tz_cdefs.h"
#include "tz_geo3d_circle.h"
#include "tz_neuroseg_plane.h"
#include "tz_neuropos.h"

__BEGIN_DECLS

#define LOCAL_NEUROSEG_PLANE_NPARAM (NEUROSEG_PLANE_NPARAM + NEUROPOS_NPARAM)

typedef struct _Local_Neuroseg_Plane {
  Neuroseg_Plane np;
  neuropos_t pos;
} Local_Neuroseg_Plane;

#define LOCAL_NEUROSEG_PLANE_RADIUS(locnp) (locnp->np.r)

typedef Local_Neuroseg_Plane* Local_Neuroseg_Plane_P;

Local_Neuroseg_Plane* New_Local_Neuroseg_Plane();
void Delete_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp);

void Set_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp, 
			      double r, double theta, double psi,
			      double offset_x, double offset_y,
			      double x, double y, double z);
void Reset_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp);

void Print_Local_Neuroseg_Plane(const Local_Neuroseg_Plane *locnp);
void Local_Neuroseg_Plane_Fwrite(const Local_Neuroseg_Plane *locseg, FILE *fp);
Local_Neuroseg_Plane* Local_Neuroseg_Plane_Fread(Local_Neuroseg_Plane *locnp, 
						 FILE *fp);

Local_Neuroseg_Plane* 
Copy_Local_Neuroseg_Plane(const Local_Neuroseg_Plane *locnp);

Geo3d_Scalar_Field* 
Local_Neuroseg_Plane_Field(const Local_Neuroseg_Plane *locnp, double step,
			   Geo3d_Scalar_Field *field);

/* variable operation routines */
int Local_Neuroseg_Plane_Var(const Local_Neuroseg_Plane *locnp, 
			     double *var[]);
void Local_Neuroseg_Plane_Set_Var(Local_Neuroseg_Plane *locnp, 
				  int var_index, double value);
int Local_Neuroseg_Plane_Param_Array(const Local_Neuroseg_Plane *locnp, 
				     double z_scale, double *param);
int Local_Neuroseg_Plane_Var_Mask_To_Index(Bitmask_t plane_mask, 
					   Bitmask_t neuropos_mask, int *index);
void Local_Neuroseg_Plane_Validate(double *var, const double *var_min, 
				   const double *var_max, const void *param);
/**********************************************************************/

double Local_Neuroseg_Plane_Score_G(const double *var, const Stack *stack);
double Local_Neuroseg_Plane_Score_Gv(const double *var, const void *param);

double Fit_Local_Neuroseg_Plane(Local_Neuroseg_Plane *locnp, const Stack *stack,
				const int *var_index, int nvar, double z_scale,
				Stack_Fit_Score *fs);

void Local_Neuroseg_Plane_Center(const Local_Neuroseg_Plane *locnp,
				 double *center);

Geo3d_Circle*
Local_Neuroseg_Plane_To_Geo3d_Circle(const Local_Neuroseg_Plane *locnp,
				     Geo3d_Circle *circle);

Local_Neuroseg_Plane *Next_Neuroseg_Plane(Local_Neuroseg_Plane *np, double step);

__END_DECLS

#endif
