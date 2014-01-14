/* tz_voxeltrans.c
 *
 * Initial write: Ting Zhao
 */

#include "tz_voxeltrans.h"

void Voxel_To_Tvoxel(const Voxel *v, Voxel_t vt)
{
  vt[0] = v->x;
  vt[1] = v->y;
  vt[2] = v->z;
}

void Tvoxel_To_Voxel(const Voxel_t vt, Voxel *v)
{
  v->x = vt[0];
  v->y = vt[1];
  v->z = vt[2];
}
