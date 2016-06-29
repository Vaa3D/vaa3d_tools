#include <iostream>
#include <vector>
#include "mst_boost_prim.h"

using namespace std;
using namespace boost;

int getMST(unsigned short *matrix, V3DLONG num_nodes, V3DLONG *plist, V3DLONG rootnode) {

    //generate list of edges and weights from adjacency matrix

    int max_edges = (num_nodes * num_nodes)/2;

    cout << "2\n";

    E edges[max_edges];
    float weights[max_edges];
    int c = 0;

    cout << "extracting matrix\n";
    for (int i = 0; i < num_nodes; ++i){
        for (int j = 0; j < i; ++j) {
            if (matrix[i*num_nodes + j] > 0) {
                edges[c] = E(i, j);
                weights[c] = matrix[i*num_nodes + j]; // 1/weight to build a max span tree
                c++;
            }
        }
    }

    typedef adjacency_list < vecS, vecS, undirectedS,
        property<vertex_distance_t, int>, property < edge_weight_t, int > > Graph;

    Graph g(edges, edges + sizeof(edges) / sizeof(E), weights, num_nodes);
    property_map<Graph, edge_weight_t>::type weightmap = get(edge_weight, g);

    std::vector < graph_traits < Graph >::vertex_descriptor >
    p(num_vertices(g));

    cout << "3\n";

    prim_minimum_spanning_tree(g, &p[rootnode]);

    for (std::size_t i = 0; i != p.size(); ++i) {
        std::cout << "parent[" << i << "] = " << p[i] << std::endl;
        plist[i] = p[i];
    }
    return EXIT_SUCCESS;
}

bool boost_mst_prim(unsigned short *adjMatrix, V3DLONG num_nodes, V3DLONG *plist, V3DLONG rootnode)
{
  typedef property < edge_weight_t, double >Weight;
  typedef adjacency_list < vecS, vecS, undirectedS,
    no_property, Weight > UndirectedGraph;
  typedef adjacency_list < vecS, vecS, directedS,
    no_property, Weight > DirectedGraph;

  cout << "1\n";

  getMST(adjMatrix, num_nodes, plist, rootnode);

  return true;
}
