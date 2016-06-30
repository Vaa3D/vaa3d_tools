#ifndef MAIN_H
#define MAIN_H

#include "boost/graph/prim_minimum_spanning_tree.hpp"
#include "boost/config.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/kruskal_min_spanning_tree.hpp"
#include "basic_surf_objs.h"

using namespace boost;

typedef std::pair<int, int> E;
typedef property < edge_weight_t, double >  Weight;
typedef adjacency_list < vecS, vecS, undirectedS, no_property, Weight > UndirectedGraph;

int getMST(unsigned short *matrix, int n_edges, int n_neurons, V3DLONG n_nodes, V3DLONG *plist, V3DLONG rootnode);
bool boost_mst_prim(unsigned short *adjMatrix, int n_edges, V3DLONG n_nodes, V3DLONG *plist, V3DLONG rootnode);


#endif // MAIN_H
