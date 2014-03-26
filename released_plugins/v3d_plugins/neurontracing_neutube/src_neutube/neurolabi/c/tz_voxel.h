/**@file tz_voxel.h
 * @brief struct for voxel
 * @author Ting Zhao
 */

#ifndef _TZ_VOXEL_H_
#define _TZ_VOXEL_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/**@addtogroup voxel_ Voxel object
 * @{
 */

/**@typedef Voxel_ 
 * @brief Voxel type. 
 *
 * This is a non-reference object 
 */
typedef struct _Voxel {
  int x;
  int y;
  int z;
} Voxel;

typedef Voxel* Voxel_P;

/*
 * New_Voxel() returns a new voxel object without initializing it. Make_Voxel()
 * also returns a new voxel object, which will be initialized by the coordinate
 * (<x>, <y>, <z>).
 */
Voxel* New_Voxel();
Voxel* Make_Voxel(int x, int y, int z);

/*
 * Set_Voxel() sets a voxel by the coordinate (<x>, <y>, <z>).
 */
void Set_Voxel(Voxel *v, int x, int y, int z);

/*
 * Delete_Voxel() deletes a voxel object.
 */
void Delete_Voxel(Voxel *v);
void Kill_Voxel(Voxel *v);

/*
 * Print_Voxel() prints a voxel to the screen.
 */
void Print_Voxel(const Voxel *v);

/**@}*/

__END_DECLS

#endif
