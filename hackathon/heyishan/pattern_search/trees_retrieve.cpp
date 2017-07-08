#include"trees_retrieve.h"
#include "compute_gmi.h"
#include "compute_morph.h"
#include "neuron_retrieve.h"
bool trees_retrieve(vector<NeuronTree> & sub_trees, const NeuronTree & pt_consensus,vector<V3DLONG> &selected_trees)
{
    cout<<"blastneuron: batch_compute"<<endl;
    QList<double*> morph_list, gmi_list;
    for(V3DLONG i=0;i<sub_trees.size();i++)
    {
        NeuronTree tmp=sub_trees[i];
        //cout<<"sub_trees[i].size="<<sub_trees[i].listNeuron.size()<<endl;
        double * feature_morph = new double[21];
        double * feature_gmi = new double[14];
        computeFeature(tmp, feature_morph);
        computeGMI(tmp, feature_gmi);
        morph_list.append(feature_morph);
        gmi_list.append(feature_gmi);
    }

    // blastneuron: global_retrieve
    cout<<"blastneuron: global_retrieve"<<endl;
    NeuronTree query =pt_consensus;
    int retrieved_num = 20;// number
    double thres = 0.1;
    V3DLONG neuronNum=sub_trees.size();
    vector<int> feature_codes, norm_codes;
    vector<vector<V3DLONG> > retrieved_all;
    feature_codes.push_back(1);
    feature_codes.push_back(2);
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
    cout<<"selected_trees_size"<<selected_trees.size()<<endl;
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        if (morph_list[i]) {delete []morph_list[i]; morph_list[i]=NULL;}
        if (gmi_list[i]) {delete []gmi_list[i]; gmi_list[i]=NULL;}
    }
    return true;
}
