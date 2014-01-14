/**@file tz_voxeltrans.h
 * @brief voxel data translation
 * @author Ting Zhao
 */

#ifndef _TZ_VOXELTRANSH_H_
#define _TZ_VOXELTRANSH_H_

#include "tz_cdefs.h"
#include "tz_tvoxel.h"
#include "tz_voxel.h"

__BEGIN_DECLS

/**@addtogroup voxeltrans_ voxel type transformation (tz_voxeltrans.h)
 * @{
 */

/*
 * Voxel_To_Tvoxel() copies the values of <v> to <vt> and TVoxel_To_Voxel()
 * copies the values of <v> to <vt>.
 */
/**@brief Turn a voxel object into a voxel type */
void Voxel_To_Tvoxel(const Voxel *v, Voxel_t vt);

/**@brief Turn a voxel type into a voxel object */
void Tvoxel_To_Voxel(const Voxel_t vt, Voxel *v);

/**@}*/

__END_DECLS

#endif 
