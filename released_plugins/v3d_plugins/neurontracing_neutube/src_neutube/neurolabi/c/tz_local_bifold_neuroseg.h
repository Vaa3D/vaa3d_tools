/**@file tz_local_bifold_neuroseg.h
 * @brief Local bifold neuroseg
 * @author Ting Zhao
 * @date 20-Apr-2008
 */

#ifndef _TZ_LOCAL_BIFOLD_NEUROSEG_H_
#define _TZ_LOCAL_BIFOLD_NEUROSEG_H_

#include "tz_cdefs.h"
#include "tz_bifold_neuroseg.h"
#include "tz_neuropos.h"

__BEGIN_DECLS

//#define LOCAL_BIFOLD_NEUROSEG_NPARAM (BIFOLD_NEUROSEG_NPARAM + NEUROPOS_NPARAM)

static const int LOCAL_BIFOLD_NEUROSEG_NPARAM = BIFOLD_NEUROSEG_NPARAM + NEUROPOS_NPARAM;

typedef struct _Local_Bifold_Neuroseg {
  Bifold_Neuroseg bn;
  neuropos_t pos;
} Local_Bifold_Neuroseg;

typedef Local_Bifold_Neuroseg* Local_Bifold_Neuroseg_P;

Local_Bifold_Neuroseg* New_Local_Bifold_Neuroseg();
void Delete_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn);

void Set_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn, 
			       double r1, double r2,
			       double r3, double r4, double h, double knot,
			       double theta2, double psi2,
			       double theta, double psi,
			       double x, double y, double z);

void Reset_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn);

void Print_Local_Bifold_Neuroseg(const Local_Bifold_Neuroseg *locbn);

Geo3d_Scalar_Field* 
Local_Bifold_Neuroseg_Field(const Local_Bifold_Neuroseg *locbn,
			   Geo3d_Scalar_Field *field);

/* variable operation routines */
int Local_Bifold_Neuroseg_Var(const Local_Bifold_Neuroseg *locbn, 
			     double *var[]);
void Local_Bifold_Neuroseg_Set_Var(Local_Bifold_Neuroseg *locbn, 
				  int var_index, double value);
int Local_Bifold_Neuroseg_Param_Array(const Local_Bifold_Neuroseg *locbn, 
				     double z_scale, double *param);
int Local_Bifold_Neuroseg_Var_Mask_To_Index(Bitmask_t plane_mask, 
					   Bitmask_t neuropos_mask, int *index);
void Local_Bifold_Neuroseg_Validate(double *var, const double *var_min, 
				    const double *var_max, const void *param);
/**********************************************************************/

double Local_Bifold_Neuroseg_Score_G(const double *var, const Stack *stack);
double Local_Bifold_Neuroseg_Score_Gv(const double *var, const void *param);

double Fit_Local_Bifold_Neuroseg(Local_Bifold_Neuroseg *locbn, 
				 const Stack *stack, 
				 const int *var_index, int nvar, double z_scale,
				 Stack_Fit_Score *fs);

__END_DECLS

#endif
