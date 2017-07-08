#include "median_swc.h"
#include  "neuron_matching_distance.h"
#include "cfloat"
#include  <iostream>
#include "point3d_util.h"
#include "resample_swc.h"
#include <algorithm>
#include <vector>





void quickSort(vector<double> &x, vector<int> &order, int left, int right) {
      int i = left, j = right;
      double tmp;
      double pivot = x[(left + right) / 2];

      /* partition */
      while (i <= j) {
            while (x[i] < pivot)
                  i++;
            while (x[j] > pivot)
                  j--;
            if (i <= j) {
                  tmp = x[i];
                  x[i] = x[j];
                  x[j] = tmp;

                  int tmp_order =order[i];
                  order[i]= order[j];
                  order[j] = tmp_order;

                  i++;
                  j--;
            }
      };

      /* recursion */
      if (left < j)
            quickSort(x, order,left, j);
      if (i < right)
            quickSort(x,order, i, right);
}



vector<int> argsort(vector<double> x)
{
    vector<int> order(x.size(), 0);
    for( int i =0 ; i<x.size(); i++){
        order[i] = i;
    }

    quickSort(x, order, 0, x.size()-1);
    return order;
}


int median_swc(vector<NeuronTree> nt_list, QString outputFileName){


    //resample input swcs, so that the matching total distances are not biased
    // by the number of nodes
    vector<NeuronTree> nt_list_resampled;
    for (int i = 0; i < nt_list.size(); i++){
        NeuronTree nt = nt_list[i];
        if (nt.listNeuron.size()>0){
            //resample with step size 1
            NeuronTree resampled = resample(nt, 1.0);
             if (resampled.listNeuron.size()>0){
                resampled.file = nt.file;
                nt_list_resampled.push_back(resampled);
             }
        }
    }



    int idx1 = -1;
//    int idx2 = -1;
//    int idx3 = -1;

    //calculate pair-wise distance, pick the one has the min sum distance
    vector<double> dis_sum_1(nt_list_resampled.size(),0.0);
    vector<double> dis_sum_2(nt_list_resampled.size(),0.0);
    vector<double> dis_sum_3(nt_list_resampled.size(),0.0);


    //report metrics into a csv file
    QFile file(outputFileName);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<outputFileName.toStdString().c_str() << endl;
        return false;
    }

    QTextStream stream (&file);
    stream<< "swc_file_name1,swc_file_name2,sum_distance,structure_difference,max_distance"<<"\n";




    for (int i = 0; i < nt_list_resampled.size()-1; i++){
        for (int j = i+1; j < nt_list_resampled.size(); j++)
        {
            NeuronDist tmp_score = resampled_neuron_matching_distance(&nt_list_resampled[i], &nt_list_resampled[j],false);
            cout <<"\n\nComputing neuron distance between " <<i<<"  and "<<j <<":"<<endl;
            cout<<"bi-direcitonal total matching distance = "<<tmp_score.matching_total_dist_ave <<endl;
            cout<<"differen-structure-average = "<<tmp_score.dist_apartnodes<<endl;
            cout<<"percent of different-structure = "<<tmp_score.percent_apartnodes<<endl<<endl;

            dis_sum_1[i] += tmp_score.matching_total_dist_ave;
            dis_sum_1[j] += tmp_score.matching_total_dist_ave;

            //dis_sum_1[i] += tmp_score.weighted_dist_ave_allnodes;
            //dis_sum_1[j] += tmp_score.weighted_dist_ave_allnodes;



//            dis_sum_2[i] += tmp_score.dist_apartnodes;
//            dis_sum_2[j] += tmp_score.dist_apartnodes;

//            dis_sum_3[i] += tmp_score.dist_max;
//            dis_sum_3[j] += tmp_score.dist_max;


            stream << nt_list_resampled[i].file <<","<< nt_list_resampled[j].file
                   <<","<< tmp_score.matching_total_dist_ave
                   <<","<<tmp_score.dist_apartnodes
                   <<","<<tmp_score.dist_max
                   <<"\n";
        }
    }


    file.close();
    double min_dis_sum_1 = DBL_MAX;
//    double min_dis_sum_2 = DBL_MAX;
//    double min_dis_sum_3 = DBL_MAX;


    for (int i = 0; i < nt_list_resampled.size(); i++){

        if (dis_sum_1[i] < min_dis_sum_1  && dis_sum_1[i] >0 )
        {
            idx1 = i;
            min_dis_sum_1 = dis_sum_1[i];
        }
//        if (dis_sum_2[i] < min_dis_sum_2 && dis_sum_2[i] >0 )
//        {
//            idx2 = i;
//            min_dis_sum_2 = dis_sum_2[i];
//        }
//        if (dis_sum_3[i] < min_dis_sum_3 && dis_sum_3[i] >0 )
//        {
//            idx3 = i;
//            min_dis_sum_3 = dis_sum_3[i];
//        }

    }


    vector<int> order  =  argsort(dis_sum_1);

    //rank swcs according to the sum distances
    //report to an ano file
    QString outputOrderAnoFileName = outputFileName+"_ranked_by_distance.ano";
    QFile file_ano(outputOrderAnoFileName);
    if (!file_ano.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<outputOrderAnoFileName.toStdString().c_str() << endl;
        return false;
    }

    QTextStream stream_ano (&file_ano);
    cout<<"Ranked distances:"<<endl;
    for (int i = 0; i < order.size(); i++){
               stream_ano<< "SWCFILE="<<nt_list[order[i]].file<<"\n";
               cout<<nt_list[order[i]].file.toStdString().c_str()<<":"<<dis_sum_1[order[i]]<<endl;
     }
    file_ano.close();
    cout <<"Output ordered swcs into anofile for viewing: "<<outputOrderAnoFileName.toStdString().c_str() << endl;

    //median case is determined by the min_dis_sum_1
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
