#include "match_swc.h"
#include "neuron_utilities/resample_swc.h"
#include "neuron_utilities/sort_swc.h"
#include<iostream>
#include<set>
using namespace std;

#define VOID 1000000000
#define PI 3.14159265359
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
#define mean(a,b) (a+b)/2.0
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)


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
        if(nt.listNeuron[i].pn<0)
        {
            b_num=childs_nt[i].size();
        }
        else{ b_num=childs_nt[i].size()+1;}
        if(b_num == t_num) {target_node_list.push_back(i);
        }
    }
    return target_node_list;
}
vector<V3DLONG> get_parent_child(V3DLONG id,NeuronTree &nt, vector<vector<V3DLONG> > & childs_nt,int SEMI)
{
    vector<V3DLONG> points;
    V3DLONG id1=id;
    V3DLONG id2=id;
    for(int i=SEMI; i>0;i--)
    {
        if(int(getParent(id1,nt))>1000000)break;
        int cur_pid=getParent(id1,nt);
        points.push_back(cur_pid);
        id1=cur_pid;
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
void match_swc(NeuronTree &nt_raw, NeuronTree &mk_raw, NeuronTree &s_mk, NeuronTree &s_mk_sorted,vector<NeuronTree> &s_forest,vector<vector<V3DLONG> > &p_to_cube)
{
    if(nt_raw.listNeuron.size()==0 || mk_raw.listNeuron.size()==0)
    {
        cout<<"input is empty."<<endl;
        return;
    }

    // resample and sort mk_raw
    NeuronTree nt= nt_raw;
    NeuronTree mk;
    double resample_step=1;
    NeuronTree mk_resample=resample(mk_raw,resample_step);
    if(!SortSWC(mk_resample.listNeuron,mk.listNeuron,VOID,0))
    {
        cout<<"sort failed"<<endl;
        return;
    }
    mk.hashNeuron.clear();
    for(V3DLONG i=0; i<mk.listNeuron.size();i++){mk.hashNeuron.insert(mk.listNeuron[i].n, i);}

    // temp save
    QString resample_name="./temp/mk_resample.swc";
    QString sort_name="./temp/mk_sort.swc";
    export_list2file(mk_resample.listNeuron,resample_name,resample_name);
    export_list2file(mk.listNeuron,sort_name,sort_name);

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
        if(distance<min_dist) {c_point=curr;min_dist=distance;}
    }
    cout<<"center is ";
    cout<<c_point.x<<"   "<<c_point.y<<"      "<<c_point.z<<endl;
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
    cout<<"max_branches="<<max_branches_mk<<endl;
    max_branches_node_list =  getTargetNode(mk,max_branches_mk);

    // get small cube
    cout<<"get small cube"<<endl;
    double S_length=VOID;
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
    cout<<"max_dist="<<max_dist;
    cout<<"candidates="<<candidates.size()<<endl;
    double ADD_LENGTH=4;
    S_length=max_dist+ADD_LENGTH;
    for(V3DLONG i=0;i<mk.listNeuron.size();i++)
    {
        if (dist(mk.listNeuron[i],c_point)<S_length) s_mk.listNeuron.push_back(mk.listNeuron[i]);
    }

    if(s_mk.listNeuron.size()!=0)
    {
        V3DLONG sort_id=s_mk.listNeuron[0].n;
        if(!SortSWC(s_mk.listNeuron,s_mk_sorted.listNeuron,sort_id,0)){cout<<"sort failed"<<endl;}
        s_mk_sorted.hashNeuron.clear();
        for(V3DLONG i=0; i<s_mk_sorted.listNeuron.size();i++){s_mk_sorted.hashNeuron.insert(s_mk_sorted.listNeuron[i].n, i);}
    }
    else {cout<<"small template is empty"<<endl;return;}

    // get small cubes in whole nt
    cout<<"get small cubes in whole nt"<<endl;
    int SEARCH_STEP = 2;
    cout<<"S_length="<<S_length<<endl;
    for(V3DLONG i=0;i<nt.listNeuron.size() - SEARCH_STEP; i+=SEARCH_STEP)
    {
        NeuronTree cube;
        cube.listNeuron.clear();
        NeuronSWC point1=nt.listNeuron[i];
        vector<V3DLONG> points;
        points.clear();
        for(V3DLONG j=0; j<nt.listNeuron.size();j++)
        {
            NeuronSWC point2=nt.listNeuron[j];
            if(dist(point1,point2) <= S_length)
            {
                cube.listNeuron.push_back(point2);
                points.push_back(j);
            }
        }
        NeuronTree cube_sorted;
        cube_sorted.listNeuron.clear();
        cube_sorted.hashNeuron.clear();
        if(cube.listNeuron.size()!=0)
        {
            V3DLONG sort_id=cube.listNeuron[0].n;
            if(!SortSWC(cube.listNeuron,cube_sorted.listNeuron,sort_id,0)){cout<<"sort failed"<<endl;}
            cube_sorted.hashNeuron.clear();
            for(V3DLONG j=0; j<cube_sorted.listNeuron.size();j++){cube_sorted.hashNeuron.insert(cube_sorted.listNeuron[j].n, j);}
        }
        else {cout<<"cube is empty"<<endl;}
        s_forest.push_back(cube_sorted);
        p_to_cube.push_back(points);
    }
    cout<<"nt_size"<<nt.listNeuron.size()<<endl;
    cout<<"s_forest="<<s_forest.size()<<endl;

    QString s_mk_sorted_name="./temp/s_mk_sorted.swc";
    export_list2file(s_mk_sorted.listNeuron,s_mk_sorted_name,s_mk_sorted_name);
//    QString cube1_name="./temp/cube1.swc";
//    QList<NeuronSWC> cube1=s_forest[1].listNeuron;
//    export_list2file(cube1,cube1_name,cube1_name);

    return;
}
