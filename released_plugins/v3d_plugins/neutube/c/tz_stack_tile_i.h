/**@file tz_stack_tile_i.h
 * @author Ting Zhao
 * @date 16-Nov-2009
 */

#ifndef _TZ_STACK_TILE_I_H_
#define _TZ_STACK_TILE_I_H_

#include "tz_cdefs.h"
#include <image_lib.h>
#include "tz_zobject.h"
#include "tz_cuboid_i.h"

__BEGIN_DECLS

typedef struct _Stack_Tile_I {
  Stack *stack;
  int offset[3];
} Stack_Tile_I;

DECLARE_ZOBJECT_INTERFACE(Stack_Tile_I)

Stack_Tile_I* Make_Stack_Tile_I(Stack *stack, int x, int y, int z);

Stack_Tile_I* Read_Stack_Tile_I(const char *filePath);

int Stack_Tile_I_Width(const Stack_Tile_I *tile);
int Stack_Tile_I_Height(const Stack_Tile_I *tile);
int Stack_Tile_I_Depth(const Stack_Tile_I *tile);

Cuboid_I* Stack_Tile_I_Boundbox(const Stack_Tile_I *tile, Cuboid_I *box);

__END_DECLS

#endif
