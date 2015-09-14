#include "median_swc.h"
#include  "neuron_sim_scores.h"
#include "cfloat"
#include  <iostream>

int median_swc(vector<NeuronTree> nt_list){
    int idx1 = -1;
    int idx2 = -1;
    int idx3 = -1;
    //calculate pair-wise distance, pick the one has the min sum distance
    double dis_sum_1[nt_list.size()];
    double dis_sum_2[nt_list.size()];
    double dis_sum_3[nt_list.size()];
    for (int i = 0; i < nt_list.size(); i++){
        dis_sum_1[i] =0 ;
        dis_sum_2[i] =0 ;
        dis_sum_3[i] =0 ;
    }

    for (int i = 0; i < nt_list.size()-1; i++){
        for (int j = i+1; j < nt_list.size(); j++)
        {

            NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&nt_list[i], &nt_list[j],false);
            cout <<"\n\nComputing neuron distance between " <<i<<"  and "<<j <<":"<<endl;
            cout<<"entire-structure-average = "<<tmp_score.dist_allnodes <<endl;
            cout<<"differen-structure-average = "<<tmp_score.dist_apartnodes<<endl;
            cout<<"percent of different-structure = "<<tmp_score.percent_apartnodes<<endl<<endl;

            dis_sum_1[i] += tmp_score.dist_allnodes;
            dis_sum_1[j] += tmp_score.dist_allnodes;

            dis_sum_2[i] += tmp_score.dist_apartnodes;
            dis_sum_2[j] += tmp_score.dist_apartnodes;

            dis_sum_3[i] += tmp_score.dist_max;
            dis_sum_3[j] += tmp_score.dist_max;
        }
    }

    double min_dis_sum_1 = DBL_MAX;
    double min_dis_sum_2 = DBL_MAX;
    double min_dis_sum_3 = DBL_MAX;
    for (int i = 0; i < nt_list.size(); i++){
       if (dis_sum_1[i] < min_dis_sum_1)
       {
               idx1 = i;
               min_dis_sum_1 = dis_sum_1[i];
       }
       if (dis_sum_2[i] < min_dis_sum_2)
       {
               idx2 = i;
               min_dis_sum_2 = dis_sum_2[i];
       }
       if (dis_sum_3[i] < min_dis_sum_3)
       {
               idx3 = i;
               min_dis_sum_3 = dis_sum_3[i];
       }

    }
    cout<<"Min total entire-structure-average = "<< min_dis_sum_1<< ", median swc id:" <<idx1 << endl;
    cout<<"Min total differen-structure-average = "<<min_dis_sum_2<< ", median swc id:" <<idx2 << endl;
    cout<<"Min total  percent of different-structure ="<< min_dis_sum_3 << ", median swc id:" << idx3 << endl;

    return idx1;
}




NeuronTree average_node_position( NeuronTree median_neuron,vector<NeuronTree> nt_list)
{
     NeuronTree median_adjusted;
     //for (int i = 0; i <median_neurn)



     return median_adjusted;
}
