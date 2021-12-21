// May 8, 2012   by Hang Xiao
#ifndef __SWC_UTILS_H__
#define __SWC_UTILS_H__
#include "my_surf_objs.h"

struct NeuronSegment
{
	vector<NeuronSegment*> child_list;
	vector<MyMarker*> markers;
};

// convert inmarkers to many connected neuron structure
void swc_decompose(vector<MyMarker*> & inmarkers, vector<vector<MyMarker*> > & out_segs);
bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<vector<MyMarker*> > & outsegs, vector<int> & seg_par);
bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<NeuronSegment*> &tree);
vector<MyMarker*> getRoots(vector<MyMarker*> & inswc);
bool reroot(vector<MyMarker*> & sbjswc, MyMarker* new_root);
double path_dist_root(vector<MyMarker*> & inswc, MyMarker* node);
double seg_length(vector<MyMarker*> & seg);
bool get_far_ends(vector<MyMarker*> & inswc, vector<MyMarker*> & ends);

#endif
