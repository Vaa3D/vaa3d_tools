#include <iostream>
#include <vector>
#include "mst_boost_prim.h"

using namespace std;
using namespace boost;

//unused kruskal method
int getkruskalMST(unsigned short *matrix, int max_edges, V3DLONG num_nodes, V3DLONG *plist, V3DLONG rootnode) {

    typedef graph_traits < UndirectedGraph >::edge_descriptor Edge;
    typedef graph_traits < UndirectedGraph >::vertex_descriptor Vertex;
    typedef std::pair<int, int> E;

    //generate list of edges and weights from adjacency matrix

    //int max_edges = (num_nodes * num_nodes)/2;

    E edges[max_edges];
    float weights[max_edges];
    int c = 0;

    cout << "extracting matrix\n";
    for (int i = 0; i < num_nodes; ++i){
        for (int j = 0; j < i; ++j) { //to avoid repeat edges
            if (matrix[i*num_nodes + j] > 0) { //if edge present
                edges[c] = E(i, j);
                weights[c] = -matrix[i*num_nodes + j]; // neg weight to build a max span tree
                c++;
            }
        }
    }

    UndirectedGraph g(edges, edges + sizeof(edges) / sizeof(E), weights, num_nodes);
    property_map<UndirectedGraph, edge_weight_t>::type weightmap = get(edge_weight, g);

    property_map < UndirectedGraph, edge_weight_t >::type weight = get(edge_weight, g);
    std::vector < Edge > spanning_tree;

    kruskal_minimum_spanning_tree(g, back_inserter(spanning_tree));

    //initialize plist

    for (int i = 0; i < num_nodes; ++i) {
        plist[i] = -3;
    }

    cout << "Print the edges in the MST:" << endl;
      for (vector < Edge >::iterator ei = spanning_tree.begin(); ei != spanning_tree.end(); ++ei) {
          cout << ": " << source(*ei, g) << " <--> " << target(*ei, g)
               << " with weight of " << weight[*ei]
               << std::endl;
          if (plist[source(*ei, g)] == -3)
              plist[source(*ei, g)] = target(*ei, g);
          else if (plist[target(*ei, g)] == -3)
              plist[target(*ei, g)] = source(*ei, g);
          else cout << "error in understanding";
    }
    return EXIT_SUCCESS;
}

int getprimMST(unsigned short *matrix, int max_edges, V3DLONG num_nodes, V3DLONG *plist, V3DLONG rootnode) {

    //generate list of edges and weights from adjacency matrix

    printf("2\n");

    //int max_edges = (num_nodes * num_nodes)/2;

    printf("max edges is %d\n", max_edges);

    E edges[max_edges];
     printf("2.5\n");
    float weights[max_edges];
    int c = 0;

    printf("3\n");

    //extract information from matrix into boost graph adjacency list input format
    for (int i = 0; i < num_nodes; ++i){
        for (int j = 0; j < i; ++j) { //to avoid repeat edges
            if (matrix[i*num_nodes + j] > 0) { //if edge present
                edges[c] = E(i, j);
                /* to make a max spanning tree, + 100 so all positive b/c boost implementation uses Djikstra
                 * which asserts all numbers > 0
                 * not cumulative so doesn't impact anything
                 */
                weights[c] = -matrix[i*num_nodes + j] + 100;
                c++;
            }
        }
    }

    printf("4\n");

    //definitions
    UndirectedGraph g(edges, edges + sizeof(edges) / sizeof(E), weights, num_nodes);
    property_map<UndirectedGraph, edge_weight_t>::type weightmap = get(edge_weight, g);
    std::vector < graph_traits < UndirectedGraph >::vertex_descriptor > p(num_vertices(g));

    prim_minimum_spanning_tree(g, &p[rootnode]);

    printf("5\n");

    for (std::size_t i = 0; i != p.size(); ++i) {
        //boost denotes a root node by it's parent being itself where v3d denotes it by parent = 1
        if (p[i] == i) {
            plist[i] = -1;
        } else
            plist[i] = p[i];
        cout << "parent of " << i << " = " << plist[i] << endl;
    }

    return EXIT_SUCCESS;
}

bool boost_mst_prim(unsigned short *adjMatrix, int n_edges, V3DLONG num_nodes, V3DLONG *plist, V3DLONG rootnode)
{

  printf("1\n");
  getprimMST(adjMatrix, n_edges, num_nodes, plist, rootnode);

  return true;
}
