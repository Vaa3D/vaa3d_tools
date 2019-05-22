#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

#include "some_class.h"



bool isRoot(NeuronTree nt,V3DLONG par);

QVector<QVector<V3DLONG>> getChildren(NeuronTree nt);

block getBlockOfOPoint(NeuronSWC p,int dx,int dy,int dz);

vector<NeuronSWC> getTPointOfBlock(NeuronTree& nt,block b);

blockTree getBlockTree(NeuronTree& nt,int dx,int dy,int dz);

void work(PARA_DEMO2 &p, V3DPluginCallback2 &callback);





































#endif // SOME_FUNCTION_H
