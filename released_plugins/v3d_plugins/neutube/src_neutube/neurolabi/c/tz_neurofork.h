/**@file tz_neurofork.h
 * @brief neuron fork
 * @author Ting Zhao
 * @date 17-Apr-2008
 */

#ifndef _TZ_NEUROFORK_H_
#define _TZ_NEUROFORK_H_

#include "tz_cdefs.h"
#include "tz_bitmask.h"
#include "tz_geo3d_scalar_field.h"

__BEGIN_DECLS

#define NEUROFORK_NPARAM 10

#define NEUROFORK_MIN_KNOT 0.05
#define NEUROFORK_MAX_KNOT 0.95

typedef struct _Neurofork {
  double r1;
  double r2;
  double r3;
  double r4;
  double r5;
  double r6;
  double h;
  double knot;
  double theta2;
  double psi2;
  double theta;
  double psi;
} Neurofork;

typedef Neurofork* Neurofork_P;

Neurofork* New_Neurofork();
void Delete_Neurofork(Neurofork *bn);

void Set_Neurofork(Neurofork *bn, double r1, double r2,
			 double r3, double r4, double h, double knot,
			 double theta2, double psi2, double theta, double psi);
void Reset_Neurofork(Neurofork *bn);

void Fprint_Neurofork(FILE *fp, const Neurofork *bn);
void Print_Neurofork(const  Neurofork *bn);

Geo3d_Scalar_Field* Neurofork_Field(const Neurofork *bn, 
					  double step,
					  Geo3d_Scalar_Field *field);

/* variable operation routines */
#define NEUROFORK_VAR_MASK_R1             0x00000001
#define NEUROFORK_VAR_MASK_R2             0x00000002
#define NEUROFORK_VAR_MASK_R3             0x00000004
#define NEUROFORK_VAR_MASK_R4             0x00000008
#define NEUROFORK_VAR_MASK_R              0x0000000F
#define NEUROFORK_VAR_MASK_H              0x00000010
#define NEUROFORK_VAR_MASK_KNOT           0x00000020
#define NEUROFORK_VAR_MASK_THETA2         0x00000040
#define NEUROFORK_VAR_MASK_PSI2           0x00000080
#define NEUROFORK_VAR_MASK_ORIENTATION2   0x000000C0
#define NEUROFORK_VAR_MASK_THETA          0x00000100
#define NEUROFORK_VAR_MASK_PSI            0x00000200
#define NEUROFORK_VAR_MASK_ORIENTATION    0x00000300 

int Neurofork_Var(const Neurofork *bn, double *var[]);
void Neurofork_Set_Var(Neurofork *bn, int var_index, double value);
int Neurofork_Var_Mask_To_Index(Bitmask_t mask, int *var_index);
/**********************************************************************/




__END_DECLS

#endif
