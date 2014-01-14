/**@file tz_rpi_neuroseg.h.h
 * @author Ting Zhao
 * @date 13-May-2010
 */

#ifndef _TZ_RPI_NEUROSEG_H_
#define _TZ_RPI_NEUROSEG_H_

#include "tz_cdefs.h"
#include "tz_zobject.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_bitmask.h"

__BEGIN_DECLS

typedef struct _Rpi_Neuroseg {
  double theta;
  double psi;
  double left_offset;
  double right_offset;
  double bottom_offset;
  double top_offset;
  double length;
} Rpi_Neuroseg;

DECLARE_ZOBJECT_INTERFACE(Rpi_Neuroseg)

#define RPI_NEUROSEG_NPARAM 7

#define RPI_NEUROSEG_VAR_MASK_THETA         0x00000001
#define RPI_NEUROSEG_VAR_MASK_PSI           0x00000002
#define RPI_NEUROSEG_VAR_MASK_ORIENTATION   0x00000003
#define RPI_NEUROSEG_VAR_MASK_LEFT_OFFSET   0x00000004
#define RPI_NEUROSEG_VAR_MASK_RIGHT_OFFSET  0x00000008
#define RPI_NEUROSEG_VAR_MASK_BOTTOM_OFFSET 0x00000010
#define RPI_NEUROSEG_VAR_MASK_TOP_OFFSET    0x00000020
#define RPI_NEUROSEG_VAR_MASK_OFFSET        0x0000003C
#define RPI_NEUROSEG_VAR_MASK_LENGTH        0x00000040

int Rpi_Neuroseg_Var_Mask_To_Index(Bitmask_t mask, int *var_index);

int Rpi_Neuroseg_Var(const Rpi_Neuroseg *seg, double *var[]);

Geo3d_Scalar_Field* Rpi_Neuroseg_Field(const Rpi_Neuroseg *seg,
    Geo3d_Scalar_Field *field);


__END_DECLS

#endif
