/**@file tz_neuropos.h
 * @brief neuron position
 * @author Ting Zhao
 * @date 23-Nov-2007
 */

#ifndef _TZ_NEUROPOS_H_
#define _TZ_NEUROPOS_H_

#include "tz_cdefs.h"
#include "tz_coordinate_3d.h"
#include "tz_bitmask.h"

__BEGIN_DECLS

/*
 * Basic assumptions:
 *   neuropos is a 3D point, which can be manipulated by any 3D spatial 
 *   transformation
 */

#define neuropos_t coordinate_3d_t
#define neuropos_tp coordinate_3d_tp
#define NEUROPOS_NPARAM 3
#define NEUROPOS_NBYTE 24

void Set_Neuropos(neuropos_t pos, double x, double y, double z);
void Reset_Neuropos(neuropos_t pos);

void Neuropos_Copy(neuropos_t pos1, const neuropos_t pos2);

void Fprint_Neuropos(FILE *fp, const neuropos_t pos);
void Print_Neuropos(const neuropos_t pos);

void Neuropos_Fwrite(const neuropos_t pos, FILE *fp);
neuropos_tp Neuropos_Fread(neuropos_t pos, FILE *fp);

void Neuropos_Coordinate(const neuropos_t pos, double *x, double *y, double *z);

void Neuropos_Translate(neuropos_t pos, double x, double y, double z);

void Neuropos_Relative_Coordinate(const neuropos_t pos, 
				  double *x, double *y, double *z);
void Neuropos_Absolute_Coordinate(const neuropos_t pos,
				  double *rx, double *ry, double *rz);

#define NEUROPOS_VAR_MASK_NONE           0x00000000
#define NEUROPOS_VAR_MASK_ALL            0x00000007

int Neuropos_Var_Mask_To_Index(Bitmask_t mask, int *var_index);
int Neuropos_Var(const neuropos_t pos, double *var[]);

void Neuropos_Set_Var(neuropos_t pos, int var_index, double value);

__END_DECLS

#endif
