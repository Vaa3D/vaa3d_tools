#ifndef RETRACEFUNCTION_H
#define RETRACEFUNCTION_H

#include <vector>

#include "v3d_interface.h"

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


vector<vector<LocationSimple> > getApp2InMarkers(QList<CellAPO> markers);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

void deleteSameBranch(NeuronTree& target, const NeuronTree& ori, double thres);

NeuronTree retrace(QString apoPath, QString eswcPath, QString brainDir, int resolution, V3DPluginCallback2 &callback);

void app2Terafly(int type, bool threshold, int app2Th, V3DPluginCallback2& callback, QWidget *parent);

void app2MultiTerafly(int type, bool threshold, int app2Th, V3DPluginCallback2& callback, QWidget *parent);

#endif // RETRACEFUNCTION_H
