#ifndef __HIERARCHY_PRUNE_H__
#define __HIERARCHY_PRUNE_H__

#include <map>
#include <set>
#include "smooth_curve.h"
#include "my_surf_objs.h"
#include "marker_radius.h"
using namespace std;

#define INTENSITY_DISTANCE_METHOD 0
#define __USE_APP_METHOD__
struct HierarchySegment
{
	HierarchySegment * parent;
	MyMarker * leaf_marker;

	MyMarker * root_marker;      
	double length;              
	int level;                   

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
template<class T> bool swc2topo_segs(vector<MyMarker*> & inswc, vector<HierarchySegment*> & topo_segs, int length_method = INTENSITY_DISTANCE_METHOD, T * inimg1d = 0, long sz0 = 0, long sz1 = 0, long sz2 = 0)
{
	if(length_method == INTENSITY_DISTANCE_METHOD && (inimg1d == 0 || sz0 == 0 || sz1 == 0 || sz2 == 0))
	{
		cerr<<"need image input for INTENSITY_DISTANCE_METHOD "<<endl;
		return false;
	}
	// 1. calc distance for every nodes
	int tol_num = inswc.size();
	map<MyMarker*, int> swc_map; for(int i = 0; i < tol_num; i++) swc_map[inswc[i]] = i;
	
	vector<MyMarker*> leaf_markers;
	//GET_LEAF_MARKERS(leaf_markers, inswc);
	vector<int> childs_num(tol_num); 
	{
		for(int i = 0; i < tol_num; i++) childs_num[i]=0;
		for(int m1 = 0; m1 < tol_num; m1++)
		{
			if(!inswc[m1]->parent) continue;
			int m2 = swc_map[inswc[m1]->parent];
			childs_num[m2]++;
		}
		for(int i = 0; i < tol_num; i++) if(childs_num[i] == 0) leaf_markers.push_back(inswc[i]);
	}
	int leaf_num = leaf_markers.size();

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;

	vector<double> topo_dists(tol_num, 0.0);  // furthest leaf distance for each tree node,initializing and save distance of segment later
	vector<MyMarker*> topo_leafs(tol_num, (MyMarker*)0);//initializing, save the leaf and constitue a leaf vector

	for(int i = 0; i < leaf_num; i++)
	{
		MyMarker * leaf_marker = leaf_markers[i];
		MyMarker * child_node = leaf_markers[i];
		MyMarker * parent_node = child_node->parent;
		int cid = swc_map[child_node];
		topo_leafs[cid] = leaf_marker;
		topo_dists[cid] = (length_method == INTENSITY_DISTANCE_METHOD) ? inimg1d[leaf_marker->ind(sz0, sz01)]/255.0 : 0; 
		while(parent_node)
		{
			int pid = swc_map[parent_node];
			double tmp_dst = (length_method == INTENSITY_DISTANCE_METHOD) ? (inimg1d[parent_node->ind(sz0, sz01)]/255.0 + topo_dists[cid]) : (dist(*child_node, *parent_node) + topo_dists[cid]);
			if(tmp_dst >= topo_dists[pid])   // >= instead of >
			{
				topo_dists[pid] = tmp_dst;
				topo_leafs[pid] = topo_leafs[cid];
			}
			else break;
			child_node = parent_node;
			cid = pid;
			parent_node = parent_node->parent;
		}
	}
	// 2. create Hierarchy Segments
	topo_segs.resize(leaf_num);
	map<MyMarker *, int> leaf_ind_map;
	for(int i = 0; i < leaf_num; i++)
	{
		topo_segs[i] = new HierarchySegment();
		leaf_ind_map[leaf_markers[i]] = i;
	}

	for(int i = 0; i < leaf_num; i++)
	{
		MyMarker * leaf_marker = leaf_markers[i];
		MyMarker * root_marker = leaf_marker;
		MyMarker * root_parent = root_marker->parent;
		int level = 1;
		while(root_parent && topo_leafs[swc_map[root_parent]] == leaf_marker)
		{
			if(childs_num[swc_map[root_marker]] >= 2) level++;
			root_marker = root_parent;
			root_parent = root_marker->parent;
		}

		double dst = topo_dists[swc_map[root_marker]];

		HierarchySegment * topo_seg = topo_segs[i];
		*topo_seg = HierarchySegment(leaf_marker, root_marker, dst, level); 

		if(root_parent == 0) topo_seg->parent = 0;
		else
		{
			MyMarker * leaf_marker2 = topo_leafs[swc_map[root_parent]];
			int leaf_ind2 = leaf_ind_map[leaf_marker2];
			topo_seg->parent = topo_segs[leaf_ind2];
		}
	}

	swc_map.clear();
	leaf_markers.clear();
	leaf_ind_map.clear();
	topo_dists.clear();
	topo_leafs.clear();
	return true;
}

// 1. will change the type of each segment
// swc_type : 0 for length heatmap, 1 for level heatmap
bool topo_segs2swc(vector<HierarchySegment*> & topo_segs, vector<MyMarker*> & outmarkers, int swc_type = 1) 
{
	if(topo_segs.empty()) return false;

	double min_dst = topo_segs[0]->length, max_dst = min_dst;
	double min_level = topo_segs[0]->level, max_level = min_level;
	for(int i = 0; i < topo_segs.size(); i++)
	{
		double dst = topo_segs[i]->length;
		min_dst = MIN(min_dst, dst);
		max_dst = MAX(max_dst, dst);

		int level = topo_segs[i]->level;
		min_level = MIN(min_level, level);
		max_level = MAX(max_level, level);
	}
	max_level = MIN(max_level, 20);                         // todo1

	cout<<"min_dst = "<<min_dst<<endl;
	cout<<"max_dst = "<<max_dst<<endl;
	cout<<"min_level = "<<min_level<<endl;
	cout<<"max_level = "<<max_level<<endl;


	max_dst -= min_dst; if(max_dst == 0.0) max_dst = 0.0000001;
	max_level -= min_level; if(max_level == 0) max_level = 1.0;
	for(int i = 0; i < topo_segs.size(); i++)
	{
		double dst = topo_segs[i]->length;
		int level = MIN(topo_segs[i]->level, max_level);    // todo1
		int color_id = (swc_type == 0) ? (dst - min_dst) / max_dst * 254.0 + 20.5 : (level - min_level)/max_level * 254.0 + 20.5;
		vector<MyMarker*> tmp_markers;
		topo_segs[i]->get_markers(tmp_markers);
		for(int j = 0; j < tmp_markers.size(); j++)
		{
			tmp_markers[j]->type = color_id;
		}
		outmarkers.insert(outmarkers.end(), tmp_markers.begin(), tmp_markers.end());
	}
	return true;
}

template<class T> bool hierarchy_prune(vector<MyMarker*> &inswc, vector<MyMarker*> & outswc, T * inimg1d, long sz0, long sz1, long sz2, double length_thresh = 10.0)
{
	vector<HierarchySegment*> topo_segs;
	swc2topo_segs(inswc, topo_segs, INTENSITY_DISTANCE_METHOD, inimg1d, sz0, sz1, sz2);
	vector<HierarchySegment*> filter_segs;
//	if(length_thresh <= 0.0)
//	{
//		vector<short int> values;
//		for(int i = 0; i < topo_segs.size(); i++)
//		{
//			values.push_back(topo_segs[i]->length * 1000 + 0.5);
//		}
//		cout<<"segment num = "<<values.size()<<endl;
//		length_thresh = otsu_threshold(values) / 1000.0;
//		cout<<"otsu length = "<<length_thresh<<endl;
//	}
	for(int i = 0; i < topo_segs.size(); i++)
	{
		if(topo_segs[i]->length >= length_thresh) filter_segs.push_back(topo_segs[i]);
		//if(topo_segs[i]->length * topo_segs[i]->level >= length_thresh) filter_segs.push_back(topo_segs[i]);
	}
	topo_segs2swc(filter_segs, outswc, 0);  
	return true;
}

// hierarchy coverage pruning
template<class T> bool happ(vector<MyMarker*> &inswc, vector<MyMarker*> & outswc, T * inimg1d, long sz0, long sz1, long sz2, double bkg_thresh = 10.0, double length_thresh = 2.0, double SR_ratio = 1.0/9.0, bool is_leaf_prune = true, bool is_smooth = true)
{
	double T_max = (1ll << sizeof(T));

	V3DLONG sz01 = sz0 * sz1;
	V3DLONG tol_sz = sz01 * sz2;

	map<MyMarker*, int> child_num;//int means the number of children
	getLeaf_markers(inswc, child_num);

	vector<HierarchySegment*> topo_segs;
	cout<<"Construct hierarchical segments"<<endl;
	swc2topo_segs(inswc, topo_segs, INTENSITY_DISTANCE_METHOD, inimg1d, sz0, sz1, sz2);
	vector<HierarchySegment*> filter_segs;
	for(int i = 0; i < topo_segs.size(); i++)
	{
		if(topo_segs[i]->length >= length_thresh) filter_segs.push_back(topo_segs[i]);
	}
	cout<<"pruned by length_thresh (segment number) : "<<topo_segs.size() <<" - "<<topo_segs.size() - filter_segs.size()<<" = "<<filter_segs.size()<<endl;
	multimap<double, HierarchySegment*> seg_dist_map;
	for(int i = 0; i < filter_segs.size(); i++)
	{
		double dst = filter_segs[i]->length;
		seg_dist_map.insert(pair<double, HierarchySegment*> (dst, filter_segs[i]));
	}

	if(1) // dark nodes pruning
	{
		int dark_num_pruned = 1;
		int iteration = 1;
		vector<bool> is_pruneable(filter_segs.size(), 0);
		cout<<"===== Perform dark node pruning ====="<<endl;
		while(dark_num_pruned > 0)
		{
			dark_num_pruned = 0;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				if(iteration > 1 && !is_pruneable[i]) continue;
				HierarchySegment * seg = filter_segs[i];
				MyMarker * leaf_marker = seg->leaf_marker;
				MyMarker * root_marker = seg->root_marker;
				if(leaf_marker == root_marker) continue;
				if(inimg1d[leaf_marker->ind(sz0, sz01)] <= bkg_thresh)
				{
					seg->leaf_marker = leaf_marker->parent;
					dark_num_pruned ++;
					is_pruneable[i] = true;
				}
				else is_pruneable[i] = false;
			}
			cout<<"\t iteration ["<<iteration++<<"] "<<dark_num_pruned<<" dark node pruned"<<endl;
		}
	}

	if(1) // dark segment pruning
	{
		set<int> delete_index_set;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			if(leaf_marker == root_marker) {delete_index_set.insert(i); continue;}
			MyMarker * p = leaf_marker;
			double sum_int = 0.0, tol_num = 0.0, dark_num = 0.0;
			while(true)
			{
				double intensity = inimg1d[p->ind(sz0, sz01)];
				sum_int += intensity;
				tol_num++;
				if(intensity <= bkg_thresh) dark_num++;
				if(p == root_marker) break;
				p = p->parent;
			}
            if(sum_int/tol_num <= bkg_thresh ||
               dark_num/tol_num >= 0.2)
                delete_index_set.insert(i);
		}
		vector<HierarchySegment*> tmp_segs;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			if(delete_index_set.find(i) == delete_index_set.end()) tmp_segs.push_back(seg);
		}
		cout<<"\t"<<delete_index_set.size()<<" dark segments are deleted"<<endl;
		filter_segs = tmp_segs;
	}

	// calculate radius for every node
	{
		cout<<"Calculating radius for every node"<<endl;
		V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			MyMarker * p = leaf_marker;
			while(true)
			{
                double real_thres = 40; if (real_thres<bkg_thresh) real_thres = bkg_thresh; //by PHC 20121012
                
				p->radius = markerRadiusXY(inimg1d, in_sz, *p, real_thres);
				if(p == root_marker) break;
				p = p->parent; 
			}
		}
	}

#ifdef __USE_APP_METHOD__
	if(1) // hierarchy coverage order pruning
#else
	if(1) // hierarchy coverage order pruning
#endif
	{ 
		cout<<"Perform hierarchical pruning"<<endl;
		T * tmpimg1d = new T[tol_sz]; memcpy(tmpimg1d, inimg1d, tol_sz * sizeof(T));
		V3DLONG tmp_sz[4] = {sz0, sz1, sz2, 1};

		multimap<double, HierarchySegment*>::reverse_iterator it = seg_dist_map.rbegin();
		//MyMarker * soma = (*it).second->root_marker;  // 2012/07 Hang, no need to consider soma
		//cout<<"soma ("<<soma->x<<","<<soma->y<<","<<soma->z<<") radius = "<<soma->radius<<" value = "<<(int)inimg1d[soma->ind(sz0, sz01)]<<endl;
		filter_segs.clear();
		set<HierarchySegment*> visited_segs;
		double tol_sum_sig = 0.0, tol_sum_rdc = 0.0;
		while(it != seg_dist_map.rend())
		{
			HierarchySegment * seg = it->second;
			if(seg->parent && visited_segs.find(seg->parent) == visited_segs.end()){it++; continue;}

			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			double SR_RATIO = SR_ratio;     // the soma area will use different SR_ratio
			//if(dist(*soma, *root_marker) <= soma->radius) SR_RATIO = 1.0;

			double sum_sig = 0;
			double sum_rdc = 0;

			if(1)
			{
				MyMarker * p = leaf_marker;
				while(true) 
				{ 
					if(tmpimg1d[p->ind(sz0, sz01)] == 0){sum_rdc += inimg1d[p->ind(sz0, sz01)];}
					else
					{
						if(0) sum_sig += inimg1d[p->ind(sz0, sz01)]; // simple stragety
						if(1)// if sphere overlap
						{
							int r = p->radius;
							V3DLONG x1 = p->x + 0.5;
							V3DLONG y1 = p->y + 0.5;
							V3DLONG z1 = p->z + 0.5;
							double sum_sphere_size = 0.0;
							double sum_delete_size = 0.0;
							for(V3DLONG kk = -r; kk <= r; kk++)
							{
								V3DLONG z2 = z1 + kk;
								if(z2 < 0 || z2 >= sz2) continue;
								for(V3DLONG jj = -r; jj <= r; jj++)
								{
									V3DLONG y2 = y1 + jj;
									if(y2 < 0 || y2 >= sz1) continue;
									for(V3DLONG ii = -r; ii <= r; ii++)
									{
										V3DLONG x2 = x1 + ii;
										if(x2 < 0 || x2 >= sz0) continue;
										if(kk*kk + jj*jj + ii*ii > r*r) continue;
										V3DLONG ind2 = z2 * sz01 + y2 * sz0 + x2;
										sum_sphere_size++;
										if(tmpimg1d[ind2] != inimg1d[ind2]){sum_delete_size ++;}
									}
								}
							}
							// the intersection between two sphere with equal size and distance = R is 5/16 (0.3125)
							// sum_delete_size/sum_sphere_size should be < 5/16 for outsize points
							if(sum_sphere_size > 0 && sum_delete_size/sum_sphere_size > 0.1) 
							{
								sum_rdc += inimg1d[p->ind(sz0, sz01)];
							}
							else sum_sig += inimg1d[p->ind(sz0, sz01)];
						}
					}
					if(p == root_marker) break;
					p = p->parent; 
				}
			}

			//double sum_sig = total_sum_int - sum_rdc;
			if(!seg->parent || sum_rdc == 0.0 || (sum_sig/sum_rdc >= SR_RATIO && sum_sig >= 1.0 * T_max))
			{
				tol_sum_sig += sum_sig;
				tol_sum_rdc += sum_rdc;

				vector<MyMarker*> seg_markers;
				MyMarker * p = leaf_marker;
				while(true){if(tmpimg1d[p->ind(sz0, sz01)] != 0) seg_markers.push_back(p); if(p == root_marker) break; p = p->parent;}
				//reverse(seg_markers.begin(), seg_markers.end()); // need to reverse if resampling

				for(int m = 0; m < seg_markers.size(); m++)
				{
					p = seg_markers[m];

					int r = p->radius;
					if(r > 0)// && tmpimg1d[p->ind(sz0, sz01)] != 0)
					{
						double rr = r * r;
						V3DLONG x = p->x + 0.5;
						V3DLONG y = p->y + 0.5;
						V3DLONG z = p->z + 0.5;
						for(V3DLONG kk = -r; kk <= r; kk++)
						{
							V3DLONG z2 = z + kk;
							if(z2 < 0 || z2 >= sz2) continue;
							for(V3DLONG jj = -r; jj <= r; jj++)
							{
								V3DLONG y2 = y + jj;
								if(y2 < 0 || y2 >= sz1) continue;
								for(V3DLONG ii = -r; ii <= r; ii++)
								{
									V3DLONG x2 = x + ii;
									if(x2 < 0 || x2 >= sz0) continue;
									double dst = ii*ii + jj*jj + kk*kk;
									if(dst > rr) continue;
									V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
									tmpimg1d[ind] = 0;
								}
							}
						}
					}
				}

				filter_segs.push_back(seg);
				visited_segs.insert(seg);     // used to delete children when parent node is delete
			}
			it++;
		}
		cout<<"prune by coverage (segment number) : "<<seg_dist_map.size() << " - "<< filter_segs.size() <<" = "<<seg_dist_map.size() - filter_segs.size()<<endl;
		cout<<"R/S ratio = "<<tol_sum_rdc/tol_sum_sig<<" ("<<tol_sum_rdc<<"/"<<tol_sum_sig<<")"<<endl;
		if(1) // evaluation
		{
			double tree_sig = 0.0; for(int m = 0; m < inswc.size(); m++) tree_sig += inimg1d[inswc[m]->ind(sz0, sz01)];
			double covered_sig = 0.0; for(int ind = 0; ind < tol_sz; ind++) if(tmpimg1d[ind] == 0) covered_sig += inimg1d[ind];
			cout<<"S/T ratio = "<<covered_sig/tree_sig<<" ("<<covered_sig<<"/"<<tree_sig<<")"<<endl;
		}
		//saveImage("test.tif", tmpimg1d, tmp_sz, V3D_UINT8);
		if(tmpimg1d){delete [] tmpimg1d; tmpimg1d = 0; }
	}

	if(0) // resampling markers or internal node pruning //this part of code has bug: many fragmentations. noted by PHC, 20120628
	{
		cout<<"resampling markers"<<endl;
		vector<MyMarker*> tmp_markers;
		topo_segs2swc(filter_segs, tmp_markers, 0); // no resampling
		child_num.clear();
		getLeaf_markers(tmp_markers, child_num);

		// calculate sampling markers
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			vector<MyMarker*> seg_markers;
			MyMarker * p = leaf_marker;
			while(true){seg_markers.push_back(p); if(p == root_marker) break; p = p->parent;}
			//reverse(seg_markers.begin(), seg_markers.end()); // need to reverse if resampling //commened by PHC, 130520 to build on Ubuntu. This should make no difference as the outside code is if (0)
			vector<MyMarker*> sampling_markers; // store resampling markers
			p = root_marker; sampling_markers.push_back(p);
			for(int m = 0; m < seg_markers.size(); m++)
			{
				MyMarker * marker = seg_markers[m];
				if(child_num[marker] > 1 || dist(*marker, *p) >= p->radius)// + marker->radius) 
				{
					sampling_markers.push_back(marker);
					p = marker;
				}
			}
			if((*sampling_markers.rbegin()) != leaf_marker) sampling_markers.push_back(leaf_marker);
			for(int m = 1; m < sampling_markers.size(); m++) sampling_markers[m]->parent = sampling_markers[m-1];
		}
	}
	
#ifdef __USE_APP_METHOD__
	if(1)//is_leaf_prune)  // leaf nodes pruning
#else
	if(0)
#endif
	{
		cout<<"Perform leaf node pruning"<<endl;

		map<MyMarker*,int> tmp_child_num;
		if(1) // get child_num of each node
		{
			vector<MyMarker*> current_markers;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				HierarchySegment * seg = filter_segs[i];
				seg->get_markers(current_markers);
			}
			for(int m = 0; m < current_markers.size(); m++) tmp_child_num[current_markers[m]] = 0;
			for(int m = 0; m < current_markers.size(); m++) 
			{
				MyMarker * par_marker = current_markers[m]->parent;
				if(par_marker) tmp_child_num[par_marker]++;
			}

		}
		int leaf_num_pruned = 1;
		int iteration = 1;
		vector<bool> is_pruneable(filter_segs.size(), 0);
		while(leaf_num_pruned > 0)
		{
			leaf_num_pruned = 0;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				if(iteration > 1 && !is_pruneable[i]) continue;
				HierarchySegment * seg = filter_segs[i];
				MyMarker * leaf_marker = seg->leaf_marker;
				MyMarker * root_marker = seg->root_marker;

				if(tmp_child_num[leaf_marker] >= 1) continue;

				assert(leaf_marker);
				MyMarker * par_marker = leaf_marker->parent;
				if(!par_marker) 
				{
					is_pruneable[i] = 0;
					continue;
				}
				int r1 = leaf_marker->radius;
				int r2 =  par_marker->radius;
				double r1_r1 = r1 * r1;
				double r2_r2 = r2 * r2;
				V3DLONG x1 = leaf_marker->x + 0.5;
				V3DLONG y1 = leaf_marker->y + 0.5;
				V3DLONG z1 = leaf_marker->z + 0.5;
				V3DLONG x2 =  par_marker->x + 0.5;
				V3DLONG y2 =  par_marker->y + 0.5;
				V3DLONG z2 =  par_marker->z + 0.5;

				double sum_leaf_int  = 0.0;
				double sum_over_int = 0.0;
				for(V3DLONG kk = -r1; kk <= r1; kk++)
				{
					V3DLONG zz = z1 + kk;
					if(zz < 0 || zz >= sz2) continue;
					for(V3DLONG jj = -r1; jj <= r1; jj++)
					{
						V3DLONG yy = y1 + jj;
						if(yy < 0 || yy >= sz1) continue;
						for(V3DLONG ii = -r1; ii <= r1; ii++)
						{
							V3DLONG xx = x1 + ii;
							if(xx < 0 || xx >= sz0) continue;
							double dst = kk * kk + jj * jj + ii * ii;
							if(dst > r1_r1) continue;
							V3DLONG ind = zz * sz01 + yy * sz0 + xx;
							sum_leaf_int += inimg1d[ind];
							if((z2 - zz) * (z2 - zz) + (y2 - yy) * (y2 - yy) + (x2 - xx) * (x2 - xx) <= r2 * r2)
							{
								sum_over_int += inimg1d[ind];
							}
						}
					}
				}
				if(sum_leaf_int > 0 && sum_over_int/sum_leaf_int >= 0.9)
				{
					leaf_num_pruned ++;
					tmp_child_num[par_marker]--;
					assert(tmp_child_num[leaf_marker] == 0);
					if(leaf_marker != root_marker)
					{
						seg->leaf_marker = par_marker;
						is_pruneable[i] = true;
					}
					else
					{
						seg->leaf_marker = NULL;
						seg->root_marker = NULL;
						is_pruneable[i] = false;
					}
				}
				else is_pruneable[i] = false;
			}
			cout<<"\t iteration ["<<iteration++<<"] "<<leaf_num_pruned<<" leaf node pruned"<<endl;
		}
		// filter out segments with single marker
		vector<HierarchySegment*> tmp_segs;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			if(leaf_marker && root_marker) tmp_segs.push_back(seg);
		}
		cout<<"\t"<<filter_segs.size() - tmp_segs.size()<<" hierarchical segments are pruned in leaf node pruning"<<endl;
		filter_segs.clear(); filter_segs = tmp_segs;
	}

#ifdef __USE_APP_METHOD__
	if(1) // joint leaf node pruning 
#else
	if(0) // joint leaf node pruning 
#endif
	{
		cout<<"Perform joint leaf node pruning"<<endl;
		cout<<"\tcompute mask area"<<endl;
		unsigned short * mask = new unsigned short[tol_sz];
		memset(mask, 0, sizeof(unsigned short) * tol_sz);
		for(int s = 0; s < filter_segs.size(); s++)
		{
			HierarchySegment * seg = filter_segs[s];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			MyMarker * p = leaf_marker;
			while(true)
			{
				int r = p->radius;
				if(r > 0)
				{
					double rr = r * r;
					V3DLONG x = p->x + 0.5;
					V3DLONG y = p->y + 0.5;
					V3DLONG z = p->z + 0.5;
					for(V3DLONG kk = -r; kk <= r; kk++)
					{
						V3DLONG z2 = z + kk;
						if(z2 < 0 || z2 >= sz2) continue;
						for(V3DLONG jj = -r; jj <= r; jj++)
						{
							V3DLONG y2 = y + jj;
							if(y2 < 0 || y2 >= sz1) continue;
							for(V3DLONG ii = -r; ii <= r; ii++)
							{
								V3DLONG x2 = x + ii;
								if(x2 < 0 || x2 >= sz0) continue;
								double dst = ii*ii + jj*jj + kk*kk;
								if(dst > rr) continue;
								V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
								mask[ind]++;
							}
						}
					}
				}
				if(p == root_marker) break;
				p = p->parent;
			}
		}
		cout<<"\tget post_segs"<<endl;
		vector<HierarchySegment*> post_segs;
		if(0) // get post order of filter_segs
		{
			multimap<double, HierarchySegment*> tmp_seg_map;
			for(int s = 0; s < filter_segs.size(); s++) 
			{
				double dst = filter_segs[s]->length;
				tmp_seg_map.insert(pair<double, HierarchySegment*>(dst, filter_segs[s]));
			}
			multimap<double, HierarchySegment*>::iterator it = tmp_seg_map.begin();
			while(it != tmp_seg_map.end())
			{
				post_segs.push_back(it->second);
				it++;
			}
		}
		else post_segs = filter_segs; // random order

		map<MyMarker*,int> tmp_child_num;
		if(1) // get child_num of each node
		{
			vector<MyMarker*> current_markers;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				HierarchySegment * seg = filter_segs[i];
				seg->get_markers(current_markers);
			}
			for(int m = 0; m < current_markers.size(); m++) tmp_child_num[current_markers[m]] = 0;
			for(int m = 0; m < current_markers.size(); m++) 
			{
				MyMarker * par_marker = current_markers[m]->parent;
				if(par_marker) tmp_child_num[par_marker]++;
			}

		}
		if(1) // start prune leaf nodes
		{
			cout<<"\tleaf node pruning"<<endl;
			int leaf_num_pruned = 1;
			int iteration = 1;
			vector<bool> is_pruneable(post_segs.size(), 0);
			while(leaf_num_pruned > 0)
			{
				leaf_num_pruned = 0;
				for(int i = 0; i < post_segs.size(); i++)
				{
					if(iteration > 1 && !is_pruneable[i]) continue;
					HierarchySegment * seg = post_segs[i];
					MyMarker * leaf_marker = seg->leaf_marker;
					MyMarker * root_marker = seg->root_marker;
					int r = leaf_marker->radius;
					if(r <= 0 )
					{
						is_pruneable[i] = 0;
						continue;
					}
					double rr = r * r;
					V3DLONG x = leaf_marker->x + 0.5;
					V3DLONG y = leaf_marker->y + 0.5;
					V3DLONG z = leaf_marker->z + 0.5;

					double covered_sig = 0; double total_sig = 0.0;
					for(V3DLONG kk = -r; kk <= r; kk++)
					{
						V3DLONG z2 = z + kk;
						if(z2 < 0 || z2 >= sz2) continue;
						for(V3DLONG jj = -r; jj <= r; jj++)
						{
							V3DLONG y2 = y + jj;
							if(y2 < 0 || y2 >= sz1) continue;
							for(V3DLONG ii = -r; ii <= r; ii++)
							{
								V3DLONG x2 = x + ii;
								if(x2 < 0 || x2 >= sz0) continue;
								double dst = ii*ii + jj*jj + kk*kk;
								if(dst > rr) continue;
								V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
								if(mask[ind] > 1) covered_sig += inimg1d[ind];
								total_sig += inimg1d[ind];
							}
						}
					}
					if(covered_sig / total_sig >= 0.9) // 90% joint cover, prune it
					{	
						if(tmp_child_num[leaf_marker] == 0) // real leaf node
						{
							leaf_num_pruned++;
							MyMarker * par_marker = leaf_marker->parent;
							if(par_marker) tmp_child_num[par_marker]--;
							if(leaf_marker != root_marker) 
							{
								seg->leaf_marker = par_marker;
								is_pruneable[i] = 1; // *** able to prune continuous
							}
							else // if(leaf_marker == root_marker) // unable to prune
							{
								seg->leaf_marker = NULL; 
								seg->root_marker = NULL;
								is_pruneable[i] = 0; // *** no marker left, unable to prune again
							}
							// unmask leaf_marker area
							{	
								for(V3DLONG kk = -r; kk <= r; kk++)
								{
									V3DLONG z2 = z + kk;
									if(z2 < 0 || z2 >= sz2) continue;
									for(V3DLONG jj = -r; jj <= r; jj++)
									{
										V3DLONG y2 = y + jj;
										if(y2 < 0 || y2 >= sz1) continue;
										for(V3DLONG ii = -r; ii <= r; ii++)
										{
											V3DLONG x2 = x + ii;
											if(x2 < 0 || x2 >= sz0) continue;
											double dst = ii*ii + jj*jj + kk*kk;
											if(dst > rr) continue;
											V3DLONG ind = z2 * sz01 + y2 * sz0 + x2;
											if(mask[ind] > 1) mask[ind]--;
										}
									}
								}
							}
						}
						else is_pruneable[i] = 1; // keep it until it is leaf node
					}
					else is_pruneable[i] = 0;
				}
				cout<<"\t iteration ["<<iteration++<<"] "<<leaf_num_pruned<<" leaf node pruned"<<endl;
			}
			// filter out segments with single marker
			vector<HierarchySegment*> tmp_segs;
			for(int i = 0; i < filter_segs.size(); i++)
			{
				HierarchySegment * seg = filter_segs[i];
				MyMarker * leaf_marker = seg->leaf_marker;
				MyMarker * root_marker = seg->root_marker;
				if(leaf_marker && root_marker) tmp_segs.push_back(seg); // filter out empty segments
			}
			cout<<"\t"<<filter_segs.size() - tmp_segs.size()<<" hierarchical segments are pruned in joint leaf node pruning"<<endl;
			filter_segs.clear(); filter_segs = tmp_segs;
		}

		if(mask){delete [] mask; mask = 0;}
	}

	if(is_smooth) // smooth curve
	{
		cout<<"Smooth the final curve"<<endl;
		for(int i = 0; i < filter_segs.size(); i++)
		{
			HierarchySegment * seg = filter_segs[i];
			MyMarker * leaf_marker = seg->leaf_marker;
			MyMarker * root_marker = seg->root_marker;
			vector<MyMarker*> seg_markers;
			MyMarker * p = leaf_marker;
			while(p != root_marker) 
			{
				seg_markers.push_back(p);
				p = p->parent;
			}
			seg_markers.push_back(root_marker);
			smooth_curve_and_radius(seg_markers, 5);
		}
	}
	outswc.clear();
	cout<<filter_segs.size()<<" segments left"<<endl;
	topo_segs2swc(filter_segs, outswc, 0); // no resampling
	
	// release hierarchical segments
	for(int i = 0; i < topo_segs.size(); i++) delete topo_segs[i];
	return true;
}

#endif
