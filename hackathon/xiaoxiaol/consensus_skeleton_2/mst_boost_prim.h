#ifndef MAIN_H
#define MAIN_H

#include "boost/graph/prim_minimum_spanning_tree.hpp"
#include "boost/config.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "basic_surf_objs.h"
#include "point3d_util.h"
#include <vector>

using namespace boost;

typedef std::pair<int, int> E;

enum edge_lastvoted_t { edge_lastvoted };

namespace boost {
    BOOST_INSTALL_PROPERTY(edge, lastvoted);
}

typedef property < edge_weight_t, double >  Weight;
typedef property < edge_lastvoted_t, int, Weight> LastVoted;

typedef adjacency_list < vecS, vecS, undirectedS, no_property, LastVoted > UndirectedGraph;
typedef UndirectedGraph::edge_descriptor Edge;
typedef std::pair<Edge, bool> EdgeQuery;

int build_adjacency_list(std::vector<NeuronTree> nt_list, QList<NeuronSWC> merge_result, int max_weight, UndirectedGraph *g );
int getprimMST(UndirectedGraph *g, V3DLONG *plist, V3DLONG rootnode);
bool boost_mst_prim(std::vector<NeuronTree>  nt_list, QList<NeuronSWC> &merge_result, int vote_threshold);

#endif // MAIN_H
