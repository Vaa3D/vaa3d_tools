/*
 *  sort_func.cpp
 *  io functions for domenu and do func 
 *
 *  Created by Wan, Yinan, on 02/01/2012.
 *  modified by OUyang , on 08/21/2018
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "show_func.h"
//#include "openSWCDialog.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include "angle_marker.h"
#include "sort_swc.h"

using namespace std;

const QString title = QObject::tr("Sort SWC Plugin");

void show_menu(V3DPluginCallback2 &callback, QWidget *parent)
{

    NeuronTree nt= callback.getSWCTeraFly();
    QList<NeuronSWC> neuron = nt.listNeuron;
    QList<NeuronSWC> dendrite;
    for(int i=0;i<neuron.size();i++){

         if (neuron[i].type==3 || neuron[i].type==4){
            NeuronSWC s;
            s.x=neuron.at(i).x;
            s.y=neuron.at(i).y;
            s.z=neuron.at(i).z;
            s.type=neuron.at(i).type;
            s.radius=neuron.at(i).radius;
            s.pn=neuron.at(i).pn;
            s.n=neuron.at(i).n;
            dendrite.push_back(s);
        }
    }
    neuron=dendrite;

    LandmarkList Markers = callback.getLandmarkTeraFly();
    //qDebug()<<Markers.size();
    //qDebug()<<Markers.at(0).x<<Markers.at(0).y<<Markers.at(0).z;

    /*for(int i=0;i<Markers.size();i++){
        if (Markers.at(i).comments=="a"){
            NeuronSWC S;
            S.x = Markers.at(i).x;
            S.y = Markers.at(i).y;
            S.z = Markers.at(i).z;
            S.r = Markers.at(i).radius;
            S.n =50000 ;
            S.pn = 60000;
            neuron.append(S);
        }
    }
    qDebug()<<neuron.size();*/
    //V3DLONG rootid;
    //rootid=neuron.size()-1;
    //V3DLONG thres;
    V3DLONG rootid;
    V3DLONG thres;
    bool ok;
    rootid = QInputDialog::getInteger(0, "Would you like to specify new root number?","New root number:(If you select 'cancel', the first root in file is set as default)",1,1,neuron.size(),1,&ok);
    if (!ok)
        rootid = VOID;
    thres = QInputDialog::getDouble(0, "Would you like to set a threshold for the newly generated link?","threshold:(If you select 'cancel', all the points will be connected automated; If you set '0', no new link will be generated)",0,0,2147483647,1,&ok);
    if (!ok)
        thres = VOID;

    QList<NeuronSWC> result;
    QList<NeuronSWC> neuron_nt;
    neuron_nt.clear();
    if (SortSWC(neuron, result ,rootid, thres))
    {
        neuron_nt=result;
    }
    neuron=neuron_nt;
    cout<<"--------------------------------------dendrite node number after sorting: "<<neuron.size()<<endl;
    NeuronTree n_t;
    QHash <int, int> hash_nt;

    for(V3DLONG j=0; j<neuron.size();j++){
        hash_nt.insert(neuron[j].n, j);
    }
    n_t.listNeuron=neuron;
    n_t.hashNeuron=hash_nt;
    nt=n_t;

    vector<double> remote_angles;
    vector<double> local_angles;
    vector<int> branchid;
    angles temp;
           temp=angle_calculate(nt);
           branchid=temp.c;
           local_angles=temp.a;
           remote_angles=temp.b;

    vector<int>:: iterator it,it1;
    for(it=++branchid.begin();it!=branchid.end();)
    {
        it1=find(branchid.begin(),it,*it);
        if(it1!=it)
            it=branchid.erase(it);
        else
            it++;
    }
    //sort(branchid.begin(),branchid.end());
    //branchid.erase(unique(branchid.begin(),branchid.end()));



    LocationSimple m;
    LandmarkList mark;
    for(int i=0;i < branchid.size();i++){
        if(local_angles.at(i)>110)
        {
            m.x=neuron.at(branchid.at(i)).x;
            m.y=neuron.at(branchid.at(i)).y;
            m.z=neuron.at(branchid.at(i)).z;            
            m.color.r = 255;
            m.color.g = 0;
            m.color.b = 0;
            mark.push_back(m);
          }
     }
    cout<<"total marker number set into terafly: "<<mark.size()<<endl;
    for(int i;i<mark.size();i++)
    {
        cout<<"the coordinates of markers:x="<<mark.at(i).x<<" y="<<mark.at(i).y<<" z="<<mark.at(i).z<<endl;
    }
    callback.setLandmarkTeraFly(mark);


   /* QList<CellAPO> angle_markers;
    for(V3DLONG i = 0; i < branchid.size(); i++)
    {
        //int sum=0;
        CellAPO t;
        if(local_angles.at(i)>90)
        {
            t.x = neuron.at(branchid.at(i)).x;
            t.y = neuron.at(branchid.at(i)).y;
            t.z = neuron.at(branchid.at(i)).z;
            t.volsize = 50;
            t.color.r = 255;
            t.color.g = 0;
            t.color.b = 0;
            angle_markers.push_back(t);
            //sum+=1;
        }
    }*/

 }




