#include "median_swc.h"
#include  "neuron_sim_scores.h"
#include "cfloat"
#include  <iostream>
#include "point3d_util.h"


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




NeuronTree average_node_position( NeuronTree median_neuron,vector<NeuronTree> nt_list, double distance_threshold)
{
    NeuronTree median_adjusted = median_neuron;
    for (int i = 0; i <median_neuron.listNeuron.size(); i++)
    {
        NeuronSWC s = median_neuron.listNeuron.at(i);
        Point3D cur;
        cur.x = s.x;
        cur.y = s.y;
        cur.z = s.z;

        vector<Point3D> cluster;
        for (int ii = 0; ii < nt_list.size(); ii++)
        {
            double min_dis = DBL_MAX;
            Point3D closest_p;
            for (V3DLONG j = 0; j < nt_list[ii].listNeuron.size(); j++)
            {
                //find its nearest node from tree ii
                Point3D p;
                NeuronSWC ns = nt_list[ii].listNeuron.at(j);
                p.x = ns.x;
                p.y = ns.y;
                p.z = ns.z;
                double dis = PointDistance(p,cur);
                if (dis < min_dis)
                {
                    min_dis = dis;
                    closest_p = p;
                }
            }
            if (min_dis < distance_threshold)
            {//if min_dis > distance_threshold,too far away, won't count to average
                cluster.push_back(closest_p);
            }
        }
        //average over the clustered location p

        if (cluster.size() >0)
        {
            Point3D average_p =  average_point(cluster);
            NeuronSWC * s_adjusted = &(median_adjusted.listNeuron[i]);
            s_adjusted->x = average_p.x;
            s_adjusted->y = average_p.y;
            s_adjusted->z = average_p.z;
        }

    }

    return median_adjusted;
}
