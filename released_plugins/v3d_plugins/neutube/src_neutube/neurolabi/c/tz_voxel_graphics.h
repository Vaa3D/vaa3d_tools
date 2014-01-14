/**@file tz_voxel_graphics.h
 * @brief voxel graphics
 * @author Ting Zhao
 */

#ifndef _TZ_VOXEL_GRAPHICS_H_
#define _TZ_VOXEL_GRAPHICS_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_object_3d.h"
#include "tz_voxel_linked_list.h"

__BEGIN_DECLS

/**@addtogroup voxel_graphics_ Voxel graphics (tz_voxel_graphics.h)
 * @{
 */

/**@brief A stack to a voxel list.
 *
 * Stack_To_Voxel_List() builds a voxel list from a stack. It puts all
 * foreground voxels (intensity > 0) into the list. The caller is responsible
 * for freeing  the returned list. It returns NULL if no foreground voxel is 
 * found.
 */
Voxel_List* Stack_To_Voxel_List(const Stack *stack);

/**@brief A line to an object.
 *
 * Line_To_Object_3d() converts a line to an object. The line is specified by
 * its two end points <start> and <end>. The order of the voxels of the
 * returned object is not necessary arranged from <start> to <end>. The caller
 * is responsible for freeing the returned object.
 */
Object_3d* Line_To_Object_3d(const Voxel_t start, const Voxel_t end);

/**@}*/

__END_DECLS

#endif
