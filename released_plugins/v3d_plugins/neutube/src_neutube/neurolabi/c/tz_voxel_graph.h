/**@file tz_voxel_graph.h
 * @brief voxel graph
 * @author Ting Zhao
 * @date 26-May-2008
 */

#ifndef _TZ_VOXEL_GRAPH_H_
#define _TZ_VOXEL_GRAPH_H_

#include "tz_cdefs.h"
#include "tz_tvoxel.h"
#include "tz_graph_defs.h"

__BEGIN_DECLS

typedef _Voxel_Graph {
  int nvoxel;
  int nedge;
  int edge_capacity;
  Graph_Edge_t *edges;
  Voxel_t *voxels;
} Voxel_Graph;

__END_DECLS

#endif
