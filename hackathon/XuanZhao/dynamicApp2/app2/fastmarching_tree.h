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

#include <QString>

#include <fstream>
#include <sstream>
#include <algorithm>

#include "stackutil.h"
#include "my_surf_objs.h"
#include "heap.h"
#include "upwind_solver.h"
#include "fastmarching_macro.h"

#include "volimg_proc_declare.h"

#include "hierarchy_prune.h"

#include "regiongrow.h"

#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

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

/*********************************************************************
 * Function : fastmarching_tree_constraint
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
template<class T> bool fastmarching_tree_constraint(MyMarker root,
                                         T * inimg1d,
                                         vector<MyMarker*> &outtree,
                                         long sz0,
                                         long sz1,
                                         long sz2,
                                         int cnn_type = 3,
                                         double bkg_thresh = 20,
                                         bool is_break_accept = false,
                                         double length = 50)
{
    enum{ALIVE = -1, TRIAL = 0, FAR = 1};

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;
    long i;
    //int cnn_type = 3;  // ?

    float * phi = 0;
    long * parent = 0;
    char * state = 0;
    float * path = 0;
    try
    {
        phi = new float[tol_sz];
        parent = new long[tol_sz];
        state = new char[tol_sz];
        path = new float[tol_sz];
        for(i = 0; i < tol_sz; i++)
        {
            phi[i] = INF;
            parent[i] = i;  // each pixel point to itself at the         statements beginning
            state[i] = FAR;
            path[i] = 0;
        }
    }
    catch (...)
    {
        cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
        if (phi) {delete []phi; phi=0;}
        if (parent) {delete []parent; parent=0;}
        if (state) {delete []state; state=0;}
        if (path) {delete []path; path=0;}
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

        int pi = prev_ind % sz0;
        int pj = (prev_ind/sz0) % sz1;
        int pk = (prev_ind/sz01) % sz2;

        path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));
        if(path[min_ind]>length){
            while (!heap.empty()) {
                HeapElemX* tmp_elem = heap.delete_min();
                elems.erase(tmp_elem->img_ind);
                delete tmp_elem;
            }
            break;
        }

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
    if (path) {delete []path; path=0;}
    return true;
}

/*****
 *T: has objects of x,y,z
 *
 *
 */
template<class T>
XYZ getDirection(vector<T*> points, double cosThres){
    int pointsNum = points.size();
    if(pointsNum<=12){
        return XYZ(-3,-3,-3);
    }
    XYZ cur,cur_front,cur_back,p1,p2;
    double cosAngle;
    for(int i=6; i<pointsNum-6; i++){
        cur = XYZ(points[i]->x,points[i]->y,points[i]->z);
        cur_front = XYZ(0,0,0);
        cur_back = XYZ(0,0,0);
        for(int j=i-6; j<i; j+=2){
            cur_front = cur_front + XYZ(points[j]->x,points[j]->y,points[j]->z);
        }
        for(int j=i+2; j<=i+6; j+=2){
            cur_back = cur_back + XYZ(points[j]->x,points[j]->y,points[j]->z);
        }
        cur_front = cur_front / XYZ(3,3,3);
        cur_back = cur_back / XYZ(3,3,3);
        p1 = cur - cur_front;
        p2 = cur_back - cur;
        cosAngle = dot(normalize(p1),normalize(p2));
        qDebug()<<i<<" cosAngle: "<<cosAngle;
        if(cosAngle < cosThres){
            return XYZ(-2,-2,-2);
        }
    }

    cur = XYZ(0,0,0);
    cur_back = XYZ(0,0,0);
    for(int i=0; i<10; i+=2){
        cur = cur + XYZ(points[i]->x,points[i]->y,points[i]->z);
    }
    for(int i=pointsNum-1; i>=pointsNum-10; i-=2){
        cur_back = cur_back + XYZ(points[i]->x,points[i]->y,points[i]->z);
    }

    cur = cur / XYZ(5,5,5);
    cur_back = cur_back / XYZ(5,5,5);

    return cur_back - cur;

}

/*****
 *T: has objects of x,y,z
 *
 *
 */
template<class T>
XYZ getDirection2(vector<T*> points, double cosThres){
    int pointsNum = points.size();
    if(pointsNum<=12){
        return XYZ(-3,-3,-3);
    }
    XYZ cur,cur_front,cur_back,p1,p2;
    double cosAngle;
    int inflectionIndex = 0;
    for(int i=6; i<pointsNum-6; i++){
        cur = XYZ(points[i]->x,points[i]->y,points[i]->z);
        cur_front = XYZ(0,0,0);
        cur_back = XYZ(0,0,0);
        int count = 0;
        for(int j=i-6; j<i; j+=2){
            if(j<inflectionIndex)
                j = inflectionIndex;
            cur_front = cur_front + XYZ(points[j]->x,points[j]->y,points[j]->z);
            count++;
            if(count>=3)
                break;
        }
        cur_front = cur_front / XYZ(count,count,count);
        count = 0;
        for(int j=i+2; j<=i+6; j+=2){
            cur_back = cur_back + XYZ(points[j]->x,points[j]->y,points[j]->z);
        }

        cur_back = cur_back / XYZ(3,3,3);
        p1 = cur - cur_front;
        p2 = cur_back - cur;
        cosAngle = dot(normalize(p1),normalize(p2));
        qDebug()<<i<<" cosAngle: "<<cosAngle;
        if(cosAngle < cosThres){
            if(cosAngle < -0.5){
                return XYZ(-4,-4,-4);
            }
            if(inflectionIndex == 0){
                inflectionIndex = i;
            }else{
                return XYZ(-2,-2,-2);
            }
        }
    }

//    cur = XYZ(0,0,0);
//    cur_back = XYZ(0,0,0);
//    for(int i=0; i<10; i+=2){
//        cur = cur + XYZ(points[i]->x,points[i]->y,points[i]->z);
//    }
//    for(int i=pointsNum-1; i>=pointsNum-10; i-=2){
//        cur_back = cur_back + XYZ(points[i]->x,points[i]->y,points[i]->z);
//    }

//    cur = cur / XYZ(5,5,5);
//    cur_back = cur_back / XYZ(5,5,5);
    cur = XYZ(points[0]->x,points[0]->y,points[0]->z);
    if(inflectionIndex == 0){
        cur_back = XYZ(0,0,0);
        for(int i=pointsNum-1; i>=pointsNum-10; i-=2){
            cur_back = cur_back + XYZ(points[i]->x,points[i]->y,points[i]->z);
        }
        cur_back = cur_back / XYZ(5,5,5);
    }else{
        cur_back = XYZ(points[inflectionIndex]->x,points[inflectionIndex]->y,points[inflectionIndex]->z);
    }

    return cur_back - cur;

}

template<class T>
int getInflectionIndex(vector<T*> points, double cosThres){
    int pointsNum = points.size();
    if(pointsNum<=12){
        return 0;
    }
    XYZ cur,cur_front,cur_back,p1,p2;
    double cosAngle;
    int inflectionIndex = 0;
    for(int i=6; i<pointsNum-6; i++){
        cur = XYZ(points[i]->x,points[i]->y,points[i]->z);
        cur_front = XYZ(0,0,0);
        cur_back = XYZ(0,0,0);
        int count = 0;
        for(int j=i-6; j<i; j+=2){
            if(j<inflectionIndex)
                j = inflectionIndex;
            cur_front = cur_front + XYZ(points[j]->x,points[j]->y,points[j]->z);
            count++;
            if(count>=3)
                break;
        }
        cur_front = cur_front / XYZ(count,count,count);
        count = 0;
        for(int j=i+2; j<=i+6; j+=2){
            cur_back = cur_back + XYZ(points[j]->x,points[j]->y,points[j]->z);
        }

        cur_back = cur_back / XYZ(3,3,3);
        p1 = cur - cur_front;
        p2 = cur_back - cur;
        cosAngle = dot(normalize(p1),normalize(p2));
        qDebug()<<i<<" cosAngle: "<<cosAngle;
        if(cosAngle < cosThres){
            if(inflectionIndex == 0){
                inflectionIndex = i;
            }else{
                return inflectionIndex;
            }
        }
    }

}

//bool segCmp(HierarchySegment* a, HierarchySegment* b){
//    return a->length > b->length;
//}

template<class T>
double getSmoothWeight(T * inimg1d, V3DLONG index, long sz0, long sz1, long sz2){

    double wights[27] = {1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,
                      1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,
                      1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27,1.0/27};

    long sz01 = sz0*sz1;
    int x = index % sz0;
    int y = (index/sz0) % sz1;
    int z = (index/sz01) % sz2;

    double sum = 0;
    for(int k=-1; k<=1; k++){
        for(int j=-1; j<=1; j++){
            for(int i=-1; i<=1; i++){
                int xx = x + i;
                int yy = y + j;
                int zz = z + k;
                if(xx<0) xx = 0; if(xx>=sz0) xx = sz0-1;
                if(yy<0) yy = 0; if(yy>=sz1) yy = sz1-1;
                if(zz<0) zz = 0; if(zz>=sz2) zz = sz2-1;
                long curIndex = zz*sz01 + yy*sz0 + xx;
                sum += inimg1d[curIndex]*wights[(k+1)*9+(j+1)*3+i+1];
            }
        }
    }

    return sum;
}

template<class T>
double getDirectionWeight(T * inimg1d, V3DLONG index, long sz0, long sz1, long sz2, XYZ direction){
    double sum = 0;

    long sz01 = sz0*sz1;
    int x = index % sz0;
    int y = (index/sz0) % sz1;
    int z = (index/sz01) % sz2;

    for(int i=0; i<10; i+=2){
        int xx = x + direction.x*i + 0.5;
        int yy = y + direction.y*i + 0.5;
        int zz = z + direction.z*i + 0.5;
        if(xx<0) xx = 0; if(xx>=sz0) xx = sz0-1;
        if(yy<0) yy = 0; if(yy>=sz1) yy = sz1-1;
        if(zz<0) zz = 0; if(zz>=sz2) zz = sz2-1;

        long curIndex = zz*sz01 + yy*sz0 + xx;
        sum += inimg1d[curIndex];
    }

    if(sum>0)
        sum /= 5;

    return sum;

}


/*********************************************************************
 * Function : fastmarching_tree_line
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
template<class T> bool fastmarching_line(MyMarker root,
                                         T * inimg1d,
                                         vector<MyMarker*> &outtree,
                                         long sz0,
                                         long sz1,
                                         long sz2,
                                         int cnn_type = 3,
                                         double bkg_thresh = 20,
                                         bool is_break_accept = false,
                                         double length = 50,
                                         XYZ lastDirection = XYZ(0,0,0),
                                         double zScale = 1)
{
    is_break_accept = true;
    bkg_thresh = 0;
    enum{ALIVE = -1, TRIAL = 0, FAR = 1, FINAL = 2, TMP = 3};

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;
//    long i;
    //int cnn_type = 3;  // ?

    float * phi = 0;
    long * parent = 0;
    char * state = 0;
    float * path = 0;
    try
    {
        phi = new float[tol_sz];
        parent = new long[tol_sz];
        state = new char[tol_sz];
        path = new float[tol_sz];
        for(long i = 0; i < tol_sz; i++)
        {
            phi[i] = INF;
            parent[i] = i;  // each pixel point to itself at the         statements beginning
            state[i] = FAR;
            path[i] = 0;
        }
    }
    catch (...)
    {
        cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
        if (phi) {delete []phi; phi=0;}
        if (parent) {delete []parent; parent=0;}
        if (state) {delete []state; state=0;}
        if (path) {delete []path; path=0;}
        return false;
    }

    // GI parameter min_int, max_int, li
    double max_int = 0; // maximum intensity, used in GI
    double min_int = INF;
    for(long i = 0; i < tol_sz; i++)
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

    int t = 1;
    int stopFlag = 0;
    while (stopFlag < 2) {

        qDebug()<<"t: "<<t;

        stopFlag += 1;
        // loop
        int time_counter = 1;
        double process1 = 0;
        
//        HeapElemX* tempElem = heap.delete_min();
//        HeapElemX* tempElemCopy = new HeapElemX(tempElem->img_ind,tempElem->value);
//        tempElemCopy->prev_ind = tempElem->prev_ind;
//        heap.insert(tempElem);

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

            int pi = prev_ind % sz0;
            int pj = (prev_ind/sz0) % sz1;
            int pk = (prev_ind/sz01) % sz2;

            path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

            if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1) && t != 1){
                stopFlag = 2;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }
//            qDebug()<<"path[min_ind]: "<<path[min_ind];
            if(path[min_ind]>length){
                stopFlag = 0;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }


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
//                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                        double factor = sqrt(ii*ii + jj*jj + kk*kk*zScale*zScale);
                        long index = d*sz01 + h*sz0 + w;
//                        if (is_break_accept)
//                        {
//                            if(inimg1d[index] <= bkg_thresh &&
//                               inimg1d[min_ind] <= bkg_thresh)
//                                continue;
//                        }
//                        else
//                        {
//                            if(inimg1d[index] <= bkg_thresh)
//                                continue;
//                        }
//                        qDebug()<<"state[index]: "<<(int)state[index];

                        if(state[index] != ALIVE && state[index] != FINAL)
                        {
                            double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;

//                            double indexSG = getSmoothWeight(inimg1d,index,sz0,sz1,sz2);
//                            double min_indSG = getSmoothWeight(inimg1d,min_ind,sz0,sz1,sz2);

//                            XYZ curDire = XYZ(w-i,h-j,d-k);
//                            curDire = normalize(curDire);
//                            double indexDG = getDirectionWeight(inimg1d,index,sz0,sz1,sz2,curDire);
//                            double min_indDG = getDirectionWeight(inimg1d,min_ind,sz0,sz1,sz2,curDire);

//                            double direFactor = 1;//exp(1.0/(dot(curDire,normalize(lastDirection))+1.01));
//                            double new_dist = phi[min_ind] + (GIG(indexSG) + GIG(min_indSG))*factor*0.5*direFactor;

//                            double new_dist = phi[min_ind] + (GIG(indexDG) + GIG(min_indDG))*factor*0.5;

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

        /*
        max_int = 0; // maximum intensity, used in GI
        min_int = INF;
        for(long ind=0; ind<tol_sz; ind++){
            if(state[ind] == TRIAL){
                state[ind] = FAR;
            }else if (state[ind] == ALIVE) {
                if (inimg1d[ind] > max_int) max_int = inimg1d[ind];
                if (inimg1d[ind] < min_int) min_int = inimg1d[ind];
                state[ind] = FAR;
            }
            phi[ind] = INF;
            path[ind] = 0;
            parent[ind] = ind;

        }
        max_int -= min_int;

        //init heap
        {
            heap.insert(tempElemCopy);
            elems[tempElemCopy->img_ind] = tempElemCopy;
            path[tempElemCopy->img_ind] = 0;
            phi[tempElemCopy->img_ind] = 0;
        }

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

            int pi = prev_ind % sz0;
            int pj = (prev_ind/sz0) % sz1;
            int pk = (prev_ind/sz01) % sz2;

            path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

            if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1) && t != 1){
                stopFlag = 2;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }
//            qDebug()<<"path[min_ind]: "<<path[min_ind];
            if(path[min_ind]>length){
                stopFlag = 0;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }


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
//                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                        double factor = sqrt(ii*ii + jj*jj + kk*kk*zScale*zScale);
                        long index = d*sz01 + h*sz0 + w;
//                        if (is_break_accept)
//                        {
//                            if(inimg1d[index] <= bkg_thresh &&
//                               inimg1d[min_ind] <= bkg_thresh)
//                                continue;
//                        }
//                        else
//                        {
//                            if(inimg1d[index] <= bkg_thresh)
//                                continue;
//                        }
//                        qDebug()<<"state[index]: "<<(int)state[index];

                        if(state[index] != ALIVE && state[index] != FINAL)
                        {
                            double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;

//                            double indexSG = getSmoothWeight(inimg1d,index,sz0,sz1,sz2);
//                            double min_indSG = getSmoothWeight(inimg1d,min_ind,sz0,sz1,sz2);

//                            XYZ curDire = XYZ(w-i,h-j,d-k);
//                            curDire = normalize(curDire);
//                            double indexDG = getDirectionWeight(inimg1d,index,sz0,sz1,sz2,curDire);
//                            double min_indDG = getDirectionWeight(inimg1d,min_ind,sz0,sz1,sz2,curDire);

//                            double direFactor = 1;//exp(1.0/(dot(curDire,normalize(lastDirection))+1.01));
//                            double new_dist = phi[min_ind] + (GIG(indexSG) + GIG(min_indSG))*factor*0.5*direFactor;

//                            double new_dist = phi[min_ind] + (GIG(indexDG) + GIG(min_indDG))*factor*0.5;

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
        */

        qDebug()<<"-------find next startPoint----------";

        vector<MyMarker*> tmptree = vector<MyMarker*>();
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
                tmptree.push_back(marker);
    //            outtree.push_back(marker);
            }
            i=-1; j = -1; k = -1;
            for(long ind = 0; ind < tol_sz; ind++)
            {
                i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
                if(state[ind] == TRIAL)
                    state[ind] = FAR;
                if(state[ind] != ALIVE) continue;
                state[ind] = FINAL; //set final
                long ind2 = parent[ind];
                MyMarker * marker1 = tmp_map[ind];
                MyMarker * marker2 = tmp_map[ind2];
                if(marker1 == marker2) marker1->parent = 0;
                else marker1->parent = marker2;
                //tmp_map[ind]->parent = tmp_map[ind2];
            }
        }
        QString s = "D:\\testDynamicTracing\\" + QString::number(t) + "_init.swc";
        saveSWC_file(s.toStdString(),tmptree);

        qDebug()<<"tmpTree size: "<<tmptree.size();

        if(tmptree.size()<2){
            break;
        }

        MyMarker* rootMarker;
        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]->parent == 0)
                rootMarker = tmptree[i];
        }
        vector<HierarchySegment*> segs = vector<HierarchySegment*>();
        swc2topo_segs<unsigned char>(tmptree,segs,1);

//        int segMaxLength = 0;
//        int segMaxIndex = -1;

        ofstream csvFile;
        QString csvPath = "D:\\testDynamicTracing\\" + QString::number(t) + "_segments.csv";
        csvFile.open(csvPath.toStdString().c_str(), ios::out);
        csvFile<<"level"<<','<<"length"<<endl;

        sort(segs.begin(),segs.end(),segCmp);
        vector<HierarchySegment*> tmpSegs = vector<HierarchySegment*>();
//        for(int i=0; i<segs.size(); i++){
//            csvFile<<segs[i]->level<<','<<segs[i]->length<<endl;
//            if(segs[i]->length > length/3){
//                tmpSegs.push_back(segs[i]);
//            }
//            if(segs[i]->root_marker == rootMarker && segs[i]->length>segMaxLength){
//                segMaxLength = segs[i]->length;
//                segMaxIndex = i;
//            }
//        }
        for(int i=0; i<15; i++){
            if(i>=segs.size())
                break;
            csvFile<<segs[i]->level<<','<<segs[i]->length<<endl;
            tmpSegs.push_back(segs[i]);
        }
        csvFile.close();
        HierarchySegment* targetSegment = 0;

        //select the candidate segment
        vector<HierarchySegment*> candidate = vector<HierarchySegment*>();
        vector<XYZ> candidateDire = vector<XYZ>();
        for(int i=0; i<tmpSegs.size(); i++){
             vector<MyMarker*> tmpMarkers = vector<MyMarker*>();
             MyMarker* marker = tmpSegs[i]->leaf_marker;

             XYZ judgeSameDire = XYZ(marker->x-rootMarker->x,marker->y-rootMarker->y,marker->z-rootMarker->z);
             tmpMarkers.push_back(marker);
             while(marker != rootMarker){
                 marker = marker->parent;
                 tmpMarkers.push_back(marker);
             }
             qDebug()<<"before tmpMarker size:"<<tmpMarkers.size();
             if(t != 1){
                 marker = outtree.back();marker = outtree.back();
                 int foreNum = 0;
                 while(marker->parent != 0){
                     tmpMarkers.push_back(marker);
                     marker = marker->parent;
                     foreNum++;
                     if(foreNum>=6)
                         break;
                 }
             }
             qDebug()<<"after tmpMarker size:"<<tmpMarkers.size();


             reverse(tmpMarkers.begin(),tmpMarkers.end());

             qDebug()<<i<<"---------------------------------------------------------";
             XYZ tmpDire;
             if(i == 0){
                 tmpDire = getDirection2(tmpMarkers,0.6);
             }else{
                 tmpDire = getDirection2(tmpMarkers,0.5);
             }
             qDebug()<<"length: "<<tmpSegs[i]->length<<" tmpDire: "<<tmpDire.x<<" "<<tmpDire.y<<" "<<tmpDire.z;
             if(tmpDire == XYZ(-3,-3,-3) || tmpDire == XYZ(-2,-2,-2) || tmpDire == XYZ(-4,-4,-4)){
                 continue;
             }else {
                 double tmpCosAngle = dot(normalize(tmpDire),normalize(lastDirection));

                 qDebug()<<"tmpCosAngle: "<<tmpCosAngle;
                 if(1/*tmpCosAngle>sqrt(2)/2*/){
                     if(i == 0){
                         targetSegment = tmpSegs[0];
                         QString sss = "D:\\testDynamicTracing\\" + QString::number(t) + "_"
                                 + QString::number(i) + "_" + QString::number(tmpCosAngle) + "_candidate.swc";
                         saveSWC_file(sss.toStdString(),tmpMarkers);

                         vector<MyMarker*> markers = vector<MyMarker*>();
                 //        segs[segMaxIndex]->get_markers(markers);
                         MyMarker* markerTmp = targetSegment->leaf_marker;
                         markers.push_back(markerTmp);
                         while (markerTmp != rootMarker) {
                             markerTmp = markerTmp->parent;
                             markers.push_back(markerTmp);
                         }
                         qDebug()<<"target markers size: "<<markers.size();



                         XYZ cur = XYZ(0,0,0);
                         XYZ cur_front = XYZ(0,0,0);
                         int mcount = 0;
                         for(int i=0; i<10; i+=2){
                             if(i>=markers.size()){
                                 break;
                             }
                             cur = cur + XYZ(markers[i]->x,markers[i]->y,markers[i]->z);
                             mcount++;
                         }
                         cur = cur/XYZ(mcount,mcount,mcount);
                         mcount = 0;
                         for(int i=markers.size()-1; i>markers.size()-10; i-=2){
                             if(i<0){
                                 break;
                             }
                             cur_front = cur_front + XYZ(markers[i]->x,markers[i]->y,markers[i]->z);
                             mcount++;
                         }
                         cur_front = cur_front/XYZ(mcount,mcount,mcount);

                         XYZ curDirection = cur - cur_front;
                         double curCosAngle = dot(normalize(curDirection),normalize(lastDirection));
                         if(curCosAngle<-0.9){
                             qDebug()<<"the longest seg ie reverse";
                             targetSegment = 0;
                         }else{
                             break;
                         }


                     }else
                     {
                         if(candidate.empty()){
                             candidate.push_back(tmpSegs[i]);
                             candidateDire.push_back(judgeSameDire);
                             QString sss = "D:\\testDynamicTracing\\" + QString::number(t) + "_"
                                     + QString::number(i) + "_" + QString::number(tmpCosAngle) + "_candidate.swc";
                             saveSWC_file(sss.toStdString(),tmpMarkers);
                         }else {
                             bool isSame = false;
                             int sameIndex = -1;
                             for(int j=0; j<candidate.size(); j++){
                                 double segsCosAngle = dot(normalize(judgeSameDire),normalize(candidateDire[j]));
                                 if(segsCosAngle>0.9){
                                     sameIndex = j;
                                     isSame = true;
                                     break;
                                 }
                             }
                             if(!isSame){
                                 candidate.push_back(tmpSegs[i]);
                                 candidateDire.push_back(judgeSameDire);
                                 QString sss = "D:\\testDynamicTracing\\" + QString::number(t) + "_"
                                         + QString::number(i) + "_" + QString::number(tmpCosAngle) + "_candidate.swc";
                                 saveSWC_file(sss.toStdString(),tmpMarkers);
                             }else{

                                 QString sss = "D:\\testDynamicTracing\\" + QString::number(t) + "_"
                                         + QString::number(i) + "_" + QString::number(tmpCosAngle) + "_Notcandidate.swc";
                                 saveSWC_file(sss.toStdString(),tmpMarkers);


//                                 MyMarker* curLeafMarker = tmpSegs[i]->leaf_marker;
//                                 MyMarker* lastLeafMarker = candidate[sameIndex]->leaf_marker;
//                                 double curLeafIntensity = 0.0, lastLeafIntensity = 0.0;

//                                 int tmpCount = 0;
//                                 for(int j=0; j<5; j++){
//                                     long curLeafInd = curLeafMarker->z*sz01
//                                             + curLeafMarker->y*sz0
//                                             + curLeafMarker->x;
//                                     long lastLeafInd = lastLeafMarker->z*sz01
//                                             + lastLeafMarker->y*sz0
//                                             + lastLeafMarker->x;
//                                     qDebug()<<j<<" "<<curLeafInd<<" "<<lastLeafInd;
//                                     curLeafIntensity += inimg1d[curLeafInd];
//                                     lastLeafIntensity += inimg1d[lastLeafInd];
//                                     tmpCount++;
//                                     if(curLeafMarker->parent != rootMarker && lastLeafMarker->parent != rootMarker){
//                                         curLeafMarker = curLeafMarker->parent;
//                                         lastLeafMarker = lastLeafMarker->parent;
//                                     }else{
//                                         break;
//                                     }

//                                 }

//                                 qDebug()<<"curLeafIntensity: "<<curLeafIntensity<<" lastLeafIntensity: "<<lastLeafIntensity;
//                                 if(curLeafIntensity>lastLeafIntensity){
//                                     candidate[sameIndex] = tmpSegs[i];
//                                 }
                             }
                         }
                     }
                 }
             }
        }

        qDebug()<<"canditate size: "<<candidate.size();



        if(!candidate.empty()){
            vector<HierarchySegment*> candidate2 = vector<HierarchySegment*>();
            for(int c=0; c<candidate.size(); c++){
                qDebug()<<c<<"-------judge candidate seg------";
                MyMarker* leafMarker = candidate[c]->leaf_marker;
                long index = leafMarker->z*sz01 + leafMarker->y*sz0 +leafMarker->x;
                phi[index] = 0;
                path[index] = 0;
                HeapElemX *elem = new HeapElemX(index, phi[index]);
                elem->prev_ind = index;
                heap.insert(elem);
                elems[index] = elem;

                while(!heap.empty())
                {
                    HeapElemX* min_elem = heap.delete_min();
                    elems.erase(min_elem->img_ind);

                    long min_ind = min_elem->img_ind;
                    long prev_ind = min_elem->prev_ind;
                    delete min_elem;

                    parent[min_ind] = prev_ind;

                    state[min_ind] = TMP;
                    int i = min_ind % sz0;
                    int j = (min_ind/sz0) % sz1;
                    int k = (min_ind/sz01) % sz2;

                    int pi = prev_ind % sz0;
                    int pj = (prev_ind/sz0) % sz1;
                    int pk = (prev_ind/sz01) % sz2;

                    path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

                    if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1) && t != 1){
                        while (!heap.empty()) {
                            HeapElemX* tmp_elem = heap.delete_min();
                            elems.erase(tmp_elem->img_ind);
                            delete tmp_elem;
                        }
                        break;
                    }

                    if(path[min_ind]>length){
                        while (!heap.empty()) {
                            HeapElemX* tmp_elem = heap.delete_min();
                            elems.erase(tmp_elem->img_ind);
                            delete tmp_elem;
                        }
                        break;
                    }

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
        //                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                                double factor = sqrt(ii*ii + jj*jj + kk*kk*zScale*zScale);
                                long index = d*sz01 + h*sz0 + w;
//                                    if (is_break_accept)
//                                    {
//                                        if(inimg1d[index] <= bkg_thresh &&
//                                           inimg1d[min_ind] <= bkg_thresh)
//                                            continue;
//                                    }
//                                    else
//                                    {
//                                        if(inimg1d[index] <= bkg_thresh)
//                                            continue;
//                                    }

                                if(state[index] != ALIVE && state[index] != FINAL && state[index] != TMP)
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

                qDebug()<<"end heap";

                vector<MyMarker*> tmptree2 = vector<MyMarker*>();
                if (1)
                {
                    int i = -1, j = -1, k = -1;
                    map<long, MyMarker*> tmp_map2;
                    for(long ind = 0; ind < tol_sz; ind++)
                    {
                        i++; if(i%sz0 == 0){i=0;j++; if(j%sz1 == 0) {j=0; k++;}}
                        if(state[ind] != TMP) continue;
                        MyMarker * marker = new MyMarker(i,j,k);
                        tmp_map2[ind] = marker;
                        tmptree2.push_back(marker);
            //            outtree.push_back(marker);
                    }
                    i=-1; j = -1; k = -1;
                    for(long ind = 0; ind < tol_sz; ind++)
                    {
                        i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
                        if(state[ind] == TRIAL)
                            state[ind] = FAR;
                        if(state[ind] != TMP) continue;
                        state[ind] = FAR; //set far
                        long ind2 = parent[ind];
                        MyMarker * marker1 = tmp_map2[ind];
                        MyMarker * marker2 = tmp_map2[ind2];
                        if(marker1 == marker2) marker1->parent = 0;
                        else marker1->parent = marker2;
                        //tmp_map[ind]->parent = tmp_map[ind2];
                    }
                }
                qDebug()<<"make tmptree2 end";

                MyMarker* rootMarker2;
                map<long,MyMarker*> indexMap = map<long,MyMarker*>();
                double midX = 0, midY = 0, midZ = 0;
                for(int i=0; i<tmptree2.size(); i++){
                    long markerIndex = tmptree2[i]->z*sz01 + tmptree2[i]->y*sz0 + tmptree2[i]->x;
                    midX += tmptree2[i]->x;
                    midY += tmptree2[i]->y;
                    midZ += tmptree2[i]->z;
                    indexMap[markerIndex] = tmptree2[i];
                    if(tmptree2[i]->parent == 0){
                        rootMarker2 = tmptree2[i];
//                        break;
                    }
                }

                midX /= tmptree2.size();
                midY /= tmptree2.size();
                midZ /= tmptree2.size();
                long midIndex = (int)(midZ+0.5)*sz01 + (int)(midY+0.5)*sz0 + (int)(midX+0.5);
                MyMarker* midMarker;
                if(indexMap.find(midIndex) != indexMap.end()){
                    midMarker = indexMap[midIndex];
                }else{
                    qDebug()<<" the seg is right";
                    candidate2.push_back(candidate[c]);
                    for(int i=0; i<tmptree2.size(); i++){
                        if(tmptree2[i]){
                            delete tmptree2[i];
                        }
                    }
                    tmptree2.clear();
                    continue;
                }

//                vector<HierarchySegment*> segs2 = vector<HierarchySegment*>();
//                swc2topo_segs<unsigned char>(tmptree2,segs2,1);

//                int segMaxLength2 = 0;
//                int segMaxIndex2 = -1;

//                qDebug()<<"convert HierarchySegment end";


//                for(int i=0; i<segs2.size(); i++){
//                    if(segs2[i]->root_marker == rootMarker2 && segs2[i]->length>segMaxLength2){
//                        segMaxLength2 = segs2[i]->length;
//                        segMaxIndex2 = i;
//                    }
//                }
//                if(segMaxIndex2 == -1){
//                    qDebug()<<"segs2 size:"<<segs2.size();
//                    continue;
//                }

                vector<MyMarker*> markers2 = vector<MyMarker*>();
//                segs2[segMaxIndex2]->get_markers(markers2);

                markers2.push_back(midMarker);
                while(midMarker!=rootMarker2){
                    midMarker = midMarker->parent;
                    markers2.push_back(midMarker);
                }

                qDebug()<<"get markers end";
                QString sss = "D:\\testDynamicTracing\\" + QString::number(t) + "_" + QString::number(c) + "_tmp_test.swc";
                saveSWC_file(sss.toStdString(),markers2);


                int foreNum = 0;
                for(int i=0; i<markers2.size(); i++){
                    MyMarker* marker = markers2[i];
                    long ind2 = marker->z*sz01 + marker->y*sz0 + marker->x;
//                        qDebug()<<(int)inimg1d[ind2]<<" bkg_thres"<<bkg_thresh;
                    if(inimg1d[ind2] >= bkg_thresh){
                        foreNum++;
                    }
                }

                qDebug()<<" caculate forenum";

                if((double)foreNum/markers2.size()>0.5){
//                    targetSegment = segs[segMaxIndex];
                    candidate2.push_back(candidate[c]);
                    qDebug()<<"foreNumratio: "<<(double)foreNum/markers2.size()<<" the seg is right";
                }else{
                    qDebug()<<"foreNumratio: "<<(double)foreNum/markers2.size()<<" the seg is wrong";
                }
                markers2.clear();
                for(int i=0; i<tmptree2.size(); i++){
                    if(tmptree2[i]){
                        delete tmptree2[i];
                    }
                }
                tmptree2.clear();
            }

            if(candidate2.empty()){
                targetSegment = segs[0];
            }else{
                qDebug()<<"candidate2 size: "<<candidate2.size();
                double lengthMax = 0;
                double angleMin = -1;
                for(int i=0; i<candidate2.size(); i++){

                    vector<MyMarker*> tmpMarkers = vector<MyMarker*>();
                    MyMarker* marker = candidate2[i]->leaf_marker;
                    tmpMarkers.push_back(marker);
                    while(marker != rootMarker){
                        marker = marker->parent;
                        tmpMarkers.push_back(marker);
                    }
                    reverse(tmpMarkers.begin(),tmpMarkers.end());

                    QString sss = "D:\\testDynamicTracing\\" + QString::number(t) + "_" + QString::number(i) + "_candidate2.swc";
                    saveSWC_file(sss.toStdString(),tmpMarkers);

                    XYZ tmpDire = getDirection(tmpMarkers,0.5);
                    qDebug()<<"length: "<<candidate2[i]->length<<" tmpDire: "<<tmpDire.x<<" "<<tmpDire.y<<" "<<tmpDire.z;

                    if(tmpDire == XYZ(-3,-3,-3) || tmpDire == XYZ(-2,-2,-2) || tmpDire == XYZ(-4,-4,-4)){
                        continue;
                    }else {
                        double tmpCosAngle = dot(normalize(tmpDire),normalize(lastDirection));
                        qDebug()<<"tmpCosAngle: "<<tmpCosAngle;
                        if(tmpCosAngle>angleMin){
                            angleMin = tmpCosAngle;
                            targetSegment = candidate2[i];
                        }
                    }
//                    double lengthTmp = 0;
//                    MyMarker* marker = candidate2[i]->leaf_marker;
//                    while(marker != rootMarker){
//                        lengthTmp += dist(*marker,*(marker->parent));
//                        marker = marker->parent;
//                    }
//                    if(lengthTmp>lengthMax){
//                        qDebug()<<i<<" candidate length: "<<lengthTmp;
//                        lengthMax = lengthTmp;
//                        targetSegment = candidate2[i];
//                    }
                }

                if(targetSegment == 0){
                    targetSegment = segs[0];
                }
            }


        }else {
            targetSegment = segs[0];
        }


/*
        if(tmpSegs.size() == 0){
            targetSegment = segs[segMaxIndex];
        }else if(tmpSegs.size() == 1){
            targetSegment = tmpSegs[0];
        }else {
            vector<HierarchySegment*> candidate = vector<HierarchySegment*>();
            for(int i=0; i<tmpSegs.size(); i++){
                 vector<MyMarker*> tmpMarkers = vector<MyMarker*>();
                 MyMarker* marker = tmpSegs[i]->leaf_marker;
                 tmpMarkers.push_back(marker);
                 while(marker != rootMarker){
                     marker = marker->parent;
                     tmpMarkers.push_back(marker);
                 }
                 reverse(tmpMarkers.begin(),tmpMarkers.end());

                 qDebug()<<i<<"---------------------------------------------------------";
                 XYZ tmpDire = getDirection(tmpMarkers,0.5);
                 qDebug()<<"length: "<<tmpSegs[i]->length<<" tmpDire: "<<tmpDire.x<<" "<<tmpDire.y<<" "<<tmpDire.z;
                 if(tmpDire == XYZ(-3,-3,-3) || tmpDire == XYZ(-2,-2,-2)){
                     continue;
                 }else {
                     double tmpCosAngle = dot(normalize(tmpDire),normalize(lastDirection));
                     qDebug()<<"tmpCosAngle: "<<tmpCosAngle;
                     if(tmpCosAngle>sqrt(2)/2){
                         candidate.push_back(tmpSegs[i]);
                     }
                 }
            }

            if(candidate.empty()){
                double lengthMax = 0;
                for(int i=0; i<tmpSegs.size(); i++){
                    double lengthTmp = 0;
                    MyMarker* marker = tmpSegs[i]->leaf_marker;
                    while(marker != rootMarker){
                        lengthTmp += dist(*marker,*(marker->parent));
                        marker = marker->parent;
                    }

                    if(lengthTmp>lengthMax){
                        qDebug()<<i<<" length: "<<lengthTmp;
                        lengthMax = lengthTmp;
                        targetSegment = tmpSegs[i];
                    }
                }
            }else {
                double lengthMax = 0;
                qDebug()<<"candidate size: "<<candidate.size();
                for(int i=0; i<candidate.size(); i++){
                    double lengthTmp = 0;
                    MyMarker* marker = candidate[i]->leaf_marker;
                    while(marker != rootMarker){
                        lengthTmp += dist(*marker,*(marker->parent));
                        marker = marker->parent;
                    }
                    if(lengthTmp>lengthMax){
                        qDebug()<<i<<" candidate length: "<<lengthTmp;
                        lengthMax = lengthTmp;
                        targetSegment = candidate[i];
                    }
                }
                if(targetSegment != segs[segMaxIndex]){
                    qDebug()<<"------targetSegment != segs[segMaxIndex]-------";
                    MyMarker* leafMarker = targetSegment->leaf_marker;
                    long index = leafMarker->z*sz01 + leafMarker->y*sz0 +leafMarker->x;
                    phi[index] = 0;
                    path[index] = 0;
                    HeapElemX *elem = new HeapElemX(index, phi[index]);
                    elem->prev_ind = index;
                    heap.insert(elem);
                    elems[index] = elem;

                    while(!heap.empty())
                    {
                        HeapElemX* min_elem = heap.delete_min();
                        elems.erase(min_elem->img_ind);

                        long min_ind = min_elem->img_ind;
                        long prev_ind = min_elem->prev_ind;
                        delete min_elem;

                        parent[min_ind] = prev_ind;

                        state[min_ind] = TMP;
                        int i = min_ind % sz0;
                        int j = (min_ind/sz0) % sz1;
                        int k = (min_ind/sz01) % sz2;

                        int pi = prev_ind % sz0;
                        int pj = (prev_ind/sz0) % sz1;
                        int pk = (prev_ind/sz01) % sz2;

                        path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

                        if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1) && t != 1){
                            while (!heap.empty()) {
                                HeapElemX* tmp_elem = heap.delete_min();
                                elems.erase(tmp_elem->img_ind);
                                delete tmp_elem;
                            }
                            break;
                        }

                        if(path[min_ind]>length){
                            while (!heap.empty()) {
                                HeapElemX* tmp_elem = heap.delete_min();
                                elems.erase(tmp_elem->img_ind);
                                delete tmp_elem;
                            }
                            break;
                        }

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
            //                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                                    double factor = sqrt(ii*ii + jj*jj + kk*kk*zScale*zScale);
                                    long index = d*sz01 + h*sz0 + w;
//                                    if (is_break_accept)
//                                    {
//                                        if(inimg1d[index] <= bkg_thresh &&
//                                           inimg1d[min_ind] <= bkg_thresh)
//                                            continue;
//                                    }
//                                    else
//                                    {
//                                        if(inimg1d[index] <= bkg_thresh)
//                                            continue;
//                                    }

                                    if(state[index] != ALIVE && state[index] != FINAL && state[index] != TMP)
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

                    qDebug()<<"end heap";

                    vector<MyMarker*> tmptree2 = vector<MyMarker*>();
                    if (1)
                    {
                        int i = -1, j = -1, k = -1;
                        map<long, MyMarker*> tmp_map2;
                        for(long ind = 0; ind < tol_sz; ind++)
                        {
                            i++; if(i%sz0 == 0){i=0;j++; if(j%sz1 == 0) {j=0; k++;}}
                            if(state[ind] != TMP) continue;
                            MyMarker * marker = new MyMarker(i,j,k);
                            tmp_map2[ind] = marker;
                            tmptree2.push_back(marker);
                //            outtree.push_back(marker);
                        }
                        i=-1; j = -1; k = -1;
                        for(long ind = 0; ind < tol_sz; ind++)
                        {
                            i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
                            if(state[ind] == TRIAL)
                                state[ind] = FAR;
                            if(state[ind] != TMP) continue;
                            state[ind] = FAR; //set far
                            long ind2 = parent[ind];
                            MyMarker * marker1 = tmp_map2[ind];
                            MyMarker * marker2 = tmp_map2[ind2];
                            if(marker1 == marker2) marker1->parent = 0;
                            else marker1->parent = marker2;
                            //tmp_map[ind]->parent = tmp_map[ind2];
                        }
                    }
                    qDebug()<<"make tmptree2 end";

                    MyMarker* rootMarker2;
                    for(int i=0; i<tmptree2.size(); i++){
                        if(tmptree2[i]->parent == 0){
                            rootMarker2 = tmptree2[i];
                            break;
                        }
                    }
                    vector<HierarchySegment*> segs2 = vector<HierarchySegment*>();
                    swc2topo_segs<unsigned char>(tmptree2,segs2,1);

                    int segMaxLength2 = 0;
                    int segMaxIndex2 = -1;

                    qDebug()<<"convert HierarchySegment end";


                    for(int i=0; i<segs2.size(); i++){
                        if(segs2[i]->root_marker == rootMarker2 && segs2[i]->length>segMaxLength2){
                            segMaxLength2 = segs2[i]->length;
                            segMaxIndex2 = i;
                        }
                    }

                    vector<MyMarker*> markers2 = vector<MyMarker*>();
                    segs2[segMaxIndex2]->get_markers(markers2);

                    qDebug()<<"get markers end";
                    QString sss = "D:\\testDynamicTracing\\" + QString::number(t) + "_tmp_test.swc";
                    saveSWC_file(sss.toStdString(),markers2);


                    int foreNum = 0;
                    for(int i=0; i<markers2.size(); i++){
                        MyMarker* marker = markers2[i];
                        long ind2 = marker->z*sz01 + marker->y*sz0 + marker->x;
//                        qDebug()<<(int)inimg1d[ind2]<<" bkg_thres"<<bkg_thresh;
                        if(inimg1d[ind2] >= bkg_thresh){
                            foreNum++;
                        }
                    }

                    qDebug()<<" caculate forenum";

                    if((double)foreNum/markers2.size()<0.5){
                        targetSegment = segs[segMaxIndex];
                        qDebug()<<"foreNumratio: "<<(double)foreNum/markers2.size()<<" the seg is not right";
                    }
                    markers2.clear();
                    for(int i=0; i<tmptree2.size(); i++){
                        if(tmptree2[i]){
                            delete tmptree2[i];
                        }
                    }
                    tmptree2.clear();


                }

            }
        }
*/
//        targetSegment = segs[0];
        qDebug()<<"targetLength: "<<targetSegment->length;

        vector<MyMarker*> tmpSwc = vector<MyMarker*>();
        topo_segs2swc(tmpSegs,tmpSwc,1);
        QString ss = "\\testDynamicTracing\\" + QString::number(t) + "_tmp.swc";
        saveSWC_file(ss.toStdString(),tmpSwc);

//        qDebug()<<"segMaxLength: "<<segMaxLength;

        // init heap
        {
            MyMarker* leafMarker = targetSegment->leaf_marker;
            long index = leafMarker->z*sz01 + leafMarker->y*sz0 +leafMarker->x;
            phi[index] = 0;
//            parent[index] = index;
            path[index] = 0;
            HeapElemX *elem = new HeapElemX(index, phi[index]);
            elem->prev_ind = index;
            heap.insert(elem);
            elems[index] = elem;
        }

        qDebug()<<"----init heap end-----";

        vector<MyMarker*> markers = vector<MyMarker*>();
//        segs[segMaxIndex]->get_markers(markers);
        MyMarker* markerTmp = targetSegment->leaf_marker;
        markers.push_back(markerTmp);
        while (markerTmp != rootMarker) {
            markerTmp = markerTmp->parent;
            markers.push_back(markerTmp);
        }
        qDebug()<<"target markers size: "<<markers.size();



        XYZ cur = XYZ(0,0,0);
        XYZ cur_front = XYZ(0,0,0);
        int mcount = 0;
        for(int i=0; i<10; i+=2){
            if(i>=markers.size()){
                break;
            }
            cur = cur + XYZ(markers[i]->x,markers[i]->y,markers[i]->z);
            mcount++;
        }
        cur = cur/XYZ(mcount,mcount,mcount);
        mcount = 0;
        for(int i=markers.size()-1; i>markers.size()-10; i-=2){
            if(i<0){
                break;
            }
            cur_front = cur_front + XYZ(markers[i]->x,markers[i]->y,markers[i]->z);
            mcount++;
        }
        cur_front = cur_front/XYZ(mcount,mcount,mcount);

        XYZ curDirection = cur - cur_front;
        double curCosAngle = dot(normalize(curDirection),normalize(lastDirection));
        if(curCosAngle<-0.9){
            qDebug()<<"reverse direction";
            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();
            break;
        }

        cur = XYZ(markers[0]->x,markers[0]->y,markers[0]->z);
        int cur_frontIndex = 6;
        if(cur_frontIndex>=markers.size())
            cur_frontIndex = markers.size()-1;
        cur_front = XYZ(markers[cur_frontIndex]->x,markers[cur_frontIndex]->y,markers[cur_frontIndex]->z);
        lastDirection = cur - cur_front;

        MyMarker* pMarker = 0;

        bkg_thresh = INT_MAX;
        int* grays = new int[markers.size()];
        vector<MyMarker*> tmpResult = vector<MyMarker*>();
        for(int i=markers.size()-1; i>=0; i--){
            MyMarker* marker = new MyMarker(markers[i]->x,markers[i]->y,markers[i]->z);
            marker->parent = pMarker;
            pMarker = marker;
            if(t != 1 && i == markers.size()-1)
                pMarker = outtree.back();
            long ind = marker->z*sz01 + marker->y*sz0 + marker->x;
            grays[i] = inimg1d[ind];
            if(inimg1d[ind]<bkg_thresh){
                bkg_thresh = inimg1d[ind];
            }
            if(i != markers.size()-1 || (i == markers.size()-1 && t == 1)){
                tmpResult.push_back(marker);
//                outtree.push_back(marker);
            }
        }
        int foreNum = 0;
        for(int i=0; i<tmpResult.size(); i++){
            MyMarker* marker = tmpResult[i];
            long ind2 = marker->z*sz01 + marker->y*sz0 + marker->x;
            if(inimg1d[ind2] > bkg_thresh){
                foreNum++;
            }
        }



        if((double)foreNum/tmpResult.size()<0.5){
            qDebug()<<"fornumRatio: "<<(double)foreNum/tmpResult.size()<<"-------------touch tip-----------------";
            stopFlag = 2;
        }
        qDebug()<<"before: "<<outtree.size();
        for(int i=0; i<tmpResult.size(); i++){
            outtree.push_back(tmpResult[i]);
        }
        qDebug()<<"after: "<<outtree.size();

//        double b_mean,b_std;
//        mean_and_std(grays,markers.size(),b_mean,b_std);
//        qDebug()<<"b_mean: "<<b_mean<<" b_std: "<<b_std;
//        double td = MIN(7,b_std);
//        bkg_thresh = b_mean - b_std*3;//b_std*0.5;
//        bkg_thresh = 0;

        sort(grays,grays+markers.size()-1);
        bkg_thresh = MAX(grays[markers.size()/4] - 5,1);

        if(grays){
            delete[] grays;
            grays = 0;
        }

        qDebug()<<"bkg_thres: "<<bkg_thresh;

        if(stopFlag == 1){
            bkg_thresh = 0;
        }else if(stopFlag == 2){
            while (!heap.empty()) {
                HeapElemX* tmp_elem = heap.delete_min();
                elems.erase(tmp_elem->img_ind);
                delete tmp_elem;
            }
        }

        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]){
                delete tmptree[i];
            }
        }
        tmptree.clear();

        qDebug()<<"----clear tmpTree end----";

        t++;
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
    if(path) {delete []path; path=0;}
    return true;
}



/*********************************************************************
 * Function : fastmarching_tree_line
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
template<class T> bool fastmarching_line2(MyMarker root,
                                         T * inimg1d,
                                         vector<MyMarker*> &outtree,
                                         long sz0,
                                         long sz1,
                                         long sz2,
                                         int cnn_type = 3,
                                         double bkg_thresh = 20,
                                         bool is_break_accept = false,
                                         double length = 50,
                                         XYZ lastDirection = XYZ(0,0,0),
                                         double zScale = 1)
{
    is_break_accept = true;
    bkg_thresh = 0;
    enum{ALIVE = -1, TRIAL = 0, FAR = 1, FINAL = 2, TMP = 3};

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;
    long i;
    //int cnn_type = 3;  // ?

    float * phi = 0;
    long * parent = 0;
    char * state = 0;
    float * path = 0;
    try
    {
        phi = new float[tol_sz];
        parent = new long[tol_sz];
        state = new char[tol_sz];
        path = new float[tol_sz];
        for(i = 0; i < tol_sz; i++)
        {
            phi[i] = INF;
            parent[i] = i;  // each pixel point to itself at the         statements beginning
            state[i] = FAR;
            path[i] = 0;
        }
    }
    catch (...)
    {
        cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
        if (phi) {delete []phi; phi=0;}
        if (parent) {delete []parent; parent=0;}
        if (state) {delete []state; state=0;}
        if (path) {delete []path; path=0;}
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

    int tt = 1;
    int stopFlag = 0;
    while (stopFlag < 2) {

        qDebug()<<"tt: "<<tt;

        stopFlag += 1;
        // loop
        int time_counter = 1;
        double process1 = 0;

        HeapElemX* tempElem = heap.delete_min();
        HeapElemX* tempElemCopy = new HeapElemX(tempElem->img_ind,tempElem->value);
        tempElemCopy->prev_ind = tempElem->prev_ind;
        heap.insert(tempElem);

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

            int pi = prev_ind % sz0;
            int pj = (prev_ind/sz0) % sz1;
            int pk = (prev_ind/sz01) % sz2;

            path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

            if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1) && tt != 1){
                stopFlag = 2;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }
//            qDebug()<<"path[min_ind]: "<<path[min_ind];
            if(path[min_ind]>length){
                stopFlag = 0;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }


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
//                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                        double factor = sqrt(ii*ii + jj*jj + kk*kk*zScale*zScale);
                        long index = d*sz01 + h*sz0 + w;

                        if(state[index] != ALIVE && state[index] != FINAL)
                        {
//                            double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;

                            double indexSG = getSmoothWeight(inimg1d,index,sz0,sz1,sz2);
                            double min_indSG = getSmoothWeight(inimg1d,min_ind,sz0,sz1,sz2);

                            XYZ curDire = XYZ(w-i,h-j,d-k);
                            curDire = normalize(curDire);
                            double indexDG = getDirectionWeight(inimg1d,index,sz0,sz1,sz2,curDire);
                            double min_indDG = getDirectionWeight(inimg1d,min_ind,sz0,sz1,sz2,curDire);

                            double direFactor = 1;//exp(1.0/(dot(curDire,normalize(lastDirection))+1.01));
                            double new_dist = phi[min_ind] + (GIG(indexSG) + GIG(min_indSG))*factor*0.5*direFactor;

//                            double new_dist = phi[min_ind] + (GIG(indexDG) + GIG(min_indDG))*factor*0.5;

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

        max_int = 0; // maximum intensity, used in GI
        min_int = INF;
        for(long ind=0; ind<tol_sz; ind++){
            if(state[ind] == TRIAL){
                state[ind] = FAR;
            }else if (state[ind] == ALIVE) {
                if (inimg1d[ind] > max_int) max_int = inimg1d[ind];
                if (inimg1d[ind] < min_int) min_int = inimg1d[ind];
                state[ind] = FAR;
            }
        }
        max_int -= min_int;

        //init heap
        {
            heap.insert(tempElemCopy);
            elems[tempElemCopy->img_ind] = tempElemCopy;
        }

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

            int pi = prev_ind % sz0;
            int pj = (prev_ind/sz0) % sz1;
            int pk = (prev_ind/sz01) % sz2;

            path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

            if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1) && tt != 1){
                stopFlag = 2;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }
//            qDebug()<<"path[min_ind]: "<<path[min_ind];
            if(path[min_ind]>length){
                stopFlag = 0;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }


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
//                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                        double factor = sqrt(ii*ii + jj*jj + kk*kk*zScale*zScale);
                        long index = d*sz01 + h*sz0 + w;

                        if(state[index] != ALIVE && state[index] != FINAL)
                        {
//                            double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;

                            double indexSG = getSmoothWeight(inimg1d,index,sz0,sz1,sz2);
                            double min_indSG = getSmoothWeight(inimg1d,min_ind,sz0,sz1,sz2);

                            XYZ curDire = XYZ(w-i,h-j,d-k);
                            curDire = normalize(curDire);
                            double indexDG = getDirectionWeight(inimg1d,index,sz0,sz1,sz2,curDire);
                            double min_indDG = getDirectionWeight(inimg1d,min_ind,sz0,sz1,sz2,curDire);

                            double direFactor = 1;//exp(1.0/(dot(curDire,normalize(lastDirection))+1.01));
                            double new_dist = phi[min_ind] + (GIG(indexSG) + GIG(min_indSG))*factor*0.5*direFactor;

//                            double new_dist = phi[min_ind] + (GIG(indexDG) + GIG(min_indDG))*factor*0.5;

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

        qDebug()<<"-------find next startPoint----------";

        vector<MyMarker*> tmptree = vector<MyMarker*>();
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
                tmptree.push_back(marker);
    //            outtree.push_back(marker);
            }
            i=-1; j = -1; k = -1;
            for(long ind = 0; ind < tol_sz; ind++)
            {
                i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
                if(state[ind] == TRIAL)
                    state[ind] = FAR;
                if(state[ind] != ALIVE) continue;
                state[ind] = TMP; //set final
                long ind2 = parent[ind];
                MyMarker * marker1 = tmp_map[ind];
                MyMarker * marker2 = tmp_map[ind2];
                if(marker1 == marker2) marker1->parent = 0;
                else marker1->parent = marker2;
                //tmp_map[ind]->parent = tmp_map[ind2];
            }
        }
        QString s = "D:\\testDynamicTracing\\" + QString::number(tt) + "_init.swc";
        saveSWC_file(s.toStdString(),tmptree);

        qDebug()<<"tmpTree size: "<<tmptree.size();

        MyMarker* rootMarker;
        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]->parent == 0)
                rootMarker = tmptree[i];
        }

        vector<MyMarker*> markers = vector<MyMarker*>();
        if(tt == 1){

            vector<HierarchySegment*> segs = vector<HierarchySegment*>();
            swc2topo_segs<unsigned char>(tmptree,segs,1);
            sort(segs.begin(),segs.end(),segCmp);

            MyMarker* marker = segs[0]->leaf_marker;
            markers.push_back(marker);
            while(marker != rootMarker){
                marker = marker->parent;
                markers.push_back(marker);
            }

        }else {
            vector<MyMarker*> tmpOutTree = vector<MyMarker*>();
//            happ(tmptree,tmpOutTree,inimg1d,sz0,sz1,sz2,bkg_thresh,0.2,6.0,false,false);
            happBasedFlag(tmptree,tmpOutTree,state,TMP,sz0,sz1,sz2);

            s = "D:\\testDynamicTracing\\" + QString::number(tt) + ".swc";
            saveSWC_file(s.toStdString(),tmpOutTree);

            int pointSize = tmpOutTree.size();
            if(pointSize == 0){
                for(int i=0; i<tmptree.size(); i++){
                    if(tmptree[i]){
                        delete tmptree[i];
                    }
                }
                tmptree.clear();
                break;
            }
            vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
            QStack<int> stack = QStack<int>();
            QHash<MyMarker*,int> hashNeuron = QHash<MyMarker*,int>();
            for(int i=0; i<pointSize; i++){
                hashNeuron.insert(tmpOutTree[i],i);
            }
            int* isRemain = new int[pointSize];
            V3DLONG ori;
            for(V3DLONG i=0 ; i<pointSize; i++){
                isRemain[i] = 0;
                if(tmpOutTree[i]->parent == 0){
                    ori = i;
                    stack.push(ori);
                    continue;
                }
                V3DLONG prtIndex = hashNeuron.value(tmpOutTree[i]->parent);
                children[prtIndex].push_back(i);
            }

            vector<V3DLONG> pointIndexs = vector<V3DLONG>();
            V3DLONG t,tmp;

            while (!stack.empty()) {
                t = stack.pop();
                pointIndexs.push_back(t);
                vector<V3DLONG> child = children[t];
                for(int i=0; i<child.size(); i++){
                    tmp = child[i];

                    while(children[tmp].size() == 1){
                        pointIndexs.push_back(tmp);
                        V3DLONG ch = children[tmp][0];
                        tmp = ch;
                    }

                    int chsz = children[tmp].size();
                    if(chsz>1){
                        stack.push(tmp);
                    }else {
                        pointIndexs.push_back(tmp);
                    }

                }
            }

            qDebug()<<"size: "<<pointIndexs.size();
            bool isSingleLine = true;
            for(V3DLONG i=0; i<pointSize; i++){
                if(children[i].size()>1){
                    isSingleLine = false;
                    break;
                }
            }
            if(isSingleLine){
                for(V3DLONG i=0; i<pointSize; i++){
                    isRemain[i] = 1;
                }
            }

            for(V3DLONG i=0; i<pointSize; i++){
                t = pointIndexs[i];
//                qDebug()<<t<<" is remain[t] "<<isRemain[t];
                if(children[t].size()>1 && isRemain[t] == 0){
                    isRemain[t] = 1;
                    vector<V3DLONG> child = children[t];
                    vector<vector<V3DLONG> > cbs;
                    qDebug()<<"i: "<<i<<" child size:"<<child.size();
                    for(int j=0; j<child.size(); j++){
                        vector<V3DLONG> cb = vector<V3DLONG>();
                        tmp = child[j];
                        cb.push_back(tmp);
                        while(children[tmp].size() == 1){
                            tmp = children[tmp][0];
                            cb.push_back(tmp);
                        }
                        qDebug()<<"j:"<<j<<" children size :"<<children[tmp].size();
                        if(children[tmp].size()>1){
                            if(children[tmp].size()>2){
                                QStack<int> tmpStack = QStack<int>();
                                tmpStack.push_back(child[j]);
                                while (!tmpStack.isEmpty()) {
                                    tmp = tmpStack.pop();
                                    isRemain[tmp] = 2;
                                    for(int kk=0; kk<children[tmp].size(); kk++){
                                        V3DLONG tmp1 = children[tmp][kk];
                                        while(children[tmp1].size() == 1){
                                            isRemain[tmp1] = 2;
                                            tmp1 = children[tmp1][0];
                                        }
                                        if(children[tmp1].size() == 0){
                                            isRemain[tmp1] = 2;
                                        }else {
                                            tmpStack.push_back(tmp1);
                                        }
                                    }
                                }
                            }else{
                                XYZ p1,p2;
                                V3DLONG tmp1 = children[tmp][0];
                                V3DLONG tmp2 = children[tmp][1];
                                while (children[tmp1].size() == 1) {
                                    tmp1 = children[tmp1][0];
                                }
                                while (children[tmp2].size() == 1) {
                                    tmp2 = children[tmp2][0];
                                }
                                p1 = XYZ(tmpOutTree[tmp1]->x - tmpOutTree[tmp]->x,
                                         tmpOutTree[tmp1]->y - tmpOutTree[tmp]->y,
                                         tmpOutTree[tmp1]->z - tmpOutTree[tmp]->z);
                                p2 = XYZ(tmpOutTree[tmp2]->x - tmpOutTree[tmp]->x,
                                         tmpOutTree[tmp2]->y - tmpOutTree[tmp]->y,
                                         tmpOutTree[tmp2]->z - tmpOutTree[tmp]->z);
                                double tmpCosAngle = dot(normalize(p1),normalize(p2));
                                qDebug()<<"next bifurcation angle: "<<tmpCosAngle;
                                if(tmpCosAngle<-0.9){
                                    QStack<int> tmpStack = QStack<int>();
                                    tmpStack.push_back(child[j]);
                                    while (!tmpStack.isEmpty()) {
                                        tmp = tmpStack.pop();
                                        isRemain[tmp] = 2;
                                        for(int kk=0; kk<children[tmp].size(); kk++){
                                            V3DLONG tmp1 = children[tmp][kk];
                                            while(children[tmp1].size() == 1){
                                                isRemain[tmp1] = 2;
                                                tmp1 = children[tmp1][0];
                                            }
                                            if(children[tmp1].size() == 0){
                                                isRemain[tmp1] = 2;
                                            }else {
                                                tmpStack.push_back(tmp1);
                                            }
                                        }
                                    }
                                }else{
                                    cbs.push_back(cb);
                                }

                            }
                        }else{
                            cbs.push_back(cb);
                        }
                    }

                    qDebug()<<"cbs size:"<<cbs.size();

                    if(cbs.size() == 1){
                        for(int kk=0; kk<cbs[0].size()-1; kk++){
                            isRemain[cbs[0][kk]] = 1;
                        }
                        if(children[cbs[0].back()].size() == 0){
                            isRemain[cbs[0].back()] = 1;
                        }
                    }else{
                        double maxCosAngle = -1;
                        int maxIndex = -1;
                        for(int j=0; j<cbs.size(); j++){
                            qDebug()<<"cbs j"<<j<<" size: "<<cbs[j].size();
                            XYZ p1 = XYZ(tmpOutTree[cbs[j].back()]->x - tmpOutTree[cbs[j].front()]->x,
                                    tmpOutTree[cbs[j].back()]->y - tmpOutTree[cbs[j].front()]->y,
                                    tmpOutTree[cbs[j].back()]->z - tmpOutTree[cbs[j].front()]->z);
                            double tmpCosAngle = dot(normalize(p1),normalize(lastDirection));
                            qDebug()<<"tmpCosAngle: "<<tmpCosAngle;
                            if(maxCosAngle<tmpCosAngle){
                                maxCosAngle = tmpCosAngle;
                                maxIndex = j;
                            }
                        }
                        qDebug()<<"maxCosAngle: "<<maxCosAngle<<" maxIndex: "<<maxIndex;
                        for(int j=0; j<cbs.size(); j++){
                            if(j == maxIndex){
                                for(int kk=0; kk<cbs[j].size()-1; kk++){
                                    isRemain[cbs[j][kk]] = 1;
                                }
                                if(children[cbs[j].back()].size() == 0){
//                                    qDebug()<<"children 0: "<<cbs[j].back();
                                    isRemain[cbs[j].back()] = 1;
                                }
                            }else{
                                QStack<int> tmpStack = QStack<int>();
                                tmpStack.push_back(cbs[j][0]);
                                while (!tmpStack.isEmpty()) {
                                    tmp = tmpStack.pop();
                                    qDebug()<<"2 "<<tmp;
                                    isRemain[tmp] = 2;
                                    for(int kk=0; kk<children[tmp].size(); kk++){
                                        V3DLONG tmp1 = children[tmp][kk];
                                        while(children[tmp1].size() == 1){
//                                            qDebug()<<"2 "<<tmp1;
                                            isRemain[tmp1] = 2;
                                            tmp1 = children[tmp1][0];
                                        }
                                        if(children[tmp1].size() == 0){
//                                            qDebug()<<"2 "<<tmp1;
                                            isRemain[tmp1] = 2;
                                        }else {
                                            tmpStack.push_back(tmp1);
                                        }
                                    }
                                }
                            }
                        }

                    }
                }
            }

            qDebug()<<"start to save marker!";
            MyMarker* marker;
            for(int i=0; i<pointSize; i++){
                if(isRemain[i] == 1 && children[i].size() == 0){
                    marker = tmpOutTree[i];
                }
            }
            markers.push_back(marker);
            while(marker != rootMarker){
                marker = marker->parent;
                markers.push_back(marker);
            }
        }
        for(long ind = 0; ind < tol_sz; ind++)
        {
            if(state[ind] == TMP){
                state[ind] = FINAL;
            }
        }


        // init heap
        {
            MyMarker* leafMarker = markers[0];
            long index = leafMarker->z*sz01 + leafMarker->y*sz0 +leafMarker->x;
            phi[index] = 0;
//            parent[index] = index;
            path[index] = 0;
            HeapElemX *elem = new HeapElemX(index, phi[index]);
            elem->prev_ind = index;
            heap.insert(elem);
            elems[index] = elem;
        }


//        qDebug()<<"tmpTree size: "<<tmptree.size();

        /*
        if(tmptree.size()<2){
            break;
        }

        MyMarker* rootMarker;
        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]->parent == 0)
                rootMarker = tmptree[i];
        }
        vector<HierarchySegment*> segs = vector<HierarchySegment*>();
        swc2topo_segs<unsigned char>(tmptree,segs,1);

        ofstream csvFile;
        QString csvPath = "D:\\testDynamicTracing\\" + QString::number(t) + "_segments.csv";
        csvFile.open(csvPath.toStdString().c_str(), ios::out);
        csvFile<<"level"<<','<<"length"<<endl;

        sort(segs.begin(),segs.end(),segCmp);
//        vector<HierarchySegment*> tmpSegs = vector<HierarchySegment*>();

        HierarchySegment* targetSegment = 0;

        MyMarker* marker = segs[0]->leaf_marker;
        vector<MyMarker*> tmpMarkers = vector<MyMarker*>();

        tmpMarkers.push_back(marker);
        while(marker != rootMarker){
            marker = marker->parent;
            tmpMarkers.push_back(marker);
        }
        qDebug()<<"before tmpMarker size:"<<tmpMarkers.size();
        if(t != 1){
            marker = outtree.back();marker = outtree.back();
            int foreNum = 0;
            while(marker->parent != 0){
                tmpMarkers.push_back(marker);
                marker = marker->parent;
                foreNum++;
                if(foreNum>=6)
                    break;
            }
        }
        reverse(tmpMarkers.begin(),tmpMarkers.end());
        XYZ tmpDire = getDirection2(tmpMarkers,0.5);

        if(tmpDire == XYZ(-2,-2,-2) || tmpDire == XYZ(-4,-4,-4)){

            int inflectionIndex = getInflectionIndex(tmpMarkers,0.5);
            MyMarker otherMarker = MyMarker(tmpMarkers[inflectionIndex]->x,
                                            tmpMarkers[inflectionIndex]->y,
                                            tmpMarkers[inflectionIndex]->z);
            MyMarker midMarker = MyMarker(0,0,0);
            MyMarker tmpMidMarker = MyMarker(0,0,0);
            MyMarker tmpOtherMarker = MyMarker(0,0,0);
            for(int i=0; i<tmptree.size(); i++){
                midMarker.x += tmptree[i]->x;
                midMarker.y += tmptree[i]->y;
                midMarker.z += tmptree[i]->z;
            }
            midMarker.x /= tmptree.size();
            midMarker.y /= tmptree.size();
            midMarker.z /= tmptree.size();
            int times = 0;
            vector<MyMarker*> midMarkers = vector<MyMarker*>();
            vector<MyMarker*> otherMarkers = vector<MyMarker*>();

            for(int i=0; i<tmptree.size(); i++){
                double d_mid = dist(*(tmptree[i]),midMarker);
                double d_other = dist(*(tmptree[i]),otherMarker);
                if(d_mid<d_other){
                    midMarkers.push_back(tmptree[i]);
                }else {
                    otherMarkers.push_back(tmptree[i]);
                }
            }

//            while(times<100){
//                for(int i=0; i<tmptree.size(); i++){
//                    double d_mid = dist(*(tmptree[i]),midMarker);
//                    double d_other = dist(*(tmptree[i]),otherMarker);
//                    if(d_mid<d_other){
//                        midMarkers.push_back(tmptree[i]);
//                    }else {
//                        otherMarkers.push_back(tmptree[i]);
//                    }
//                }
//                double shiftD = 0;
//                tmpMidMarker = MyMarker(0,0,0);
//                for(int i=0; i<midMarkers.size(); i++){
//                    tmpMidMarker.x += midMarkers[i]->x;
//                    tmpMidMarker.y += midMarkers[i]->y;
//                    tmpMidMarker.z += midMarkers[i]->z;
//                }
//                if(midMarkers.size()>0){
//                    tmpMidMarker.x /= midMarkers.size();
//                    tmpMidMarker.y /= midMarkers.size();
//                    tmpMidMarker.z /= midMarkers.size();
//                }

//                shiftD += dist(midMarker,tmpMidMarker);

//                tmpOtherMarker = MyMarker(0,0,0);
//                for(int i=0; i<otherMarkers.size(); i++){
//                    tmpOtherMarker.x += otherMarkers[i]->x;
//                    tmpOtherMarker.y += otherMarkers[i]->y;
//                    tmpOtherMarker.z += otherMarkers[i]->z;
//                }
//                if(otherMarkers.size()>0){
//                    tmpOtherMarker.x /= otherMarkers.size();
//                    tmpOtherMarker.y /= otherMarkers.size();
//                    tmpOtherMarker.z /= otherMarkers.size();
//                }

//                shiftD += dist(otherMarker,tmpOtherMarker);

//                if(shiftD>1){
//                    midMarker = tmpMidMarker;
//                    otherMarker = tmpOtherMarker;
//                    midMarkers.clear();
//                    otherMarkers.clear();
//                }else{
//                    break;
//                }
//                times++;
//            }
            qDebug()<<"mid size:"<<midMarkers.size();

            for(int i=0; i<otherMarkers.size(); i++){
                long index = otherMarkers[i]->ind(sz0,sz01);
                state[index] = FAR;
            }
            segs.clear();
            swc2topo_segs<unsigned char>(midMarkers,segs,1);

            QString s = "D:\\testDynamicTracing\\" + QString::number(t) + "_initClass.swc";
            saveSWC_file(s.toStdString(),midMarkers);

            sort(segs.begin(),segs.end(),segCmp);

            targetSegment = segs[0];

        }else if (tmpDire == XYZ(-3,-3,-3)) {
            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();
            break;
        }
        else{
            targetSegment = segs[0];
        }

        qDebug()<<"targetLength: "<<targetSegment->length;

        // init heap
        {
            MyMarker* leafMarker = targetSegment->leaf_marker;
            long index = leafMarker->z*sz01 + leafMarker->y*sz0 +leafMarker->x;
            phi[index] = 0;
//            parent[index] = index;
            path[index] = 0;
            HeapElemX *elem = new HeapElemX(index, phi[index]);
            elem->prev_ind = index;
            heap.insert(elem);
            elems[index] = elem;
        }

        qDebug()<<"----init heap end-----";

        vector<MyMarker*> markers = vector<MyMarker*>();
//        segs[segMaxIndex]->get_markers(markers);
        MyMarker* markerTmp = targetSegment->leaf_marker;
        markers.push_back(markerTmp);
        while (markerTmp != rootMarker) {
            markerTmp = markerTmp->parent;
            markers.push_back(markerTmp);
        }
        qDebug()<<"target markers size: "<<markers.size();

        */



        XYZ cur = XYZ(0,0,0);
        XYZ cur_front = XYZ(0,0,0);
        int mcount = 0;
        for(int i=0; i<10; i+=2){
            if(i>=markers.size()){
                break;
            }
            cur = cur + XYZ(markers[i]->x,markers[i]->y,markers[i]->z);
            mcount++;
        }
        cur = cur/XYZ(mcount,mcount,mcount);
        mcount = 0;
        for(int i=markers.size()-1; i>markers.size()-10; i-=2){
            if(i<0){
                break;
            }
            cur_front = cur_front + XYZ(markers[i]->x,markers[i]->y,markers[i]->z);
            mcount++;
        }
        cur_front = cur_front/XYZ(mcount,mcount,mcount);

        XYZ curDirection = cur - cur_front;
        double curCosAngle = dot(normalize(curDirection),normalize(lastDirection));
        if(curCosAngle<-0.9){
            qDebug()<<"reverse direction";
            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();
            break;
        }

        cur = XYZ(markers[0]->x,markers[0]->y,markers[0]->z);
        int cur_frontIndex = 6;
        if(cur_frontIndex>=markers.size())
            cur_frontIndex = markers.size()-1;
        cur_front = XYZ(markers[cur_frontIndex]->x,markers[cur_frontIndex]->y,markers[cur_frontIndex]->z);
        lastDirection = cur - cur_front;

        MyMarker* pMarker = 0;

        bkg_thresh = INT_MAX;
        int* grays = new int[markers.size()];
        vector<MyMarker*> tmpResult = vector<MyMarker*>();
        for(int i=markers.size()-1; i>=0; i--){
            MyMarker* marker = new MyMarker(markers[i]->x,markers[i]->y,markers[i]->z);
            marker->parent = pMarker;
            pMarker = marker;
            if(tt != 1 && i == markers.size()-1)
                pMarker = outtree.back();
            long ind = marker->z*sz01 + marker->y*sz0 + marker->x;
            grays[i] = inimg1d[ind];
            if(inimg1d[ind]<bkg_thresh){
                bkg_thresh = inimg1d[ind];
            }
            if(i != markers.size()-1 || (i == markers.size()-1 && tt == 1)){
                tmpResult.push_back(marker);
//                outtree.push_back(marker);
            }
        }
        int foreNum = 0;
        for(int i=0; i<tmpResult.size(); i++){
            MyMarker* marker = tmpResult[i];
            long ind2 = marker->z*sz01 + marker->y*sz0 + marker->x;
            if(inimg1d[ind2] > bkg_thresh){
                foreNum++;
            }
        }



        if((double)foreNum/tmpResult.size()<0.5){
            qDebug()<<"fornumRatio: "<<(double)foreNum/tmpResult.size()<<"-------------touch tip-----------------";
            stopFlag = 2;
        }
        qDebug()<<"before: "<<outtree.size();
        for(int i=0; i<tmpResult.size(); i++){
            outtree.push_back(tmpResult[i]);
        }
        qDebug()<<"after: "<<outtree.size();

//        double b_mean,b_std;
//        mean_and_std(grays,markers.size(),b_mean,b_std);
//        qDebug()<<"b_mean: "<<b_mean<<" b_std: "<<b_std;
//        double td = MIN(7,b_std);
//        bkg_thresh = b_mean - b_std*3;//b_std*0.5;
//        bkg_thresh = 0;

        sort(grays,grays+markers.size()-1);
        bkg_thresh = MAX(grays[markers.size()/4] - 5,1);

        if(grays){
            delete[] grays;
            grays = 0;
        }

        qDebug()<<"bkg_thres: "<<bkg_thresh;

        if(stopFlag == 1){
            bkg_thresh = 0;
        }else if(stopFlag == 2){
            while (!heap.empty()) {
                HeapElemX* tmp_elem = heap.delete_min();
                elems.erase(tmp_elem->img_ind);
                delete tmp_elem;
            }
        }

        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]){
                delete tmptree[i];
            }
        }
        tmptree.clear();

        qDebug()<<"----clear tmpTree end----";

        tt++;
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
    if(path) {delete []path; path=0;}
    return true;
}

struct sBlock
{
    int x0,x1,y0,y1,z0,z1;
    int direction;
    sBlock() {
        x0 = x1 = y0 = y1 = z0 = z1;
        direction = 0;
    }
};

template<class T>
bool fastmarching_ultratracer(MyMarker root,
                              T * inimg1d,
                              vector<MyMarker*> &outtree,
                              long sz0,
                              long sz1,
                              long sz2,
                              int direction = 0,
                              int cnn_type = 3,
                              double length = 30)
{
    long tolSZ = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;
    sBlock s = sBlock();

    int rootX = root.x + 0.5;
    int rootY = root.y + 0.5;
    int rootZ = root.z + 0.5;

    switch (direction) {
    case 1:
        s.x0 = rootX;
        s.x1 = rootX + length*2;
        s.y0 = rootY - length;
        s.y1 = rootY + length;
        s.z0 = rootZ - length;
        s.z1 = rootZ + length;
        break;
    case 2:
        s.x0 = rootX - length*2;
        s.x1 = rootX;
        s.y0 = rootY - length;
        s.y1 = rootY + length;
        s.z0 = rootZ - length;
        s.z1 = rootZ + length;
        break;
    case 3:
        s.x0 = rootX - length;
        s.x1 = rootX + length;
        s.y0 = rootY;
        s.y1 = rootY + length*2;
        s.z0 = rootZ - length;
        s.z1 = rootZ + length;
        break;
    case 4:
        s.x0 = rootX - length;
        s.x1 = rootX + length;
        s.y0 = rootY - length*2;
        s.y1 = rootY;
        s.z0 = rootZ - length;
        s.z1 = rootZ + length;
        break;
    case 5:
        s.x0 = rootX - length;
        s.x1 = rootX + length;
        s.y0 = rootY - length;
        s.y1 = rootY + length;
        s.z0 = rootZ;
        s.z1 = rootZ + length*2;
        break;
    case 6:
        s.x0 = rootX - length;
        s.x1 = rootX + length;
        s.y0 = rootY - length;
        s.y1 = rootY + length;
        s.z0 = rootZ - length*2;
        s.z1 = rootZ;
        break;
    default:
        return false;
    }

    if(s.x0<0 || s.x0 >= sz0) return true;
    if(s.x1<0 || s.x1 >= sz0) return true;
    if(s.y0<0 || s.y0 >= sz1) return true;
    if(s.y1<0 || s.y1 >= sz1) return true;
    if(s.z0<0 || s.z0 >= sz2) return true;
    if(s.z1<0 || s.z1 >= sz2) return true;

    long sTolSZ = (length*2+1)*(length*2+1)*(length*2+1);
    long sSZXY = (length*2+1)*(length*2+1);
    long sSZXZ = (length*2+1)*(length*2+1);
    long sSZYZ = (length*2+1)*(length*2+1);
    T* sData1d = new T[sTolSZ];

    for(int i=0; i<=length*2; i++){
        for(int j=0; j<=length*2; j++){
            for(int k=0; k<=length*2; k++){
                long sIndex = k*(length*2+1)*(length*2+1) + j*(length*2+1) + i;
                long imgIndex = (k+s.z0)*sz01 + (j+s.y0)*sz0 + i+s.x0;
                sData1d[sIndex] = inimg1d[imgIndex];
            }
        }
    }

    double sMean, sStd;
    mean_and_std(sData1d,sTolSZ,sMean,sStd);

    T* planeXY0 = new T[sSZXY];
    T* planeXY1 = new T[sSZXY];
    T* planeXZ0 = new T[sSZXZ];
    T* planeXZ1 = new T[sSZXZ];
    T* planeYZ0 = new T[sSZYZ];
    T* planeYZ1 = new T[sSZYZ];

    for(int i=0; i<=length*2; i++){
        for(int j=0; j<=length*2; j++){
            int k = 0;
            long planeIndex = j*(length*2+1) + i;
            long sIndex = k*(length*2+1)*(length*2+1) + j*(length*2+1) + i;
            planeXY0[planeIndex] = sData1d[sIndex];
        }
    }

    for(int i=0; i<=length*2; i++){
        for(int j=0; j<=length*2; j++){
            int k = length*2;
            long planeIndex = j*(length*2+1) + i;
            long sIndex = k*(length*2+1)*(length*2+1) + j*(length*2+1) + i;
            planeXY1[planeIndex] = sData1d[sIndex];
        }
    }

    for(int i=0; i<=length*2; i++){
        for(int k=0; k<=length*2; k++){
            int j = 0;
            long planeIndex = k*(length*2+1) + i;
            long sIndex = k*(length*2+1)*(length*2+1) + j*(length*2+1) + i;
            planeXZ0[planeIndex] = sData1d[sIndex];
        }
    }

    for(int i=0; i<=length*2; i++){
        for(int k=0; k<=length*2; k++){
            int j = length*2;
            long planeIndex = k*(length*2+1) + i;
            long sIndex = k*(length*2+1)*(length*2+1) + j*(length*2+1) + i;
            planeXZ1[planeIndex] = sData1d[sIndex];
        }
    }

    for(int j=0; j<=length*2; j++){
        for(int k=0; k<=length*2; k++){
            int i = 0;
            long planeIndex = k*(length*2+1) + j;
            long sIndex = k*(length*2+1)*(length*2+1) + j*(length*2+1) + i;
            planeYZ0[planeIndex] = sData1d[sIndex];
        }
    }

    for(int j=0; j<=length*2; j++){
        for(int k=0; k<=length*2; k++){
            int i = length*2;
            long planeIndex = k*(length*2+1) + j;
            long sIndex = k*(length*2+1)*(length*2+1) + j*(length*2+1) + i;
            planeYZ1[planeIndex] = sData1d[sIndex];
        }
    }

    int threshold = sMean + 0.5*sStd;

    vector<XYZ> pointsAll;

    vector<XYZ> pointsXY0,pointsXY1,pointsXZ0,pointsXZ1,pointsYZ0,pointsYZ1;
    getSignals3d(planeXY0,length*2+1,length*2+1,length*2+1,pointsAll,1,threshold,30);
    getSignals3d(planeXY1,length*2+1,length*2+1,length*2+1,pointsAll,2,threshold,30);
    getSignals3d(planeXZ0,length*2+1,length*2+1,length*2+1,pointsAll,3,threshold,30);
    getSignals3d(planeXZ1,length*2+1,length*2+1,length*2+1,pointsAll,4,threshold,30);
    getSignals3d(planeYZ0,length*2+1,length*2+1,length*2+1,pointsAll,5,threshold,30);
    getSignals3d(planeYZ1,length*2+1,length*2+1,length*2+1,pointsAll,6,threshold,30);

    MyMarker* sRoot = MyMarker(rootX-s.x0,rootY-s.y0,rootZ-s.z0);

    vector<MyMarker*> targets;
    int sRootIndex = -1;

    for(int i=0; i<pointsAll.size(); i++){
        XYZ p = pointsAll[i];
        if((p.x == sRoot->x && (p.x == 0 || p.x == length*2)) ||
                (p.y == sRoot->y && (p.y == 0 || p.y == length*2)) ||
                (p.z == sRoot->z && (p.z == 0 || p.z == length*2))){

        }
    }




//    unsigned char* planeXY0Flag, planeXY1Flag, planeXZ0Flag, planeXZ1Flag, planeYZ0Flag, planeYZ1Flag;
//    int planeXY0Count = regionGrow(planeXY0,planeXY0Flag,length*2+1,length*2+1,threshold,30);
//    int planeXY1Count = regionGrow(planeXY1,planeXY1Flag,length*2+1,length*2+1,threshold,30);
//    int planeXZ0Count = regionGrow(planeXZ0,planeXZ0Flag,length*2+1,length*2+1,threshold,30);
//    int planeXZ1Count = regionGrow(planeXZ1,planeXZ1Flag,length*2+1,length*2+1,threshold,30);
//    int planeYZ0Count = regionGrow(planeYZ0,planeYZ0Flag,length*2+1,length*2+1,threshold,30);
//    int planeYZ1Count = regionGrow(planeYZ1,planeYZ1Flag,length*2+1,length*2+1,threshold,30);

//    int planeAllCount = planeXY0Count + planeXY1Count + planeXZ0Count + planeXZ1Count + planeYZ0Count + planeYZ1Count;








    if(sData1d){
        delete[] sData1d;
        sData1d = 0;
    }

    return true;

}

bool getMarkersBetweenZX(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2, double r){
    double A = m2.x - m1.x;
    double B = m2.y - m1.y;
    double C = m2.z - m1.z;
    double R = (r == 0) ? (m2.radius - m1.radius) : 0;
    double D = sqrt(A*A + B*B + C*C);
    A = A/D; B = B/D; C = C/D; R = R/D;

    double ctz = A/sqrt(A*A + B*B);
    double stz = B/sqrt(A*A + B*B);

    double cty = C/sqrt(A*A + B*B + C*C);
    double sty = sqrt(A*A + B*B)/sqrt(A*A + B*B + C*C);

    double x0 = m1.x;
    double y0 = m1.y;
    double z0 = m1.z;
    double r0 = (r == 0) ? m1.radius : r;

    set<MyMarker> marker_set;

    for(double t = 0.0; t <= dist(m1, m2); t += 1.0)
    {
        MyMarker marker;
        int cx = x0 + A*t + 0.5;
        int cy = y0 + B*t + 0.5;
        int cz = z0 + C*t + 0.5;
        int radius = r0 + R*t + 0.5;
        int radius2 = radius * radius;

        for(int k = -radius/4; k <= radius/4; k++)
        {
            for(int j = -radius; j <= radius; j++)
            {
                for(int i = -radius; i <= radius; i++)
                {
                    if(i * i + j * j + k * k > radius2) continue;
                    double x = i, y = j, z = k;
                    double x1, y1, z1;


                    //rotate_coordinate_z_clockwise(ctz, stz, x, y, z);
                    //rotate_along_y_clockwise     (cty, sty, x, y, z);
                    //rotate_coordinate_x_anticlock(ctz, stz, x, y, z);
                    //translate_to(cx, cy, cz, x, y, z);
                    y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
                    x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
                    z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
                    x += cx; y += cy; z += cz;
                    x = (int)(x+0.5);
                    y = (int)(y+0.5);
                    z = (int)(z+0.5);
                    marker_set.insert(MyMarker(x, y, z));
                }
            }
        }
    }

    allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());
    return true;
}

template<class T>
bool segToMask(T* state, MyMarker* leaf, MyMarker* root, long sz0, long sz1, long sz2, double r, int flag){
    long sz01 = sz0 * sz1;
    MyMarker* p = leaf;
    while (p != root) {
        MyMarker* par = p->parent;
        vector<MyMarker> tmpMarkers;
        tmpMarkers.clear();
        getMarkersBetweenZX(tmpMarkers,*p,*par,r);
//        cout<<"px:"<<p->x<<endl;
//        cout<<"tmpMarker size:"<<tmpMarkers.size()<<endl;
        for(int j = 0; j < tmpMarkers.size(); j++)
        {
            int x = tmpMarkers[j].x;
            int y = tmpMarkers[j].y;
            int z = tmpMarkers[j].z;
            if(x < 0 || x >= sz0 || y < 0 || y >= sz1 || z < 0 || z >= sz2) continue;
            state[z*sz01 + y * sz0 + x] = (T)flag;
        }
        p = par;
    }
    return true;
}

template<class T>
bool twoPointsToMask(T* state, MyMarker* p1, MyMarker* p2, long sz0, long sz1, long sz2, double r, int flag){
    long sz01 = sz0 * sz1;
    vector<MyMarker> tmpMarkers;
    getMarkersBetweenZX(tmpMarkers, *p1, *p2, r);

    for(int j = 0; j < tmpMarkers.size(); j++)
    {
        int x = tmpMarkers[j].x;
        int y = tmpMarkers[j].y;
        int z = tmpMarkers[j].z;
        if(x < 0 || x >= sz0 || y < 0 || y >= sz1 || z < 0 || z >= sz2) continue;
        state[z*sz01 + y * sz0 + x] = (T)flag;
    }

    return true;
}

template <class T>
bool computeCubePcaEigVec(T* data1d, V3DLONG* sz,
                          V3DLONG x0, V3DLONG y0, V3DLONG z0,
                          V3DLONG wx, V3DLONG wy, V3DLONG wz,
                          double &pc1, double &pc2, double &pc3,
                          double *vec1, double *vec2, double *vec3){
    V3DLONG xb = x0 - wx; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
    V3DLONG xe = x0 + wx; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
    V3DLONG yb = y0 - wy; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
    V3DLONG ye = y0 + wy; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;
    V3DLONG zb = z0 - wz; if(zb<0) zb = 0; if(zb >= sz[2]) zb = sz[2] - 1;
    V3DLONG ze = z0 + wz; if(ze<0) ze = 0; if(ze >= sz[2]) ze = sz[2] - 1;

    V3DLONG i,j,k;
    V3DLONG index;

    double w;

    //first get the center of mass
    double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
    for (k = zb; k <= ze; k++)
    {
        for (j = yb; j <= ye; j++)
        {
            for (i = xb; i <= xe; i++)
            {
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = (double) data1d[index];
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;
            }
        }
    }

    if (s>0)
    {
        xm /= s; ym /= s; zm /= s;
        mv = s / ((double)((ze - zb + 1)*(ye - yb + 1)*(xe - xb + 1)));
    }
    else
    {
        printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
        return false;
    }

    double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
    double dfx, dfy, dfz;
    for (k = zb; k <= ze; k++)
    {
        dfz = double(k) - zm;
        for (j = yb; j <= ye; j++)
        {
            dfy = double(j) - ym;
            for (i = xb; i <= xe; i++)
            {
                dfx = double(i) - xm;

                //                w = img3d[k][j][i]; //140128
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = data1d[index] - mv;  if (w<0) w = 0; //140128 try the new formula

                cc11 += w*dfx*dfx;
                cc12 += w*dfx*dfy;
                cc13 += w*dfx*dfz;
                cc22 += w*dfy*dfy;
                cc23 += w*dfy*dfz;
                cc33 += w*dfz*dfz;
            }
        }
    }

    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
//            if(count%1000 == 0){
//                qDebug()<<" cc11:"<<cc11<<" cc12:"<<cc12<<" cc13:"<<cc13<<" cc22:"<<cc22<<" cc23:"<<cc23<<" cc33:"<<cc33;
//            }



    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix, DD, VV);

        //output the result
        pc1 = DD(3);
        pc2 = DD(2);
        pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

        //output the vector
        for (int i = 1; i <= 3; i++){
            vec1[i-1] = VV(i, 3);
            vec2[i-1] = VV(i, 2);
            vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
        }
    }catch (...)
    {
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
        return false;
    }

    return true;
}

template<class T>
bool computeLocalPcaEigVec(T* data1d, vector<MyMarker*> tmptree,
                           long sz0, long sz1, long sz2,
                           double &pc1, double &pc2, double &pc3,
                           double *vec1, double *vec2, double *vec3){
    long sz01 = sz0*sz1;
    V3DLONG index;

    double w;

    //first get the center of mass
    double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
    int size = 0;
    for(int i=0; i<tmptree.size(); i++){
        int x = tmptree[i]->x;
        int y = tmptree[i]->y;
        int z = tmptree[i]->z;
        if(x<0 || x>=sz0 || y<0 || y>=sz1 || z<0 || z>=sz2)
            continue;
        size++;
        index = z*sz01 + y*sz0 + x;
        w = (double) data1d[index];
        xm += w*x;
        ym += w*y;
        zm += w*z;
        s += w;
    }

    if(s>0){
        xm /= s; ym /= s; zm /= s;
        mv = s/(double)size;
    }else{
        printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
        return false;
    }

    double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
    double dfx, dfy, dfz;
    for(int i=0; i<tmptree.size(); i++){
        int x = tmptree[i]->x;
        int y = tmptree[i]->y;
        int z = tmptree[i]->z;
        if(x<0 || x>=sz0 || y<0 || y>=sz1 || z<0 || z>=sz2)
            continue;

        dfz = (double)z - zm;
        dfy = (double)y - ym;
        dfx = (double)x - xm;

        index = z*sz01 + y*sz0 + x;
        w = data1d[index] - mv; if (w<0) w = 0;

        cc11 += w*dfx*dfx;
        cc12 += w*dfx*dfy;
        cc13 += w*dfx*dfz;
        cc22 += w*dfy*dfy;
        cc23 += w*dfy*dfz;
        cc33 += w*dfz*dfz;
    }

    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;

    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix, DD, VV);

        //output the result
        pc1 = DD(3);
        pc2 = DD(2);
        pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

        //output the vector
        for (int i = 1; i <= 3; i++){
            vec1[i-1] = VV(i, 3);
            vec2[i-1] = VV(i, 2);
            vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
        }
    }catch (...)
    {
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
        return false;
    }

    return true;
}

//template<class T>
bool computeLocalPcaEigVec2(vector<MyMarker*> tmptree,
                           long sz0, long sz1, long sz2,
                           double &pc1, double &pc2, double &pc3,
                           double *vec1, double *vec2, double *vec3){
    long sz01 = sz0*sz1;
    V3DLONG index;

    double w = 1.0;

    //first get the center of xyz
    double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
    int size = 0;
    for(int i=0; i<tmptree.size(); i++){
        int x = tmptree[i]->x;
        int y = tmptree[i]->y;
        int z = tmptree[i]->z;
//        if(x<0 || x>=sz0 || y<0 || y>=sz1 || z<0 || z>=sz2)
//            continue;
//        size++;
        index = z*sz01 + y*sz0 + x;
//        w = (double) data1d[index];
        xm += w*x;
        ym += w*y;
        zm += w*z;
        s += w;
    }

    if(s>0){
        xm /= s; ym /= s; zm /= s;
        mv = s/(double)tmptree.size();
    }else{
        printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
        return false;
    }

    double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
    double dfx, dfy, dfz;
    for(int i=0; i<tmptree.size(); i++){
        int x = tmptree[i]->x;
        int y = tmptree[i]->y;
        int z = tmptree[i]->z;
        if(x<0 || x>=sz0 || y<0 || y>=sz1 || z<0 || z>=sz2)
            continue;

        dfz = (double)z - zm;
        dfy = (double)y - ym;
        dfx = (double)x - xm;

        index = z*sz01 + y*sz0 + x;
        w = 1;

        cc11 += w*dfx*dfx;
        cc12 += w*dfx*dfy;
        cc13 += w*dfx*dfz;
        cc22 += w*dfy*dfy;
        cc23 += w*dfy*dfz;
        cc33 += w*dfz*dfz;
    }

    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;

    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix, DD, VV);

        //output the result
        pc1 = DD(3);
        pc2 = DD(2);
        pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

        //output the vector
        for (int i = 1; i <= 3; i++){
            vec1[i-1] = VV(i, 3);
            vec2[i-1] = VV(i, 2);
            vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
        }
    }catch (...)
    {
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
        return false;
    }

    return true;
}

vector<MyMarker*> mergeSegs(vector<vector<MyMarker* > >& markersVec){
    int npath = markersVec.size();

    vector<MyMarker*> same_segment;
    if(npath<1) {
        return same_segment;
    }else if(npath<2){
        return markersVec[0];
    }

//    vector<vector<MyMarker*> > all_segment;

    int maxSize = 0;

    int* isSame = new int[npath];
    for(int i=0; i<npath; i++){
        maxSize = MAX(maxSize,markersVec[i].size());
        isSame[i] = 1;
    }

    int nSameNode;

    MyMarker* par = 0;
    for(int i=0; i<maxSize; i++){
        nSameNode = 0;
        for(int j=0; j<npath; j++){
            if(isSame[j] == 1){
                nSameNode++;
            }
        }

        if(nSameNode == 0){
            break;
        }
//        cout<<i<<" nSameNode: "<<nSameNode<<endl;

        map<int,int> sameMap = map<int,int>();
        sameMap.clear();

        MyMarker* p = 0;
        for(int j=0; j<npath; j++){
            if(isSame[j] == 1){
                MyMarker* pp = markersVec[j][i];
                int index = pp->z*100 + pp->y*10 + pp->x;
                if(sameMap.find(index) == sameMap.end()){
                    sameMap[index] = j;
                }else{
                    int findIndex = sameMap[index];
                    isSame[findIndex] = 2;
                    isSame[j] = 2;
                    if(!p){
                        p = new MyMarker(pp->x,pp->y,pp->z);
                    }
                }
            }
        }

        for(int j=0; j<npath; j++){
            if(isSame[j] == 1){
                MyMarker* curP = par;
                for(int k=i; k<markersVec[j].size(); k++){
                    MyMarker* pp = markersVec[j][k];
                    MyMarker* cur = new MyMarker(pp->x,pp->y,pp->z);
                    cur->parent = curP;
                    same_segment.push_back(cur);
                    curP = cur;
                }
                isSame[j] = 3;
            }else if(isSame[j] == 2){
                isSame[j] = 1;
            }
        }

        if(p){
            p->parent = par;
            same_segment.push_back(p);
            par = p;
        }

    }

    if(isSame){
        delete[] isSame;
        isSame = 0;
    }

    return same_segment;

}


template<class T>
bool fastmarching_ultratracer2(MyMarker* root,
                               T * inimg1d,
                               MyMarker* foreMarker,
                               vector<MyMarker*> &outtree,
                               float * phi,
                               long * parent,
                               char * state,
                               float * path,
                               long sz0,
                               long sz1,
                               long sz2,
                               int cnn_type = 3,
                               double length = 30,
                               XYZ lastDire = XYZ(0,0,0)){

    QElapsedTimer timer2;
    timer2.start();

    enum{ALIVE = -1, TRIAL = 0, FAR = 1, FINAL = 2, TMP = 3};

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;

//    float * phi = 0;
//    long * parent = 0;
//    char * state = 0;
//    float * path = 0;
//    try
//    {
//        phi = new float[tol_sz];
//        parent = new long[tol_sz];
//        state = new char[tol_sz];
//        path = new float[tol_sz];
//        for(long i = 0; i < tol_sz; i++)
//        {
//            phi[i] = INF;
//            parent[i] = i;  // each pixel point to itself at the         statements beginning
//            state[i] = FAR;
//            path[i] = 0;
//        }
//    }
//    catch (...)
//    {
//        cout << "********* Fail to allocate memory. quit fastmarching_tree()." << endl;
//        if (phi) {delete []phi; phi=0;}
//        if (parent) {delete []parent; parent=0;}
//        if (state) {delete []state; state=0;}
//        if (path) {delete []path; path=0;}
//        return false;
//    }

    // GI parameter min_int, max_int, li
    double max_int = 0; // maximum intensity, used in GI
    double min_int = INF;
    for(long i = 0; i < tol_sz; i++)
    {
        if (inimg1d[i] > max_int) max_int = inimg1d[i];
        else if (inimg1d[i] < min_int) min_int = inimg1d[i];
    }
    max_int -= min_int;

    qint64 etime2 = timer2.elapsed();
    cout<<"initial cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

    cout<<"start to mask"<<endl;

    double maskR = 5;
    MyMarker* tMakrer = root;
    while (tMakrer->parent != 0) {
        tMakrer = tMakrer->parent;
        if(dist(*tMakrer,*root) > maskR + 0.5){
            break;
        }
    }
//    segToMask(state,tMakrer,foreMarker,sz0,sz1,sz2,maskR,FINAL);

    MyMarker* tForeMarker = tMakrer;
    int cc = 0;
    while(tForeMarker != foreMarker){
        tForeMarker = tForeMarker->parent;
        cc++;
        if(cc>5){
            break;
        }
    }
    twoPointsToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FINAL);

//    maskR = 8;

    etime2 = timer2.elapsed();
    cout<<"mask cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

    cout<<"mask end"<<endl;

    // initialization

    // init state and phi for root
    long rootx = root->x + 0.5;
    long rooty = root->y + 0.5;
    long rootz = root->z + 0.5;

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

    double pc1,pc2,pc3;
    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];

    double lastpc3 = -1;

    int t = 0;
    int stopFlag = 0;
    vector<vector<MyMarker*> > markersVec;

    vector<vector<MyMarker*> > maskMakersVec;
    while(t<4){

        HeapElemX* tempElem = heap.delete_min();
        HeapElemX* tempElemCopy = new HeapElemX(tempElem->img_ind,tempElem->value);
        tempElemCopy->prev_ind = tempElem->prev_ind;
        heap.insert(tempElem);


        vector<long> aliveIndexs = vector<long>();
        aliveIndexs.clear();

        long lastInd = -1;
        while(!heap.empty())
        {
            HeapElemX* min_elem = heap.delete_min();
            elems.erase(min_elem->img_ind);

            long min_ind = min_elem->img_ind;
            long prev_ind = min_elem->prev_ind;
            delete min_elem;

            parent[min_ind] = prev_ind;

            state[min_ind] = ALIVE;
            aliveIndexs.push_back(min_ind);
            int i = min_ind % sz0;
            int j = (min_ind/sz0) % sz1;
            int k = (min_ind/sz01) % sz2;

            int pi = prev_ind % sz0;
            int pj = (prev_ind/sz0) % sz1;
            int pk = (prev_ind/sz01) % sz2;

            path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

            if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1)){
                stopFlag = 1;
                lastInd = min_ind;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    aliveIndexs.push_back(tmp_elem->img_ind);
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }
    //            qDebug()<<"path[min_ind]: "<<path[min_ind];
            if(path[min_ind]>length){
                cout<<"path:"<<path[min_ind]<<endl;
                lastInd = min_ind;
                while (!heap.empty()) {
                    HeapElemX* tmp_elem = heap.delete_min();
                    aliveIndexs.push_back(tmp_elem->img_ind);
                    elems.erase(tmp_elem->img_ind);
                    delete tmp_elem;
                }
                break;
            }


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

                        if(dist_L2(XYZ(i,j,k),XYZ(rootx,rooty,rootz))<length/3){
                            XYZ curDire = XYZ(w-i,h-j,d-k);
                            double cosAngle = dot(normalize(curDire),normalize(lastDire));
                            if(cosAngle<0){
                                continue;
                            }
                        }

                        int offset = ABS(ii) + ABS(jj) + ABS(kk);
                        if(offset == 0 || offset > cnn_type) continue;
                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                        long index = d*sz01 + h*sz0 + w;

                        if(state[index] != ALIVE && state[index] != FINAL)
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

        cout<<"heap end"<<endl;

        etime2 = timer2.elapsed();
        cout<<t<<" heap cost "<<etime2<<" milliseconds"<<endl;
        timer2.restart();

        vector<MyMarker*> tmptree = vector<MyMarker*>();

        vector<MyMarker*> maxSegMarkers;
        cout<<"lastind: "<<lastInd<<endl;
        // save current swc tree
        if (1)
        {
            int i = -1, j = -1, k = -1;
            map<long, MyMarker*> tmp_map;
//            for(long ind = 0; ind < tol_sz; ind++)
//            {
//                i++; if(i%sz0 == 0){i=0;j++; if(j%sz1 == 0) {j=0; k++;}}
//                if(state[ind] != ALIVE) continue;
//                MyMarker * marker = new MyMarker(i,j,k);
//                tmp_map[ind] = marker;
//                tmptree.push_back(marker);
//            }
            for(long ii = 0; ii < aliveIndexs.size(); ii++)
            {
                long ind = aliveIndexs[ii];
                i = ind % sz0;
                j = (ind/sz0) % sz1;
                k = (ind/sz01) % sz2;
                if(state[ind] != ALIVE) continue;
                MyMarker * marker = new MyMarker(i,j,k);
                tmp_map[ind] = marker;
                tmptree.push_back(marker);
            }
            for(long ii = 0; ii < aliveIndexs.size(); ii++)
            {
                long ind = aliveIndexs[ii];
                i = ind % sz0;
                j = (ind/sz0) % sz1;
                k = (ind/sz01) % sz2;
                if(state[ind] == TRIAL)
                    state[ind] = FAR;
                if(state[ind] != ALIVE) continue;
    //            state[ind] = FINAL; //set final
                long ind2 = parent[ind];
                MyMarker * marker1 = tmp_map[ind];
                MyMarker * marker2 = tmp_map[ind2];
                if(marker1 == marker2) marker1->parent = 0;
                else marker1->parent = marker2;
                //tmp_map[ind]->parent = tmp_map[ind2];
            }
//            i=-1; j = -1; k = -1;
//            for(long ind = 0; ind < tol_sz; ind++)
//            {
//                i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
//                if(state[ind] == TRIAL)
//                    state[ind] = FAR;
//                if(state[ind] != ALIVE) continue;
//    //            state[ind] = FINAL; //set final
//                long ind2 = parent[ind];
//                MyMarker * marker1 = tmp_map[ind];
//                MyMarker * marker2 = tmp_map[ind2];
//                if(marker1 == marker2) marker1->parent = 0;
//                else marker1->parent = marker2;
//                //tmp_map[ind]->parent = tmp_map[ind2];
//            }
            if(lastInd == -1){
                if(tempElemCopy) delete tempElemCopy;
                for(int i=0; i<tmptree.size(); i++){
                    if(tmptree[i]){
                        delete tmptree[i];
                    }
                }
                tmptree.clear();
                for(long ii=0; ii<aliveIndexs.size(); ii++){
                    long ind = aliveIndexs[ii];
                    if(state[ind] == TRIAL || state[ind] == ALIVE){
                        state[ind] = FAR;
                    }
                    phi[ind] = INF;
                    path[ind] = 0;
                    parent[ind] = ind;
                }
                break;
            }

            MyMarker* tmpMarker = tmp_map[lastInd];
            maxSegMarkers.push_back(tmpMarker);
            while(tmpMarker->parent != 0){
                tmpMarker = tmpMarker->parent;
                maxSegMarkers.push_back(tmpMarker);
            }
        }
        QString s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty)
                + "_" + QString::number(rootz) + "_" + QString::number(t) + "_init.swc";
        saveSWC_file(s.toStdString(),tmptree);

        etime2 = timer2.elapsed();
        cout<<t<<" save markers cost "<<etime2<<" milliseconds"<<endl;
        timer2.restart();

        qDebug()<<t<<" tmpTree size: "<<tmptree.size();

        if(tmptree.size() <= 1){
            if(tempElemCopy) delete tempElemCopy;
            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();
            for(long ii=0; ii<aliveIndexs.size(); ii++){
                long ind = aliveIndexs[ii];
                if(state[ind] == TRIAL || state[ind] == ALIVE){
                    state[ind] = FAR;
                }
                phi[ind] = INF;
                path[ind] = 0;
                parent[ind] = ind;
            }
            break;
        }

        computeLocalPcaEigVec2(tmptree,sz0,sz1,sz2,pc1,pc2,pc3,vec1,vec2,vec3);
        qDebug()<<QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
               <<" t: "<<t<<" pc1 pc2 pc3: "<<pc1<<" "<<pc2<<" "<<pc3;

        etime2 = timer2.elapsed();
        cout<<t<<" compute eigvec cost "<<etime2<<" milliseconds"<<endl;
        timer2.restart();

        if((lastpc3 == -1 && pc3>2.5) || (lastpc3 != -1 && pc3>2) || (lastpc3 != -1 && pc3/lastpc3>2)){
            if(tempElemCopy) delete tempElemCopy;
            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();
            for(long ii=0; ii<aliveIndexs.size(); ii++){
                long ind = aliveIndexs[ii];
                if(state[ind] == TRIAL || state[ind] == ALIVE){
                    state[ind] = FAR;
                }
                phi[ind] = INF;
                path[ind] = 0;
                parent[ind] = ind;
            }
            break;
        }



//        vector<HierarchySegment*> segs = vector<HierarchySegment*>();
//        swc2topo_segs<unsigned char>(tmptree,segs,1);

//        if(segs.empty()){
//            if(tempElemCopy) delete tempElemCopy;
//            for(int i=0; i<tmptree.size(); i++){
//                if(tmptree[i]){
//                    delete tmptree[i];
//                }
//            }
//            tmptree.clear();
//            break;
//        }

//        sort(segs.begin(),segs.end(),segCmp);

//        vector<MyMarker*> maxSegMarkers;
//        segs[0]->get_markers(maxSegMarkers);
//        segToMask(state,maxSegMarkers.front(),*(maxSegMarkers.begin()+5),sz0,sz1,sz2,maskR,FINAL);


        etime2 = timer2.elapsed();
        cout<<t<<" in mask cost "<<etime2<<" milliseconds"<<endl;
        timer2.restart();

        vector<MyMarker*> tmpMarkers;
        MyMarker* curP = 0;
        for(int i=maxSegMarkers.size()-1; i>=0; i--){
            MyMarker* pp = maxSegMarkers[i];
            MyMarker* p = new MyMarker(pp->x,pp->y,pp->z);
            p->parent = curP;
            tmpMarkers.push_back(p);
            curP = p;
        }

        twoPointsToMask(state,tmpMarkers.back(),*(tmpMarkers.end()-5),sz0,sz1,sz2,maskR,FINAL);

        vector<MyMarker*> maskMakers;
        maskMakers.push_back(tmpMarkers.back());
        maskMakers.push_back(*(tmpMarkers.end()-5));
        maskMakersVec.push_back(maskMakers);


        markersVec.push_back(tmpMarkers);

        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]){
                delete tmptree[i];
            }
        }
        tmptree.clear();

//        for(long ind=0; ind<tol_sz; ind++){
//            if(state[ind] == TRIAL){
//                state[ind] = FAR;
//            }else if (state[ind] == ALIVE) {
//    //            if (inimg1d[ind] > max_int) max_int = inimg1d[ind];
//    //            if (inimg1d[ind] < min_int) min_int = inimg1d[ind];
//                state[ind] = FAR;
//            }
//            phi[ind] = INF;
//            path[ind] = 0;
//            parent[ind] = ind;
//        }
        for(long ii=0; ii<aliveIndexs.size(); ii++){
            long ind = aliveIndexs[ii];
            if(state[ind] == TRIAL || state[ind] == ALIVE){
                state[ind] = FAR;
            }
            phi[ind] = INF;
            path[ind] = 0;
            parent[ind] = ind;
        }

        //init heap
        {
            heap.insert(tempElemCopy);
            elems[tempElemCopy->img_ind] = tempElemCopy;
            path[tempElemCopy->img_ind] = 0;
            phi[tempElemCopy->img_ind] = 0;
        }

        etime2 = timer2.elapsed();
        cout<<t<<" release cost "<<etime2<<" milliseconds"<<endl;
        timer2.restart();

        t++;
        lastpc3 = pc3;

        if(stopFlag == 1){
            break;
        }


    }

    cout<<"before markersVec size: "<<markersVec.size()<<endl;
/*
    if(markersVec.size()>1){

        for(int i=0; i<maskMakersVec.size(); i++){
            MyMarker* p1 = maskMakersVec[i][0];
            MyMarker* p2 = maskMakersVec[i][1];
            twoPointsToMask(state,p1,p2,sz0,sz1,sz2,maskR,FAR);
        }
        maskMakersVec.clear();
        for(int i=0; i<markersVec.size(); i++){
            for(int j=0; j<markersVec[i].size(); j++){
                if(markersVec[i][j]){
                    delete markersVec[i][j];
                }
            }
        }
        markersVec.clear();

        state[root_ind] = ALIVE;
        phi[root_ind] = 0.0;

        // init heap
        {
            long index = root_ind;
            HeapElemX *elem = new HeapElemX(index, phi[index]);
            elem->prev_ind = index;
            heap.insert(elem);
            elems[index] = elem;
        }

        double lastpc3 = -1;

        t = 0;
        stopFlag = 0;
        while(t<4){

            HeapElemX* tempElem = heap.delete_min();
            HeapElemX* tempElemCopy = new HeapElemX(tempElem->img_ind,tempElem->value);
            tempElemCopy->prev_ind = tempElem->prev_ind;
            heap.insert(tempElem);


            vector<long> aliveIndexs = vector<long>();
            aliveIndexs.clear();

            long lastInd = -1;
            while(!heap.empty())
            {
                HeapElemX* min_elem = heap.delete_min();
                elems.erase(min_elem->img_ind);

                long min_ind = min_elem->img_ind;
                long prev_ind = min_elem->prev_ind;
                delete min_elem;

                parent[min_ind] = prev_ind;

                state[min_ind] = ALIVE;
                aliveIndexs.push_back(min_ind);
                int i = min_ind % sz0;
                int j = (min_ind/sz0) % sz1;
                int k = (min_ind/sz01) % sz2;

                int pi = prev_ind % sz0;
                int pj = (prev_ind/sz0) % sz1;
                int pk = (prev_ind/sz01) % sz2;

                path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

                if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1)){
                    stopFlag = 1;
                    lastInd = min_ind;
                    while (!heap.empty()) {
                        HeapElemX* tmp_elem = heap.delete_min();
                        aliveIndexs.push_back(tmp_elem->img_ind);
                        elems.erase(tmp_elem->img_ind);
                        delete tmp_elem;
                    }
                    break;
                }
        //            qDebug()<<"path[min_ind]: "<<path[min_ind];
                if(path[min_ind]>length*2){
                    cout<<"path:"<<path[min_ind]<<endl;
                    lastInd = min_ind;
                    while (!heap.empty()) {
                        HeapElemX* tmp_elem = heap.delete_min();
                        aliveIndexs.push_back(tmp_elem->img_ind);
                        elems.erase(tmp_elem->img_ind);
                        delete tmp_elem;
                    }
                    break;
                }


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

                            if(dist_L2(XYZ(i,j,k),XYZ(rootx,rooty,rootz))<length/3){
                                XYZ curDire = XYZ(w-i,h-j,d-k);
                                double cosAngle = dot(normalize(curDire),normalize(lastDire));
                                if(cosAngle<0){
                                    continue;
                                }
                            }

                            int offset = ABS(ii) + ABS(jj) + ABS(kk);
                            if(offset == 0 || offset > cnn_type) continue;
                            double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                            long index = d*sz01 + h*sz0 + w;

                            if(state[index] != ALIVE && state[index] != FINAL)
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

            cout<<"heap end"<<endl;

            etime2 = timer2.elapsed();
            cout<<t<<" heap cost "<<etime2<<" milliseconds"<<endl;
            timer2.restart();

            vector<MyMarker*> tmptree = vector<MyMarker*>();

            vector<MyMarker*> maxSegMarkers;
            cout<<"lastind: "<<lastInd<<endl;
            // save current swc tree
            if (1)
            {
                int i = -1, j = -1, k = -1;
                map<long, MyMarker*> tmp_map;
    //            for(long ind = 0; ind < tol_sz; ind++)
    //            {
    //                i++; if(i%sz0 == 0){i=0;j++; if(j%sz1 == 0) {j=0; k++;}}
    //                if(state[ind] != ALIVE) continue;
    //                MyMarker * marker = new MyMarker(i,j,k);
    //                tmp_map[ind] = marker;
    //                tmptree.push_back(marker);
    //            }
                for(long ii = 0; ii < aliveIndexs.size(); ii++)
                {
                    long ind = aliveIndexs[ii];
                    i = ind % sz0;
                    j = (ind/sz0) % sz1;
                    k = (ind/sz01) % sz2;
                    if(state[ind] != ALIVE) continue;
                    MyMarker * marker = new MyMarker(i,j,k);
                    tmp_map[ind] = marker;
                    tmptree.push_back(marker);
                }
                for(long ii = 0; ii < aliveIndexs.size(); ii++)
                {
                    long ind = aliveIndexs[ii];
                    i = ind % sz0;
                    j = (ind/sz0) % sz1;
                    k = (ind/sz01) % sz2;
                    if(state[ind] == TRIAL)
                        state[ind] = FAR;
                    if(state[ind] != ALIVE) continue;
        //            state[ind] = FINAL; //set final
                    long ind2 = parent[ind];
                    MyMarker * marker1 = tmp_map[ind];
                    MyMarker * marker2 = tmp_map[ind2];
                    if(marker1 == marker2) marker1->parent = 0;
                    else marker1->parent = marker2;
                    //tmp_map[ind]->parent = tmp_map[ind2];
                }
    //            i=-1; j = -1; k = -1;
    //            for(long ind = 0; ind < tol_sz; ind++)
    //            {
    //                i++; if(i%sz0 == 0){i=0; j++; if(j%sz1==0){j=0; k++;}}
    //                if(state[ind] == TRIAL)
    //                    state[ind] = FAR;
    //                if(state[ind] != ALIVE) continue;
    //    //            state[ind] = FINAL; //set final
    //                long ind2 = parent[ind];
    //                MyMarker * marker1 = tmp_map[ind];
    //                MyMarker * marker2 = tmp_map[ind2];
    //                if(marker1 == marker2) marker1->parent = 0;
    //                else marker1->parent = marker2;
    //                //tmp_map[ind]->parent = tmp_map[ind2];
    //            }
                if(lastInd == -1){
                    if(tempElemCopy) delete tempElemCopy;
                    for(int i=0; i<tmptree.size(); i++){
                        if(tmptree[i]){
                            delete tmptree[i];
                        }
                    }
                    tmptree.clear();
                    for(long ii=0; ii<aliveIndexs.size(); ii++){
                        long ind = aliveIndexs[ii];
                        if(state[ind] == TRIAL || state[ind] == ALIVE){
                            state[ind] = FAR;
                        }
                        phi[ind] = INF;
                        path[ind] = 0;
                        parent[ind] = ind;
                    }
                    break;
                }

                MyMarker* tmpMarker = tmp_map[lastInd];
                maxSegMarkers.push_back(tmpMarker);
                while(tmpMarker->parent != 0){
                    tmpMarker = tmpMarker->parent;
                    maxSegMarkers.push_back(tmpMarker);
                }
            }
            QString s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty)
                    + "_" + QString::number(rootz) + "_" + QString::number(t) + "_init2.swc";
            saveSWC_file(s.toStdString(),tmptree);

            etime2 = timer2.elapsed();
            cout<<t<<" save markers cost "<<etime2<<" milliseconds"<<endl;
            timer2.restart();

            qDebug()<<t<<" tmpTree size: "<<tmptree.size();

            if(tmptree.size() <= 1){
                if(tempElemCopy) delete tempElemCopy;
                for(int i=0; i<tmptree.size(); i++){
                    if(tmptree[i]){
                        delete tmptree[i];
                    }
                }
                tmptree.clear();
                for(long ii=0; ii<aliveIndexs.size(); ii++){
                    long ind = aliveIndexs[ii];
                    if(state[ind] == TRIAL || state[ind] == ALIVE){
                        state[ind] = FAR;
                    }
                    phi[ind] = INF;
                    path[ind] = 0;
                    parent[ind] = ind;
                }
                break;
            }

            computeLocalPcaEigVec2(tmptree,sz0,sz1,sz2,pc1,pc2,pc3,vec1,vec2,vec3);
            qDebug()<<QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                   <<" t: "<<t<<" pc1 pc2 pc3: "<<pc1<<" "<<pc2<<" "<<pc3;

            etime2 = timer2.elapsed();
            cout<<t<<" compute eigvec cost "<<etime2<<" milliseconds"<<endl;
            timer2.restart();

            if((lastpc3 == -1 && pc3>2.5) || (lastpc3 != -1 && pc3>2) || (lastpc3 != -1 && pc3/lastpc3>2)){
                if(tempElemCopy) delete tempElemCopy;
                for(int i=0; i<tmptree.size(); i++){
                    if(tmptree[i]){
                        delete tmptree[i];
                    }
                }
                tmptree.clear();
                for(long ii=0; ii<aliveIndexs.size(); ii++){
                    long ind = aliveIndexs[ii];
                    if(state[ind] == TRIAL || state[ind] == ALIVE){
                        state[ind] = FAR;
                    }
                    phi[ind] = INF;
                    path[ind] = 0;
                    parent[ind] = ind;
                }
                break;
            }



    //        vector<HierarchySegment*> segs = vector<HierarchySegment*>();
    //        swc2topo_segs<unsigned char>(tmptree,segs,1);

    //        if(segs.empty()){
    //            if(tempElemCopy) delete tempElemCopy;
    //            for(int i=0; i<tmptree.size(); i++){
    //                if(tmptree[i]){
    //                    delete tmptree[i];
    //                }
    //            }
    //            tmptree.clear();
    //            break;
    //        }

    //        sort(segs.begin(),segs.end(),segCmp);

    //        vector<MyMarker*> maxSegMarkers;
    //        segs[0]->get_markers(maxSegMarkers);
    //        segToMask(state,maxSegMarkers.front(),*(maxSegMarkers.begin()+5),sz0,sz1,sz2,maskR,FINAL);


            etime2 = timer2.elapsed();
            cout<<t<<" in mask cost "<<etime2<<" milliseconds"<<endl;
            timer2.restart();

            vector<MyMarker*> tmpMarkers;
            MyMarker* curP = 0;
            for(int i=maxSegMarkers.size()-1; i>=0; i--){
                MyMarker* pp = maxSegMarkers[i];
                MyMarker* p = new MyMarker(pp->x,pp->y,pp->z);
                p->parent = curP;
                tmpMarkers.push_back(p);
                curP = p;
            }

            twoPointsToMask(state,tmpMarkers.back(),*(tmpMarkers.end()-30),sz0,sz1,sz2,maskR,FINAL);

            vector<MyMarker*> maskMakers;
            maskMakers.push_back(tmpMarkers.back());
            maskMakers.push_back(*(tmpMarkers.end()-30));
            maskMakersVec.push_back(maskMakers);


            markersVec.push_back(tmpMarkers);

            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();

    //        for(long ind=0; ind<tol_sz; ind++){
    //            if(state[ind] == TRIAL){
    //                state[ind] = FAR;
    //            }else if (state[ind] == ALIVE) {
    //    //            if (inimg1d[ind] > max_int) max_int = inimg1d[ind];
    //    //            if (inimg1d[ind] < min_int) min_int = inimg1d[ind];
    //                state[ind] = FAR;
    //            }
    //            phi[ind] = INF;
    //            path[ind] = 0;
    //            parent[ind] = ind;
    //        }
            for(long ii=0; ii<aliveIndexs.size(); ii++){
                long ind = aliveIndexs[ii];
                if(state[ind] == TRIAL || state[ind] == ALIVE){
                    state[ind] = FAR;
                }
                phi[ind] = INF;
                path[ind] = 0;
                parent[ind] = ind;
            }

            //init heap
            {
                heap.insert(tempElemCopy);
                elems[tempElemCopy->img_ind] = tempElemCopy;
                path[tempElemCopy->img_ind] = 0;
                phi[tempElemCopy->img_ind] = 0;
            }

            etime2 = timer2.elapsed();
            cout<<t<<" release cost "<<etime2<<" milliseconds"<<endl;
            timer2.restart();

            t++;
            lastpc3 = pc3;

            if(stopFlag == 1){
                break;
            }


        }
    }
*/
    cout<<"markersVec size: "<<markersVec.size()<<endl;
    if(markersVec.size() == 0){
        map<long, HeapElemX*>::iterator mit = elems.begin();
        while (mit != elems.end())
        {
            HeapElemX * elem = mit->second; delete elem; mit++;
        }

//        if(phi){delete [] phi; phi = 0;}
//        if(parent){delete [] parent; parent = 0;}
//        if(state) {delete [] state; state = 0;}
//        if(path) {delete []path; path=0;}

        if(vec1){delete[] vec1; vec1 = 0;}
        if(vec2){delete[] vec2; vec2 = 0;}
        if(vec3){delete[] vec3; vec3 = 0;}
        return true;
    }

    vector<MyMarker*> markers = mergeSegs(markersVec);

    QString s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
            + "_size" + QString::number(markersVec.size()) +  +  "_partial2.swc";
    saveSWC_file(s.toStdString(),markers);

    vector<MyMarker*> leafMarkers;
    MyMarker* rootMarker;

    map<MyMarker*,int> markersMap;
    for(int i=0; i<markers.size(); i++){
        markersMap[markers[i]] = i;
    }
    vector<vector<int> > children = vector<vector<int> >(markers.size(), vector<int>());

    for(int i=0; i<markers.size(); i++){
        if(markers[i]->parent == 0){
            rootMarker = markers[i];
            continue;
        }
        children[markersMap[markers[i]->parent]].push_back(i);
    }

    for(int i=0; i<markers.size(); i++){
        if(children[i].size() == 0){
            leafMarkers.push_back(markers[i]);
        }
    }

    int rootMarkerIndex = markersMap[rootMarker];
    for(int i=0; i<children[rootMarkerIndex].size(); i++){
        int cIndex = children[rootMarkerIndex][i];
        markers[cIndex]->parent = root;
    }

    for(int i=0; i<markers.size(); i++){
        if(markers[i] == rootMarker)
            continue;
        outtree.push_back(markers[i]);
    }

    if(stopFlag == 1){
        map<long, HeapElemX*>::iterator mit = elems.begin();
        while (mit != elems.end())
        {
            HeapElemX * elem = mit->second; delete elem; mit++;
        }

//        if(phi){delete [] phi; phi = 0;}
//        if(parent){delete [] parent; parent = 0;}
//        if(state) {delete [] state; state = 0;}
//        if(path) {delete []path; path=0;}

        if(vec1){delete[] vec1; vec1 = 0;}
        if(vec2){delete[] vec2; vec2 = 0;}
        if(vec3){delete[] vec3; vec3 = 0;}
        return true;
    }

    etime2 = timer2.elapsed();
    cout<<" add marker cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();


//    V3DLONG wx = 5, wy = 5, wz = 5;
//    double pc1,pc2,pc3;
//    double* vec1 = new double[3];
//    double* vec2 = new double[3];
//    double* vec3 = new double[3];
//    V3DLONG sz[4] = {sz0,sz1,sz2,1};
//    for(int i=0; i<maxSegMarkers.size(); i++){
//        MyMarker* p = maxSegMarkers[i];
//        computeCubePcaEigVec(inimg1d,sz,p->x,p->y,p->z,wx,wy,wz,pc1,pc2,pc3,vec1,vec2,vec3);
//        qDebug()<<i<<" pc1 pc2 pc3: "<<pc1<<" "<<pc2<<" "<<pc3;
//        p->radius = pc1/pc2;
//    }


//    MyMarker* mParent = root;
//    for(int i=maxSegMarkers.size()-2; i>=0; i--){
//        MyMarker* p = new MyMarker(maxSegMarkers[i]->x,maxSegMarkers[i]->y,maxSegMarkers[i]->z);
//        p->parent = mParent;
//        p->radius = pc1/pc2;
//        if(pc1/pc2<5){
//            p->type = 5;
//        }else {
//            p->type = 2;
//        }

//        if(pc1/pc3<5){
//            p->type = 7;
//        }

//        outtree.push_back(p);
//        mParent = p;
//    }

//    s = "D:\\testDynamicTracing\\" +QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz) + "_partial.swc";
//    saveSWC_file(s.toStdString(),maxSegMarkers);

//    s = "D:\\testDynamicTracing\\" +QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz) + "_partial2.swc";
//    saveSWC_file(s.toStdString(),maxSegMarkers2);


    map<long, HeapElemX*>::iterator mit = elems.begin();
    while (mit != elems.end())
    {
        HeapElemX * elem = mit->second; delete elem; mit++;
    }

//    if(phi){delete [] phi; phi = 0;}
//    if(parent){delete [] parent; parent = 0;}
//    if(state) {delete [] state; state = 0;}
//    if(path) {delete []path; path=0;}

    if(vec1){delete[] vec1; vec1 = 0;}
    if(vec2){delete[] vec2; vec2 = 0;}
    if(vec3){delete[] vec3; vec3 = 0;}

//    if(stopFlag == 1){
//        qDebug()<<"touch tip";
//        return true;
//    }

//    if(pc1/pc3<5 || pc1/pc2<10){
//        qDebug()<<"pc1/pc3"<<pc1/pc3;
//        return true;
//    }

//    fastmarching_ultratracer2(mParent,inimg1d,root,outtree,sz0,sz1,sz2,cnn_type,length);

    etime2 = timer2.elapsed();
    cout<<" release memory cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

//    for(long i = 0; i < tol_sz; i++){
//        phi[i] = INF;
//        parent[i] = i;  // each pixel point to itself at the         statements beginning
//        state[i] = FAR;
//        path[i] = 0;
//    }
    for(int i=0; i<maskMakersVec.size(); i++){
        MyMarker* p1 = maskMakersVec[i][0];
        MyMarker* p2 = maskMakersVec[i][1];
        twoPointsToMask(state,p1,p2,sz0,sz1,sz2,maskR,FAR);
    }

    for(int i=0; i<leafMarkers.size(); i++){
        MyMarker* leafMarker = leafMarkers[i];
        MyMarker* leafForeMarker = leafMarker;
        int c = 0;
        while(leafForeMarker != root){
            leafForeMarker = leafForeMarker->parent;
            c++;
            if(c>5){
                break;
            }
        }
        lastDire = XYZ(leafMarker->x-leafForeMarker->x,leafMarker->y-leafForeMarker->y,leafMarker->z-leafForeMarker->z);

        fastmarching_ultratracer2(leafMarker,inimg1d,root,outtree,phi,parent,state,path,
                                  sz0,sz1,sz2,cnn_type,length,lastDire);
    }

    return true;

}

template<class T>
bool fastmarching_core(MyMarker* root,
                       T * inimg1d,
                       vector<MyMarker*> &tmptree,
                       vector<MyMarker*> &tmpMarkers,
                       int& stopFlag,
                       double max_int,
                       double min_int,
                       float * phi,
                       long * parent,
                       char * state,
                       float * path,
                       long sz0,
                       long sz1,
                       long sz2,
                       double lineThres,
                       int cnn_type = 3,
                       double length = 30,
                       XYZ lastDire = XYZ(0,0,0)){

    long sz01 = sz0*sz1;
    enum{ALIVE = -1, TRIAL = 0, FAR = 1, FINAL = 2, TMP = 3};

    // init state and phi for root
    long rootx = root->x + 0.5;
    long rooty = root->y + 0.5;
    long rootz = root->z + 0.5;

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

    vector<long> aliveIndexs = vector<long>();
    aliveIndexs.clear();

    long lastInd = -1;

    while(!heap.empty())
    {
        HeapElemX* min_elem = heap.delete_min();
        elems.erase(min_elem->img_ind);

        long min_ind = min_elem->img_ind;
        long prev_ind = min_elem->prev_ind;
        delete min_elem;

        parent[min_ind] = prev_ind;

        state[min_ind] = ALIVE;
        aliveIndexs.push_back(min_ind);
        int i = min_ind % sz0;
        int j = (min_ind/sz0) % sz1;
        int k = (min_ind/sz01) % sz2;

        int pi = prev_ind % sz0;
        int pj = (prev_ind/sz0) % sz1;
        int pk = (prev_ind/sz01) % sz2;

        path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

        if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1)){
            stopFlag = 1;
            lastInd = min_ind;
            while (!heap.empty()) {
                HeapElemX* tmp_elem = heap.delete_min();
                aliveIndexs.push_back(tmp_elem->img_ind);
                elems.erase(tmp_elem->img_ind);
                delete tmp_elem;
            }
            break;
        }
//            qDebug()<<"path[min_ind]: "<<path[min_ind];
        if(path[min_ind]>length){
            cout<<"path:"<<path[min_ind]<<endl;
            lastInd = min_ind;
            while (!heap.empty()) {
                HeapElemX* tmp_elem = heap.delete_min();
                aliveIndexs.push_back(tmp_elem->img_ind);
                elems.erase(tmp_elem->img_ind);
                delete tmp_elem;
            }
            break;
        }


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

                    if(dist_L2(XYZ(i,j,k),XYZ(rootx,rooty,rootz))<length/6){
                        XYZ curDire = XYZ(w-i,h-j,d-k);
                        double cosAngle = dot(normalize(curDire),normalize(lastDire));
                        if(cosAngle<0){
                            continue;
                        }
                    }

                    int offset = ABS(ii) + ABS(jj) + ABS(kk);
                    if(offset == 0 || offset > cnn_type) continue;
                    double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                    long index = d*sz01 + h*sz0 + w;

                    if(state[index] != ALIVE && state[index] != FINAL)
                    {
//                        double cFactor = exp(abs(inimg1d[index]-lineThres));
                        double lineThresMax = lineThres + 10;
                        double lineThresMin = lineThres - 10;
                        double cFactor = 1;
//                        if(inimg1d[index]>=lineThresMin && inimg1d[index]<=lineThresMax)
//                            cFactor = 1;
//                        else{
//                            if(inimg1d[index]>lineThres){
//                                cFactor = exp(inimg1d[index]-lineThres);
//                            }else{
//                                cFactor = exp(lineThres-inimg1d[index]);
//                            }
//                        }

                        double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5*cFactor;
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

    cout<<"heap end"<<endl;

//    etime2 = timer2.elapsed();
//    cout<<t<<" heap cost "<<etime2<<" milliseconds"<<endl;
//    timer2.restart();

//    vector<MyMarker*> tmptree = vector<MyMarker*>();

//    vector<MyMarker*> maxSegMarkers;
    cout<<"lastind: "<<lastInd<<endl;
    // save current swc tree
    if (1)
    {
        int i = -1, j = -1, k = -1;
        map<long, MyMarker*> tmp_map;

        for(long ii = 0; ii < aliveIndexs.size(); ii++)
        {
            long ind = aliveIndexs[ii];
            i = ind % sz0;
            j = (ind/sz0) % sz1;
            k = (ind/sz01) % sz2;
            if(state[ind] != ALIVE) continue;
            MyMarker * marker = new MyMarker(i,j,k);
            tmp_map[ind] = marker;
            tmptree.push_back(marker);
        }
        for(long ii = 0; ii < aliveIndexs.size(); ii++)
        {
            long ind = aliveIndexs[ii];
            i = ind % sz0;
            j = (ind/sz0) % sz1;
            k = (ind/sz01) % sz2;
            if(state[ind] == TRIAL)
                state[ind] = FAR;
            if(state[ind] != ALIVE) continue;
//            state[ind] = FINAL; //set final
            long ind2 = parent[ind];
            MyMarker * marker1 = tmp_map[ind];
            MyMarker * marker2 = tmp_map[ind2];
            if(marker1 == marker2) marker1->parent = 0;
            else marker1->parent = marker2;
            //tmp_map[ind]->parent = tmp_map[ind2];
        }
        if(lastInd == -1){
//            for(int i=0; i<tmptree.size(); i++){
//                if(tmptree[i]){
//                    delete tmptree[i];
//                }
//            }
//            tmptree.clear();
            for(long ii=0; ii<aliveIndexs.size(); ii++){
                long ind = aliveIndexs[ii];
                if(state[ind] == TRIAL || state[ind] == ALIVE){
                    state[ind] = FAR;
                }
                phi[ind] = INF;
                path[ind] = 0;
                parent[ind] = ind;
            }
            map<long, HeapElemX*>::iterator mit = elems.begin();
            while (mit != elems.end())
            {
                HeapElemX * elem = mit->second; delete elem; mit++;
            }
            return false;
        }

        vector<MyMarker*> maxSegMarkers;
        MyMarker* tmpMarker = tmp_map[lastInd];
        maxSegMarkers.push_back(tmpMarker);
        while(tmpMarker->parent != 0){
            tmpMarker = tmpMarker->parent;
            maxSegMarkers.push_back(tmpMarker);
        }

//        vector<MyMarker*> tmpMarkers;
        MyMarker* curP = 0;
        for(int i=maxSegMarkers.size()-1; i>=0; i--){
            MyMarker* pp = maxSegMarkers[i];
            MyMarker* p = new MyMarker(pp->x,pp->y,pp->z);
            p->parent = curP;
            tmpMarkers.push_back(p);
            curP = p;
        }

//        QString s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty)
//                + "_" + QString::number(rootz) + "_" + QString::number(t) + "_init.swc";
//        saveSWC_file(s.toStdString(),tmptree);
    }

    for(long ii=0; ii<aliveIndexs.size(); ii++){
        long ind = aliveIndexs[ii];
        if(state[ind] == TRIAL || state[ind] == ALIVE){
            state[ind] = FAR;
        }
        phi[ind] = INF;
        path[ind] = 0;
        parent[ind] = ind;
    }

    return true;

}
template<class T>
bool saveMaskImage(T* inimg1d, char * state, unsigned char* maskImage,  long sz0, long sz1, long sz2,
                   QString imageName, V3DPluginCallback2 & callback){
    long tol_sz = sz0*sz1*sz2;
//    unsigned char* maskImage = new unsigned char[tol_sz];
    for(long i=0; i<tol_sz; i++){
        if(state[i] != 2){
            maskImage[i] = (unsigned char) inimg1d[i];
        }else{
            maskImage[i] = 0;
        }
    }
    V3DLONG sz[4] = {sz0,sz1,sz2,1};
    int dataType = 1;
//    saveImage(imageName.toStdString().c_str(),maskImage,sz,dataType);
//    Image4DSimple image;
//    image.setData(maskImage,sz0,sz1,sz2,1,V3D_UINT8);
//    image.saveImage(imageName.toStdString().c_str());
//    V3DPluginCallback2 callback;
    simple_saveimage_wrapper(callback,imageName.toStdString().c_str(),maskImage,sz,dataType);
    return true;
}

template<class T>
bool onePointsToMask(T* state, MyMarker* p, long sz0, long sz1, long sz2, double r, int flag){
    for(int k=-r; k<=r; k++){
        for(int j=-r; j<=r; j++){
            for(int i=-r; i<=r; i++){
                double radius = sqrt(k*k + j*j + i*i);
                if(radius>r)
                    continue;
                int z = p->z + k;
                int y = p->y + j;
                int x = p->x + i;
                if(x < 0 || x >= sz0 || y < 0 || y >= sz1 || z < 0 || z >= sz2) continue;
                long index = z*sz0*sz1 + y*sz0 + x;
                state[index] = (T)flag;
            }
        }
    }
    return true;

}

template<class T>
bool fastmarching_ultratracer2_line(MyMarker* root,
                               T * inimg1d,
                               MyMarker* foreMarker,
                               vector<MyMarker*> &outtree,
                               float * phi,
                               long * parent,
                               char * state,
                               float * path,
                               long sz0,
                               long sz1,
                               long sz2,
                               double lineThres,
                               V3DPluginCallback2& callback,
                               int cnn_type = 3,
                               double length = 30,
                               XYZ lastDire = XYZ(0,0,0)){

    QElapsedTimer timer2;
    timer2.start();

    enum{ALIVE = -1, TRIAL = 0, FAR = 1, FINAL = 2, TMP = 3};

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;

    // GI parameter min_int, max_int, li
    double max_int = 0; // maximum intensity, used in GI
    double min_int = INF;
    for(long i = 0; i < tol_sz; i++)
    {
        if (inimg1d[i] > max_int) max_int = inimg1d[i];
        else if (inimg1d[i] < min_int) min_int = inimg1d[i];
    }
    max_int -= min_int;

    qint64 etime2 = timer2.elapsed();
    cout<<"initial cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

    cout<<"start to mask"<<endl;

    unsigned char* maskImage = new unsigned char[tol_sz];

    double maskR = 10;
    MyMarker* tMakrer = root;
    while (tMakrer->parent != 0) {
        tMakrer = tMakrer->parent;
        if(dist(*tMakrer,*root) > maskR + 0.5){
            break;
        }
    }
//    segToMask(state,tMakrer,foreMarker,sz0,sz1,sz2,maskR,FINAL);

    MyMarker* tForeMarker = tMakrer;
    int cc = 0;
    while(tForeMarker != foreMarker){
        tForeMarker = tForeMarker->parent;
        cc++;
        if(cc>5){
            break;
        }
    }
    twoPointsToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FINAL);
//    segToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FINAL);

//    maskR = 8;

    etime2 = timer2.elapsed();
    cout<<"mask cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

    cout<<"mask end"<<endl;

    // initialization

    // init state and phi for root
    long rootx = root->x + 0.5;
    long rooty = root->y + 0.5;
    long rootz = root->z + 0.5;

    long root_ind = rootz*sz01 + rooty*sz0 + rootx;

    QString imageName = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
            + "_init1.v3draw";
//    saveMaskImage(inimg1d,state,maskImage,sz0,sz1,sz2,imageName,callback);

//    state[root_ind] = ALIVE;
//    phi[root_ind] = 0.0;

//    BasicHeap<HeapElemX> heap;
//    map<long, HeapElemX*> elems;

    // init heap
//    {
//        long index = root_ind;
//        HeapElemX *elem = new HeapElemX(index, phi[index]);
//        elem->prev_ind = index;
//        heap.insert(elem);
//        elems[index] = elem;
//    }

    double pc1,pc2,pc3;
    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];

    double lastpc3 = -1;

    int t = 0;
    int stopFlag = 0;

    vector<MyMarker*> tmptree = vector<MyMarker*>();

    vector<MyMarker*> tmpMarkers;

    fastmarching_core(root,inimg1d,tmptree,tmpMarkers,stopFlag,max_int,min_int,phi,parent,state,path,sz0,sz1,sz2,lineThres,cnn_type,length,lastDire);

    QString s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
            + "_init0.swc";
//    saveSWC_file(s.toStdString(),tmptree);

    /*
    vector<vector<MyMarker*> > markersVec;

    vector<vector<MyMarker*> > maskMakersVec;

    vector<long> aliveIndexs = vector<long>();
    aliveIndexs.clear();

    long lastInd = -1;
    while(!heap.empty())
    {
        HeapElemX* min_elem = heap.delete_min();
        elems.erase(min_elem->img_ind);

        long min_ind = min_elem->img_ind;
        long prev_ind = min_elem->prev_ind;
        delete min_elem;

        parent[min_ind] = prev_ind;

        state[min_ind] = ALIVE;
        aliveIndexs.push_back(min_ind);
        int i = min_ind % sz0;
        int j = (min_ind/sz0) % sz1;
        int k = (min_ind/sz01) % sz2;

        int pi = prev_ind % sz0;
        int pj = (prev_ind/sz0) % sz1;
        int pk = (prev_ind/sz01) % sz2;

        path[min_ind] = path[prev_ind] + dist(MyMarker(i,j,k),MyMarker(pi,pj,pk));

        if((i<1 || i>=sz0-1 || j<1 || j>=sz1-1 || k<1 || k>=sz2-1)){
            stopFlag = 1;
            lastInd = min_ind;
            while (!heap.empty()) {
                HeapElemX* tmp_elem = heap.delete_min();
                aliveIndexs.push_back(tmp_elem->img_ind);
                elems.erase(tmp_elem->img_ind);
                delete tmp_elem;
            }
            break;
        }
//            qDebug()<<"path[min_ind]: "<<path[min_ind];
        if(path[min_ind]>length){
            cout<<"path:"<<path[min_ind]<<endl;
            lastInd = min_ind;
            while (!heap.empty()) {
                HeapElemX* tmp_elem = heap.delete_min();
                aliveIndexs.push_back(tmp_elem->img_ind);
                elems.erase(tmp_elem->img_ind);
                delete tmp_elem;
            }
            break;
        }


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

                    if(dist_L2(XYZ(i,j,k),XYZ(rootx,rooty,rootz))<length/3){
                        XYZ curDire = XYZ(w-i,h-j,d-k);
                        double cosAngle = dot(normalize(curDire),normalize(lastDire));
                        if(cosAngle<0){
                            continue;
                        }
                    }

                    int offset = ABS(ii) + ABS(jj) + ABS(kk);
                    if(offset == 0 || offset > cnn_type) continue;
                    double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                    long index = d*sz01 + h*sz0 + w;

                    if(state[index] != ALIVE && state[index] != FINAL)
                    {
//                        double cFactor = exp(abs(inimg1d[index]-lineThres));
                        double lineThresMax = lineThres + 10;
                        double lineThresMin = lineThres - 10;
                        double cFactor = 1;
//                        if(inimg1d[index]>=lineThresMin && inimg1d[index]<=lineThresMax)
//                            cFactor = 1;
//                        else{
//                            if(inimg1d[index]>lineThres){
//                                cFactor = exp(inimg1d[index]-lineThres);
//                            }else{
//                                cFactor = exp(lineThres-inimg1d[index]);
//                            }
//                        }

                        double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5*cFactor;
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

    cout<<"heap end"<<endl;

    etime2 = timer2.elapsed();
    cout<<t<<" heap cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

    vector<MyMarker*> tmptree = vector<MyMarker*>();

    vector<MyMarker*> maxSegMarkers;
    cout<<"lastind: "<<lastInd<<endl;
    // save current swc tree
    if (1)
    {
        int i = -1, j = -1, k = -1;
        map<long, MyMarker*> tmp_map;

        for(long ii = 0; ii < aliveIndexs.size(); ii++)
        {
            long ind = aliveIndexs[ii];
            i = ind % sz0;
            j = (ind/sz0) % sz1;
            k = (ind/sz01) % sz2;
            if(state[ind] != ALIVE) continue;
            MyMarker * marker = new MyMarker(i,j,k);
            tmp_map[ind] = marker;
            tmptree.push_back(marker);
        }
        for(long ii = 0; ii < aliveIndexs.size(); ii++)
        {
            long ind = aliveIndexs[ii];
            i = ind % sz0;
            j = (ind/sz0) % sz1;
            k = (ind/sz01) % sz2;
            if(state[ind] == TRIAL)
                state[ind] = FAR;
            if(state[ind] != ALIVE) continue;
//            state[ind] = FINAL; //set final
            long ind2 = parent[ind];
            MyMarker * marker1 = tmp_map[ind];
            MyMarker * marker2 = tmp_map[ind2];
            if(marker1 == marker2) marker1->parent = 0;
            else marker1->parent = marker2;
            //tmp_map[ind]->parent = tmp_map[ind2];
        }
        if(lastInd == -1){
            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();
            for(long ii=0; ii<aliveIndexs.size(); ii++){
                long ind = aliveIndexs[ii];
                if(state[ind] == TRIAL || state[ind] == ALIVE){
                    state[ind] = FAR;
                }
                phi[ind] = INF;
                path[ind] = 0;
                parent[ind] = ind;
            }
            map<long, HeapElemX*>::iterator mit = elems.begin();
            while (mit != elems.end())
            {
                HeapElemX * elem = mit->second; delete elem; mit++;
            }

            if(vec1){delete[] vec1; vec1 = 0;}
            if(vec2){delete[] vec2; vec2 = 0;}
            if(vec3){delete[] vec3; vec3 = 0;}
            return true;
        }

        MyMarker* tmpMarker = tmp_map[lastInd];
        maxSegMarkers.push_back(tmpMarker);
        while(tmpMarker->parent != 0){
            tmpMarker = tmpMarker->parent;
            maxSegMarkers.push_back(tmpMarker);
        }

        QString s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty)
                + "_" + QString::number(rootz) + "_" + QString::number(t) + "_init.swc";
        saveSWC_file(s.toStdString(),tmptree);
    }

    etime2 = timer2.elapsed();
    cout<<" save markers cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

    */

    if(tmptree.size() <= 1){
        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]){
                delete tmptree[i];
            }
        }
        tmptree.clear();

        if(vec1){delete[] vec1; vec1 = 0;}
        if(vec2){delete[] vec2; vec2 = 0;}
        if(vec3){delete[] vec3; vec3 = 0;}
        return true;
    }

    computeLocalPcaEigVec2(tmptree,sz0,sz1,sz2,pc1,pc2,pc3,vec1,vec2,vec3);
    qDebug()<<QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
           <<" pc1 pc2 pc3: "<<pc1<<" "<<pc2<<" "<<pc3;

    if(pc3>8){
        max_int = 0;
        min_int = INT_MAX;
        for(int i=0; i<tmptree.size(); i++){
            long index = tmptree[i]->ind(sz0,sz01);
            if (inimg1d[index] > max_int) max_int = inimg1d[index];
            else if (inimg1d[index] < min_int) min_int = inimg1d[index];
        }
        max_int -= min_int;
//        if(max_int>255) max_int = 255;
        tmptree.clear();
        tmpMarkers.clear();
        fastmarching_core(root,inimg1d,tmptree,tmpMarkers,stopFlag,max_int,min_int,phi,parent,state,path,sz0,sz1,sz2,lineThres,cnn_type,length,lastDire);

        s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                + "_init1.swc";
//        saveSWC_file(s.toStdString(),tmptree);

        computeLocalPcaEigVec2(tmptree,sz0,sz1,sz2,pc1,pc2,pc3,vec1,vec2,vec3);
        qDebug()<<"after: "<<QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
               <<" pc1 pc2 pc3: "<<pc1<<" "<<pc2<<" "<<pc3;

        if(pc3>8){
            for(int i=0; i<tmptree.size(); i++){
                if(tmptree[i]){
                    delete tmptree[i];
                }
            }
            tmptree.clear();

            if(vec1){delete[] vec1; vec1 = 0;}
            if(vec2){delete[] vec2; vec2 = 0;}
            if(vec3){delete[] vec3; vec3 = 0;}
            return true;
        }
    }

    etime2 = timer2.elapsed();
    cout<<" compute eigvec cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();



    for(int i=0; i<tmptree.size(); i++){
        if(tmptree[i]){
            delete tmptree[i];
        }
    }
    tmptree.clear();

    if(stopFlag == 1){

        tmpMarkers[1]->parent = root;

        for(int i=1; i<tmpMarkers.size(); i++){
            outtree.push_back(tmpMarkers[i]);
        }

        if(vec1){delete[] vec1; vec1 = 0;}
        if(vec2){delete[] vec2; vec2 = 0;}
        if(vec3){delete[] vec3; vec3 = 0;}
        return true;
    }


    s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
            + "_p1.swc";
//    saveSWC_file(s.toStdString(),tmpMarkers);

    twoPointsToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FAR);
//    segToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FAR);
    MyMarker* tmpEndMarker;
    MyMarker* tMarker2 ;
    double tmpLength = 0;
    for(int i=tmpMarkers.size()-1; i>0; i--){
        tmpLength += dist(*(tmpMarkers[i]),*(tmpMarkers[i-1]));
        if(tmpLength>length/5.0){
            tMarker2= tmpMarkers[i];
            break;
        }
    }
    cout<<"tmpLength: "<<tmpLength<<endl;

    tmpEndMarker = tMarker2;
    while (tmpEndMarker->parent != 0) {
        tmpEndMarker = tmpEndMarker->parent;
        if(dist(*tmpEndMarker,*tMarker2) > maskR + 0.5){
            break;
        }
    }

    twoPointsToMask(state,tmpMarkers.back(),tMarker2,sz0,sz1,sz2,maskR,FINAL);

    imageName = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                + "_init2.v3draw";
//    saveMaskImage(inimg1d,state,maskImage,sz0,sz1,sz2,imageName,callback);

//    segToMask(state,tmpMarkers.back(),tMarker2,sz0,sz1,sz2,maskR,FINAL);
    XYZ tmpLastDire = XYZ(tmpEndMarker->x-tMarker2->x,tmpEndMarker->y-tMarker2->y,tmpEndMarker->z-tMarker2->z);
    vector<MyMarker*> tmptree2;
    vector<MyMarker*> tmpMarkers2;
    fastmarching_core(tmpEndMarker,inimg1d,tmptree2,tmpMarkers2,stopFlag,max_int,min_int,phi,parent,state,path,
                      sz0,sz1,sz2,lineThres,cnn_type,length*5/6.0,tmpLastDire);

    s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                +  "_init2.swc";
//    saveSWC_file(s.toStdString(),tmptree2);

    s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                +  "_p2.swc";
//    saveSWC_file(s.toStdString(),tmpMarkers2);

    twoPointsToMask(state,tmpMarkers.back(),tMarker2,sz0,sz1,sz2,maskR,FAR);

    map<long,int> markersMap;
    for(int i=0; i<tmptree2.size(); i++){
        long index = tmptree2[i]->ind(sz0,sz01);
        markersMap[index] = i;
    }
    if(markersMap.find(root_ind) == markersMap.end()){

        tmpMarkers.clear();
        tmptree.clear();
        twoPointsToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FINAL);
        fastmarching_core(root,inimg1d,tmptree,tmpMarkers,stopFlag,max_int,min_int,phi,parent,state,path,
                          sz0,sz1,sz2,lineThres,cnn_type,length*3,lastDire);

        MyMarker* tMarker3 = tmpMarkers.back();

        while (tMarker3 != tmpMarkers.front()) {
            tMarker3 = tMarker3->parent;
            if(dist(*tMarker3,*tmpMarkers.back())>maskR+0.5){
                break;
            }
        }
        twoPointsToMask(state,tMarker3,tmpMarkers.back(),sz0,sz1,sz2,maskR,FINAL);
        onePointsToMask(state,tMarker3,sz0,sz1,sz2,maskR/2,FAR);

        tmpMarkers2.clear();
        tmptree2.clear();

        tmpLastDire = XYZ(tMarker3->x-tmpMarkers.back()->x,tMarker3->y-tmpMarkers.back()->y,tMarker3->z-tmpMarkers.back()->z);
        fastmarching_core(tMarker3,inimg1d,tmptree2,tmpMarkers2,stopFlag,max_int,min_int,phi,parent,state,path,
                          sz0,sz1,sz2,lineThres,cnn_type,length*4.5,tmpLastDire);

        MyMarker* tMarker4;
        MyMarker* tMarker5;
        tmpLength = 0;
        for(int i=0; i<tmpMarkers2.size()-1; i++){
            tmpLength += dist(*tmpMarkers2[i],*tmpMarkers2[i+1]);
            if(tmpLength>length*1.5){
                tMarker4 = tmpMarkers2[i];
                break;
            }
        }

        tmpLength = 0;
        for(int i=tmpMarkers2.size()-1; i>0; i--){
            tmpLength += dist(*tmpMarkers2[i],*tmpMarkers2[i-1]);
            if(tmpLength>length*1.5){
                tMarker5 = tmpMarkers2[i];
                break;
            }
        }

        twoPointsToMask(state,tMarker4,tmpMarkers2.front(),sz0,sz1,sz2,maskR,FINAL);
        twoPointsToMask(state,tMarker5,tmpMarkers2.back(),sz0,sz1,sz2,maskR,FINAL);

        tmpMarkers.clear();
        tmptree.clear();
        twoPointsToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FINAL);
        fastmarching_core(root,inimg1d,tmptree,tmpMarkers,stopFlag,max_int,min_int,phi,parent,state,path,
                          sz0,sz1,sz2,lineThres,cnn_type,length*3,lastDire);






//        MyMarker* tMarker3 = tmpMarkers.back();
//        for(int i=0; i<tmpMarkers2.size(); i++){
//            if(dist(*tmpMarkers2[i],*tmpMarkers2.front())>maskR+0.5){
//                tMarker3 = tmpMarkers2[i];
//                break;
//            }
//        }
//        twoPointsToMask(state,tMarker3,tmpMarkers2.back(),sz0,sz1,sz2,maskR,FINAL);
//        twoPointsToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FINAL);

//        onePointsToMask(state,root,sz0,sz1,sz2,maskR,FAR);

        imageName = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                    + "_init3.v3draw";
//        saveMaskImage(inimg1d,state,maskImage,sz0,sz1,sz2,imageName,callback);

//        segToMask(state,tMarker3,tmpMarkers2.front(),sz0,sz1,sz2,maskR,FINAL);
//        segToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FINAL);

//        vector<MyMarker*> tmptree2;
//        vector<MyMarker*> tmpMarkers2;
//        tmptree.clear();
//        tmpMarkers.clear();
//        fastmarching_core(root,inimg1d,tmptree,tmpMarkers,stopFlag,max_int,min_int,phi,parent,state,path,
//                          sz0,sz1,sz2,lineThres,cnn_type,length,lastDire);

        s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                    +  "_init3.swc";
//        saveSWC_file(s.toStdString(),tmptree);

        s = "D:\\testDynamicTracing\\" + QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
                    +  "_p3.swc";
//        saveSWC_file(s.toStdString(),tmpMarkers);


        twoPointsToMask(state,tMarker4,tmpMarkers2.front(),sz0,sz1,sz2,maskR,FAR);
        twoPointsToMask(state,tMarker5,tmpMarkers2.back(),sz0,sz1,sz2,maskR,FAR);
//        twoPointsToMask(state,tMarker3,tmpMarkers2.back(),sz0,sz1,sz2,maskR,FAR);
//        twoPointsToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FAR);

//        segToMask(state,tMarker3,tmpMarkers2.front(),sz0,sz1,sz2,maskR,FAR);
//        segToMask(state,tMakrer,tForeMarker,sz0,sz1,sz2,maskR,FAR);

        computeLocalPcaEigVec2(tmptree,sz0,sz1,sz2,pc1,pc2,pc3,vec1,vec2,vec3);
        qDebug()<<"init 3: "<<QString::number(rootx) + "_" + QString::number(rooty) + "_" + QString::number(rootz)
               <<" pc1 pc2 pc3: "<<pc1<<" "<<pc2<<" "<<pc3;

//        if(pc3>5){
//            for(int i=0; i<tmptree.size(); i++){
//                if(tmptree[i]){
//                    delete tmptree[i];
//                }
//            }
//            tmptree.clear();

//            if(vec1){delete[] vec1; vec1 = 0;}
//            if(vec2){delete[] vec2; vec2 = 0;}
//            if(vec3){delete[] vec3; vec3 = 0;}
//            return true;
//        }

        for(int i=0; i<tmptree.size(); i++){
            if(tmptree[i]){
                delete tmptree[i];
            }
        }
        tmptree.clear();

        tmpEndMarker = 0;


//        qDebug()<<"find not start point";
//        if(vec1){delete[] vec1; vec1 = 0;}
//        if(vec2){delete[] vec2; vec2 = 0;}
//        if(vec3){delete[] vec3; vec3 = 0;}
//        return true;
    }

    twoPointsToMask(state,tmpMarkers.back(),tMarker2,sz0,sz1,sz2,maskR,FAR);
//    segToMask(state,tmpMarkers.back(),tMarker2,sz0,sz1,sz2,maskR,FAR);

    for(int i=0; i<tmptree2.size(); i++){
        if(tmptree2[i]){
            delete tmptree2[i];
        }
    }
    tmptree2.clear();


    tmpMarkers[1]->parent = root;

    for(int i=1; i<tmpMarkers.size(); i++){
        outtree.push_back(tmpMarkers[i]);
        if(tmpEndMarker == tmpMarkers[i]){
            break;
        }
    }

//    double lineThresTmp = 0;
//    for(int i=0; i<tmpMarkers.size(); i++){
//        long index = tmpMarkers[i]->ind(sz0,sz01);
//        lineThresTmp += inimg1d[index];
//    }

//    if(tmpMarkers.size()>0){
//        lineThresTmp /= tmpMarkers.size();
//    }
//    lineThres = (lineThres + lineThresTmp)/2;

    qDebug()<<"lineThres: "<<lineThres;

    if(stopFlag == 1){

        if(maskImage){
            delete[] maskImage;
            maskImage = 0;
        }
        if(vec1){delete[] vec1; vec1 = 0;}
        if(vec2){delete[] vec2; vec2 = 0;}
        if(vec3){delete[] vec3; vec3 = 0;}
        return true;
    }

    etime2 = timer2.elapsed();
    cout<<" add marker cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();


    if(vec1){delete[] vec1; vec1 = 0;}
    if(vec2){delete[] vec2; vec2 = 0;}
    if(vec3){delete[] vec3; vec3 = 0;}

    if(maskImage){
        delete[] maskImage;
        maskImage = 0;
    }


    etime2 = timer2.elapsed();
    cout<<" release memory cost "<<etime2<<" milliseconds"<<endl;
    timer2.restart();

//    MyMarker* leafMarker = tmpMarkers.back();
    MyMarker* leafMarker;
    if(tmpEndMarker == 0){
        leafMarker = tmpMarkers.back();
    }else{
        leafMarker = tmpEndMarker;
    }
    MyMarker* leafForeMarker = leafMarker;

    int c = 0;
    while(leafForeMarker != root){
        leafForeMarker = leafForeMarker->parent;
        c++;
        if(c>5){
            break;
        }
    }
    lastDire = XYZ(leafMarker->x-leafForeMarker->x,leafMarker->y-leafForeMarker->y,leafMarker->z-leafForeMarker->z);

    fastmarching_ultratracer2_line(leafMarker,inimg1d,root,outtree,phi,parent,state,path,
                              sz0,sz1,sz2,lineThres,callback,cnn_type,length,lastDire);

    return true;

}

#endif
