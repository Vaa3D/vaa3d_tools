/**@file tz_bifold_neuroseg.h
 * @brief bifold neuroseg
 * @author Ting Zhao
 * @date 17-Apr-2008
 */

#ifndef _TZ_BIFOLD_NEUROSEG_H_
#define _TZ_BIFOLD_NEUROSEG_H_

#include "tz_cdefs.h"
#include "tz_bitmask.h"
#include "tz_geo3d_scalar_field.h"

__BEGIN_DECLS

#define BIFOLD_NEUROSEG_NPARAM 10

#define BIFOLD_NEUROSEG_MIN_KNOT 0.05
#define BIFOLD_NEUROSEG_MAX_KNOT 0.95

typedef struct _Bifold_Neuroseg {
  double r1;
  double r2;
  double r3;
  double r4;
  double h;
  double knot;
  double theta2;
  double psi2;
  double theta;
  double psi;
} Bifold_Neuroseg;

typedef Bifold_Neuroseg* Bifold_Neuroseg_P;

Bifold_Neuroseg* New_Bifold_Neuroseg();
void Delete_Bifold_Neuroseg(Bifold_Neuroseg *bn);

void Set_Bifold_Neuroseg(Bifold_Neuroseg *bn, double r1, double r2,
			 double r3, double r4, double h, double knot,
			 double theta2, double psi2, double theta, double psi);
void Reset_Bifold_Neuroseg(Bifold_Neuroseg *bn);

void Fprint_Bifold_Neuroseg(FILE *fp, const Bifold_Neuroseg *bn);
void Print_Bifold_Neuroseg(const  Bifold_Neuroseg *bn);

Geo3d_Scalar_Field* Bifold_Neuroseg_Field(const Bifold_Neuroseg *bn,
					  Geo3d_Scalar_Field *field);

/* variable operation routines */
#define BIFOLD_NEUROSEG_VAR_MASK_R1             0x00000001
#define BIFOLD_NEUROSEG_VAR_MASK_R2             0x00000002
#define BIFOLD_NEUROSEG_VAR_MASK_R3             0x00000004
#define BIFOLD_NEUROSEG_VAR_MASK_R4             0x00000008
#define BIFOLD_NEUROSEG_VAR_MASK_R              0x0000000F
#define BIFOLD_NEUROSEG_VAR_MASK_H              0x00000010
#define BIFOLD_NEUROSEG_VAR_MASK_KNOT           0x00000020
#define BIFOLD_NEUROSEG_VAR_MASK_THETA2         0x00000040
#define BIFOLD_NEUROSEG_VAR_MASK_PSI2           0x00000080
#define BIFOLD_NEUROSEG_VAR_MASK_ORIENTATION2   0x000000C0
#define BIFOLD_NEUROSEG_VAR_MASK_THETA          0x00000100
#define BIFOLD_NEUROSEG_VAR_MASK_PSI            0x00000200
#define BIFOLD_NEUROSEG_VAR_MASK_ORIENTATION    0x00000300 

int Bifold_Neuroseg_Var(const Bifold_Neuroseg *bn, double *var[]);
void Bifold_Neuroseg_Set_Var(Bifold_Neuroseg *bn, int var_index, double value);
int Bifold_Neuroseg_Var_Mask_To_Index(Bitmask_t mask, int *var_index);
/**********************************************************************/




__END_DECLS

#endif
