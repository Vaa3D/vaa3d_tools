/*
 1 function Dijkstra(Graph, source):
 2     for each vertex v in Graph:           // Initializations
 3         dist[v] := infinity               // Unknown distance function from s to v
 4         previous[v] := undefined
 5     dist[source] := 0                     // Distance from s to s
 6     Q := copy(Graph)                      // Set of all unvisited vertices
 7     while Q is not empty:                 // The main loop
 8         u := extract_min(Q)               // Remove best vertex from priority queue; returns source on first iteration
 9         for each neighbor v of u:
10             alt = dist[u] + length(u, v)
11             if alt < dist[v]              // Relax (u,v)
12                 dist[v] := alt
13                 previous[v] := u
14
15	
16	P.C. Lee cis dept. NCTU
*/
//#include "Graph_AdjacencyList.h";
#ifndef GRAPH_ADJ_3D_H
#include "Graph_AdjacencyList3D.h"
#endif

#ifndef _IPT_H
#include "IPT.h"
#endif

//using namespace MyGridGraph_2;
#ifndef DIJKSTRA_H
#define DIJKSTRA_H
const float COST_MUTIPLIER = 1.0;
const float SKELETON_AWARD = -0.5;

using namespace MyGridGraph_3D;


void Single_Dijkstra_GridGraph_3D(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path); //std::vector< GridNode_3D >& All_Path); 
void Single_Dijkstra_GridGraph_3D(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path, bool*** skeleMap, unsigned char *inImg, 
								  const int width, const int height, const int zSize);
void Single_Dijkstra_GridGraph_3D(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path, bool*** skeleMap, unsigned char* CostDT, 
								  unsigned char* inImg, const int width, const int height, const int zSize );

void Single_Dijkstra_GridGraph_3D_Ver2(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path, bool*** skeleMap, unsigned char* CostDT, 
								  unsigned char* inImg, const int width, const int height, const int zSize );

double GvfCostFunction3D( GridNode_3D SrcNode, GridNode_3D node, float scaler = COST_MUTIPLIER );
double GvfWithMedialCost3D( GridNode_3D SrcNode, GridNode_3D node, bool*** skeleMap, float scaler = COST_MUTIPLIER, float skeleton_const = SKELETON_AWARD );
double DTWithMedialCost3D( GridNode_3D SrcNode, GridNode_3D node, bool*** skeleMap, unsigned char* CostDT, 
						  const int width, const int height, const int zSize, float scaler = COST_MUTIPLIER, float skeleton_const = SKELETON_AWARD );

double DTWithMedialCost3D_Ver2( GridNode_3D SrcNode, GridNode_3D node, bool*** skeleMap, unsigned char* CostDT, 
						  const int width, const int height, const int zSize );
#endif
