#ifndef __NEURON_TREE_ALIGN_H__
#define __NEURON_TREE_ALIGN_H__
#include "swc_utils.h"
#include "seg_weight.h"
#include <algorithm>

// make sure tree1 and tree2 are bottom up order
struct SetIndexPair
{
	short int ind1;
	short int ind2;
	short int set1; // 0,1,2,3
	short int set2; // 0,1,2,3
	bool operator<(const SetIndexPair & others) const
	{ 
		if (ind1<others.ind1) return true;
		if (ind1>others.ind1) return false;
		if (ind2<others.ind2) return true;
		if (ind2>others.ind2) return false;
		if (set1<others.set1) return true;
		if (set1>others.set1) return false;
		if (set2<others.set2) return true;
		if (set2>others.set2) return false;
		return false;
	}
};


template<class T> double neuron_tree_align(vector<T*> &tree1, vector<T*> &tree2, vector<double> & w, vector<pair<int, int> > & result);

void merge_multi_match(vector<pair<int, int> > & result, vector<vector<int> > & pairs_merged1, vector<vector<int> > & pairs_merged2);

void convert_matchmap_2swc(vector<map<MyMarker*, MyMarker*> > & inmap, vector<MyMarker*> & outswc);

bool neuron_mapping_dynamic(vector<MyMarker*> &inswc1, vector<MyMarker*> & inswc2, vector<map<MyMarker*, MyMarker*> > & result_map);

#endif
