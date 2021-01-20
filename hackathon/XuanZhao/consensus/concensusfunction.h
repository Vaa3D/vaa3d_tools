#ifndef CONCENSUSFUNCTION_H
#define CONCENSUSFUNCTION_H

#include "v3d_interface.h"

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

struct partialFeature
{
    double Length = 0, Max_Path = 0;
    int Max_Order = 0, N_bifs = 0, N_branch = 0, N_tips = 0;
    partialFeature() {
        Length = Max_Path = 0;
        Max_Order = N_bifs = N_branch = N_tips = 0;
    }
};

partialFeature computePartialFeature(NeuronTree &nt);

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result);
bool sortSWC(NeuronTree& nt);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

void mirrorImage(unsigned char* pdata, V3DLONG* inSZ, int mirrorDirection);

void rotateImage(unsigned char* inData, unsigned char* &outData, V3DLONG* inSZ, V3DLONG* outSZ, int rotateAxis, int angle, int &minX, int &minY, int &minZ);

NeuronTree getApp2WithParameter(V3DPluginCallback2& callback, int downSampleTimes, int mirrorDirection, int th, int type);

vector<NeuronTree> getApp2NeuronTrees(int app2Th, V3DPluginCallback2& callback, QWidget* parent);

NeuronTree consensus(vector<NeuronTree> trees, Image4DSimple *inImg, LandmarkList m, V3DPluginCallback2& callback);

int* getThresholdByKmeans(unsigned char* pdata, V3DLONG* sz, int k);
int *getThresholdByKmeans(Image4DSimple* image, int k);

NeuronTree getApp2RotateImage(Image4DSimple *image, LandmarkList m, int rotateAxis, int angle, int th, bool b_256cube);

void normalImage(unsigned char* pdata, V3DLONG* sz);

bool consensus(QString imagePath, LandmarkList m, bool kmeansTh, V3DPluginCallback2& callback);

#endif // CONCENSUSFUNCTION_H
