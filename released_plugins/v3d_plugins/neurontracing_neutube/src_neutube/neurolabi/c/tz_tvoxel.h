/**@file tz_tvoxel.h
 * @brief voxel type
 * @author Ting Zhao
 * @date 04-Jan-2008
 */

#ifndef __TZ_TVOXEL_H_
#define __TZ_TVOXEL_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#define INT_VOXEL_TYPE

typedef int svoxel_t;
typedef svoxel_t Voxel_t[3];

#define voxel_t Voxel_t

/*
 * Copy_Tvoxel() copies <src> to <dst>.
 */
void Copy_Tvoxel(Voxel_t dst, const Voxel_t src); 

/*
 * Print_Tvoxel prints <v> to the standard output.
 */
void Print_Tvoxel(const Voxel_t v);

void Set_Tvoxel(Voxel_t v, int x, int y, int z);

__END_DECLS

#endif
