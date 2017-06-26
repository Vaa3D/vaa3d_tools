#include "match_swc.h"
#include "resample_swc.h"
#include "sort_swc.h"
#include<iostream>
#include<set>
//#include "find_feature.h"
//#include "match_swc.h"
//#include "sim_measure.h"
//#include "test_plugin.h"
using namespace std;



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
vector<V3DLONG> get_parent_child(V3DLONG id,NeuronTree &nt, vector<vector<V3DLONG> > & childs_nt,int SEMI)
{
    vector<V3DLONG> points;
    V3DLONG id1=id;
    V3DLONG id2=id;
    cout<<"id1="<<id1<<endl;
    cout<<int(getParent(id1,nt))<<endl;
    for(int i=SEMI; i<0;i++)
    {
        if(int(getParent(id1,nt))>1000000)break;
        int cur_pid=getParent(id1,nt);
        points.push_back(cur_pid);
        id1=cur_pid;SEMI-=1;
    }
    for(int i=0;i<childs_nt[id2].size();i++)
    {
        V3DLONG cur_child = childs_nt[id2][i];
        for(int j=0;j<childs_nt[cur_child].size();j++)
        {
            V3DLONG cur_child2 = childs_nt[cur_child][j];
            points.push_back(cur_child2);
        }
        points.push_back(cur_child);
    }
    points.push_back(id);
    return points;
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

void match_swc(const NeuronTree &nt_raw, const NeuronTree &mk_raw, NeuronTree &s_mk, vector<NeuronTree> s_forest)
{
     cout<<"******************This is 1*********************"<<endl;

    if(nt_raw.listNeuron.size()==0 || mk_raw.listNeuron.size()==0)
    {
        cout<<"input is empty."<<endl;
        return;
    }
    cout<<"******************This is 2*********************"<<endl;
    // resample nt_raw ; resample and sort mk_raw
    NeuronTree nt,mk;
    double resample_step=2;
    nt = resample(nt_raw,resample_step);

    cout<<"******************This is 3*********************"<<endl;


    NeuronTree mk_resample = resample(mk_raw,1);
    V3DLONG mk_root_id=mk_resample.listNeuron[0].n;
    cout<<"mk_root_id="<<mk_root_id<<endl;

    if(!SortSWC(mk_resample.listNeuron,mk.listNeuron,mk_root_id,0))
    {
        cout<<"sort failed"<<endl;
        return;
    }
    cout<<"******************This is 4*********************"<<endl;
    mk.hashNeuron.clear();
    nt.hashNeuron.clear();
    for(V3DLONG i=0; i<mk.listNeuron.size();i++){mk.hashNeuron.insert(mk.listNeuron[i].n, i);}
    for(V3DLONG i=0; i<nt.listNeuron.size();i++){nt.hashNeuron.insert(nt.listNeuron[i].n, i);}
    // temp save
    QString resample_name="/home/lxf/Desktop/mk_resample.swc";
    QString sort_name="/home/hys/Desktop/ml_neuron/data/mk_sort.swc";
    export_list2file(mk_resample.listNeuron,resample_name,resample_name);
    export_list2file(mk.listNeuron,sort_name,sort_name);
    QString nt_name="/home/lxf/Desktop/nt_resample.swc";
    export_list2file(nt.listNeuron,nt_name,nt_name);
//    nt=nt_raw;
//    mk=mk_resample;

    // find target point
    /*
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
    */

    // attribute of template
    double Width, Height, Depth;
    double xmin,ymin,zmin;
    xmin = ymin = zmin = VOID;
    double xmax,ymax,zmax;
    xmax = ymax = zmax;
    double xcenter,ycenter,zcenter;
    for (int V3DLONG i=0;i<mk.listNeuron.size();i++)
    {
        NeuronSWC curr = mk.listNeuron.at(i);
        xmin = min(xmin,curr.x); ymin = min(ymin,curr.y); zmin = min(zmin,curr.z);
        xmax = max(xmax,curr.x); ymax = max(ymax,curr.y); zmax = max(zmax,curr.z);
    }
    Width = xmax-xmin;
    Height = ymax-ymin;
    Depth = zmax-zmin;
    xcenter= mean(xmin,xmax); ycenter=mean(ymin,ymax); zcenter=mean(zmin,zmax);

    cout<<" find the nearest point from center"<<endl;
    NeuronSWC c_point;
    double min_dist=VOID;
    for(V3DLONG i=0;i<mk.listNeuron.size();i++)
    {
        NeuronSWC curr = mk.listNeuron.at(i);
        double distance=sqrt((curr.x-xcenter)*(curr.x-xcenter)+(curr.y-ycenter)*(curr.y-ycenter)+(curr.z-zcenter)*(curr.z-zcenter));
        if(distance<min_dist) c_point=curr;
    }

    // find max_branches node and their parent-child set
    cout<<"find max_branches node and their parent-child set"<<endl;
    vector<V3DLONG> max_branches_node_list;
    QList<NeuronSWC> candidates;
    vector<vector<V3DLONG> > childs_mk;
    V3DLONG max_branches_mk=0;
    V3DLONG nt_size=nt.listNeuron.size();
    V3DLONG mk_size=mk.listNeuron.size();

    cout<<"nt_size="<<nt_size<<endl;
    cout<<"mk_size="<<mk_size<<endl;
    getChildNum(mk,childs_mk);
    for(V3DLONG i=0;i<mk_size;i++)
    {
        if(mk.listNeuron[i].pn<0){max_branches_mk=max(max_branches_mk,childs_mk[i].size() );}
        else{max_branches_mk=max(max_branches_mk,childs_mk[i].size()+1);}
    }
    max_branches_node_list =  getTargetNode(mk,max_branches_mk);

    // get small cube
    cout<<"get small cube"<<endl;
    double S_length=VOID;
    double L_length=0;
    int SENI=2;
    double max_dist=0;
    for(V3DLONG i=0;i<max_branches_node_list.size();i++)    //Need to find real candidates
    {
        V3DLONG id = max_branches_node_list[i];
        vector<V3DLONG> points;
        points=get_parent_child(id,mk, childs_mk,SENI);
        for(int j=0;j<points.size();j++)
        {
            candidates.push_back(mk.listNeuron[points[j]]);
        }
    }
    for(int i=0;i<candidates.size();i++)
    {
        double temp_dist;
        temp_dist=dist(c_point,candidates[i]);
        max_dist=max(max_dist,temp_dist);
    }
    cout<<"candidates="<<candidates.size()<<endl;
    S_length=max_dist;
    for(V3DLONG i=0;i<mk.listNeuron.size();i++)
    {
        if (dist(mk.listNeuron[i],c_point)<S_length) s_mk.listNeuron.push_back(mk.listNeuron[i]);
    }

    NeuronTree s_mk_sorted;\
    V3DLONG mk_sorted_root_id=VOID;
    //if(s_mk.listNeuron.size()>0) mk_sorted_root_id=s_mk.listNeuron[0].n;
    SortSWC(s_mk.listNeuron,s_mk_sorted.listNeuron,mk_sorted_root_id,0);
    //sort


    s_mk_sorted.hashNeuron.clear();
    for(V3DLONG i=0; i<s_mk_sorted.listNeuron.size();i++){s_mk_sorted.hashNeuron.insert(s_mk_sorted.listNeuron[i].n, i);}

    // get small cubes in whole nt
    cout<<"get small cubes in whole nt"<<endl;
    int SEARCH_STEP = 2;
    for(V3DLONG i=0;i<nt.listNeuron.size() - SEARCH_STEP; i+=SEARCH_STEP)
    {
        NeuronTree cube;
        NeuronSWC point1=nt.listNeuron[i];
        for(V3DLONG j=0; j<nt.listNeuron.size();j++)
        {
            NeuronSWC point2=nt.listNeuron[j];
            if(dist(point1,point2) <= S_length) cube.listNeuron.push_back(point2);
        }

        //sort

        NeuronTree cube_sorted;\
        V3DLONG cube_sorted_root_id = VOID;
        //if(cube.listNeuron.size()>0) cube_sorted_root_id=cube.listNeuron[0].n;
        SortSWC(cube.listNeuron,cube_sorted.listNeuron,cube_sorted_root_id,0);



        cube_sorted.hashNeuron.clear();
        for(V3DLONG j=0; j<cube_sorted.listNeuron.size();j++){cube_sorted.hashNeuron.insert(cube_sorted.listNeuron[j].n, j);}
        s_forest.push_back(cube_sorted);
    }
    cout<<"nt_size"<<nt.listNeuron.size()<<endl;
    cout<<"s_forest="<<s_forest.size()<<endl;

    /*
    // calculate the small and large cube_sorted's lenght of side
    double S_length, L_length;
    S_length=VOID;
    L_length=0;
    for(V3DLONG i=0;i<childs_mk.size();i++)
    {
        if(childs_mk[i].size()==0)
        {
            NeuronSWC S=mk.listNeuron[i];
            double distance=dist(S,S);  //Need to find the point which has most branches.
            S_length=min(S_length,distance);
            L_length=max(L_length,distance);
        }
    }
    */

    /*
    // get substructure in the cube
    QList<NeuronSWC> result;
    for(V3DLONG i=0; i<target_node_list.size(); i++)
    {
        QList<NeuronSWC> cube;
        NeuronSWC T = nt.listNeuron[target_node_list[i]]; //whether to plus one.
        NeuronSWC T2 = nt.listNeuron[target_node_list[i]+1];
       NeuronSWC T3 = nt.listNeuron[target_node_list[i]+2];
       if(i!=0){
       NeuronSWC T4 = nt.listNeuron[target_node_list[i]-1];
       result.push_back(T4);
       }
        for(V3DLONG j=0; j<nt.listNeuron.size();j++)
        {
            NeuronSWC S=nt.listNeuron[j];
            double distance=dist(S,T);
            if(distance<=Width)
            {
                cube.push_back(S);
            }
        }
        result.push_back(T);
        result.push_back(T2);
        result.push_back(T3);
        //result.push_back(T4);
    }
    */
    QString result_name="/home/lxf/Desktop/cube_sorted1.swc";
    QList<NeuronSWC> result=s_mk_sorted.listNeuron;
    export_list2file(result,result_name,result_name);

    return;
}
