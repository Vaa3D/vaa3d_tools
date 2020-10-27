#ifndef AXONTRACE_H
#define AXONTRACE_H

#include <vector>

#include "v3d_interface.h"

#define INF 3.4e+38

using namespace std;

static NeuronTree finalResult;

struct imageBlock{
    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    int overlap;
    LocationSimple startMarker;

    imageBlock(){
        start_x = start_y = start_z = end_x = end_y = end_z = 0;
        overlap = 10;
    }

    imageBlock(V3DLONG x0, V3DLONG x1, V3DLONG y0, V3DLONG y1, V3DLONG z0, V3DLONG z1, int overlap = 10){
        start_x = x0;
        start_y = y0;
        start_z = z0;
        end_x = x1;
        end_y = y1;
        end_z = z1;
        this->overlap = overlap;
    }

    void getGlobelNeuronTree(NeuronTree& nt);
    void getLocalNeuronTree(NeuronTree& nt);

    NeuronTree cutBlockSWC(const NeuronTree& nt);

    NeuronTree getMaskTree();

    void getMaskImage(unsigned char* inimg, unsigned char* &outimg, V3DLONG* sz, double r);

    NeuronTree getNeuronTree(QString brainPath, V3DPluginCallback2 &callback, double maskR);

    void getTipBlock(NeuronTree& tree, BoundingBox box, vector<imageBlock>& blockList);


};

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result);

bool sortSWC(NeuronTree& nt);

NeuronTree ultratracerAxon(QString brainPath, BoundingBox box, NeuronTree ori, V3DPluginCallback2& callback);

void ultratracerAxonTerafly(V3DPluginCallback2 &callback, QWidget *parent);

BoundingBox getGlobalBoundingBox(V3DPluginCallback2 &callback);

imageBlock getFirstBlock(V3DPluginCallback2& callback, BoundingBox box);

void getImageBlockByTip(LocationSimple tip, vector<imageBlock> &blockList, int block_size, BoundingBox box);

void getMainTree(NeuronTree& nt);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

void mergeFinalResult(NeuronTree nt);


#endif // AXONTRACE_H
