// May 8, 2012   by Hang Xiao
#ifndef __SWC_UTILS_H__
#define __SWC_UTILS_H__
#include "my_surf_objs.h"

struct NeuronSegment
{
    vector<NeuronSegment*> child_list;
    vector<MyMarker*> markers;
};

bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<NeuronSegment*> &tree);

#endif
