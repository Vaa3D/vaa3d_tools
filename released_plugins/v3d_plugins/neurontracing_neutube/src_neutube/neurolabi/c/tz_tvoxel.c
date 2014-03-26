/* tz_tvoxel.c
 *
 * 04-Jan-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tz_tvoxel.h"

void Copy_Tvoxel(Voxel_t dst, const Voxel_t src)
{
  memcpy(dst,  src, sizeof(Voxel_t));
}

void Print_Tvoxel(const Voxel_t v)
{
  printf("%d, %d, %d\n", v[0], v[1], v[2]);
}

void Set_Tvoxel(Voxel_t v, int x, int y, int z)
{
  v[0] = x;
  v[1] = y;
  v[2] = z;
}
