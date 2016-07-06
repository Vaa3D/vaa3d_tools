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


int build_adjacency_list(vector<NeuronTree> nt_list, QList<NeuronSWC> merge_result, UndirectedGraph &g ){

    //MERGE Code from Build adj matrix






    //generate list of edges and weights from adjacency matrix
    E edges[max_edges];
    float weights[max_edges];
    int c = 0;

    //extract information from matrix into boost graph adjacency list input format
    for (int i = 0; i < num_nodes; ++i){
        for (int j = 0; j < i; ++j) { //to avoid repeat edges
            if (matrix[i*num_nodes + j] > 0) { //if edge present
                edges[c] = E(i, j);
                /* to make a max spanning tree, + number of neurons so all positive b/c boost implementation uses Djikstra
                 * which asserts all numbers > 0
                 * not cumulative so doesn't impact anything
                 */
                weights[c] = num_neurons - matrix[i*num_nodes + j];
                c++;
            }
        }
    }

    //definitions
    g = new UndirectedGraph (edges, edges + sizeof(edges) / sizeof(E), weights, num_nodes);
    property_map<UndirectedGraph, edge_weight_t>::type weightmap = get(edge_weight, g);
    std::vector < graph_traits < UndirectedGraph >::vertex_descriptor > p(num_vertices(g));

    return 0;
}


int getprimMST( UndirectedGraph g, int max_edges, int num_neurons, V3DLONG num_nodes, V3DLONG *plist, V3DLONG rootnode) {

    prim_minimum_spanning_tree(g, &p[rootnode]);

    for (std::size_t i = 0; i != p.size(); ++i) {
        //boost denotes a root node by it's parent being itself where v3d denotes it by parent = 1
        if (p[i] == i) {
            plist[i] = -1;
        } else
            plist[i] = p[i];
        //cout << "parent of " << i << " = " << plist[i] << endl;
    }

    return EXIT_SUCCESS;
}



bool boost_mst_prim(vector<NeuronTree>  nt_list, QList<NeuronSWC> &merge_result, int TYPE_MERGED)
{
    long rootnode =0;

    cout <<"\nComputing max-spanning tree" <<endl;
    V3DLONG * plist;
    V3DLONG num_nodes = merge_result.size();
    try {
        plist = new V3DLONG[num_nodes];
    } catch (...) {
        cout << "out of memory" << endl;
        return false;
    }

    UndirectedGraph g;
    build_adjacency_list(nt_list,merge_result, TYPE_MERGED, g);


    getprimMST(g, n_edges, nt_list.size(), num_nodes, plist, rootnode);


    // code the edge votes into type for visualization
    //    graph: duplicate swc nodes are allowed to accomandate mutiple parents for the child node, no root id,
    QList<NeuronSWC> node_list = merge_result;
    merge_result.clear();
    for (V3DLONG i = 0;i <num_nodes;i++)
    {
        V3DLONG p = plist[i];

        if (p == -1){
            //cout << i << " is a root node" << endl;
            //root
            NeuronSWC tmp;
            tmp.x = node_list[i].x;
            tmp.y = node_list[i].y;
            tmp.z = node_list[i].z;

            tmp.type = 1;
            tmp.pn = -1;
            tmp.r =  node_list[i].r;
            tmp.n = i+1;
            tmp.fea_val.push_back(0);

            merge_result.append(tmp);

            continue;
        }

        unsigned int edgeVote = adjMatrix[i*num_nodes + p];

        if (edgeVote >= vote_threshold ){
            NeuronSWC tmp;
            tmp.x = node_list[i].x;
            tmp.y = node_list[i].y;
            tmp.z = node_list[i].z;

            tmp.type = 3;
            tmp.pn = p + 1;  //parent id, form the edge
            tmp.r =  node_list[i].r;
            tmp.n = i+1;
            tmp.fea_val.push_back(edgeVote);

            merge_result.append(tmp);
        } else {
            printf("edge connecting %d and %d with vote %d is discarded.\n",i, p, edgeVote);
        }
    }
    return true;




}
