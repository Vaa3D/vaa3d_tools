#ifndef RESAMPLE_SWC_H
#define RESAMPLE_SWC_H
#include <vector>
#include "basic_surf_objs.h"

using namespace std;

struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
};

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;


void resample_path(Segment * seg, double step);
NeuronTree resample(NeuronTree input, double step);


#endif
