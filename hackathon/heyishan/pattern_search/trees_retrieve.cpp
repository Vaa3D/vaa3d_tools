#include"trees_retrieve.h"
#include "compute_gmi.h"
#include "compute_morph.h"
#include "neuron_retrieve.h"
#include "pre_process.h"
#include "my_sort.h"
bool trees_retrieve(vector<NeuronTree> & sub_trees, const NeuronTree & pt_consensus,vector<V3DLONG> &selected_trees)
{
    cout<<"blastneuron: batch_compute"<<endl;
    QList<double*> morph_list, gmi_list;
    double step_size=2;
    double prune_size=2.0;
    V3DLONG neuronNum =  sub_trees.size();

//    QString outfile("features.csv");
//    FILE * fp;
//    fp = fopen(qPrintable(outfile), "w");

//    QString header = "swc_file,num_nodes,soma_surface,num_stems,"
//                     "num_bifurcations,num_branches,num_of_tips,"
//                     "overall_x_span,overall_y_span,overall_z_span,"
//                     "average_diameter,total_length,total_surface,"
//                     "total_volume,max_euclidean_distance,max_path_distance,"
//                     "max_branch_order,average_contraction,average_fragmentation,"
//                     "parent_daughter_ratio,bifurcation_angle_local,"
//                     "bifurcation_angle_remote,"
//                     "moment1,moment2,moment3,moment4,moment5,moment6,"
//                     "moment7,moment8,moment9,moment10,moment11,moment12,"
//                     "moment13,ave_R\n";

//    fprintf(fp,qPrintable(header));

    for(V3DLONG i=0;i<sub_trees.size();i++)
    {
        NeuronTree tmp;
        if(sub_trees[i].listNeuron.size()>10)
        {
            tmp=pre_process(sub_trees[i],step_size,prune_size);
        }
        else
        {
            tmp = sub_trees[i];
        }
        //test
//        tmp =sort(tmp,VOID,VOID);
//        QString sub_name = "subtree/sub_"+QString::number(i)+".swc";
//        writeSWC_file(sub_name,tmp);

        double * feature_morph = new double[21];
        double * feature_gmi = new double[14];
        computeFeature(tmp, feature_morph);
        computeGMI(tmp, feature_gmi);
        feature_morph[19]=0;    feature_morph[20]=0;
        morph_list.append(feature_morph);
        gmi_list.append(feature_gmi);

//        for(int j=0;j<21;j++)
//        {
//            fprintf(fp,", %.8f", feature_morph[j]);
//        }
//        for(int j=0;j<14;j++)
//        {
//            fprintf(fp,", %.8f", feature_gmi[j]);
//        }
//        fprintf(fp,"\n");
//         }
//        fclose(fp);
    }

    cout<<"morph_list_size="<<morph_list.size()<<"  gmi_list_size= "<<gmi_list.size()<<endl;


    // blastneuron: global_retrieve
    cout<<"blastneuron: global_retrieve"<<endl;
    NeuronTree query =pt_consensus;
    //int retrieved_num = sub_trees.size()/200;// number
    int retrieved_num = 15;
    if(retrieved_num<10) retrieved_num=10;
    double thres = 0.1;
    vector<int> feature_codes, norm_codes;
    vector<vector<V3DLONG> > retrieved_all;
    feature_codes.push_back(1);
    //feature_codes.push_back(2);
    norm_codes.push_back(1);
    //norm_codes.push_back(2);
    for  (int i=0;i<feature_codes.size();i++)
    {
        for (int j=0;j<norm_codes.size();j++)
        {
            vector<V3DLONG> retrieved_tmp;
            if (feature_codes[i]==1)
            {
                if (!neuron_retrieve(query, morph_list, retrieved_tmp, neuronNum, feature_codes[i], norm_codes[j]))
                {
                    fprintf(stderr,"Error in neuron_retrieval.\n");
                    return false;
                }
            }
            else if (feature_codes[i]==2)
            {
                if (!neuron_retrieve(query, gmi_list, retrieved_tmp, neuronNum, feature_codes[i], norm_codes[j]))
                {
                    fprintf(stderr,"Error in neuron_retrieval.\n");
                    return false;
                }
            }
            retrieved_all.push_back(retrieved_tmp);
        }

    }
    //cout<<retrieved_all.size()<<endl;
    cout<<"compute_intersect"<<endl;
    vector<V3DLONG>	retrieved_id;
    int rej_thres = 6;//if top 5 candidates of both method have no intersection, consider this query does not have matched neuron

    if (!compute_intersect(retrieved_all, retrieved_id, retrieved_num, rej_thres))
    {
        printf("No similar neurons exist in the database.\n");
    }
    cout<<"compute over"<<endl;
    selected_trees=retrieved_id;
//    for (V3DLONG i=0;i<neuronNum;i++)
//    {
//        if (morph_list[i]) {delete []morph_list[i]; morph_list[i]=NULL;}
//        if (gmi_list[i]) {delete []gmi_list[i]; gmi_list[i]=NULL;}
//    }
    morph_list.clear();
    gmi_list.clear();
    return true;
}
