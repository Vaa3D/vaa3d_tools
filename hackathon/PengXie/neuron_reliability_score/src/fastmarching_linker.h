/*****************************************************************
 * file : fastmarching_linker.h,  Hang Xiao, Jan 18, 2012
 * 
 * fastmarching_linker : link two swc segments by using fastmarching
 *****************************************************************/
#ifndef __FASTMARCHING_LINKER_H__
#define __FASTMARCHING_LINKER_H__

#include <cstdlib>
#include <cmath>  // sqrt
#include <cassert>
#include <vector>
#include <set>
#include <map>
#include <iostream> // cerr

#include "my_surf_objs.h"
#include "stackutil.h"
#include "simple_c.h"
#include "heap.h"
#include "common_macro.h"

using namespace std;

#define INF 3.4e+38             // float INF

#define GI(ind) exp(li*pow((1-(inimg1d[ind]-min_int)/max_int),2))

/******************************************************************************
 * Fast marching based region growing, will give out the nearest two markers in two different sets
 * 
 * Input :  sub_markers     the markers which will do fast marching
 *          tar_markers     the target markers which will stop marching
 *          inimg1d         original input image
 *
 * Output : linker          the path from sub_markers to tar_markers, the first markr is in tar_markers, the last marker is in sub_markers
 *
 * very similar to voronoi diagram construction, consecutive diagram won't connect
 * *****************************************************************************/
template<class T> bool fastmarching_linker(vector<MyMarker> &sub_markers,vector<MyMarker> & tar_markers, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
    //cout<<"cnn_type = "<<cnn_type<<endl;

	float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
	map<long, MyMarker> sub_map, tar_map;
	for(long i = 0; i < tar_markers.size(); i++)
	{
		int x = tar_markers[i].x + 0.5;
		int y = tar_markers[i].y + 0.5;
		int z = tar_markers[i].z + 0.5;
		long ind = z*sz01 + y*sz0 + x;
		tar_map[ind] = tar_markers[i];
	}

	for(long i = 0; i < sub_markers.size(); i++)
	{
		int x = sub_markers[i].x + 0.5;
		int y = sub_markers[i].y + 0.5;
		int z = sub_markers[i].z + 0.5;
		long ind = z*sz01 + y*sz0 + x;
		sub_map[ind] = sub_markers[i];
	}

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

	vector<long> submarker_inds;
	for(long s = 0; s < sub_markers.size(); s++) {
		int i = sub_markers[s].x + 0.5;
		int j = sub_markers[s].y + 0.5;
		int k = sub_markers[s].z + 0.5;
		if(i < 0 || i >= sz0 || j < 0 || j >= sz1 || k < 0 || k >= sz2) continue;
		long ind = k*sz01 + j*sz0 + i;
		submarker_inds.push_back(ind);
		state[ind] = ALIVE; 
		phi[ind] = 0.0;
	}
	int * parent = new int[tol_sz]; for(int ind = 0; ind < tol_sz; ind++) parent[ind] = ind;

	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	// init heap
	for(long s = 0; s < submarker_inds.size(); s++)
	{
		long index = submarker_inds[s];
		HeapElemX *elem = new HeapElemX(index, phi[index]);
		elem->prev_ind = index;
		heap.insert(elem);
		elems[index] = elem;
	}
	// loop
	int time_counter = sub_markers.size();
    //double process1 = time_counter*1000.0/tol_sz;
	long stop_ind = -1;
	while(!heap.empty())
	{
        //double process2 = (time_counter++)*1000.0/tol_sz;
        //if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		parent[min_ind] = min_elem->prev_ind;
		if(tar_map.find(min_ind) != tar_map.end()){stop_ind = min_ind; break;}

		delete min_elem;

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
		// assert(!mask_values.empty());
	}

	V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
	double thresh = 20;
	// connect markers according to disjoint set
	{
		// add tar_marker
		long ind = stop_ind;
		MyMarker tar_marker = tar_map[stop_ind];
		MyMarker * new_marker = new MyMarker(tar_marker.x, tar_marker.y, tar_marker.z);
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		new_marker->parent = 0; //tar_marker;

		outswc.push_back(new_marker);

		MyMarker * par_marker = new_marker;
		ind = parent[ind];
		while(sub_map.find(ind) == sub_map.end())
		{
			int i = ind % sz0;
			int j = ind/sz0 % sz1;
			int k = ind/sz01 % sz2;
			new_marker = new MyMarker(i,j,k);
			new_marker->parent = par_marker;
			new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
			outswc.push_back(new_marker);
			par_marker = new_marker;
			ind = parent[ind];
		}
		// add sub_marker
		MyMarker sub_marker = sub_map[ind];
		new_marker = new MyMarker(sub_marker.x, sub_marker.y, sub_marker.z);
		new_marker->parent = par_marker;
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		outswc.push_back(new_marker);
	}
    //cout<<outswc.size()<<" markers linked"<<endl;
	//for(int i = 0; i < sub_markers.size(); i++) outswc.push_back(sub_markers[i]);
	//for(int i = 0; i < tar_markers.size(); i++) outswc.push_back(tar_markers[i]);
	
	if(phi) {delete [] phi; phi = 0;}
	if(parent) {delete [] parent; parent = 0;}
	return true;
}
/******************************************************************************
 * Fast marching based region growing, will give out the nearest two markers in two different sets
 * 
 * Input :  sub_markers     the markers which will do fast marching
 *          tar_markers     the target markers which will stop marching
 *          inimg1d         original input image
 *
 * Output : linker          the path from sub_markers to tar_markers, the first markr is in tar_markers, the last marker is in sub_markers
 *
 * very similar to voronoi diagram construction, consecutive diagram won't connect
 * *****************************************************************************/
template<class T> bool fastmarching_linker(vector<MyMarker*> &sub_markers,vector<MyMarker*> & tar_markers, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
    //cout<<"cnn_type = "<<cnn_type<<endl;

	float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
	map<long, MyMarker *> sub_map, tar_map;
	for(long i = 0; i < tar_markers.size(); i++)
	{
		int x = tar_markers[i]->x + 0.5;
		int y = tar_markers[i]->y + 0.5;
		int z = tar_markers[i]->z + 0.5;
		long ind = z*sz01 + y*sz0 + x;
		tar_map[ind] = tar_markers[i];
	}

	for(long i = 0; i < sub_markers.size(); i++)
	{
		int x = sub_markers[i]->x + 0.5;
		int y = sub_markers[i]->y + 0.5;
		int z = sub_markers[i]->z + 0.5;
		long ind = z*sz01 + y*sz0 + x;
		sub_map[ind] = sub_markers[i];
	}


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

	vector<long> submarker_inds;
	for(long s = 0; s < sub_markers.size(); s++) {
		int i = sub_markers[s]->x + 0.5;
		int j = sub_markers[s]->y + 0.5;
		int k = sub_markers[s]->z + 0.5;
		long ind = k*sz01 + j*sz0 + i;
		submarker_inds.push_back(ind);
		state[ind] = ALIVE; 
		phi[ind] = 0.0;
	}
	int * parent = new int[tol_sz]; for(int ind = 0; ind < tol_sz; ind++) parent[ind] = ind;

	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	// init heap
	for(long s = 0; s < submarker_inds.size(); s++)
	{
		long index = submarker_inds[s];
		HeapElemX *elem = new HeapElemX(index, phi[index]);
		elem->prev_ind = index;
		heap.insert(elem);
		elems[index] = elem;
	}
	// loop
	int time_counter = sub_markers.size();
    //double process1 = time_counter*1000.0/tol_sz;
	long stop_ind = -1;
	while(!heap.empty())
	{
        //double process2 = (time_counter++)*1000.0/tol_sz;
        //if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		parent[min_ind] = min_elem->prev_ind;
		if(tar_map.find(min_ind) != tar_map.end()){stop_ind = min_ind; break;}

		delete min_elem;

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
		// assert(!mask_values.empty());
	}

	V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
	double thresh = 20;
	// connect markers according to disjoint set
	{
		// add tar_marker
		long ind = stop_ind;
		MyMarker * tar_marker = tar_map[stop_ind];
		MyMarker * new_marker = new MyMarker(tar_marker->x, tar_marker->y, tar_marker->z);
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		new_marker->parent = tar_marker;

		outswc.push_back(new_marker);

		MyMarker * par_marker = new_marker;
		ind = parent[ind];
		while(sub_map.find(ind) == sub_map.end())
		{
			int i = ind % sz0;
			int j = ind/sz0 % sz1;
			int k = ind/sz01 % sz2;
			new_marker = new MyMarker(i,j,k);
			new_marker->parent = par_marker;
			new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
			outswc.push_back(new_marker);
			par_marker = new_marker;
			ind = parent[ind];
		}
		// add sub_marker
		MyMarker * sub_marker = sub_map[ind];
		new_marker = new MyMarker(sub_marker->x, sub_marker->y, sub_marker->z);
		new_marker->parent = par_marker;
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		outswc.push_back(new_marker);
	}
    //cout<<outswc.size()<<" markers linked"<<endl;
	//for(int i = 0; i < sub_markers.size(); i++) outswc.push_back(sub_markers[i]);
	//for(int i = 0; i < tar_markers.size(); i++) outswc.push_back(tar_markers[i]);
	
	if(state) {delete [] state; state = 0;}
	if(phi) {delete [] phi; phi = 0;}
	if(parent) {delete [] parent; parent = 0;}
	return true;
}

/******************************************************************************
 * Fast marching based region growing, will connect sub_marker and tar_marker
 * 
 * Input :  sub_marker      the subject marker
 *          tar_marker      the target marker
 *          inimg1d         original input image
 *
 * Output : outswc          the path from sub_markers to tar_markers, the first markr is in tar_markers, the last marker is in sub_markers
 *
 * very similar to voronoi diagram construction, consecutive diagram won't connect
 * *****************************************************************************/
template<class T> bool fastmarching_linker(MyMarker sub_marker, MyMarker tar_marker, T * inimg1d, vector<MyMarker*> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
    //cout<<"cnn_type = "<<cnn_type<<endl;

	long sub_ind = (long)(sub_marker.z + 0.5) * sz01 + (long)(sub_marker.y + 0.5) * sz0 + (long)(sub_marker.x + 0.5);
	long tar_ind = (long)(tar_marker.z + 0.5) * sz01 + (long)(tar_marker.y + 0.5) * sz0 + (long)(tar_marker.x + 0.5);
	
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
	float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
	char * state = new char[tol_sz]; for(long i = 0; i < tol_sz; i++) state[i] = FAR;
	int * parent = new int[tol_sz]; for(int ind = 0; ind < tol_sz; ind++) parent[ind] = ind;

	state[sub_ind] = ALIVE;  phi[sub_ind] = 0.0;

	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	// init heap
	{
		long index = sub_ind;
		HeapElemX *elem = new HeapElemX(index, phi[index]);
		elem->prev_ind = index;
		heap.insert(elem);
		elems[index] = elem;
	}
	// loop
	int time_counter = 1;
    //double process1 = time_counter*1000.0/tol_sz;
	long stop_ind = -1;
	while(!heap.empty())
	{
        //double process2 = (time_counter++)*1000.0/tol_sz;
        //if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		parent[min_ind] = min_elem->prev_ind;
		if(min_ind == tar_ind){stop_ind = min_ind; break;}

		delete min_elem;

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
		// assert(!mask_values.empty());
	}

	V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
	double thresh = 20;
	// connect markers according to disjoint set
	{
		// add tar_marker
		long ind = stop_ind;
		MyMarker * new_marker = new MyMarker(tar_marker.x, tar_marker.y, tar_marker.z);
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		new_marker->parent = 0;//	tar_marker;

		outswc.push_back(new_marker);

		MyMarker * par_marker = new_marker;
		ind = parent[ind];
		while(ind != sub_ind)
		{
			int i = ind % sz0;
			int j = ind/sz0 % sz1;
			int k = ind/sz01 % sz2;
			new_marker = new MyMarker(i,j,k);
			new_marker->parent = par_marker;
			new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
			outswc.push_back(new_marker);
			par_marker = new_marker;
			ind = parent[ind];
		}
		// add sub_marker
		new_marker = new MyMarker(sub_marker.x, sub_marker.y, sub_marker.z);
		new_marker->parent = par_marker;
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		outswc.push_back(new_marker);
	}
    //cout<<outswc.size()<<" markers linked"<<endl;
	//for(int i = 0; i < sub_markers.size(); i++) outswc.push_back(sub_markers[i]);
	//for(int i = 0; i < tar_markers.size(); i++) outswc.push_back(tar_markers[i]);
	
	if(state) {delete [] state; state = 0;}
	if(phi) {delete [] phi; phi = 0;}
	if(parent) {delete [] parent; parent = 0;}
	return true;
}

/******************************************************************************
 * Fast marching based region growing, will grow sub_marker to between tar_marker1 and tar_marker2
 * 
 * Input :  sub_marker      the subject marker
 *          tar_marker1     the target line marker1
 *          tar_marker2     the target line marker2
 *          inimg1d         original input image
 *
 * Output : outswc          the path from sub_markers to tar_markers, the first markr is in tar_markers, the last marker is in sub_markers
 *
 * very similar to voronoi diagram construction, consecutive diagram won't connect
 * *****************************************************************************/

template<class T> bool fastmarching_linker(MyMarker sub_marker, MyMarker tar_marker1, MyMarker tar_marker2, T * inimg1d, vector<MyMarker*> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
    //cout<<"cnn_type = "<<cnn_type<<endl;

	long sub_ind = (long)(sub_marker.z + 0.5) * sz01 + (long)(sub_marker.y + 0.5) * sz0 + (long)(sub_marker.x + 0.5);
	set<long> tar_inds;
	double tar_dst = sqrt((tar_marker1.x - tar_marker2.x) * (tar_marker1.x - tar_marker2.x) + \
			(tar_marker1.y - tar_marker2.y) * (tar_marker1.y - tar_marker2.y) + \
			(tar_marker1.z - tar_marker2.z) * (tar_marker1.z - tar_marker2.z));
	double tar_tx = (tar_marker2.x - tar_marker1.x) / tar_dst;
	double tar_ty = (tar_marker2.y - tar_marker1.y) / tar_dst;
	double tar_tz = (tar_marker2.z - tar_marker1.z) / tar_dst;
	for(double r = 0.0; r < tar_dst+1; r++)
	{
		int x = tar_marker1.x + tar_tx * r + 0.5;
		int y = tar_marker1.y + tar_ty * r + 0.5;
		int z = tar_marker1.z + tar_tz * r + 0.5;
		long tar_ind = z * sz01 + y * sz0 + x;
		//cout<<"("<<x<<","<<y<<","<<z<<") "<<(int)(inimg1d[tar_ind])<<endl;
		tar_inds.insert(tar_ind);
	}
	
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
	float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
	char * state = new char[tol_sz]; for(long i = 0; i < tol_sz; i++) state[i] = FAR;
	int * parent = new int[tol_sz]; for(int ind = 0; ind < tol_sz; ind++) parent[ind] = ind;

	state[sub_ind] = ALIVE;  phi[sub_ind] = 0.0;

	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	// init heap
	{
		long index = sub_ind;
		HeapElemX *elem = new HeapElemX(index, phi[index]);
		elem->prev_ind = index;
		heap.insert(elem);
		elems[index] = elem;
	}
	// loop
	int time_counter = 1;
    //double process1 = time_counter*1000.0/tol_sz;
	long stop_ind = -1;
	while(!heap.empty())
	{
        //double process2 = (time_counter++)*1000.0/tol_sz;
        //if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		parent[min_ind] = min_elem->prev_ind;
		if(tar_inds.find(min_ind) != tar_inds.end()){stop_ind = min_ind; break;}

		delete min_elem;

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
		// assert(!mask_values.empty());
	}

	V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
	double thresh = 20;
	// connect markers according to disjoint set
	{
		// add tar_marker
		long ind = stop_ind;
		MyMarker * new_marker = new MyMarker(ind%sz0, ind/sz0 % sz1, ind/sz01 % sz2);
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		new_marker->parent = 0;//	tar_marker;

		outswc.push_back(new_marker);

		MyMarker * par_marker = new_marker;
		ind = parent[ind];
		while(ind != sub_ind)
		{
			int i = ind % sz0;
			int j = ind/sz0 % sz1;
			int k = ind/sz01 % sz2;
			new_marker = new MyMarker(i,j,k);
			new_marker->parent = par_marker;
			new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
			outswc.push_back(new_marker);
			par_marker = new_marker;
			ind = parent[ind];
		}
		// add sub_marker
		new_marker = new MyMarker(sub_marker.x, sub_marker.y, sub_marker.z);
		new_marker->parent = par_marker;
		new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
		outswc.push_back(new_marker);
	}
    //cout<<outswc.size()<<" markers linked"<<endl;
	
	if(state) {delete [] state; state = 0;}
	if(phi) {delete [] phi; phi = 0;}
	if(parent) {delete [] parent; parent = 0;}
	return true;
}

/******************************************************************************
 * function : fastmarching_voronoi
 *
 * Fast marching based voronoi by using intensity distance
 * 
 * Input :  seed_markers     the markers which will do fast marching
 *          inimg1d         original input image
 *
 * Output : linker          the path from seed_markers to tar_markers, the first markr is in tar_markers, the last marker is in seed_markers
 *
 * very similar to voronoi diagram construction, consecutive diagram won't connect
 * *****************************************************************************/
template<class T1, class T2> bool fastmarching_voronoi(vector<MyMarker> &seed_markers, T1 * inimg1d, T2 * &voro_mask, float * & phi, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
	//cout<<"cnn_type = "<<cnn_type<<endl;

	if(phi==0) {phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}}
	char * state = new char[tol_sz]; for(long i = 0; i < tol_sz; i++) state[i] = FAR;
	if(voro_mask == 0) voro_mask = new T2[tol_sz]; for(long i = 0; i < tol_sz; i++) voro_mask[i] = 0;

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
	
	// init heap
	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	cout<<"seed_markers.size() = "<<seed_markers.size()<<endl;
	for(long s = 0; s < seed_markers.size(); s++) {
		int i = seed_markers[s].x + 0.5;
		int j = seed_markers[s].y + 0.5;
		int k = seed_markers[s].z + 0.5;
		//cout<<"("<<i<<","<<j<<","<<k<<")"<<endl;
		if(i < 0 || i >= sz0 || j < 0 || j >= sz1 || k < 0 || k >= sz2) continue;
		long ind = k*sz01 + j*sz0 + i;
		state[ind] = ALIVE; 
		phi[ind] = 0.0;
		voro_mask[ind] = s+1;

		HeapElemX *elem = new HeapElemX(ind, phi[ind]);
		elem->prev_ind = ind;
		heap.insert(elem);
		elems[ind] = elem;
	}

	// loop
	int time_counter = seed_markers.size();
    //double process1 = time_counter*1000.0/tol_sz;
	long stop_ind = -1;
	while(!heap.empty())
	{
        //double process2 = (time_counter++)*1000.0/tol_sz;
        //if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		voro_mask[min_ind] = voro_mask[min_elem->prev_ind];

		delete min_elem;

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
		// assert(!mask_values.empty());
	}

	if(state) {delete [] state; state = 0;}
	//if(phi) {delete [] phi; phi = 0;}
	return true;
}

/******************************************************************************
 * function : fastmarching_voronoi
 *
 * Fast marching based voronoi by using intensity distance
 * 
 * Input :  sub_markers     the seed markers which will do fast marching
 * 			tar_markers     the target markers, all the target markers will be marched to
 *          inimg1d         original input image
 *
 * Output : linker          the path from seed_markers to tar_markers, the first markr is in tar_markers, the last marker is in seed_markers
 *
 * very similar to voronoi diagram construction, consecutive diagram won't connect
 * *****************************************************************************/
template<class T> bool fastmarching_voronoi(vector<MyMarker> &tar_markers, T * inimg1d, float * & phi, char * & state,  int * &voro_mask, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
	//cout<<"cnn_type = "<<cnn_type<<endl;

	if(phi==0 || state == 0 || voro_mask == 0) return false;

	// GI parameter min_int, max_int, li
	double max_int = 0; // maximum intensity, used in GI
	double min_int = INF;
	for(long i = 0; i < tol_sz; i++) 
	{
		if(inimg1d[i] > max_int) max_int = inimg1d[i];
		if(inimg1d[i] < min_int) min_int = inimg1d[i];
		if(state[i] == FAR) assert(voro_mask[i] == 0);
		else if(state[i] == ALIVE)assert(voro_mask[i] > 0);
	}
	max_int -= min_int;
	double li = 10;
	
	// init heap
	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	vector<long> tar_inds(tar_markers.size());
	int tar_num = 0;
	for(long m = 0; m < tar_markers.size(); m++) {
		int i = tar_markers[m].x + 0.5;
		int j = tar_markers[m].y + 0.5;
		int k = tar_markers[m].z + 0.5;
		if(i < 0 || i >= sz0 || j < 0 || j >= sz1 || k < 0 || k >= sz2){cout<<"("<<i<<","<<j<<","<<k<<") outside"<<endl; continue;}
		long ind = k*sz01 + j*sz0 + i;
		tar_inds[tar_num++] = ind;
	}

	// create heap
	{
		long i = -1, j = -1, k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1 == 0){j=0; k++;}}
			if(state[ind] == FAR) continue;
			for(int kk = -1; kk <= 1; kk++)
			{
				long k2 = k + kk;
				if(k2 < 0 || k2 >= sz2) continue;
				for(int jj = -1; jj <= 1; jj++)
				{
					long j2 = j + jj;
					if(j2 < 0 || j2 >= sz1) continue;
					for(int ii = -1; ii <= 1; ii++)
					{
						long i2 = i + ii;
						if(i2 < 0 || i2 >= sz0) continue;
						int offset = ABS(ii) + ABS(jj) + ABS(kk);
						if(offset > 1) continue;
						long ind2 = k2 * sz01 + j2 * sz0 + i2;
						if(state[ind2] == FAR)
						{
							HeapElemX * elem = new HeapElemX(ind, phi[ind]);
							elem->prev_ind = ind;
							heap.insert(elem);
							elems[ind] = elem;
							goto FM_EXIT0;
						}
					}
				}
			}
FM_EXIT0:
			ind=ind;
		}
	}

	// loop
	int time_counter = 0;
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*1000.0/tol_sz;
		if(process2 - process1 >= 1){
            //cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;
			bool is_break = true;
			for(int m = 0; m < tar_num; m++)
			{
				if(state[tar_inds[m]] == FAR) assert(voro_mask[tar_inds[m]] == 0);
				else if(state[tar_inds[m]] == ALIVE) assert(voro_mask[tar_inds[m]] > 0);
				if(state[tar_inds[m]] == FAR || voro_mask[tar_inds[m]] == 0){is_break = false; break;}
			}
			if(is_break) break;
		}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		voro_mask[min_ind] = voro_mask[min_elem->prev_ind];

		delete min_elem;

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
		// assert(!mask_values.empty());
	}
	if(!elems.empty())
	{
		map<long, HeapElemX *>::iterator it = elems.begin();
		while(it != elems.end())
		{
			HeapElemX * elem = it->second;
			delete elem;
			it++;
		}
	}
	return true;
}
#endif
