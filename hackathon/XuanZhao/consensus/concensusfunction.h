#ifndef CONCENSUSFUNCTION_H
#define CONCENSUSFUNCTION_H

#include "v3d_interface.h"

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result);
bool sortSWC(NeuronTree& nt);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

void mirrorImage(unsigned char* pdata, V3DLONG* inSZ, int mirrorDirection);

void rotateImage(unsigned char* inData, unsigned char* outData, V3DLONG* inSZ, V3DLONG* outSZ, int rotateAxis, int angle);

NeuronTree getApp2WithParameter(V3DPluginCallback2& callback, int downSampleTimes, int mirrorDirection, int th, int type);

vector<NeuronTree> getApp2NeuronTrees(int app2Th, V3DPluginCallback2& callback, QWidget* parent);

NeuronTree consensus(vector<NeuronTree> trees, Image4DSimple *inImg, LandmarkList landMarkers, V3DPluginCallback2& callback);

int* getThresholdByKmeans(unsigned char* pdata, V3DLONG* sz, int k);
int *getThresholdByKmeans(Image4DSimple* image, int k);

#endif // CONCENSUSFUNCTION_H
