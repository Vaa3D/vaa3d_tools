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
    V3DLONG para1=10;     //resample para
    V3DLONG para2=5;        //lens para
    V3DLONG para3=1;        //step para
    V3DLONG para4=0;        //prune para
    bool model1 = 0;       //1 for.v3dpbd,0 for terafly
    bool model2 = 1;        //1 for choose point ,0 for local alignment
    V3DLONG model3 = 1;      //0 for 1 class.1 for 2 class,2 for 3 class
};
bool sort_with_standard(QList<NeuronSWC> & neuron1, QList<NeuronSWC>  & neuron2,QList<NeuronSWC> &result);
vector<MyMarker*> nt2mm(QList<NeuronSWC> & inswc, QString fileSaveName);
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName);
void SplitString(const string& s, vector<string>& v, const string& c);
bool find_wrong_area(Input_para &PARA,V3DPluginCallback2 &callback,bool bmenu,QWidget *parent);
QList<NeuronSWC> choose_alignment(QList<NeuronSWC> &neuron,QList<NeuronSWC> &gold,double thres1,double thres2);
NeuronTree mm2nt(vector<MyMarker*> & inswc, QString fileSaveName);
double calculate_diameter(NeuronTree nt, vector<V3DLONG> branches);
bool prune_branch(NeuronTree &nt, NeuronTree & result, double prune_size);


QList<NeuronSWC> match_point(QList<NeuronSWC> &swc1,QList<NeuronSWC> &swc2);



#endif // FIND_WRONG_AREA_H
