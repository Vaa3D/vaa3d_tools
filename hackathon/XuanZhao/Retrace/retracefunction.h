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

#endif // RETRACEFUNCTION_H
