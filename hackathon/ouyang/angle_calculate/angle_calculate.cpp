#include "angle_calculate.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <algorithm>
using namespace std;

#define PI 3.14159265359
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)

//double Width=0, Height=0, Depth=0, Diameter=0, Length=0, Volume=0, Surface=0, Hausdorff=0;

bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined)
{
    V3DLONG neuronNum = linker.size();
    if (neuronNum<=0)
    {
        cout<<"the linker file is empty, please check your data."<<endl;
        return false;
    }
    V3DLONG offset = 0;
    combined = linker[0];
    for (V3DLONG i=1;i<neuronNum;i++)
    {
        V3DLONG maxid = -1;
        for (V3DLONG j=0;j<linker[i-1].size();j++)
            if (linker[i-1][j].n>maxid) maxid = linker[i-1][j].n;
        offset += maxid+1;
        for (V3DLONG j=0;j<linker[i].size();j++)
        {
            NeuronSWC S = linker[i][j];
            S.n = S.n+offset;
            if (S.pn>=0) S.pn = S.pn+offset;
            combined.append(S);
        }
    }
};

QVector<QVector<V3DLONG> > childs;
struct angles angle_calculate(const NeuronTree & nt)
{
    QList<NeuronSWC> neuron = nt.listNeuron;
    struct angles result;
    vector<long> ids;
    // Reorder tree ids so that neuron.at(i).n=i+1
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids.push_back(neuron.at(i).n);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn !=-1)
        {
            neuron[i].pn=find(ids.begin(), ids.end(),neuron.at(i).pn) - ids.begin()+1;
        }
    }
    //get the childslist
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    //qDebug()<<childs.size();
    //find the dendrite id
    vector<int> typelist;
    for(int i=0;i<neuron.size();i++){
        if (neuron[i].type==3 || neuron[i].type==4){
            typelist.push_back(i);}
    }
    //qDebug()<<typelist.size();
    //find the branch points id of dendrite
    vector<int> branchid;
        for (int i=0;i<typelist.size();i++)
        {

            int sum=0;
            int a=typelist.at(i);
            for(int j=1;j<typelist.size();j++)
            {
                int b=typelist.at(j);
                if (neuron.at(a).n==neuron.at(b).pn)
               {
              sum=sum+1;}
            }
            if(sum>1 && sum<3)
            {
                branchid.push_back(a);
            }
        }
     qDebug()<<branchid.size();
     //calculate both local and remote angle
     //iI found that many branch nodes' child point is itself

        vector<double> localang;
        vector<double> remoteang;
        for(int i=0;i<branchid.size();i++)
        {
            double max_local_ang = 0;
            //double max_remote_ang = 0;
            double local_ang;
            double remote_ang;

            int ch_local1 = childs[branchid.at(i)][0];
            int ch_local2 = childs[branchid.at(i)][1];
            local_ang = angle(neuron.at(branchid.at(i)),neuron.at(ch_local1),neuron.at(ch_local2));
            qDebug()<<local_ang;

            int ch_remote1 = getRemoteChild(branchid.at(i)).at(0);
            int ch_remote2 = getRemoteChild(branchid.at(i)).at(1);
            remote_ang = angle(neuron.at(branchid.at(i)),neuron.at(ch_remote1),neuron.at(ch_remote2));

            //if (local_ang<=0)
            //   local_ang = 360-local_ang;
            //if (local_ang==local_ang)
              //  max_local_ang = max(max_local_ang,local_ang);
            //if (remote_ang==remote_ang)
            //    max_remote_ang = max(max_remote_ang,remote_ang);

            localang.push_back(local_ang);
            remoteang.push_back(remote_ang);
            //i choose to plot remote angle,but both angles should be ploted in R,i just don't know how to plot two kinds of nodes in R
        }
      result.b=localang;
      result.a=remoteang;
      result.c=branchid;
      return result;
      qDebug()<<localang.size()<<branchid.size();
};

QVector<V3DLONG> getRemoteChild(int t)
{
    QVector<V3DLONG> rchildlist;
    rchildlist.clear();
    int tmp;
    for (int i=0;i<childs[t].size();i++)
    {
        tmp = childs[t].at(i);
        while (childs[tmp].size()==1)
            tmp = childs[tmp].at(0);
        rchildlist.append(tmp);
    }
    return rchildlist;
}
