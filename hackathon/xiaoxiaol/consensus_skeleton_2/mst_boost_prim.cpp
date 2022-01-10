#include <iostream>
#include <vector>
#include "mst_boost_prim.h"
#include "basic_surf_objs.h"

using namespace std;
using namespace boost;


int build_adjacency_list(vector<NeuronTree> nt_list, QList<NeuronSWC> merge_result, int max_weight, UndirectedGraph *g){
    //generate list of edges and weights from adjacency matrix
    int max_edges = merge_result.size()-1;

    /* to make a max spanning tree, + number of neurons so all positive b/c boost implementation uses Djikstra
     * which asserts all numbers > 0
     * not cumulative so doesn't impact anything
     */

    for (int i=0;i<nt_list.size();i++)
    {
        /*
        int voted[max_edges];
        for (int i=0; i< max_edges; i++){
            voted[i] = 0;
        }
        */

        for (V3DLONG j=0;j<nt_list[i].listNeuron.size();j++)
        {
            NeuronSWC cur = nt_list[i].listNeuron[j];
            if (cur.seg_id < 0 ){
                //didn't get clustered to the consensus node( too far away)
                continue;
            }
            V3DLONG n_id,pn_id;
            n_id = cur.seg_id;// mapped consensu node id

            if (cur.pn == -1 )
            {//root, no edge connection
                continue;}

            V3DLONG pidx = nt_list[i].hashNeuron.value(cur.pn);
            pn_id = nt_list[i].listNeuron[pidx].seg_id;

            if (pn_id < 0  || pn_id == n_id){continue;} //skip

            EdgeQuery edgeq = edge(n_id, pn_id, *g);
            if (edgeq.second) { //edge exists in graph
                if (get(edge_lastvoted_t(), *g, edgeq.first) != i) { //if == i, then tree has already voted for this edge
                    int weight = get(edge_weight_t(), *g, edgeq.first);
                    put(edge_weight_t(), *g, edgeq.first, weight-1); //min tree so decrease weight
                    put(edge_lastvoted_t(), *g, edgeq.first, i); //updating that this tree voted for this edge
                }
            } else add_edge(n_id, pn_id, LastVoted(i, Weight(max_weight - 1)), *g); //number of neuron trees minus weight to convert to a min tree



            }


        }

    return 0;
}


int getprimMST(UndirectedGraph *g, V3DLONG *plist, V3DLONG rootnode) {

    //definition
    property_map<UndirectedGraph, edge_weight_t>::type weightmap = get(edge_weight, *g);
    vector < graph_traits < UndirectedGraph >::vertex_descriptor > p(num_vertices(*g));

    prim_minimum_spanning_tree(*g, &p[rootnode]);

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



bool boost_mst_prim(vector<NeuronTree>  nt_list, QList<NeuronSWC> &merge_result, int vote_threshold)
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

    UndirectedGraph g(num_nodes);
    int max_weight = nt_list.size();
    build_adjacency_list(nt_list, merge_result, max_weight, &g);


    getprimMST(&g, plist, rootnode);

    // code the edge votes into type for visualization
    QList<NeuronSWC> node_list = merge_result;
    merge_result.clear();

    for (V3DLONG i = 0;i <num_nodes;i++)
    {
        V3DLONG p = plist[i];

        if (p == -1){
            //root
            NeuronSWC tmp;
            tmp.x = node_list[i].x;
            tmp.y = node_list[i].y;
            tmp.z = node_list[i].z;

            tmp.type = 1;
            tmp.pn = -1;
            tmp.r =  node_list[i].r;
            tmp.n = i+1;
            tmp.fea_val.push_back(1.0);//a default edge vote for root
            merge_result.append(tmp);

            continue;
        }

        EdgeQuery edgeq = edge(i, p, g);
        int edge_weight = get(edge_weight_t(), g, edgeq.first);

        unsigned int edgeVote = max_weight - edge_weight; //getting the difference gives us back the number of votes


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


    }
    return true;
}
