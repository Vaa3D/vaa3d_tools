#include "sholl_swc.h"
#include <vector>
#include <iostream>
#include <algorithm>

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

double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
    double xx = s1.x-s2.x;
    double yy = s1.y-s2.y;
    double zz = s1.z-s2.z;
    return (xx*xx+yy*yy+zz*zz);
};

vector<double> ShollSWC(QList<NeuronSWC> & neuron, double step)
{
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

    //double somax,somay,somaz;
    int soma_line;
    vector <double> distance;

    // This loop finds the soma
    for(int i= 0; i<neuron.size(); i++)
    {
        if (neuron.at(i).pn == -1)
        {
            //somax= neuron.at(i).x;
            //somay= neuron.at(i).y;
            //somaz= neuron.at(i).z;
            soma_line = i; // This line assigns the value i to the variable soma_line. Given that all the nodes of neuron will be explored (line 44) and only the node that has parent id -1 will be true in the if (line 46), soma_line will be assigned the soma node.
        }
    }
    for(int i=0;i<neuron.size();i++)
    {
       double s;
       s=computeDist2(neuron.at(i),neuron.at(soma_line));
       distance.push_back(s);
    }
    double max = *max_element(distance.begin(),distance.end());

    //int sum=0;(distance.at(parent
    //for(int r=1;r<max;r+=50)
    //{
      //  for(int j=o;j<distance.size();j++)
        //{
          //  if(dis.[j]<r)
            //{
              // sum=sum+1;
           // }
    //}
    vector<int> tipslist;
    for (int i=0;i<neuron.size();i++)
    {
        int sum=0;
        for (int j=0;j<neuron.size();j++)
        {
            if (neuron.at(i).n==neuron.at(j).pn)
            {
                sum=sum+1;
            }
        }
        if (sum<1)
            {
                tipslist.push_back(i);
            }
     }
    qDebug()<< tipslist.size();

    vector<double> radius;
    vector<double> crossings;
    if(step<1 || step==VOID)
    {
        step=1;
    }
    radius.push_back(0);
    crossings.push_back(0);
    for (int i=int(step);i<max;i+=int(step))
    {
         radius.push_back(i);
         int crss = 0;
         for(int j=0; j<distance.size();j++)
         {
             if(distance.at(j)>=double(i) && distance.at(j)<double(i+step))
             {
                 V3DLONG parent = neuron.at(j).pn-1;
                 if(parent == -2) parent = soma_line;
                 if(distance.at(parent)<double(i))
                 {
                     crss++;
                 }
             }
             if(distance.at(j)<=double(i) && distance.at(j)>double(i-step))
             {
                 V3DLONG parent = neuron.at(j).pn-1;
                 if(parent == -2) parent = soma_line;
                 if(distance.at(parent)>double(i))
                 {
                     crss++;
                 }
             }
         }
         crossings.push_back(crss);
    }
    for(int i=0; i<crossings.size(); i++)
    {
        qDebug() << radius.at(i) << crossings.at(i);
    }

    //qDebug() << max << somay << somaz;

    return crossings;
}
