/**@file tz_stack_tile_set.h
 * @author Ting Zhao
 * @date 16-Nov-2009
 */

#ifndef _TZ_STACK_TILE_SET_H_
#define _TZ_STACK_TILE_SET_H_

#include "tz_cdefs.h"
#include "tz_stack_tile_arraylist.h"
#include "tz_mc_stack.h"

__BEGIN_DECLS

typedef Stack_Tile_Arraylist Stack_Tile_Set;

#define New_Stack_Tile_Set New_Stack_Tile_Arraylist
#define Kill_Stack_Tile_Set Kill_Stack_Tile_Arraylist

Stack* Stack_Tile_Set_Merge(Stack_Tile_Set *ts);

__END_DECLS

#endif
