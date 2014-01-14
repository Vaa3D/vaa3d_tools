/* tz_object_tmpl.c
 *
 * 05-Nov-2007 Initial write: Ting Zhao
 */

#include <utilities.h>
#include "tz_objdef.h"
#include "tz_object_tmpl.h"

static inline int object_t_asize(const Object_T *object_t)
{ return object_t->size * sizeof(int); }

void Default_Object_T(Object_T *object_t)
{
  object_t->size = 0;
  object_t->array = NULL;
}

DEFINE_OBJECT_MANAGEMENT(Object_T, array, asize, object_t)

Object_T *Make_Object_T(int size)
{ 
  Object_T *object_t = new_object_t(sizeof(int) * size, "Make_Object_T");
  object_t->size = size;

  return object_t;
}
