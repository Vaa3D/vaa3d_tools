#ifndef BRANCH_TIP_DETECTION_H
#define BRANCH_TIP_DETECTION_H

#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "v3d_interface.h"

#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

#ifndef VOID
#define VOID 1000000000
#endif

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
//void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs);
bool branch_tip_detection(V3DPluginCallback2 &callback,QList<NeuronSWC> & neuron,QString fileOpenName,QWidget *parent);
//QList<NeuronSWC> removedupSWC(QList<NeuronSWC> & neuron,QString fileOpenName);
QList<NeuronSWC> SortSWC(QList<NeuronSWC> & neuron,V3DLONG newrootid, double thres, double root_dist_thres, QList<CellAPO> markers,QString fileOpenName);
V3DLONG down_child(QList<NeuronSWC> result,vector<vector<V3DLONG> > childs,V3DLONG i);


#endif // BRANCH_TIP_DETECTION_H
