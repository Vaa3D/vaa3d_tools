#ifndef __HIERARCHY_PRUNE_H__
#define __HIERARCHY_PRUNE_H__

#include <map>
#include <set>
#include "smooth_curve.h"
using namespace std;

#define INTENSITY_DISTANCE_METHOD 0
#define __USE_APP_METHOD__
struct HierarchySegment
{
	HierarchySegment * parent;
	MyMarker * leaf_marker;
	MyMarker * root_marker;      // its parent marker is in current segment's parent segment
	double length;               // the length from leaf to root
	int level;                   // the segments number from leaf to root

	HierarchySegment()
	{
		leaf_marker = 0;
		root_marker = 0;
		length = 0;
		level = 1;
		parent = 0;
	}
	HierarchySegment(MyMarker * _leaf, MyMarker * _root, double _len, int _level)
	{
		leaf_marker = _leaf;
		root_marker = _root;
		length = _len;
		level = _level;
		parent = 0;
	}

	void get_markers(vector<MyMarker*> & outswc)
	{
		if(!leaf_marker || !root_marker) return;
		MyMarker * p = leaf_marker;
		while(p != root_marker)
		{
			outswc.push_back(p);
			p = p->parent;
		}
		outswc.push_back(root_marker);
	}
};

// There is no overlap between HierarchySegment
template<class T> bool swc2topo_segs(vector<MyMarker*> & inswc, vector<HierarchySegment*> & topo_segs,
int length_method = INTENSITY_DISTANCE_METHOD, T * inimg1d = 0, long sz0 = 0, long sz1 = 0, long sz2 = 0);


//bool topo_segs2swc(vector<HierarchySegment*> & topo_segs, vector<MyMarker*> & outmarkers, int swc_type = 1);

// 1. will change the type of each segment
// swc_type : 0 for length heatmap, 1 for level heatmap
bool topo_segs2swc1(vector<HierarchySegment*> & topo_segs, vector<MyMarker*> & outmarkers, int swc_type = 1);


template<class T> bool hierarchy_prune(vector<MyMarker*> &inswc, vector<MyMarker*> & outswc, T * inimg1d, long sz0, long sz1, long sz2, double length_thresh = 10.0);


// hierarchy coverage pruning
template<class T> bool happ(vector<MyMarker*> &inswc, vector<MyMarker*> & outswc,
 T * inimg1d, long sz0, long sz1, long sz2, double bkg_thresh = 10.0,
 double length_thresh = 2.0, double SR_ratio = 1.0/9.0, bool is_leaf_prune = true, bool is_smooth = true);


#endif
