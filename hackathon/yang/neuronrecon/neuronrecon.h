// "optimal constructing of neuron trees from voxels"
// -Yang
// 09/12/2017


#ifndef _NEURONRECON_H_
#define _NEURONRECON_H_

//
#include <QtGlobal>
#include <vector>
#include "v3d_interface.h"
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include <iostream>
#include "basic_4dimage.h"
#include "algorithm"
#include <string.h>
#include <cmath>
#include <climits>
#include <numeric>
#include <algorithm>
#include <string>

using namespace std;

//
class Point
{
public:
    Point();
    ~Point();

public:
    void setLocation(float x, float y, float z);
    void setRadius(float r);
    void setValue(float v);

public:
    float x, y, z; // location
    float radius; // radius
    float val; // intensity value
    vector<V3DLONG> parents;
    vector<V3DLONG> children;
    V3DLONG n; // #
    bool visited;
    int type; // -1 cell body; 0 tip point; 1 regular point; 3 branch point
};

//
class PointCloud
{
public:
    PointCloud();
    ~PointCloud();

public:
    int getPointCloud(QStringList files);
    int savePointCloud(QString filename);
    int savePC2SWC(PointCloud pc, QString filename);
    int addPointFromNeuronTree(NeuronTree nt);

    float distance(Point a, Point b);

    int getBranchPoints(QString filename);
    int getNeurites(QString filename);

    int resample();

public:
    vector<Point> points;
};

#endif // _NEURONRECON_H_
