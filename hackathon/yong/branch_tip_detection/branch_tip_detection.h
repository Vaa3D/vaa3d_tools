#ifndef BRANCH_TIP_DETECTION_H
#define BRANCH_TIP_DETECTION_H

#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "v3d_interface.h"

//struct Point;
//struct Point
//{
//    double x,y,z,r;
//    V3DLONG type;
//    Point* p;
//    V3DLONG childNum;
//};
//typedef vector<Point*> Tree;
//typedef vector<Point*> subtree;


//void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs);
bool branch_tip_detection(V3DPluginCallback2 &callback,QList<NeuronSWC> & neuron,QString fileOpenName,QWidget *parent);
//bool removedupSWC(QList<NeuronSWC> & neuron,QList<NeuronSWC> &result);
QList<NeuronSWC> removedupSWC(QList<NeuronSWC> & neuron,QString fileOpenName);
bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);


#endif // BRANCH_TIP_DETECTION_H
