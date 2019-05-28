#ifndef CUT_IMAGE_F_H
#define CUT_IMAGE_F_H

#include "cut_image_p.h"



bool isRoot(NeuronTree& nt,V3DLONG par);

QVector<QVector<V3DLONG>> getChildren(NeuronTree& nt);

block getBlockOfOPoint(NeuronSWC& p,int dx,int dy,int dz);

vector<NeuronSWC> getTPointOfBlock(NeuronTree& nt,block b);

blockTree getBlockTree(NeuronTree& nt,int dx,int dy,int dz);

bool getTif_Eswc_Marker(parameter1& p,V3DPluginCallback2 &callback);






















#endif // CUT_IMAGE_F_H
