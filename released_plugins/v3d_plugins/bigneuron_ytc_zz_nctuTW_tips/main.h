#ifndef GRAPH_ADJ_3D_H
#include "Graph_AdjacencyList3D.h"
#endif

typedef struct  _EndPoint3D
{
	_EndPoint3D():pathLength(.0),visited(false){} 
	MyGridGraph_3D::_GridNode_3D node;
	float pathLength;
	bool visited;
}SkeleEndNode3D;

