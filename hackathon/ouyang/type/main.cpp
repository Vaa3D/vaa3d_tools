
/*
 *plugin for find the wrong type in constructed neuron
 and make a maker for wrong point
 developed by Ou Yang 9.11.2018*/



# include "main.h"
# include <vector>;
# include "v3d_message.h"
# include <v3d_interface.h>
# include <iostream>
#include  <algorithm>



using namespace std;

// find the suspecious wrong type of neuron type
QVector<QVector<V3DLONG> > childs;
void detect_type(V3DPluginCallback2 &callback, QWidget *parent)
  {
     NeuronTree tree1=callback.getSWCTeraFly();
     LandmarkList Markers = callback.getLandmarkTeraFly();
     QList<NeuronSWC> tree1swc=tree1.listNeuron;
     LandmarkList result;
     vector<int> kinds;
     for (int i=0;i<tree1swc.size();i++)
     {
       kinds.push_back(tree1swc.at(i).type);
     }
     //for(int i=100;i<150;i++){
     //cout<<"-------------================-"<<kinds.at(i)<<endl;}
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
     int ct=0;
     for (int i=0;i<20;i++)
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
     /*int ct=0;
     for(int i=j+1;i<kinds.size();i++){
         if(kinds.at(j) != kinds.at(i)) ct++;
     }*/
     cout<<"---------------the number of neuron points: "<<kinds.size()<<endl;
     cout<<"---------------the kinds number of neuron type: "<<ct<<endl;

     //get the childslist
     V3DLONG neuronNum = tree1.listNeuron.size();
     childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
     for (V3DLONG i=0;i<neuronNum;i++)
     {
         V3DLONG par = tree1.listNeuron[i].pn;
         if (par<0) continue;
         childs[tree1.hashNeuron.value(par)].push_back(i);
     }
     vector<int> suspoint;
     for (int i=0;i<tree1swc.size();i++)
     {
         if (childs[i].size()==2)
         {
             int child1=childs[i][0];
             int child2=childs[i][1];
             if(tree1swc.at(i).type != tree1swc.at(child1).type) suspoint.push_back(child1);
             else if(tree1swc.at(i).type != tree1swc.at(child2).type) suspoint.push_back(child2);
         }
         else if(childs[i].size()==1)
         {
             int child1=childs[i][0];
             if(tree1swc.at(i).type != tree1swc.at(child1).type) {suspoint.push_back(child1);}
         }
     }
     cout<<"=========the suspoints number: "<<suspoint.size()<<endl;
     LocationSimple m;
     for(int i=0;i<suspoint.size();i++)
     {
             m.x=tree1swc.at(suspoint.at(i)).x;
             m.y=tree1swc.at(suspoint.at(i)).y;
             m.z=tree1swc.at(suspoint.at(i)).z;
             m.color.r = 255;
             m.color.g = 255;
             m.color.b = 255;
             result.push_back(m);
           }
     for (int i=0;i<Markers.size();i++)
     {
             result.push_back(Markers.at(i));
     }
     callback.setLandmarkTeraFly(result);

  }



