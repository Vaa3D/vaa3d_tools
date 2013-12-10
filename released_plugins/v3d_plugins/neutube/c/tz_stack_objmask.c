/* tz_stack_objmask.c
 *
 * 10-Mar-2008 Initial write: Ting Zhao
 */

#include "tz_error.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_utils.h"
#include "tz_stack_objmask.h"

#define STACK_FOREGROUND_AREA_O(stack_array, obj)			\
  {									\
    int i;								\
    for (i = 0; i < obj->size; i++) {					\
      int offset = Stack_Util_Offset(obj->voxels[i][0], obj->voxels[i][1], \
				     obj->voxels[i][2],			\
				     stack->width, stack->height,	\
				     stack->depth);			\
      if (offset >= 0) {						\
	area += (stack_array[offset] != 0);				\
      }									\
    }									\
  }

int Stack_Foreground_Area_O(const Stack *stack, const Object_3d *obj)
{
  int area = 0;
  
  if ((stack != NULL) && (obj != NULL)) {
    DEFINE_SCALAR_ARRAY_ALL(array, stack);
    
    switch (stack->kind) {
    case GREY:
      STACK_FOREGROUND_AREA_O(array_grey, obj);
      break;
    case GREY16:
      STACK_FOREGROUND_AREA_O(array_grey16, obj);
      break;
    case FLOAT32:
      STACK_FOREGROUND_AREA_O(array_float32, obj);
      break;
    case FLOAT64:
      STACK_FOREGROUND_AREA_O(array_float64, obj);
      break;
    default:
      TZ_ERROR(ERROR_DATA_TYPE);
      break;
    }
  }
	   
  return area;
}
