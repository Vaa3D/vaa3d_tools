#include "get_substructure.h"
#include "region_match_plugin.h"
#include "match_swc.h"
#include "blastneuron/compute_morph.h"
#include"blastneuron/compute_gmi.h"
#include "blastneuron/pre_process.h"
#include "blastneuron/neuron_retrieve.h"
void get_substructure(const NeuronTree &s_mk, const vector<NeuronTree>&s_forest,vector<V3DLONG>&selected_cube)
{
    // blastneuron: batch_compute
    QList<double*> morph_list, gmi_list;
//    double * feature_morph = new double[21];
//    double * feature_gmi = new double[14];
//    computeFeature(s_mk, feature_morph);
//    computeGMI(s_mk, feature_gmi);
//    morph_list.append(feature_morph);
//    gmi_list.append(feature_gmi);
    cout<<"blastneuron: batch_compute"<<endl;
    for(V3DLONG i=0;i<s_forest.size();i++)
    {
        NeuronTree tmp=s_forest[i];
        double * feature_morph = new double[21];
        double * feature_gmi = new double[14];
        computeFeature(tmp, feature_morph);
        computeGMI(tmp, feature_gmi);
        morph_list.append(feature_morph);
        gmi_list.append(feature_gmi);
    }

    // blastneuron: global_retrieve
    cout<<"blastneuron: global_retrieve"<<endl;
    NeuronTree query =s_mk;
    int retrieved_num = 10;// number
    double thres = 0.1;
    V3DLONG neuronNum=s_forest.size();
    vector<int> feature_codes, norm_codes;
    vector<vector<V3DLONG> > retrieved_all;
    feature_codes.push_back(1);
    //feature_codes.push_back(2);
    norm_codes.push_back(1);
    norm_codes.push_back(2);
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
                    return ;
                }
            }
            else if (feature_codes[i]==2)
            {
                if (!neuron_retrieve(query, gmi_list, retrieved_tmp, neuronNum, feature_codes[i], norm_codes[j]))
                {
                    fprintf(stderr,"Error in neuron_retrieval.\n");
                    return ;
                }
            }
            retrieved_all.push_back(retrieved_tmp);
            for(int m=0;m<20;m++)
            {
                cout<<retrieved_tmp[m]<<"       ";
            }
            v3d_msg("1");
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
    selected_cube=retrieved_id;
    cout<<"selected_cube_size"<<selected_cube.size()<<endl;
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        if (morph_list[i]) {delete []morph_list[i]; morph_list[i]=NULL;}
        if (gmi_list[i]) {delete []gmi_list[i]; gmi_list[i]=NULL;}
    }
}


