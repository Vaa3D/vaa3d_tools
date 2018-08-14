/*
 *  sort_func.cpp
 *  io functions for domenu and do func 
 *
 *  Created by Wan, Yinan, on 02/01/2012.
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "show_func.h"
#include "openSWCDialog.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <vector>
#include <iostream>
#include <algorithm>

#include <angle_marker.h>
using namespace std;

const QString title = QObject::tr("Sort SWC Plugin");

void show_menu(V3DPluginCallback2 &callback, QWidget *parent)
{

	OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
	if (!openDlg->exec())
		return;

    QString fileOpenName = openDlg->file_name;
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
         nt = openDlg->nt;
    }
    QList<NeuronSWC> neuron = nt.listNeuron;

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


    //LocationSimple m;
    //LandmarkList mark;
    //for(int i=0;i<Angles.size();i++){
      //  if(Angles.at(i)>90)
        //{
          //  m.x=neuron.at(branchid.at(i)).x;
            //m.y=neuron.at(branchid.at(i)).y;
            //m.z=neuron.at(branchid.at(i)).z;
            //mark.push_back(m);
       // }
    //}
    QList<CellAPO> angle_markers;

    for(V3DLONG i = 0; i < branchid.size(); i++)
    {
        //int sum=0;
        CellAPO t;
        if(remote_angles.at(i)>90)
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
    }

    //for(int i=0;i<branchid.size();i++){
      //  qDebug()<<branchid.at(i);
    //}
    qDebug()<<branchid.size()<<remote_angles.size()<<angle_markers.size();

    QString apo_name = fileOpenName + ".apo";
    writeAPO_file(apo_name,angle_markers);
    QString linker_name = fileOpenName + ".ano";
    QFile qf_anofile(linker_name);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        v3d_msg("Cannot open file for writing!");
        return;
    }

    QTextStream out(&qf_anofile);
    out << "SWCFILE=" << QFileInfo(fileOpenName).fileName()<<endl;
    out << "APOFILE=" << QFileInfo(apo_name).fileName()<<endl;
    v3d_msg(QString("Save the linker file to: %1 Complete!").arg(linker_name));
 }




