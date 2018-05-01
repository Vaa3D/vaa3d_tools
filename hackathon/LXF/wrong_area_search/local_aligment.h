#ifndef LOCAL_ALIGMENT_H
#define LOCAL_ALIGMENT_H
#include "neuron_tree_align.h"
#include<time.h>

template<class T> double neuron_tree_align(vector<T*> &tree1, vector<T*> &tree2, vector<double> & w, vector<pair<int, int> > & result);
void merge_multi_match(vector<pair<int, int> > & result, vector<vector<int> > & pairs_merged1, vector<vector<int> > & pairs_merged2);
bool neuron_mapping_dynamic(vector<MyMarker*> &inswc1, vector<MyMarker*> & inswc2, vector<map<MyMarker*, MyMarker*> > & result_map); //result_map[i] is the mapping result of the ith segment
void convert_matchmap_2swc(vector<map<MyMarker*, MyMarker*> > & inmap, vector<MyMarker*> & outswc);
#endif // LOCAL_ALIGMENT_H
