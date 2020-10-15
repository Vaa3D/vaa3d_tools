#ifndef PRUNING_H
#define PRUNING_H

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

#include <vector>

#include "v3d_interface.h"

using namespace std;

void pruningCross(NeuronTree& nt);

void pruningCross2(NeuronTree& nt, double bifurcationD);

NeuronTree pruningSoma(NeuronTree& nt, double times);

NeuronTree pruningByLength(NeuronTree& nt, double lengthT);

void getAdjacentChildSegment(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD);

NeuronTree pruningByType(NeuronTree& nt, int type);

void pruningCross3(NeuronTree& nt, double bifurcationD);

void getChildSegment(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD);

void pruningSegmentByAngle(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD, int* isRemain);

void getAdjacentChildSegment(NeuronTree& nt, vector<vector<V3DLONG> > children, vector<vector<V3DLONG> > &cbs, V3DLONG t, double bifurcationD, int* isRemain);

NeuronTree pruningInit(NeuronTree& nt, unsigned char *pdata, long long *sz, double bifurcationD, double somaTimes);

void getHierarchySegmentLength(NeuronTree& nt, ofstream &csvFile, unsigned char *pdata, long long *sz);



#endif // PRUNING_H
