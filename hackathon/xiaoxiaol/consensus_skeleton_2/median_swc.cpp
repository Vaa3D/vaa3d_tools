#include "median_swc.h"
#include  "neuron_sim_scores.h"
#include "cfloat"
#include  <iostream>

int median_swc(vector<NeuronTree> nt_list){
    int idx = -1;
    //calculate pair-wise distance, pick the one has the min sum distance
    double min_dis_sum = DBL_MAX;
    for (int i = 0; i < nt_list.size()-1; i++){
        double dis_sum = 0;
        for (int j = i+1; j < nt_list.size(); j++)
        {
            cout <<"\nComputing neuron distance between " <<i<<"  and "<<j <<":"<<endl;
            NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&nt_list[i], &nt_list[j],false);
            dis_sum += tmp_score.dist_allnodes;
        }
        if (min_dis_sum > dis_sum)
        {
            min_dis_sum = dis_sum;
            idx = i;
        }

    }
    return idx;
}




NeuronTree average_node_position( NeuronTree median_neuron,vector<NeuronTree> nt_list)
{
     NeuronTree median_adjusted;

     return median_adjusted;
}
