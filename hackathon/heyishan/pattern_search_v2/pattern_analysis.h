#ifndef PATTERN_ANALYSIS_H
#define PATTERN_ANALYSIS_H

#include "v3d_message.h"
#include "basic_surf_objs.h"
#include <v3d_interface.h>
#include <vector>
using namespace std;

struct Boundary
{
    float minx;
    float miny;
    float minz;
    float maxx;
    float maxy;
    float maxz;
};

double dist_p2p(double x1, double y1, double z1, double x2, double y2, double z2);

bool pattern_analysis(const NeuronTree &nt,const NeuronTree &boundary,vector<NeuronTree> & pt_list, vector<double> & pt_lens, vector<int>& pt_nums);
NeuronTree fill_boundary(const NeuronTree &nt, const Boundary & b);
NeuronTree rmSmallPart(NeuronTree & nt_sorted);
#endif // PATTERN_ANALYSIS_H
