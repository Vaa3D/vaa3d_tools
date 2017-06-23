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

void match_swc(const NeuronTree &nt_raw, const NeuronTree &mk_raw)
{
    if(nt_raw.listNeuron.size()==0 || mk_raw.listNeuron.size()==0)
    {
        cout<<"input is empty."<<endl;
        return;
    }
    // resample nt_raw ; resample and sort mk_raw
    NeuronTree nt,mk;
    double resample_step=1;
    nt = resample(nt_raw,resample_step);

    NeuronTree mk_resample = resample(mk_raw,1);
    V3DLONG mk_root_id=mk_resample.listNeuron[0].n;
    cout<<"mk_root_id="<<mk_root_id<<endl;
    if(!SortSWC(mk_resample.listNeuron,mk.listNeuron,mk_root_id,0))
    {
        cout<<"sort failed"<<endl;
        return;
    }
    QString resample_name="/home/hys/Desktop/ml_neuron/data/mk_resample.swc";
    QString sort_name="/home/hys/Desktop/ml_neuron/data/mk_sort.swc";
    export_list2file(mk_resample.listNeuron,resample_name,resample_name);
    export_list2file(mk.listNeuron,sort_name,sort_name);
//    nt=nt_raw;
//    mk=mk_resample;
    vector<vector<V3DLONG> > childs_nt;
    vector<vector<V3DLONG> > childs_mk;
    V3DLONG max_branches_mk=0;
    V3DLONG nt_size=nt.listNeuron.size();
    V3DLONG mk_size=mk.listNeuron.size();

    cout<<"nt_size="<<nt_size<<endl;
    cout<<"mk_size="<<mk_size<<endl;
    getChildNum(nt,childs_nt);
    getChildNum(mk,childs_mk);
    for(V3DLONG i=0;i<mk_size;i++)
    {
        //V3DLONG id=mk.listNeuron[i].n;
        cout<<childs_mk.size()<<endl;
        cout<<i<<"="<<childs_mk[i].size()<<"  ";
        if(mk.listNeuron[i].pn<0){max_branches_mk=max(max_branches_mk,childs_mk[i].size() );}
        else{max_branches_mk=max(max_branches_mk,childs_mk[i].size()+1);}
    }
    cout<<endl;
    cout<<"max_branches_mk="<<max_branches_mk<<endl;
    return;

}
