#ifndef AXONTRACE_H
#define AXONTRACE_H

#include <vector>

#include "v3d_interface.h"

#define INF 3.4e+38

using namespace std;

static NeuronTree finalResult;

static QString tmpImageDir = "";

void setTmpImageDir(QString imageDir);

template <class T>
void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
    V3DLONG i, j,k,n,count;
    double t, temp;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                V3DLONG curpos1 = i* mCount + j*iImageWidth;
                V3DLONG curpos2 = j* iImageWidth + k;
                temp = 0;
                count = 0;
                for(n =1 ; n <= d  ;n++)
                {
                    if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}
                    if (k+(h*n)< iImageWidth) { temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
                    if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
                    if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//
                    if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
                }
                t =  apsInput[curpos]-temp/(count);
                aspOutput[curpos]= (t > 0)? t : 0;
            }
        }
    }
}

void BinaryProcess(unsigned char* &apsInput, V3DLONG *in_sz);


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

    XYZ getLastDirection();


};

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, float d = 1);

bool sortSWC(NeuronTree& nt, float d = 1);

NeuronTree ultratracerAxon(QString brainPath, BoundingBox box, NeuronTree ori, V3DPluginCallback2& callback);

void ultratracerAxonTerafly(V3DPluginCallback2 &callback, QWidget *parent);

BoundingBox getGlobalBoundingBox(V3DPluginCallback2 &callback);

BoundingBox getGlobalBoundingBox(QString brainPath);

imageBlock getFirstBlock(V3DPluginCallback2& callback, BoundingBox box);

void getImageBlockByTip(LocationSimple tip, vector<imageBlock> &blockList, int block_size, BoundingBox box);

void getMainTree(NeuronTree& nt);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

void mergeFinalResult(NeuronTree nt);

NeuronTree ultratracerAxonTerafly(QString brainPath, NeuronTree ori, QString imageDir, V3DPluginCallback2 &callback);


#endif // AXONTRACE_H
