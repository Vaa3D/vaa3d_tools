//last change: by PHC, 2013-02-13. adjust memory allocation to make it more robust

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

#include "stackutil.h"
#include "my_surf_objs.h"
#include "heap.h"
#include "upwind_solver.h"
#include "fastmarching_macro.h"
using namespace std;


#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

struct HeapElemXX : public HeapElem
{
	MyMarker * parent_marker;
	HeapElemXX(long _ind, double _value, MyMarker * _parent_marker) : HeapElem(_ind, _value)
	{
		parent_marker = _parent_marker;
	}
};


/*********************************************************************
 * Function : fastmarching_linear_tree
 *
 * Features : 
 * 1. Create fast marcing tree from root marker only
 * 2. Background (intensity less than bkg_thresh) will be ignored. 
 * 3. The distance is the sum of intensity ***
 *
 * Input : root          root marker
 *         inimg1d       original 8bit image
 *
 * Output : tree         output swc
 *          phi          the distance for each pixels
 * *******************************************************************/
template<class T> bool fastmarching_linear_tree(MyMarker root, T * inimg1d, vector<MyMarker*> &outtree, int sz0, int sz1, int sz2, int cnn_type = 3, double bkg_thresh = 1)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 3;  // ?

	float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
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
	double process1 = 0;

	while(!heap.empty())
	{
		double process2 = (time_counter++)*10000.0/tol_sz;
		if(process2 - process1 >= 1)
		{
			cout<<"\r"<<((int)process2)/100.0<<"%";cout.flush(); process1 = process2;
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
					if(inimg1d[index] <= bkg_thresh) continue;

					if(state[index] != ALIVE)
					{
						double new_dist = phi[min_ind] + (1.0 - (inimg1d[index] - min_ind)/max_int)*1000.0;
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
	
	// save current swc tree
	if(1)
	{
		int i = -1, j = -1, k = -1;
		map<long, MyMarker*> tmp_map;
		for(long ind = 0; ind < tol_sz; ind++) 
		{
			i++; if(i%sz0 == 0){i=0;j++; if(j%sz1 == 0) {j=0; k++;}}
			if(state[ind] != ALIVE) continue;
			MyMarker * marker = new MyMarker(i,j,k);
			tmp_map[ind] = marker;
			outtree.push_back(marker);
		}
		i=-1; j = -1; k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] != ALIVE) continue;
			long ind2 = parent[ind];
			MyMarker * marker1 = tmp_map[ind];
			MyMarker * marker2 = tmp_map[ind2];
			if(marker1 == marker2) marker1->parent = 0;
			else marker1->parent = marker2;
			//tmp_map[ind]->parent = tmp_map[ind2];
		}
	}
	// over
	
	map<long, HeapElemX*>::iterator mit = elems.begin(); while(mit != elems.end()){HeapElemX * elem = mit->second; delete elem; mit++;}

	if(phi){delete [] phi; phi = 0;}
	if(parent){delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}


/*********************************************************************
 * Function : fastmarching_tree
 *
 * Features : 
 * 1. Create fast marcing tree from root marker only
 * 2. Background (intensity 0) will be ignored. 
 * 3. Graph augumented distance is used
 *
 * Input : root          root marker
 *         inimg1d       original 8bit image
 *
 * Output : tree         output swc
 *          phi          the distance for each pixels
 * *******************************************************************/
template<class T> bool fastmarching_tree(MyMarker root,
                                         T * inimg1d,
                                         vector<MyMarker*> &outtree,
                                         long sz0,
                                         long sz1,
                                         long sz2,
                                         int cnn_type = 3,
                                         double bkg_thresh = 20,
                                         bool is_break_accept = false)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
    long i;
	//int cnn_type = 3;  // ?

    float * phi = 0;
    long * parent = 0;
	char * state = 0;
    try
    {
        phi = new float[tol_sz];
        parent = new long[tol_sz];
        state = new char[tol_sz];
        for(i = 0; i < tol_sz; i++)
        {
            phi[i] = INF;
            parent[i] = i;  // each pixel point to itself at the         statements beginning
            state[i] = FAR;
        }
    }
    catch (...)
    {
        cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
        if (phi) {delete []phi; phi=0;}
        if (parent) {delete []parent; parent=0;}
        if (state) {delete []state; state=0;}
        return false;
    }

	// GI parameter min_int, max_int, li
	double max_int = 0; // maximum intensity, used in GI
	double min_int = INF;
	for(i = 0; i < tol_sz; i++) 
	{
		if (inimg1d[i] > max_int) max_int = inimg1d[i];
		else if (inimg1d[i] < min_int) min_int = inimg1d[i];
	}
	max_int -= min_int;
	double li = 10;
	
	// initialization

	// init state and phi for root
	long rootx = root.x + 0.5;
	long rooty = root.y + 0.5;
	long rootz = root.z + 0.5;

	long root_ind = rootz*sz01 + rooty*sz0 + rootx;

	state[root_ind] = ALIVE; 
	phi[root_ind] = 0.0;

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
	double process1 = 0;
    
	while(!heap.empty())
	{
		double process2 = (time_counter++)*10000.0/tol_sz;
        //cout<<"\r"<<((int)process2)/100.0<<"%";cout.flush();
		if(process2 - process1 >= 1)
		{
			cout<<"\r"<<((int)process2)/100.0<<"%";cout.flush(); process1 = process2;
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
					if (is_break_accept)
                    {
                        if(inimg1d[index] <= bkg_thresh &&
                           inimg1d[min_ind] <= bkg_thresh)
                            continue;
                    }
					else
                    {
                        if(inimg1d[index] <= bkg_thresh)
                            continue;
                    }

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
							if (phi[index] > new_dist)
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
	
	// save current swc tree
	if (1)
	{
		int i = -1, j = -1, k = -1;
		map<long, MyMarker*> tmp_map;
		for(long ind = 0; ind < tol_sz; ind++) 
		{
			i++; if(i%sz0 == 0){i=0;j++; if(j%sz1 == 0) {j=0; k++;}}
			if(state[ind] != ALIVE) continue;
			MyMarker * marker = new MyMarker(i,j,k);
			tmp_map[ind] = marker;
			outtree.push_back(marker);
		}
		i=-1; j = -1; k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] != ALIVE) continue;
			long ind2 = parent[ind];
			MyMarker * marker1 = tmp_map[ind];
			MyMarker * marker2 = tmp_map[ind2];
			if(marker1 == marker2) marker1->parent = 0;
			else marker1->parent = marker2;
			//tmp_map[ind]->parent = tmp_map[ind2];
		}
	}
	// over
	
	map<long, HeapElemX*>::iterator mit = elems.begin();
    while (mit != elems.end())
    {
        HeapElemX * elem = mit->second; delete elem; mit++;
    }

	if(phi){delete [] phi; phi = 0;}
	if(parent){delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}


/*********************************************************************
 * Function : fastmarching_tree
 *
 * Features : 
 * 1. Create fast marcing tree from root marker
 * 2. Background (intensity 0) will be ignored. 
 * 3. Euclidean distance is used
 *
 * Input : root          root marker
 *         inimg1d       original 8bit image
 *
 * Output : tree         output swc
 *          phi          the distance for each pixels
 * *******************************************************************/
// inimg1d is binary
template<class T> bool fastmarching_tree_old(MyMarker root, T * inimg1d, vector<MyMarker*> &tree, double * & phi, int sz0, int sz1, int sz2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};
	
	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;

	int rootx = (int)(root.x + 0.5);
	int rooty = (int)(root.y + 0.5);
	int rootz = (int)(root.z + 0.5);
	long root_ind = rootz*sz01 + rooty*sz0 + rootx;
	if(inimg1d[root_ind] == 0){cerr<<"the root position is not in forground"<<endl; return false;}

	int cnn_type = 1;  // cnn_type should be 1

	phi = new double[tol_sz];
	for(long i = 0; i < tol_sz; i++) phi[i] = INF;

	// initialization
	char * state = new char[tol_sz];
	for(long i = 0; i < tol_sz; i++) state[i] = FAR;
	state[root_ind] = ALIVE; phi[root_ind] = 0.0;

	BasicHeap<HeapElemXX> heap;
	map<long, HeapElemXX*> elems;

	// init state around root
	int i = rootx, j = rooty, k = rootz;
	MyMarker * root_node = new MyMarker(i,j,k);
	tree.push_back(root_node);

	int w, h, d;
	for(int kk = -1; kk <= 1; kk++)
	{
		d = k + kk;
		if(d < 0 || d >= sz2) continue;
		for(int jj = -1; jj <= 1; jj++)
		{
			h = j + jj;
			if(h < 0 || h >= sz1) continue;
			for(int ii = -1; ii <= 1; ii++)
			{
				w = i + ii;
				if(w < 0 || w >= sz0) continue;
				int offset = ABS(ii) + ABS(jj) + ABS(kk);
				if(offset == 0 || offset > cnn_type) continue;
				long index = d*sz01 + h*sz0 + w;
				if(inimg1d[index] == 0) continue;
				if(state[index] == FAR)
				{
					state[index] = TRIAL;
					double u1 = INF;
					double u2 = INF;
					double u3 = INF;
					if(w - 1 >= 0 && state[index - 1] == ALIVE) u1 = u1 < phi[index - 1]? u1: phi[index -1 ];
					if(w + 1 < sz0 && state[index + 1] == ALIVE) u1 = u1 < phi[index + 1] ? u1: phi[index + 1];
					if(h - 1 >= 0 && state[index - sz0] == ALIVE) u2 = u2 < phi[index - sz0] ? u2:phi[index-sz0];
					if(h + 1 < sz1 && state[index + sz0] == ALIVE) u2 = u2 < phi[index + sz0] ? u2:phi[index + sz0];
					if(d - 1 >=0 && state[index - sz0*sz1] == ALIVE) u3 = u3 < phi[index - sz0*sz1] ? u3: phi[index -sz0*sz1];
					if(d + 1 < sz2 && state[index + sz0*sz1] == ALIVE) u3 = u3 < phi[index + sz0*sz1] ? u3: phi[index + sz0*sz1];
					vector<double> parameters;
					if( u1 != INF) parameters.push_back(u1);
					if( u2 != INF) parameters.push_back(u2);
					if( u3 != INF) parameters.push_back(u3);
					phi[index] = upwind_solver(parameters);
					HeapElemXX *elem = new HeapElemXX(index, phi[index], root_node);
					heap.insert(elem);
					elems[index] = elem;
				}
			}
		}
	}
	// loop
	int time_counter = 1;
	int process1 = 0, process2 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*1000.0/tol_sz;
		if(process2 - process1 >= 1)
		{
			cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;
		}
		HeapElemXX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		state[min_ind] = ALIVE;
		int i = min_ind % sz0;
		int j = (min_ind/sz0) % sz1;
		int k = (min_ind/sz01) % sz2;

		MyMarker * cur_marker = new MyMarker(i,j,k);
		cur_marker->parent = min_elem->parent_marker;
		tree.push_back(cur_marker);
		delete min_elem;
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
					long index = d*sz01 + h*sz0 + w;
					if(inimg1d[index] == 0) continue;
					if(state[index] != ALIVE)
					{
						double u1 = INF;
						double u2 = INF;
						double u3 = INF;
						if(w - 1 >= 0 && state[index - 1] == ALIVE) u1 = u1 < phi[index - 1]? u1: phi[index -1 ];
						if(w + 1 < sz0 && state[index + 1] == ALIVE) u1 = u1 < phi[index + 1] ? u1: phi[index + 1];
						if(h - 1 >= 0 && state[index - sz0] == ALIVE) u2 = u2 < phi[index - sz0] ? u2:phi[index-sz0];
						if(h + 1 < sz1 && state[index + sz0] == ALIVE) u2 = u2 < phi[index + sz0] ? u2:phi[index + sz0];
						if(d - 1 >=0 && state[index - sz0*sz1] == ALIVE) u3 = u3 < phi[index - sz0*sz1] ? u3: phi[index -sz0*sz1];
						if(d + 1 < sz2 && state[index + sz0*sz1] == ALIVE) u3 = u3 < phi[index + sz0*sz1] ? u3: phi[index + sz0*sz1];
						vector<double> parameters;
						if( u1 != INF) parameters.push_back(u1);
						if( u2 != INF) parameters.push_back(u2);
						if( u3 != INF) parameters.push_back(u3);
						double solver_result = upwind_solver(parameters);
						if(state[index] == FAR)
						{
							phi[index] = solver_result;
							HeapElemXX * elem = new HeapElemXX(index, phi[index], cur_marker);
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > solver_result)
							{
								phi[index] = solver_result;
								HeapElemXX * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
								elem->parent_marker = cur_marker;
							}
						}
					}
				}
			}
		}
	}
	for(long i = 0; i < tol_sz; i++) if(phi[i] == INF) phi[i] = 0;
	if(state) {delete [] state; state = 0;}
	return true;
}
/******************************************************************************
 * Fast marching based tree construction
 * 1. use graph augmented distance (GD)
 * 2. stop when all target marker are marched
 * 
 * Input : root      root marker
 *         target    the set of target markers
 *         inimg1d   original input image
 *
 * Output : outtree   output tracing result
 *
 * Notice : 
 * 1. the input pixel number should not be larger than 2G if sizeof(long) == 4
 * 2. target markers should not contain root marker
 * 3. the root marker in outswc, is point to itself
 * 4. the cnn_type is default 3
 * *****************************************************************************/

template<class T> bool fastmarching_tree(MyMarker root, vector<MyMarker> &target, T * inimg1d, vector<MyMarker*> &outtree, long sz0, long sz1, long sz2, int cnn_type = 3)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
    //int cnn_type = 3;  // ?

    //float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
    //long * parent = new long[tol_sz]; for(long i = 0; i < tol_sz; i++) parent[i] = i;  // each pixel point to itself at the beginning

    long i;
    float * phi = 0;
    long * parent = 0;
    char * state = 0;
    try
    {
        phi = new float[tol_sz];
        parent = new long[tol_sz];
        state = new char[tol_sz];
        for(i = 0; i < tol_sz; i++)
        {
            phi[i] = INF;
            parent[i] = i;  // each pixel point to itself at the         statements beginning
            state[i] = FAR;
        }
    }
    catch (...)
    {
        cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
        if (phi) {delete []phi; phi=0;}
        if (parent) {delete []parent; parent=0;}
        if (state) {delete []state; state=0;}
        return false;
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
    //char * state = new char[tol_sz];
    //for(long i = 0; i < tol_sz; i++) state[i] = FAR;

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
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*100000.0/tol_sz;
		if(process2 - process1 >= 1)
		{
			cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
			bool is_break = true; for(int t = 0; t < target_inds.size(); t++){long tind = target_inds[t]; if(parent[tind] == tind && tind != root_ind) {is_break = false; break;}}
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
	// save current swc tree
	if(1)
	{
		int i = -1, j = -1, k = -1;
		map<long, MyMarker*> tmp_map;
		for(long ind = 0; ind < tol_sz; ind++) 
		{
			i++; if(i%sz0 == 0){i=0;j++; if(j%sz1 == 0) {j=0; k++;}}
			if(state[ind] != ALIVE) continue;
			MyMarker * marker = new MyMarker(i,j,k);
			tmp_map[ind] = marker;
			outtree.push_back(marker);
		}
		i=-1; j = -1; k = -1;
		for(long ind = 0; ind < tol_sz; ind++)
		{
			i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
			if(state[ind] != ALIVE) continue;
			long ind2 = parent[ind];
			MyMarker * marker1 = tmp_map[ind];
			MyMarker * marker2 = tmp_map[ind2];
			if(marker1 == marker2) marker1->parent = 0;
			else marker1->parent = marker2;
			//tmp_map[ind]->parent = tmp_map[ind2];
		}
	}
	// over
	
	map<long, HeapElemX*>::iterator mit = elems.begin(); while(mit != elems.end()){HeapElemX * elem = mit->second; delete elem; mit++;}

	if(phi){delete [] phi; phi = 0;}
	if(parent){delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}

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
 * 4. the cnn_type is default 3
 * *****************************************************************************/

template<class T1, class T2> bool fastmarching_tracing(MyMarker root, vector<MyMarker> &target, T1 * inimg1d, vector<MyMarker*> &outswc, T2 * &phi, int sz0, int sz1, int sz2, int cnn_type = 3)
{
	int rootx = root.x + 0.5;
	int rooty = root.y + 0.5;
	int rootz = root.z + 0.5;
	if(rootx < 0 || rootx >= sz0 || rooty < 0 || rooty >= sz1 || rootz < 0 || rootz >= sz2) 
	{
		cerr<<"Invalid root marker("<<root.x<<","<<root.y<<","<<root.z<<")"<<endl; return false;
	}

	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 3;  // ?

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

	long root_ind = rootz*sz01 + rooty*sz0 + rootx;
	state[root_ind] = ALIVE; 
	phi[root_ind] = 0.0;

	vector<long> target_inds;
	for(long t = 0; t < target.size(); t++) {
		int i = target[t].x + 0.5;
		int j = target[t].y + 0.5;
		int k = target[t].z + 0.5;
		if(i < 0 || i >= sz0 || j < 0 || j >= sz1 || k < 0 || k >= sz2)
		{
			cerr<<"t = "<<t+1<<", invalid target marker("<<target[t].x<<","<<target[t].y<<","<<target[t].z<<")"<<endl;
			continue;
		}
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
	double process1 = 0;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*100000.0/tol_sz;
		if(process2 - process1 >= 1)
		{
			cout<<"\r"<<((int)process2)/1000.0<<"%";cout.flush(); process1 = process2;
			bool is_break = true; for(int t = 0; t < target_inds.size(); t++){long tind = target_inds[t]; if(parent[tind] == tind && tind != root_ind) {is_break = false; break;}}
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

			if(p == parent[p]) 
			{
				assert(p == root.ind(sz0,sz01));
				assert(marker_map.find(root.ind(sz0,sz01)) != marker_map.end());
				break;
			}
			else p = parent[p];
		}
	}
	if(marker_map.find(root.ind(sz0,sz01)) == marker_map.end())
	{
		cout<<"break here"<<endl;
	}
	map<long, MyMarker*>::iterator it = marker_map.begin();
	V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
	while(it != marker_map.end())
	{
		long tind = it->first;
		MyMarker * marker = it->second;
		MyMarker * parent_marker = marker_map[parent[tind]];
		marker->parent = parent_marker;
		marker->radius = markerRadius(inimg1d, in_sz, *marker, 20);
		outswc.push_back(marker);
		it++;
	}
	
	map<long, HeapElemX*>::iterator mit = elems.begin();
	while(mit != elems.end()){HeapElemX * elem = mit->second; delete elem; mit++;}
	if(parent){delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}

#endif
