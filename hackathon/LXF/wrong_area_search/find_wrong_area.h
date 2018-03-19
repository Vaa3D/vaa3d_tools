#ifndef FIND_WRONG_AREA_H
#define FIND_WRONG_AREA_H
#include"sort_swc.h"
#include"basic_surf_objs.h"
#include "my_surf_objs.h"
#include "neuron_tree_align.h"
#include<v3d_interface.h>
#include<vector>
struct Input_para
{
    QString filename1;
    QString filename2;
    QString filename3;
    V3DLONG para1=3;
    V3DLONG para2=3;
};
bool sort_with_standard(QList<NeuronSWC> & neuron1, QList<NeuronSWC>  & neuron2,QList<NeuronSWC> &result);
vector<MyMarker*> nt2mm(QList<NeuronSWC> & inswc, QString fileSaveName);
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName);
void SplitString(const string& s, vector<string>& v, const string& c);
bool find_wrong_area(Input_para &PARA,V3DPluginCallback2 &callback,bool bmenu,QWidget *parent);
QList<NeuronSWC> choose_alignment(QList<NeuronSWC> &neuron,QList<NeuronSWC> &gold,double thres1,double thres2);
NeuronTree mm2nt(vector<MyMarker*> & inswc, QString fileSaveName);


QList<NeuronSWC> match_point(QList<NeuronSWC> &swc1,QList<NeuronSWC> &swc2);



#endif // FIND_WRONG_AREA_H
