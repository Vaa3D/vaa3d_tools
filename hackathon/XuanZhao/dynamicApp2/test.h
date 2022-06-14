#ifndef TEST_H
#define TEST_H

#include "v3d_interface.h"
#include <vector>
#include <queue>

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

using namespace std;

float calPrecision(NeuronTree& traced, NeuronTree& manual, XYZ origin, float d_thres);

void setNeuronTreeHash(NeuronTree &nt);

NeuronTree getLongPath(NeuronTree& nt, XYZ origin);

NeuronTree getChildTree(NeuronTree& nt, XYZ origin);




#endif // TEST_H
