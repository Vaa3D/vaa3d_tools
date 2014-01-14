/* tz_voxel.h
 *
 * Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <utilities.h>
#include "tz_voxel.h"

Voxel* New_Voxel()
{
  Voxel *v = (Voxel *) Guarded_Malloc(sizeof(Voxel), "New_Voxel");
  return v;
}

Voxel* Make_Voxel(int x, int y, int z)
{
  Voxel *v = New_Voxel();
  Set_Voxel(v, x, y, z);

  return v;
}

void Set_Voxel(Voxel *v, int x, int y, int z)
{
  v->x = x;
  v->y = y;
  v->z = z;
}

void Delete_Voxel(Voxel *v)
{
  free(v);
}

void Kill_Voxel(Voxel *v)
{
  Delete_Voxel(v);
}

void Print_Voxel(const Voxel *v)
{
  printf("Voxel: (%d, %d, %d)\n", v->x, v->y, v->z);
}
