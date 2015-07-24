/*****************************************************************
 * file : fastmarching_tree.h,  Hang Xiao, Jan 18, 2012
 *
 * fastmarching_tree
 * fastmarching_tracing
 *
 * **************************************************************/

#ifndef __FAST_MARCHING_TREE_H__
#define __FAST_MARCHING_TREE_H__

#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <iostream>

#include "my_surf_objs.h"
#include "heap.h"

using namespace std;

extern bool TRACE_VERBOSE;

#define GI(ind) exp(li*pow((1-(inimg1d[ind]-min_int)/max_int),2))

//min_val = 0, max_val = 71949; 
//min_val = 0, max_val = 586; 

//#define INF 1.0e300
#define INF 3.4e+38

/******************************************************************************
 * Fast marching based manual tracing, with root marker and a set of target marker
 * 
 * Input : root      root marker
 *         target    the set of target markers
 *         inimg1d   original input image
 *
 * Output : outswc   output tracing result
 *          phi      finial distance value for each pixel [todo : replace INF to 0]
 *
 * Notice : 
 * 1. the input pixel number should not be larger than 2G if sizeof(long) == 4
 * 2. target markers should not contain root marker
 * 3. the root marker in outswc, is point to itself
 * *****************************************************************************/

template<class T1, class T2> bool fastmarching_tracing(MyMarker root, vector<MyMarker> &target, T1 * inimg1d, vector<MyMarker*> &outswc, T2 * &phi, int sz0, int sz1, int sz2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	int cnn_type = 3;  // ?

	if(phi == 0) phi = new T2[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
	long * parent = new long[tol_sz]; for(long i = 0; i < tol_sz; i++) parent[i] = i;  // each pixel point to itself at the beginning

	// GI parameter min_int, max_int, li
	double max_int = 0; // maximum intensity, used in GI
	double min_int = INF;
	for(long i = 0; i < tol_sz; i++) 
	{
		if(inimg1d[i] > max_int) max_int = inimg1d[i];
		if(inimg1d[i] < min_int) min_int = inimg1d[i];
	}
	max_int -= min_int;
	double li = 10;
	
	// initialization
	char * state = new char[tol_sz];
	for(long i = 0; i < tol_sz; i++) state[i] = FAR;

	// init state and phi for root
	int rootx = root.x + 0.5;
	int rooty = root.y + 0.5;
	int rootz = root.z + 0.5;

	long root_ind = rootz*sz01 + rooty*sz0 + rootx;
	state[root_ind] = ALIVE; 
	phi[root_ind] = 0.0;

	vector<long> target_inds;
	for(long t = 0; t < target.size(); t++) {
		int i = target[t].x + 0.5;
		int j = target[t].y + 0.5;
		int k = target[t].z + 0.5;
		long ind = k*sz01 + j*sz0 + i;
		target_inds.push_back(ind);
		//if(ind == root_ind) {cerr<<"please remove root marker from target markers"<<endl; exit(0);}
	}

	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	// init heap
	{
		long index = root_ind;
		HeapElemX *elem = new HeapElemX(index, phi[index]);
		elem->prev_ind = index;
		heap.insert(elem);
		elems[index] = elem;
	}

	// loop
	int time_counter = 1;
	double process1 = time_counter*1000.0/tol_sz;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*1000.0/tol_sz;
		if(process2 - process1 >= 1)
		{
			cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;
			bool is_break = true; for(int t = 0; t < target_inds.size(); t++){long tind = target_inds[t]; if(tind != root_ind && parent[tind] == tind) {is_break = false; break;}}
			if(is_break) break;
		}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		long prev_ind = min_elem->prev_ind;
		delete min_elem;

		parent[min_ind] = prev_ind;

		state[min_ind] = ALIVE;
		int i = min_ind % sz0; 
		int j = (min_ind/sz0) % sz1; 
		int k = (min_ind/sz01) % sz2;

		int w, h, d;
		for(int kk = -1; kk <= 1; kk++)
		{
			d = k+kk;
			if(d < 0 || d >= sz2) continue;
			for(int jj = -1; jj <= 1; jj++)
			{
				h = j+jj;
				if(h < 0 || h >= sz1) continue;
				for(int ii = -1; ii <= 1; ii++)
				{
					w = i+ii;
					if(w < 0 || w >= sz0) continue;
					int offset = ABS(ii) + ABS(jj) + ABS(kk);
					if(offset == 0 || offset > cnn_type) continue;
					double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
					long index = d*sz01 + h*sz0 + w;

					if(state[index] != ALIVE)
					{
						double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;
						long prev_ind = min_ind;

						if(state[index] == FAR)
						{
							phi[index] = new_dist;
							HeapElemX * elem = new HeapElemX(index, phi[index]);
							elem->prev_ind = prev_ind;
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > new_dist)
							{
								phi[index] = new_dist;
								HeapElemX * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
								elem->prev_ind = prev_ind;
							}
						}
					}
				}
			}
		}
	}

	// extract the tree nodes containing target markers
	map<long, MyMarker *> marker_map;
	for(int t = 0; t < target_inds.size(); t++) 
	{
		long tind = target_inds[t];
		long p = tind;
		while(true)
		{
			if(marker_map.find(p) != marker_map.end()) break;
			int i = p % sz0;
			int j = p/sz0 % sz1;
			int k = p/sz01 % sz2;
			MyMarker * marker = new MyMarker(i,j,k);
			marker_map[p] = marker;

			if(p == parent[p]) break;
			else p = parent[p];
		}
	}
	map<long, MyMarker*>::iterator it = marker_map.begin();
	V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
	while(it != marker_map.end())
	{
		long tind = it->first;
		MyMarker * marker = it->second;
		MyMarker * parent_marker = marker_map[parent[tind]];
		marker->parent = parent_marker;
		//marker->radius = markerRadius(inimg1d, in_sz, *marker, 1);
		outswc.push_back(marker);
		it++;
	}

	if(parent){delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}

#endif
