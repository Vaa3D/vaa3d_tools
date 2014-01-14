/**@file tz_local_rpi_neuroseg.h
 * @author Ting Zhao
 * @date 13-May-2010
 */

#ifndef _TZ_LOCAL_RPI_NEUROSEG_H_
#define _TZ_LOCAL_RPI_NEUROSEG_H_

#include "tz_cdefs.h"
#include "tz_rpi_neuroseg.h"
#include "tz_neuropos.h"
#include "tz_workspace.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_local_neuroseg.h"

__BEGIN_DECLS

const static int LOCAL_RPI_NEUROSEG_NPARAM = RPI_NEUROSEG_NPARAM + NEUROPOS_NPARAM;

#define DECLARE_LOCAL_RPI_NEUROSEG_VAR_NAME				\
  const static char *Local_Rpi_Neuroseg_Var_Name[] = {			\
    "theta", "psi", "left offset", "right offset", "bottom offset", 	\
    "top offset", "length", "x", "y", "z"				\
  };

#define DECLARE_LOCAL_RPI_NEUROSEG_VAR_MIN(var_min)			\
  double var_min[] = {-Infinity, -Infinity, -10.0, -10.0,  		\
		      -10.0, -10.0, 3.0, -Infinity, -Infinity, -Infinity};

#define DECLARE_LOCAL_RPI_NEUROSEG_VAR_MAX(var_max)			\
  double var_max[] = {Infinity, Infinity, 10.0, 10.0,	  		\
		      10.0, 10.0, 20.0, Infinity, Infinity, Infinity};

/*#define MIN_GRADIENT 1e-5*/ /*obsolete. minimal score gradient */

/**@brief A data structure for local neuron segment.
 *
 * Local_Neuroseg consists of two parts, a neuron segment and its position. The
 * position is the coordinates of the default reference point, which is defined
 * as Neuropos_Reference.
 */
typedef struct _Local_Rpi_Neuroseg {
  Rpi_Neuroseg seg;
  neuropos_t pos;
} Local_Rpi_Neuroseg;


DECLARE_ZOBJECT_INTERFACE(Local_Rpi_Neuroseg)

int Local_Rpi_Neuroseg_Var_Mask_To_Index(Bitmask_t neuroseg_mask, 
    Bitmask_t neuropos_mask, int *index);

void Local_Rpi_Neuroseg_Set_Var(Local_Rpi_Neuroseg *seg, int var_index, 
    double value);
void Local_Rpi_Neuroseg_Validate(double *var, const double *var_min, 
    const double *var_max, const void *param);

Geo3d_Scalar_Field* Local_Rpi_Neuroseg_Field(const Local_Rpi_Neuroseg *locseg,
    Geo3d_Scalar_Field *field);

double Local_Rpi_Neuroseg_Score_W(const Local_Rpi_Neuroseg *locseg, 
    const Stack *stack, double z_scale, Receptor_Score_Workspace *ws);

double Local_Rpi_Neuroseg_Score_G(const double *var, const void *param);
double Local_Rpi_Neuroseg_Score_R(const double *var, const void *param);
double Local_Rpi_Neuroseg_Score_P(const Local_Rpi_Neuroseg *locseg, 
    const Stack *stack, double z_scale, Stack_Fit_Score *fs);
double Fit_Local_Rpi_Neuroseg_Pr(Local_Rpi_Neuroseg *locseg, 
    const Stack *stack, const int *var_index, int nvar, int *var_link,
    double *var_min, double *var_max, double z_scale, Stack_Fit_Score *fs);
double Fit_Local_Rpi_Neuroseg_W(Local_Rpi_Neuroseg *locseg, const Stack *stack,
    double z_scale, Receptor_Fit_Workspace *ws);

Local_Neuroseg* 
Local_Rpi_Neuroseg_To_Local_Neuroseg(const Local_Rpi_Neuroseg *rpiseg,
    Local_Neuroseg *locseg);
void Local_Rpi_Neuroseg_From_Local_Neuroseg(Local_Rpi_Neuroseg *rpiseg,
    const Local_Neuroseg *locseg);
double Local_Rpi_Neuroseg_Optimize_W(Local_Rpi_Neuroseg *rpiseg, 
    const Stack *stack, double z_scale, int option, Locseg_Fit_Workspace *ws);

__END_DECLS

#endif
