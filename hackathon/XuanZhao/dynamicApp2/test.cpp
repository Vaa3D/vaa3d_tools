#include "test.h"
#include "neuron_sim_scores.h"
#include <iostream>
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"
#include "axontrace.h"

float calPrecision(NeuronTree &traced, NeuronTree &manual, XYZ origin, float d_thres){
    sortSWC(traced, 8);
    NeuronTree traced_long = getLongPath(traced, origin);
    NeuronTree manual_origin = getChildTree(manual, origin);
    qDebug()<<"start resample";

    NeuronTree traced_long_resample = resample(traced_long, 5);
    setNeuronTreeHash(traced_long_resample);

    float traced_correct_length = 0;
    for(V3DLONG i=0; i<traced_long_resample.listNeuron.size(); ++i){
        NeuronSWC s = traced_long_resample.listNeuron[i];
        if(!traced_long_resample.hashNeuron.contains(s.parent)){
            continue;
        }
        XYZ pt = XYZ(s.x, s.y, s.z);
        double tmp_d = dist_pt_to_swc(pt, &manual_origin);
        if(tmp_d < d_thres){
            V3DLONG par_index = traced_long_resample.hashNeuron.value(s.parent);
            traced_correct_length += zx_dist(s, traced_long_resample.listNeuron[par_index]);
        } else {
//            qDebug()<< "out of range: "<<tmp_d;
        }
    }

    NeuronTree manual_long = getLongPath(manual_origin, origin);
    NeuronTree manual_long_resample = resample(manual_long, 5);
    setNeuronTreeHash(manual_long_resample);
    qDebug()<<"traced "<<traced.listNeuron.size();
    qDebug()<<"traced long "<<traced_long.listNeuron.size();
    qDebug()<<"traced long "<<traced_long_resample.listNeuron.size();
    qDebug()<<"manual "<<manual.listNeuron.size();
    qDebug()<<"manual origin "<<manual_origin.listNeuron.size();
    qDebug()<<"manual long "<<manual_long.listNeuron.size();
    qDebug()<<"manual long resample "<<manual_long_resample.listNeuron.size();
    float manual_long_length = 0;
    for(V3DLONG i=0; i<manual_long_resample.listNeuron.size(); ++i){
        NeuronSWC s = manual_long_resample.listNeuron[i];
        if(!manual_long_resample.hashNeuron.contains(s.parent)){
            continue;
        }
        V3DLONG par_index = manual_long_resample.hashNeuron.value(s.parent);
        manual_long_length += zx_dist(s, manual_long_resample.listNeuron[par_index]);
    }

    if(manual_long_length <= 0){
        return 0;
    }
    cout<<"traced_long_d: "<<traced_correct_length<<endl;
    cout<<"manual_long_d: "<<manual_long_length<<endl;

    return traced_correct_length/manual_long_length >= 1 ? 1 : traced_correct_length/manual_long_length;
}

void setNeuronTreeHash(NeuronTree &nt) {
    nt.hashNeuron.clear();
    for (V3DLONG i = 0; i < nt.listNeuron.size(); ++i) {
        nt.hashNeuron.insert(nt.listNeuron[i].n,i);
    }
}

NeuronTree getLongPath(NeuronTree &nt, XYZ origin){
    V3DLONG size = nt.listNeuron.size();
    vector<float> dis_to_origin = vector<float>(size, 0);
    setNeuronTreeHash(nt);
    double origin_min_d = 10000;
    V3DLONG origin_index = -1;
    vector<vector<V3DLONG>> children = vector<vector<V3DLONG>>(size,vector<V3DLONG>());
    for(V3DLONG i=0; i<size; ++i){
        V3DLONG par = nt.listNeuron[i].parent;
        if(nt.hashNeuron.contains(par)){
            children[nt.hashNeuron.value(par)].push_back(i);
        } else {
            qDebug()<<"root index: "<<nt.listNeuron[i].n;
        }
        int tmp_d = zx_dist(nt.listNeuron[i], origin);
        if(tmp_d < origin_min_d){
            origin_index = i;
            origin_min_d = tmp_d;
        }
    }
    qDebug()<<"origin_index"<<origin_index;

    int count = 1;
    while (children[origin_index].size() < 2) {
        count++;
        V3DLONG par = nt.listNeuron[origin_index].parent;
        if(!nt.hashNeuron.contains(par)){
            break;
        }
        origin_index = nt.hashNeuron.value(par);
        qDebug()<<count<<": "<<origin_index;
        if(count > 30){
            break;
        }
    }
    qDebug()<<"origin_index"<<origin_index;
    dis_to_origin[origin_index] = 0;
    nt.listNeuron[origin_index].parent = -1;

    queue<V3DLONG> q;
    q.push(origin_index);
    float dis_to_origin_max = 0;
    V3DLONG dis_to_origin_max_index = -1;
    while (!q.empty()) {
        V3DLONG cur_index = q.front();
        q.pop();
        for(V3DLONG& child : children[cur_index]){
            dis_to_origin[child] = dis_to_origin[cur_index] + zx_dist(nt.listNeuron[child], nt.listNeuron[cur_index]);
            q.push(child);
            if(dis_to_origin[child] > dis_to_origin_max){
                dis_to_origin_max_index = child;
                dis_to_origin_max = dis_to_origin[child];
            }
        }
    }

    qDebug()<<"start make line";

    NeuronTree result;
    V3DLONG tmp_index = dis_to_origin_max_index;
    qDebug()<<"tmp_index: "<<tmp_index;
    while(tmp_index != origin_index){
        NeuronSWC s = nt.listNeuron[tmp_index];
        result.listNeuron.push_back(s);
        tmp_index = nt.hashNeuron.value(s.parent);
    }
    result.listNeuron.push_back(nt.listNeuron[origin_index]);



    setNeuronTreeHash(result);
    qDebug()<<"end get line";

    return result;
}

NeuronTree getChildTree(NeuronTree &nt, XYZ origin){
    V3DLONG size = nt.listNeuron.size();
    setNeuronTreeHash(nt);
    vector<vector<V3DLONG>> children = vector<vector<V3DLONG>>(size,vector<V3DLONG>());
    double origin_min_d = 10000;
    V3DLONG origin_index = -1;
    for(V3DLONG i=0; i<size; ++i){
        V3DLONG par = nt.listNeuron[i].parent;
        if(nt.hashNeuron.contains(par)){
            children[nt.hashNeuron.value(par)].push_back(i);
        }
        int tmp_d = zx_dist(nt.listNeuron[i], origin);
        if(tmp_d < origin_min_d){
            origin_index = i;
            origin_min_d = tmp_d;
        }
    }

    NeuronTree result;
    nt.listNeuron[origin_index].parent = -1;
    queue<V3DLONG> q;
    q.push(origin_index);
    while (!q.empty()) {
        V3DLONG cur_index = q.front();
        result.listNeuron.push_back(nt.listNeuron[cur_index]);
        q.pop();
        for(V3DLONG& child : children[cur_index]){
            q.push(child);
        }
    }
    setNeuronTreeHash(result);

    return result;
}
