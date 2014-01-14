/**@file tz_doubly_linked_list_defs.h
 * @brief definitions for doubly linked lists
 * @author Ting Zhao
 * @date 27-Nov-2007 
 */

#ifndef _TZ_DOUBLY_LINKED_LIST_DEFS_H_
#define _TZ_DOUBLY_LINKED_LIST_DEFS_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#define DL_UNDEFINED 10 /* undefined value. All other flag values should not
			   be this value.*/

/*
 * doubly list directions. The following facts should be always held for
 * future changes:
 * DL_FORWARD + DL_BACKWARD = DL_BOTHDIR
 * DL_FORWARD = -DL_BACKWARD
 * (the solution is unique though)
 * DL_UNKNOWN does not equal to any other value
 */
typedef enum {
  DL_FORWARD = 1, DL_BACKWARD = -1, DL_BOTHDIR = 0, DL_UNKNOWN = 10
} Dlist_Direction_e; 

/*
 * doubly list ends. The following facts should be always held for
 * future changes:
 * DL_HEAD != DL_TAIL
 * DL_HEAD = -DL_TAIL
 */
typedef enum {DL_HEAD = -1, DL_TAIL = 1, DL_UNKNOWN_END} Dlist_End_e;

__END_DECLS

#endif 
