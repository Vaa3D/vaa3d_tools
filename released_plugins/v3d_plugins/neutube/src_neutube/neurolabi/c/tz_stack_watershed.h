/**@file tz_stack_watershed.h
 * @brief stack watershed (obsolete)
 * @author Ting Zhao
 * @date 31-May-2008
 */

#ifndef _TZ_STACK_WATERSHED_H_
#define _TZ_STACK_WATERSHED_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_zobject.h"

__BEGIN_DECLS

#include "water_shed.h"

#if 0
#define WSHED    0
#define ONQUEUE -1
#define MASK    -2
#define INIT    -3
#define MINIM   -4
#define IGNORED  -5

typedef struct _Watershed_3d {
  int nlabels;
  Stack *labels;
  int nbasins;
  int *seeds;
  int conn;
} Watershed_3d;

typedef struct _Watershed_3d_Workspace {
  int *array;
  Stack *mask;
  int max_area;
  int conn;
} Watershed_3d_Workspace;

Watershed_3d* New_Watershed_3d();
void Free_Watershed_3d(Watershed_3d *watershed);

void Kill_Watershed_3d(Watershed_3d *watershed);

Watershed_3d_Workspace * New_Watershed_3d_Workspace();
void Free_Watershed_3d_Workspace(Watershed_3d_Workspace *ws);

void Kill_Watershed_3d_Workspace(Watershed_3d_Workspace *ws);


Watershed_3d *Build_3D_Watershed(const Stack *stack, Watershed_3d *result, 
				 Watershed_3d_Workspace *ws);
#endif

typedef struct _Stack_Watershed_Workspace {
  int *array;
  Stack *mask; /* 1~244 seed; 255 barrier */
  int conn;
  int min_level;
  int start_level;
  double *weights;
} Stack_Watershed_Workspace;

enum { STACK_WATERSHED_MAX_SEED = 244, STACK_WATERSHED_BARRIER = 255};

DECLARE_ZOBJECT_INTERFACE(Stack_Watershed_Workspace)

Stack_Watershed_Workspace* Make_Stack_Watershed_Workspace(Stack *stack);

void Stack_Watershed_Infer_Parameter(const Stack *stack, 
				     Stack_Watershed_Workspace *ws);

/**@brief Create barrier for gaps along Z axis
 *
 * Stack_Watershed_Zgap_Barrier() sets the value of any z-gap voxel in <mask> to
 * the barrier value. A voxel in <mask> is a z-gap voxel if it satisfies two 
 * conditions:
 *   1. It has value 0;
 *   2. Its position is in a local minimal intersity interval along z in 
 *      <stack>.
 */
void Stack_Watershed_Zgap_Barrier(const Stack *stack, Stack *mask);

/**@brief 3D seeded watershed.
 *
 * Current version only supports 8 bit or 16 bit stack. 
*/
Stack* Stack_Watershed(const Stack *stack, Stack_Watershed_Workspace *ws);

/**@brief Watershed area shrink
 */
Stack* Stack_Region_Border_Shrink(const Stack *stack,
				  Stack_Watershed_Workspace *ws);

__END_DECLS

#endif
