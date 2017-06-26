#include "match_swc.h"
#include "neuron_utilities/resample_swc.h"
#include "neuron_utilities/sort_swc.h"
#include<iostream>

using namespace std;

#define max(a,b) (a)>(b)?(a):(b)

void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs)
{
    V3DLONG nt_size=nt.listNeuron.size();
    childs=vector<vector<V3DLONG> > (nt_size,vector<V3DLONG>() );
    for (V3DLONG i=0; i<nt_size;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
}

vector<V3DLONG> getTargetNode(NeuronTree & nt, V3DLONG t_num)
{
    vector<V3DLONG> target_node_list;
    vector<vector<V3DLONG> > childs_nt;
    V3DLONG siz=nt.listNeuron.size();
    getChildNum(nt,childs_nt);
    for(V3DLONG i=0;i<siz;i++)
    {
        V3DLONG b_num=0;
        if(nt.listNeuron[i].pn<0) b_num=childs_nt[i].size();
        else b_num=childs_nt[i].size()+1;
        if(b_num == t_num) target_node_list.push_back(i);
    }
    return target_node_list;
}

void match_swc(const NeuronTree &nt_raw, const NeuronTree &mk_raw)
{
    if(nt_raw.listNeuron.size()==0 || mk_raw.listNeuron.size()==0)
    {
        cout<<"input is empty."<<endl;
        return;
    }
    // resample nt_raw ; resample and sort mk_raw
    NeuronTree nt,mk;
    double resample_step=2;
    nt = resample(nt_raw,resample_step);

    NeuronTree mk_resample = resample(mk_raw,1);
    V3DLONG mk_root_id=mk_resample.listNeuron[0].n;
    cout<<"mk_root_id="<<mk_root_id<<endl;
    if(!SortSWC(mk_resample.listNeuron,mk.listNeuron,mk_root_id,0))
    {
        cout<<"sort failed"<<endl;
        return;
    }
    mk.hashNeuron.clear();
    nt.hashNeuron.clear();
    for(V3DLONG i=0; i<mk.listNeuron.size();i++){mk.hashNeuron.insert(mk.listNeuron[i].n, i);}
    for(V3DLONG i=0; i<nt.listNeuron.size();i++){nt.hashNeuron.insert(nt.listNeuron[i].n, i);}
    // temp save
    QString resample_name="/home/hys/Desktop/ml_neuron/data/mk_resample.swc";
    QString sort_name="/home/hys/Desktop/ml_neuron/data/mk_sort.swc";
    export_list2file(mk_resample.listNeuron,resample_name,resample_name);
    export_list2file(mk.listNeuron,sort_name,sort_name);
    QString nt_name="/home/hys/Desktop/ml_neuron/data/nt_resamplet.swc";
    export_list2file(nt.listNeuron,nt_name,nt_name);
//    nt=nt_raw;
//    mk=mk_resample;
    vector<vector<V3DLONG> > childs_mk;
    V3DLONG max_branches_mk=0;
    V3DLONG nt_size=nt.listNeuron.size();
    V3DLONG mk_size=mk.listNeuron.size();

    cout<<"nt_size="<<nt_size<<endl;
    cout<<"mk_size="<<mk_size<<endl;
    getChildNum(mk,childs_mk);
    for(V3DLONG i=0;i<mk_size;i++)
    {
        cout<<i<<"="<<childs_mk[i].size()<<"  ";
        if(mk.listNeuron[i].pn<0){max_branches_mk=max(max_branches_mk,childs_mk[i].size() );}
        else{max_branches_mk=max(max_branches_mk,childs_mk[i].size()+1);}
    }
    cout<<endl;
    cout<<"max_branches_mk="<<max_branches_mk<<endl;

    vector<V3DLONG> target_node_list;
    target_node_list=getTargetNode(nt,max_branches_mk);
    cout<<endl;
    for(V3DLONG i=0;i<target_node_list.size();i++) cout<<target_node_list[i]+1<<"    ";
    return;

}
