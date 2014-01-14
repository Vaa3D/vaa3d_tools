/**@file tz_sp_grow.h
 * @author Ting Zhao
 * @date 24-Dec-2009
 */

#ifndef _TZ_SP_GROW_H_
#define _TZ_SP_GROW_H_

#include "tz_cdefs.h"
#include "tz_stdint.h"
#include "tz_stack_graph.h"
#include "tz_zobject.h"

__BEGIN_DECLS

#define SP_GROW_WORKSPACE_ARGC 10

typedef struct _Sp_Grow_Workspace {
  size_t size; /* number of voxels */
  double *dist; /* buffer for distances */
  double *length; /* buffer of path length */
  int *path; /* buffer for paths */
  int *checked; /* buffer for growing status */
  uint8_t *flag; /* buffer for voxel properties */
  uint8_t *mask; /* 1: target; 2: source; 3: barrier; 4: super conductor */
  int conn; /* neighborhood defintion */
  Weight_Func_t *wf; /* weight function */
  double argv[STACK_GRAPH_WORKSPACE_ARGC]; /* arguments for wf */
  double resolution[3]; /* resolution */
  double value; /* distance value */
  double fgratio; /* background ratio */
  int sp_option; /* shortest path option (1 for maxmin) */
  int width;
  int height;
  int depth;
} Sp_Grow_Workspace;

enum {SP_GROW_TARGET = 1, SP_GROW_SOURCE, SP_GROW_BARRIER, SP_GROW_CONDUCTOR};

DECLARE_ZOBJECT_INTERFACE(Sp_Grow_Workspace)

/**@brief Set the mask of sp grow workspace.
 *
 * Sp_Grow_Workspace_Set_Mask() 
 */
void Sp_Grow_Workspace_Set_Mask(Sp_Grow_Workspace *sgw, uint8_t *mask);

Int_Arraylist* Stack_Sp_Grow(const Stack *stack, const size_t *seeds, 
			     int nseed, const size_t *target, int ntarget, 
			     Sp_Grow_Workspace *sgw);

void Stack_Sp_Grow_Draw_Result(Stack *canvas, const Int_Arraylist *path,
			       const Sp_Grow_Workspace *sgw);

void Stack_Sp_Grow_Infer_Parameter(Sp_Grow_Workspace *sgw, const Stack *stack);

/**@brief Find the furthest point to the source points.
 *
 * Stack_Sp_Grow_Furthest_Point() returns the index of the point that has a path
 * with largest number of voxels, which is returned by <max_count>.
 */
ssize_t Stack_Sp_Grow_Furthest_Point(Sp_Grow_Workspace *sgw, int *max_count);

__END_DECLS

#endif
