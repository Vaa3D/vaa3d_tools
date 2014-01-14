/**@file tz_object_3d_operation.h
 * @brief 3d object operation
 * @author Ting Zhao
 * @date 16-Dec-2006
 */

#ifndef _TZ_OBJECT_3D_OPERATION_H_
#define _TZ_OBJECT_3D_OPERATION_H_

#include "tz_cdefs.h"
#include "tz_object_3d.h"
#include "tz_object_3d_linked_list.h"

__BEGIN_DECLS

/*
 * Object_Intersect() returns the intersection of the two objects <obj1> and
 * <obj2>. It returns NULL if the intersection is empty; otherwise the return
 * is a new object.
 */
Object_3d* Object_Intersect(const Object_3d *obj1, const Object_3d *obj2);

/*
 * Object_Union() returns the union of the two objects <obj1> and <obj2> and
 * the return is a new object.
 */
Object_3d* Object_Union(const Object_3d *obj1, const Object_3d *obj2);

/*
 * Object_Sub() subtracts obj2 by obj1 and returns the resulted object list.
 */
Object_3d_List* Object_Sub(const Object_3d *obj1, const Object_3d *obj2);

Object_3d* Object_3d_Boundary(const Object_3d *obj1, int conn);

Object_3d* Object_3d_Border(const Object_3d *obj1, const Object_3d *obj2);

__END_DECLS

#endif
