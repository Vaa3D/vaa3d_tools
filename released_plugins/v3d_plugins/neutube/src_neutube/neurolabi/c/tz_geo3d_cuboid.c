#include <stdlib.h>
#include "tz_geo3d_cuboid.h"
#include "tz_utilities.h"

DEFINE_ZOBJECT_INTERFACE(Geo3d_Cuboid)

void Default_Geo3d_Cuboid(Geo3d_Cuboid *cuboid)
{
  cuboid->cb[0] = 0.0;
  cuboid->cb[1] = 0.0;
  cuboid->cb[2] = 0.0;
  cuboid->ce[0] = 0.0;
  cuboid->ce[1] = 0.0;
  cuboid->ce[2] = 0.0;
}

void Clean_Geo3d_Cuboid(Geo3d_Cuboid *cuboid)
{
  cuboid->cb[0] = 0.0;
  cuboid->cb[1] = 0.0;
  cuboid->cb[2] = 0.0;
  cuboid->ce[0] = 0.0;
  cuboid->ce[1] = 0.0;
  cuboid->ce[2] = 0.0;  
}

void Print_Geo3d_Cuboid(Geo3d_Cuboid *cuboid)
{
  printf("Cuboid: (%g, %g, %g) -> (%g, %g, %g)\n", 
	 cuboid->cb[0], cuboid->cb[1], cuboid->cb[2],
	 cuboid->ce[0], cuboid->ce[1], cuboid->ce[2]);
}
