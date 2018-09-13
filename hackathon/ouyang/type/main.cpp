
/*
 *plugin for find the wrong type in constructed neuron
 and make a maker for wrong point
 developed by Ou Yang 9.11.2018*/



# include "main.h"
# include <vector>;
# include "v3d_message.h"
# include <v3d_interface.h>
# include <iostream>
# include  <algorithm>
# include "sort_swc.h"



using namespace std;

// find the suspecious wrong type of neuron type
QVector<QVector<V3DLONG> > childs;
void detect_type(V3DPluginCallback2 &callback, QWidget *parent)
  {
     NeuronTree tree1=callback.getSWCTeraFly();
     LandmarkList Markers = callback.getLandmarkTeraFly();
     QList<NeuronSWC> tree1swc=tree1.listNeuron;
     LandmarkList result1;

     V3DLONG rootid=VOID;
     V3DLONG thres=100;
     QList<NeuronSWC> result;
     QList<NeuronSWC> neuron_nt;
     neuron_nt.clear();
     if (SortSWC(tree1swc, result ,rootid, thres))
     {
         neuron_nt=result;
     }
     tree1swc=neuron_nt;
     NeuronTree n_t;
     QHash <int, int> hash_nt;

     for(V3DLONG j=0; j<tree1swc.size();j++){
         hash_nt.insert(tree1swc[j].n, j);
     }
     n_t.listNeuron=tree1swc;
     n_t.hashNeuron=hash_nt;
     tree1=n_t;

     vector<int> ids;
     // Reorder tree ids so that neuron.at(i).n=i+1
     for(V3DLONG i=0;i<tree1swc.size();i++)
     {
         ids.push_back(tree1swc.at(i).n);
     }
     for(V3DLONG i=0;i<tree1swc.size();i++)
     {
         tree1swc[i].n=i+1;
         if(tree1swc.at(i).pn !=-1)
         {
             tree1swc[i].pn=find(ids.begin(), ids.end(),tree1swc.at(i).pn) - ids.begin()+1;
         }
     }
     /*ids.clear();
     for (int i=0;i<tree1swc.size();i++){
         int cc=tree1swc.at(i).n;
         ids.push_back(cc);
     }
     NeuronTree n_t;
     QHash <int, int> hash_nt;

     for(V3DLONG j=0; j<tree1swc.size();j++){
         hash_nt.insert(tree1swc[j].n, j);
     }
     n_t.listNeuron=tree1swc;
     n_t.hashNeuron=hash_nt;
     tree1=n_t;
     //for(int i=100;i<150;i++){
     //cout<<"-------------================-"<<tree1swc.at(i).n<<endl;}
     /*int temp;
     int j=0;
     for(int i=j+1;i<kinds.size();i++)
     {
         if (kinds.at(j)>=kinds.at(i)){
             temp=kinds.at(j);
             kinds.at(j)=kinds.at(i);
             kinds.at(i)=temp;
         }
     }*/
     vector<int> kinds;
     for (int i=0;i<tree1swc.size();i++)
     {
       kinds.push_back(tree1swc.at(i).type);
     }
     int max = *max_element(kinds.begin(),kinds.end());
     int ct=0;
     for (int i=0;i<max;i++)
     {
         for (int j=0;j<kinds.size();j++)
         {
             if(kinds.at(j)==i)
             {
                 ct=ct+1;
                 break;
             }
         }
     }
     cout<<"---------------the kinds number of neuron type: "<<ct<<endl;

     //get the childslist
     /*V3DLONG neuronNum = tree1.listNeuron.size();
     childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
     for (V3DLONG i=0;i<neuronNum;i++)
     {
         V3DLONG par = tree1.listNeuron[i].pn;
         if (par<0) continue;
         childs[tree1.hashNeuron.value(par)].push_back(i);
     }
     vector<int> suspoint;
     //for (int i=0;i<tree1swc.size();i++)
     //{
         /*if (childs[i].size()==2)
         {
             int child1=childs[i][0];
             int child2=childs[i][1];
             if(tree1swc.at(i).type != tree1swc.at(child1).type) suspoint.push_back(child1);
             else if(tree1swc.at(i).type != tree1swc.at(child2).type) suspoint.push_back(child2);
         }
         if(childs[i].size()==1)
         {
             int child1=childs[i][0];
             int type1=tree1swc.at(i).type; int type2=tree1swc.at(child1).type;
             if (type1 != type2)
             suspoint.push_back(child1);
             if(tree1swc.at(i).type != tree1swc.at(child1).type) {suspoint.push_back(child1);}
         }
       }*/
     /*vector<int> suspoint;
     for (int i=0;i<tree1swc.size();i++)
      {
         int child1=childs[i][0];
         int type1=tree1swc.at(i).type; int type2=tree1swc.at(child1).type;
         if (type1 == type2)
         suspoint.push_back(child1);

       }*/

     // use iterator to find the points having different types
     vector<int> suspoint;
     for (int i=0;i<tree1swc.size();i++){
         int type1=tree1swc.at(i).type;
         int pt=tree1swc.at(i).pn;
         if(pt!=-1)
         {
             vector<int>::iterator iter=find(ids.begin(), ids.end(), pt);
             int a=distance(ids.begin(),iter);
             int type2=tree1swc.at(a-1).type;
             if(type1 != type2)
                 suspoint.push_back(i);}
     }

     /*vector<int> suspoint;
     int j=0;
     for (int i=j+1;i<kinds.size();i++){
         if (kinds.at(i)!=kinds.at(j))
             suspoint.push_back(i);
     }*/



     cout<<"================the suspoints number: "<<suspoint.size()<<endl;
     LocationSimple m;
     for(int i=0;i<suspoint.size();i++)
     {
             m.x=tree1swc.at(suspoint.at(i)).x;
             m.y=tree1swc.at(suspoint.at(i)).y;
             m.z=tree1swc.at(suspoint.at(i)).z;
             m.color.r = 255;
             m.color.g = 255;
             m.color.b = 255;
             result1.push_back(m);
           }
     for (int i=0;i<Markers.size();i++)
     {
             result1.push_back(Markers.at(i));
     }
     callback.setLandmarkTeraFly(result1);

  }



