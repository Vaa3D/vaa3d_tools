#ifndef FIND_WRONG_AREA_H
#define FIND_WRONG_AREA_H
#include"sort_swc.h"
#include"basic_surf_objs.h"
#include "my_surf_objs.h"
#include "neuron_tree_align.h"
#include<v3d_interface.h>
#include<vector>
bool sort_with_standard(QList<NeuronSWC> & neuron1, QList<NeuronSWC>  & neuron2,QList<NeuronSWC> &result);
vector<MyMarker*> nt2mm(QList<NeuronSWC> & inswc, QString fileSaveName);
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName);
bool find_wrong_area(const V3DPluginArgList & input, V3DPluginArgList & output);
QList<NeuronSWC> choose_long_alignment(QList<NeuronSWC> &neuron,double thres);

#endif // FIND_WRONG_AREA_H
