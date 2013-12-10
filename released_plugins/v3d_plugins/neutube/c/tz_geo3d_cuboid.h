/**@file tz_geo3d_cuboid.h
 * @brief Cuboid
 * @author Ting Zhao
 * @date 26-Mar-2010
 */

#ifndef _TZ_GEO3D_CUBOID_H_
#define _TZ_GEO3D_CUBOID_H_

#include "tz_cdefs.h"
#include "tz_zobject.h"

__BEGIN_DECLS

typedef struct _Geo3d_Cuboid {
  double cb[3];
  double ce[3];
} Geo3d_Cuboid;

DECLARE_ZOBJECT_INTERFACE(Geo3d_Cuboid)

__END_DECLS

#endif
