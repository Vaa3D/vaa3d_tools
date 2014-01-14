/* tz_voxel_linked_list_ext.c
 *
 * 25-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "tz_voxeltrans.h"
#include "tz_voxel_linked_list_ext.h"

Object_3d* Voxel_List_To_Object_3d(Voxel_List *voxels, Object_3d *obj)
{
  if (voxels == NULL) {
    return NULL;
  }


  int nvoxel = Voxel_List_Length(voxels);

  if (obj != NULL) {
    if (obj->voxels == NULL) {
      obj->size = nvoxel;
      obj->conn = 0;
      Set_Object_3d_Asize(obj, Object_3d_Asize(obj->size));
    } else if (obj->size != nvoxel) {
      return NULL;
    }
  } else {
    obj = Make_Object_3d(nvoxel, 0);
  }

  int i = 0;
  while (voxels != NULL) {
    Voxel_To_Tvoxel(voxels->data, obj->voxels[i]);
    i++;
    voxels = voxels->next;
  }

  return obj;
}

