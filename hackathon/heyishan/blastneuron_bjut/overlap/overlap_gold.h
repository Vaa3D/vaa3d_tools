#ifndef OVERLAP_GOLD_H
#define OVERLAP_GOLD_H

#include<v3d_interface.h>
#include<vector>
#include "my_surf_objs.h"
#include"v3d_message.h"
#include<QtGui>
bool overlap_gold(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName);
bool evaluate_radius(vector<MyMarker*> & inswc, QString raw_img, V3DPluginCallback2 &callback);
bool prune_short_tree(vector<MyMarker*> & inswc, vector<MyMarker*> & outswc, double prune_thres);
QStringList importFileList_addnumbersort(const QString & curFilePath);
vector<MyMarker*> nt2mm(QList<NeuronSWC> & inswc, QString fileSaveName);
NeuronTree mm2nt(vector<MyMarker*> & inswc, QString fileSaveName);
bool inter_node_pruning(NeuronTree & nt, vector<MyMarker*>& final_out_swc, vector<MyMarker*> &final_out_swc_updated);
#endif // OVERLAP_GOLD_H
