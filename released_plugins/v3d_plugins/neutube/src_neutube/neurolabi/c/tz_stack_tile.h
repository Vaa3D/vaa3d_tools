/**@file tz_stack_tile.h
 * @author Ting Zhao
 * @date 16-Nov-2009
 */

#ifndef _TZ_STACK_TILE_H_
#define _TZ_STACK_TILE_H_

#include "tz_cdefs.h"
#include <image_lib.h>
#include "tz_zobject.h"

__BEGIN_DECLS

typedef struct _Stack_Tile {
  Stack *stack;
  double offset[3];
} Stack_Tile;

DECLARE_ZOBJECT_INTERFACE(Stack_Tile)

Stack_Tile* Make_Stack_Tile(Stack *stack, const double *offset);
Stack_Tile* Make_Stack_Tile_D(Stack *stack, double x, double y, double z);

Stack_Tile* Read_Stack_Tile(const char *filePath);

__END_DECLS

#endif
