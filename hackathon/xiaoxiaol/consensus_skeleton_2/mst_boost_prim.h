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

int getMST(uUndirectedGraph g, int max_edges, int num_neurons, V3DLONG num_nodes, V3DLONG *plist, V3DLONG rootnode);
int build_adjacency_list(vector<NeuronTree>  nt_list, QList<NeuronSWC> merge_result,UndirectedGraph &g );
bool boost_mst_prim(vector<NeuronTree>  nt_list, QList<NeuronSWC> &merge_result, int TYPE_MERGED);


#endif // MAIN_H
