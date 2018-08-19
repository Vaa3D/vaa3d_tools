#include "sholl_swc.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <math.h>
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
    double xx = 0.2*(s1.x)-0.2*(s2.x);
    double yy = 0.2*(s1.y)-0.2*(s2.y);
    double zz = s1.z-s2.z;
    return sqrt(xx*xx+yy*yy+zz*zz);
};

int typenum(QList<NeuronSWC> &neuron){
    int sumtype=0;
    for (int i=0;i<neuron.size();i++){
        if (neuron[i].type==3 || neuron[i].type==4 || neuron[i].pn == -1)
            sumtype++;
    }
    return sumtype;
}


vector<double> ShollSWC(QList<NeuronSWC> & neuron, double step)
{


  /* QList<NeuronSWC> & typeneuron;
   for(int j=0;j<sumtype;j++){
         int i=0;
         typeneuron.at(i)=neuron.at(typelist.at(j));
         i+=1;
      }
   neuron=typeneuron;

   QList<NeuronSWC> *typeneuron=new QList<NeuronSWC>[sumtype];
   for(int j=0;j<sumtype;j++){
         int i=0;
         NeuronSWC s=neuron.at(typelist.at(j));
         typeneuron->at(i)=s;qDebug()<<
         i+=1;
      }
   neuron=*typeneuron;*/

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

    int sumtype=typenum(neuron);
    //qDebug()<<sumtype;

    vector<double> typelist;

    for(int i=0;i<neuron.size();i++){
        if (neuron[i].type==3 || neuron[i].type==4 || neuron[i].pn ==-1){
            typelist.push_back(i);}
    }
    //qDebug()<<typelist.size();
    int soma_line;
    vector <double> distance;

    // This loop finds the soma
    for(int i= 0; i<neuron.size(); i++)
    {
        if (neuron.at(i).pn == -1)
        {
            soma_line = i;
        }
    }
    for(int i=0;i<sumtype;i++)
    {
       double s;
       s=computeDist2(neuron.at(typelist[i]),neuron.at(soma_line));
       distance.push_back(s);
    }
    double max = *max_element(distance.begin(),distance.end());
    qDebug()<< max;
    //qDebug()<< distance.size();
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
    //qDebug()<< tipslist.size();

    vector<double> radius;
    vector<double> crossings;
    if(step<1 || step==VOID)
    {
        step=1;
    }
    radius.push_back(0);
    crossings.push_back(0);
    for (double i=int(step);i<max;i+=int(step))
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
    return crossings;
}
