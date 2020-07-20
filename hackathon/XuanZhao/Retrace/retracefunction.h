#ifndef RETRACEFUNCTION_H
#define RETRACEFUNCTION_H

#include <vector>

#include "v3d_interface.h"

#define INF 3.4e+38

static NeuronTree finalResult;

static int colorNum = 48;

static unsigned char basicColorTable[][3] = {
    {  0,   0,    0},
    {170,   0,    0},
    {  0,  85,    0},
    {170,  85,    0},
    {  0, 170,    0},
    {170, 170,    0},
    {  0, 255,    0},
    {170, 255,    0},
    {  0,   0,  127},
    {170,   0,  127},
    {  0,  85,  127},
    {170,  85,  127},
    {  0, 170,  127},
    {170, 170,  127},
    {  0, 255,  127},
    {170, 255,  127},
    {  0,   0,  255},
    {170,   0,  255},
    {  0,  85,  255},
    {170,  85,  255},
    {  0, 170,  255},
    {170, 170,  255},
    {  0, 255,  255},
    {170, 255,  255},
    { 85,   0,    0},
    {255,   0,    0},
    { 85,  85,    0},
    {255,  85,    0},
    { 85, 170,    0},
    {255, 170,    0},
    { 85, 255,    0},
    {255, 255,    0},
    { 85,   0,  127},
    {255,   0,  127},
    { 85,  85,  127},
    {255,  85,  127},
    { 85, 170,  127},
    {255, 170,  127},
    { 85, 255,  127},
    {255, 255,  127},
    { 85,   0,  255},
    {255,   0,  255},
    { 85,  85,  255},
    {255,  85,  255},
    { 85, 170,  255},
    {255, 170,  255},
    { 85, 255,  255},
    {255, 255,  255}
};

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

XYZ getLineDirection(const vector<NeuronSWC> &points);

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result);

bool sortSWC(NeuronTree& nt);

//bool operator<(const Triple& p1, const Triple& p2);

struct Triple{
    unsigned char r, g, b;
    Triple(){
        r = g = b = 0;
    }
    Triple(unsigned char r, unsigned char g, unsigned char b){
        this->r = r;
        this->g = g;
        this->b = b;
    }
//    ~Triple();

    bool operator <(const Triple& other) const;
//    friend operator<(const Triple& p1, const Triple& p2);
};

void changeContrast(unsigned char* input, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, double times);

vector<vector<LocationSimple> > getApp2InMarkers(QList<CellAPO> markers);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

void deleteSameBranch(NeuronTree& target, const NeuronTree& ori);

void deleteSameBranch(NeuronTree &target, const NeuronTree &ori, double dis);

NeuronTree retrace(QString apoPath, QString eswcPath, QString brainDir, int resolution, V3DPluginCallback2 &callback);

void app2Terafly(int type, bool threshold, int app2Th, double contrastT, V3DPluginCallback2& callback, QWidget *parent);

void app2MultiTerafly(int type, bool threshold, int app2Th, double contrastT, V3DPluginCallback2& callback, QWidget *parent);


void ultratracerTerafly(int type, V3DPluginCallback2& callback, QWidget *parent);

void deleteBranchByDirection(NeuronTree& target, int direction);

QList<LandmarkList> group_tips(LandmarkList tips,int block_size, int direction);

struct imageBlock{
    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    int overlap;
    int direction;
    LandmarkList startMarkers;
    imageBlock(){
        start_x = start_y = start_z = end_x = end_y = end_z = 0;
        overlap = 10;
        direction = 0;
    }

    imageBlock(V3DLONG x0, V3DLONG x1, V3DLONG y0, V3DLONG y1, V3DLONG z0, V3DLONG z1, int overlap = 10, int direction = 0){
        start_x = x0;
        start_y = y0;
        start_z = z0;
        end_x = x1;
        end_y = y1;
        end_z = z1;
        this->overlap = overlap;
        this->direction = direction;
    }

    vector<NeuronTree> getNeuronTrees(QString brainPath, V3DPluginCallback2 &callback);
    void getGlobelNeuronTrees(vector<NeuronTree>& trees);
    void getTipBlocks(vector<NeuronTree>& trees, BoundingBox box, vector<imageBlock>& blockList);

    void getTipBlocks(NeuronTree& tree, BoundingBox box, vector<imageBlock>& blockList);

    NeuronTree getNeuronTree(QString brainPath, V3DPluginCallback2 &callback);

    NeuronTree cutBlockSWC(const NeuronTree& nt);

    void getGlobelNeuronTree(NeuronTree& nt);
    void getLocalNeuronTree(NeuronTree& nt);
};

void normalImage(unsigned char* pdata, V3DLONG* sz);

int* getThresholdByKmeans(unsigned char* pdata, V3DLONG* sz, int k);
int *getThresholdByKmeans(Image4DSimple* image, int k);

void getImageBlockByTips(LandmarkList tips, vector<imageBlock>& blockList, int block_size, int direction, BoundingBox box);

NeuronTree ultratracerInBox(QString brainPath, imageBlock block, NeuronTree ori, BoundingBox box, V3DPluginCallback2& callback);

BoundingBox getGlobalBoundingBox(V3DPluginCallback2 &callback);

imageBlock getFirstBlock(V3DPluginCallback2& callback, BoundingBox box);

int getDirection(XYZ p);

void getImageBlockByTip(LocationSimple tip, vector<imageBlock> &blockList, int block_size, int direction, BoundingBox box);

void getMainTree(NeuronTree& nt);

NeuronTree ultratracerInBox2(QString brainPath, imageBlock block, NeuronTree ori, BoundingBox box, V3DPluginCallback2& callback);

void mergeFinalResult(NeuronTree nt);

bool writeBlock(V3DPluginCallback2 &callback, QWidget* parent);

bool readBlocks(const QString& filename, vector<imageBlock>& blocks, vector<BoundingBox>& boxs, QString &brainPath);

bool tracingPipeline(QString imageBlockPath, QString swcFile, V3DPluginCallback2& callback);


void mirrorImage(unsigned char* pdata, V3DLONG* inSZ, int mirrorDirection);

void rotateImage(unsigned char* inData, unsigned char* &outData, V3DLONG* inSZ, V3DLONG* outSZ, int rotateAxis, int angle, int &minX, int &minY, int &minZ);

NeuronTree getApp2RotateImage(Image4DSimple *image, LocationSimple m, int rotateAxis, int angle, int th);

NeuronTree getApp2WithParameter(Image4DSimple* image, LocationSimple m, int downSampleTimes, int mirrorDirection, int th, int type);

vector<NeuronTree> getApp2NeuronTrees(int app2Th, Image4DSimple *image, LocationSimple m);

NeuronTree consensus(vector<NeuronTree> trees, Image4DSimple *inImg, LocationSimple m, V3DPluginCallback2& callback);

NeuronTree consensus(Image4DSimple* image, LocationSimple m, bool kmeansTh, V3DPluginCallback2& callback);

#endif // RETRACEFUNCTION_H
