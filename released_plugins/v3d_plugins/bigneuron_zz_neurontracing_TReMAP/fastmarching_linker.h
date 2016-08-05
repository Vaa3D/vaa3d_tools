// Feb 27, 2012      Hang Xiao

#ifndef __FASTMARCHING_LINKER_H__
#define __FASTMARCHING_LINKER_H__

#include <cstdlib>
#include <cmath>  // sqrt
#include <cassert>
#include <vector>
#include <map>
#include <iostream> // cerr
#include <set>
//#include <time.h>
#include "heap.h"

using namespace std;

#define INF 3.4e+38             // float INF

#ifndef MAX_INT
#define MAX_INT  2147483647
#endif

#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

// Get the project point from one point to line (nm,fm) in 3d
#ifndef GET_POINT_TO_LINE_PROJECT
#define GET_POINT_TO_LINE_PROJECT(inmarker, outmarker, nm, fm)  \
{\
	double dx0 = fm.x - nm.x;\
	double dy0 = fm.y - nm.y;\
	double dz0 = fm.z - nm.z;\
	double dd0 = sqrt(dx0*dx0 + dy0*dy0 + dz0*dz0);\
	assert(dd0 != 0.0);\
	dx0 /= dd0; dy0 /= dd0; dz0 /= dd0;\
	double dx1 = inmarker.x - nm.x;\
	double dy1 = inmarker.y - nm.y;\
	double dz1 = inmarker.z - nm.z;\
	double dd1 = sqrt(dx1*dx1 + dy1*dy1 + dz1*dz1);\
	dx1 /= dd1; dy1 /= dd1; dz1 /= dd1;\
	double cos_theta = dx0 * dx1 + dy0 * dy1 + dz0 * dz1;\
	double len = dd1 * cos_theta;\
	outmarker.x = nm.x + dx0 * len;\
	outmarker.y = nm.y + dy0 * len;\
	outmarker.z = nm.z + dz0 * len;\
}

#endif

// convert marker in original coordinate to new coordinate
// a, b, c  the vector of new coordinate axis according to original coordinate, make sure |a| = |b| = |c| = 1
// inmarker is marker in current coordinate
// outmarker is marker in new coordinate
#ifndef GET_POSITION_IN_NEW_COORDINATE
#define GET_POSITION_IN_NEW_COORDINATE(inmarker, outmarker, orig, a, b, c) \
{\
        double dx = inmarker[0] - orig_marker[0];\
        double dy = inmarker[1] - orig_marker[1];\
        double dz = inmarker[2] - orig_marker[2];\
        double dd = sqrt(dx*dx + dy*dy + dz*dz);\
        if(dd == 0.0)\
		{\
			outmarker[0] = 0;\
			outmarker[1] = 0;\
			outmarker[2] = 0;\
		}\
        else\
		{\
			dx /= dd; dy /= dd; dz /= dd;\
			double cos_theta_x = dx * aa[0] + dy * aa[1] + dz * aa[2];\
			double cos_theta_y = dx * bb[0] + dy * bb[1] + dz * bb[2];\
			double cos_theta_z = dx * cc[0] + dy * cc[1] + dz * cc[2];\
			outmarker[0] = dd * cos_theta_x;\
			outmarker[1] = dd * cos_theta_y;\
			outmarker[2] = dd * cos_theta_z;\
		}\
}
#endif

// inmarker is in new coordinate system
// outmarker is in current coordinate system
#ifndef GET_POSITION_IN_ORIG_COORDINATE
#define GET_POSITION_IN_ORIG_COORDINATE(inmarker, outmarker, orig, a, b, c) \
{\
		outmarker[0] = orig[0] + inmarker[0] * a[0] + inmarker[1] * b[0] + inmarker[2] * c[0];\
		outmarker[1] = orig[1] + inmarker[0] * a[1] + inmarker[1] * b[1] + inmarker[2] * c[1];\
		outmarker[2] = orig[2] + inmarker[0] * a[2] + inmarker[1] * b[2] + inmarker[2] * c[2];\
}
#endif


//#define GI(ind) exp(li*pow((1-(inimg1d[ind]-min_int)/max_int),2))

#define GI(ind) givals[(int)((inimg1d[ind] - min_int)/max_int*255)]

static double givals[256] = {22026.5,   20368, 18840.3, 17432.5, 16134.8, 14938.4, 13834.9, 12816.8,
11877.4, 11010.2, 10209.4,  9469.8, 8786.47, 8154.96, 7571.17, 7031.33,
6531.99, 6069.98, 5642.39, 5246.52, 4879.94, 4540.36, 4225.71, 3934.08,
 3663.7, 3412.95, 3180.34,  2964.5, 2764.16, 2578.14, 2405.39,  2244.9,
2095.77, 1957.14, 1828.24, 1708.36, 1596.83, 1493.05, 1396.43, 1306.47,
1222.68, 1144.62, 1071.87, 1004.06, 940.819, 881.837, 826.806, 775.448,
727.504, 682.734, 640.916, 601.845, 565.329, 531.193, 499.271, 469.412,
441.474, 415.327, 390.848, 367.926, 346.454, 326.336, 307.481, 289.804,
273.227, 257.678, 243.089, 229.396, 216.541, 204.469, 193.129, 182.475,
172.461, 163.047, 154.195, 145.868, 138.033, 130.659, 123.717, 117.179,
111.022,  105.22, 99.7524, 94.5979, 89.7372, 85.1526,  80.827, 76.7447,
 72.891, 69.2522, 65.8152, 62.5681, 59.4994, 56.5987,  53.856, 51.2619,
48.8078, 46.4854, 44.2872, 42.2059, 40.2348, 38.3676, 36.5982, 34.9212,
33.3313, 31.8236, 30.3934, 29.0364, 27.7485,  26.526,  25.365, 24.2624,
23.2148, 22.2193,  21.273, 20.3733, 19.5176, 18.7037, 17.9292,  17.192,
16.4902,  15.822, 15.1855,  14.579, 14.0011, 13.4503, 12.9251, 12.4242,
11.9464, 11.4905, 11.0554, 10.6401, 10.2435, 9.86473, 9.50289, 9.15713,
8.82667, 8.51075, 8.20867, 7.91974, 7.64333, 7.37884, 7.12569, 6.88334,
6.65128, 6.42902,  6.2161, 6.01209, 5.81655, 5.62911, 5.44938, 5.27701,
5.11167, 4.95303, 4.80079, 4.65467, 4.51437, 4.37966, 4.25027, 4.12597,
4.00654, 3.89176, 3.78144, 3.67537, 3.57337, 3.47528, 3.38092, 3.29013,
3.20276, 3.11868, 3.03773,  2.9598, 2.88475, 2.81247, 2.74285, 2.67577,
2.61113, 2.54884, 2.48881, 2.43093, 2.37513, 2.32132, 2.26944, 2.21939,
2.17111, 2.12454, 2.07961, 2.03625, 1.99441, 1.95403, 1.91506, 1.87744,
1.84113, 1.80608, 1.77223, 1.73956, 1.70802, 1.67756, 1.64815, 1.61976,
1.59234, 1.56587, 1.54032, 1.51564, 1.49182, 1.46883, 1.44664, 1.42522,
1.40455,  1.3846, 1.36536,  1.3468,  1.3289, 1.31164, 1.29501, 1.27898,
1.26353, 1.24866, 1.23434, 1.22056,  1.2073, 1.19456, 1.18231, 1.17055,
1.15927, 1.14844, 1.13807, 1.12814, 1.11864, 1.10956, 1.10089, 1.09262,
1.08475, 1.07727, 1.07017, 1.06345, 1.05709, 1.05109, 1.04545, 1.04015,
1.03521,  1.0306, 1.02633, 1.02239, 1.01878,  1.0155, 1.01253, 1.00989,
1.00756, 1.00555, 1.00385, 1.00246, 1.00139, 1.00062, 1.00015,       1};


/* struct MyMarker */
/* { */
/*      MyMarker *parent; */
/*      double x, y, z, radius; */
/*      MyMarker(){x=y=z=radius=0;} */
/*      MyMarker(double _x, double _y, double _z) {x=_x;y=_y;z=_z;radius=0;} */
/*      bool operator != (MyMarker &in) */
/*      { */
/*           return x!=in.x && y!=in.y && z!=in.z; */
/*      } */
/* }; */

/*struct MyMarker
{
     double x;
     double y;
     double z;
     double radius;
     int type;
     MyMarker* parent;
     MyMarker(){x=y=z=radius=0.0; type = 3; parent=0;}
     MyMarker(double _x, double _y, double _z) {x = _x; y = _y; z = _z; radius = 0.0; type = 3; parent = 0;}
     MyMarker(const MyMarker & v){x=v.x; y=v.y; z=v.z; radius = v.radius; type = v.type; parent = v.parent;}


     bool operator<(const MyMarker & other) const{
          if(z > other.z) return false;
          if(z < other.z) return true;
          if(y > other.y) return false;
          if(y < other.y) return true;
          if(x > other.x) return false;
          if(x < other.x) return true;
          return false;
     }
     bool operator==(const MyMarker & other) const{
          return (z==other.z && y==other.y && x==other.x);
     }
     bool operator!=(const MyMarker & other) const{
          return (z!=other.z || y!=other.y || x!=other.x);
     }

	double & operator [] (const int i) {
		assert(i >= 0 && i <= 2);
		return (i==0) ? x : ((i==1) ? y : z);
	}

     long long ind(long long sz0, long long sz01)
     {
          return ((long long)(z+0.5) * sz01 + (long long)(y+0.5)*sz0 + (long long)(x+0.5));
     }
};*/


void clean_fm_marker_vector(vector<MyMarker*> &outswc)
{
    for (V3DLONG i=outswc.size()-1;i>=0;i--)
    {
         if (outswc.at(i)) {delete outswc.at(i); outswc.at(i)=0; }
        outswc.pop_back();
    }
}


#ifndef GET_LINE_MARKERS
#define GET_LINE_MARKERS(marker1, marker2, outmarkers) \
{\
    set<MyMarker> marker_set;\
    double dst = sqrt((marker1.x - marker2.x) * (marker1.x - marker2.x) + \
            (marker1.y - marker2.y) * (marker1.y - marker2.y) + \
            (marker1.z - marker2.z) * (marker1.z - marker2.z));\
    if(dst==0.0) outmarkers.push_back(marker1);\
    else\
    {\
        double tx = (marker2.x - marker1.x) / dst;\
        double ty = (marker2.y - marker1.y) / dst;\
        double tz = (marker2.z - marker1.z) / dst;\
        for(double r = 0.0; r <= dst; r++)\
        {\
            int x = marker1.x + tx * r + 0.5;\
            int y = marker1.y + ty * r + 0.5;\
            int z = marker1.z + tz * r + 0.5;     \
            marker_set.insert(MyMarker(x,y,z));\
        }\
        outmarkers.insert(outmarkers.begin(), marker_set.begin(), marker_set.end());\
    }\
}
#endif

/*double dist(MyMarker a, MyMarker b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}*/



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


// if time==0, then do not consider consuming time, time is in seconds
template<class T> bool fastmarching_linker(vector<MyMarker> &sub_markers,vector<MyMarker> & tar_markers,
     T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2) //float time,
{
        int ALIVE = -1;
        int TRIAL = 0;
        int FARST = 1;

        //clock_t t1=clock(); // start time
        V3DLONG tol_sz = sz0 * sz1 * sz2;
        V3DLONG sz01 = sz0 * sz1;

        if (tol_sz<=0) {cout << "wrong size info in fastmarching_linker"<< endl; return false;}

        //int cnn_type = 2;  // ?
        cout<<"cnn_type = "<<cnn_type<<endl;
        cout<<"fastmarching_linker";

        V3DLONG i;
        float * phi = new float[tol_sz];
        for(i = 0; i < tol_sz; i++){phi[i] = INF;}
        map<V3DLONG, MyMarker> sub_map, tar_map;
        for(i = 0; i < tar_markers.size(); i++)
        {
                V3DLONG x = tar_markers[i].x + 0.5;
                V3DLONG y = tar_markers[i].y + 0.5;
                V3DLONG z = tar_markers[i].z + 0.5;
                V3DLONG ind = z*sz01 + y*sz0 + x;
                //assert(x >= 0 && x <=sz0-1 && y >= 0 && y <=sz1-1 && z >= 0 && z <=sz2-1); //Hang indicated that this is the problem why there would be crashing, 120405.
                if (x >= 0 && x < sz0 && y >= 0 && y < sz1 && z >= 0 && z < sz2)
                    tar_map[ind] = tar_markers[i];
        }

        for(i = 0; i < sub_markers.size(); i++)
        {
                V3DLONG x = sub_markers[i].x + 0.5;
                V3DLONG y = sub_markers[i].y + 0.5;
                V3DLONG z = sub_markers[i].z + 0.5;
                V3DLONG ind = z*sz01 + y*sz0 + x;
                //assert(x >= 0 && x < sz0 && y >= 0 && y < sz1 && z >= 0 && z < sz2);
                if (x >= 0 && x <= sz0-1 && y >= 0 && y <= sz1-1 && z >= 0 && z <= sz2-1)
                    sub_map[ind] = sub_markers[i];
        }

        // GI parameter min_int, max_int, li
        double max_int = 0; // maximum intensity, used in GI
        double min_int = INF;
        for(i = 0; i < tol_sz; i++)
        {
                if(inimg1d[i] > max_int) max_int = inimg1d[i];
                if(inimg1d[i] < min_int) min_int = inimg1d[i];
        }
        max_int -= min_int;
        double li = 10;

        // initialization
        char * state = new char[tol_sz];
        for(i = 0; i < tol_sz; i++) state[i] = FARST;

        vector<V3DLONG> submarker_inds;
        for(V3DLONG s = 0; s < sub_markers.size(); s++) {
                V3DLONG ii = (V3DLONG)(sub_markers[s].x + 0.5);
                V3DLONG jj = (V3DLONG)(sub_markers[s].y + 0.5);
                V3DLONG kk = (V3DLONG)(sub_markers[s].z + 0.5);
                V3DLONG ind = kk*sz01 + jj*sz0 + ii;
                submarker_inds.push_back(ind);
                state[ind] = ALIVE;
                phi[ind] = 0.0;
        }
        cout << "totalsize=" << tol_sz <<endl;
        V3DLONG * parent = new V3DLONG[tol_sz]; for(V3DLONG ind = 0; ind < tol_sz; ind++) parent[ind] = ind;

        BasicHeap<HeapElemX> heap;
        map<V3DLONG, HeapElemX*> elems;

        // init heap
        for(V3DLONG s = 0; s < submarker_inds.size(); s++)
        {
                V3DLONG index = submarker_inds[s];
                HeapElemX *elem = new HeapElemX(index, phi[index]);
                elem->prev_ind = index;
                heap.insert(elem);
                elems[index] = elem;
        }
        // loop
        int time_counter = sub_markers.size();
        double process1 = time_counter*1000.0/tol_sz;
        V3DLONG stop_ind = -1;
        cout << "now prepare test heap";
        while(!heap.empty())
        {
                double process2 = (time_counter++)*1000.0/tol_sz;
                if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}
                // time consuming until this pos
                //if((time!=0)&&(((clock()-t1) > time*CLOCKS_PER_SEC) ))
                //{
                //     return false;
                // }

                HeapElemX* min_elem = heap.delete_min();
                elems.erase(min_elem->img_ind);

                V3DLONG min_ind = min_elem->img_ind;
                parent[min_ind] = min_elem->prev_ind;
                if(tar_map.find(min_ind) != tar_map.end()){stop_ind = min_ind; break;}

                delete min_elem;

                state[min_ind] = ALIVE;
                V3DLONG i = min_ind % sz0;
                V3DLONG j = (min_ind/sz0) % sz1;
                V3DLONG k = (min_ind/sz01) % sz2;
                V3DLONG w, h, d;

                for(V3DLONG kk = -1; kk <= 1; kk++)
                {
                        d = k+kk;
                        if(d < 0 || d >= sz2) continue;
                        for(V3DLONG jj = -1; jj <= 1; jj++)
                        {
                                h = j+jj;
                                if(h < 0 || h >= sz1) continue;
                                for(V3DLONG ii = -1; ii <= 1; ii++)
                                {
                                        w = i+ii;
                                        if(w < 0 || w >= sz0) continue;
                                        V3DLONG offset = ABS(ii) + ABS(jj) + ABS(kk);
                                        if(offset == 0 || offset > cnn_type) continue;
                                        double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
                                        V3DLONG index = d*sz01 + h*sz0 + w;

                                        if(state[index] != ALIVE)
                                        {
                                                double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;
                                                V3DLONG prev_ind = min_ind;

                                                if(state[index] == FARST)
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
        cout<<"set thres=20";
        // connect markers according to disjoint set
        {
                // add tar_marker
                V3DLONG ind = stop_ind;
                MyMarker tar_marker = tar_map[stop_ind];
                MyMarker * new_marker = new MyMarker(tar_marker.x, tar_marker.y, tar_marker.z);
                //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
                new_marker->parent = 0; //tar_marker;

                outswc.push_back(new_marker);

                MyMarker * par_marker = new_marker;
                ind = parent[ind];
                while(sub_map.find(ind) == sub_map.end())
                {
                        V3DLONG i = ind % sz0;
                        V3DLONG j = ind/sz0 % sz1;
                        V3DLONG k = ind/sz01 % sz2;
                        new_marker = new MyMarker(i,j,k);
                        new_marker->parent = par_marker;
                        //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
                        outswc.push_back(new_marker);
                        par_marker = new_marker;
                        ind = parent[ind];
                }
                // add sub_marker
                MyMarker sub_marker = sub_map[ind];
                new_marker = new MyMarker(sub_marker.x, sub_marker.y, sub_marker.z);
                new_marker->parent = par_marker;
                //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
                outswc.push_back(new_marker);
        }
        //reverse(outswc.begin(), outswc.end());

        cout<<outswc.size()<<" markers linked"<<endl;
        //for(int i = 0; i < sub_markers.size(); i++) outswc.push_back(sub_markers[i]);
        //for(int i = 0; i < tar_markers.size(); i++) outswc.push_back(tar_markers[i]);


        if(!elems.empty()) for(map<V3DLONG, HeapElemX*>::iterator it = elems.begin(); it != elems.end(); it++) delete it->second;
        if(phi) {delete [] phi; phi = 0;}
        if(parent) {delete [] parent; parent = 0;}
        if(state) {delete [] state; state = 0;}
        return true;
}


/******************************************************************************
 * Fast marching based curve drawing 1 , will draw a line between a bounch of rays
 *
 * Input :  nm1,fm1,nm2, fm2   near_marker1, far_marker1, near_marker2, far_marker2
 *          inimg1d         original input image
 *
 * Output : outswc          the output stroke
 *
 * Algorithm :
 * 1. find a shortest path from first to last ray
 * 2. use bounding box between (nm1, fm1) and (nm2, fm2)
 * *****************************************************************************/

template<class T> bool fastmarching_linker(MyMarker nm1, MyMarker fm1, MyMarker nm2, MyMarker fm2, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{

#define COS_THETA_UNIT(A, B) (A[0] * B[0] + A[1] * B[1] + A[2] * B[2])
#define MAKE_UNIT(V) {double len = sqrt(V[0] * V[0] + V[1] * V[1] + V[2] * V[2]); V[0] /= len; V[1] /= len; V[2] /= len;}

	long sz01 = sz0 * sz1;

	// 1. calc rt
	double rt[3] = {0.0, 0.0, 0.0};
	if(nm1 != fm1)
	{
		double tx1 = fm1.x - nm1.x;
		double ty1 = fm1.y - nm1.y;
		double tz1 = fm1.z - nm1.z;
		double dst1 = sqrt(tx1 * tx1 + ty1 * ty1 + tz1 * tz1);
		rt[0] = tx1 / dst1;
		rt[1] = ty1 / dst1;
		rt[2] = tz1 / dst1;
	}
	else if(nm2 != fm2)
	{
		double tx2 = fm2.x - nm2.x;
		double ty2 = fm2.y - nm2.y;
		double tz2 = fm2.z - nm2.z;
		double dst2 = sqrt(tx2 * tx2 + ty2 * ty2 + tz2 * tz2);
		rt[0] = tx2 / dst2;
		rt[1] = ty2 / dst2;
		rt[2] = tz2 / dst2;
	}
	else
	{
		cerr<<"Error : nm1 == nm2 && fm1 == fm2"<<endl;
		return false;
	}
	// 2. calc different vectors
	double n1n2[3] = {nm2.x - nm1.x, nm2.y - nm1.y, nm2.z - nm1.z};
	MAKE_UNIT(n1n2);
	double n2n1[3] = {-n1n2[0], -n1n2[1], -n1n2[2]};

	double f1f2[3] = {fm2.x - fm1.x, fm2.y - fm1.y, fm2.z - fm1.z};
	MAKE_UNIT(f1f2);
	double f2f1[3] = {-f1f2[0], -f1f2[1], -f1f2[2]};

	double n1f1[3] = {rt[0], rt[1], rt[2]};
	double f1n1[3] = {-rt[0], -rt[1], -rt[2]};

	double n2f2[3] = {rt[0], rt[1], rt[2]};
	double f2n2[3] = {-rt[0], -rt[1], -rt[2]};

	int margin = 5;

	// 1. get initial rectangel
	MyMarker rect[4] = {nm1, nm2, fm2, fm1};
	double cos_n1, cos_n2, cos_f1, cos_f2;
	if((cos_n1 = COS_THETA_UNIT(n1f1, n1n2)) < 0.0)
	{
		double d = dist(nm1, nm2) * (-cos_n1);
		rect[0] = MyMarker(nm1.x - d * rt[0], nm1.y - d * rt[1], nm1.z - d * rt[2]);
		cout<<"cos_n1 = "<<cos_n1<<endl;
	}
	if((cos_n2 = COS_THETA_UNIT(n2f2, n2n1)) < 0.0)
	{
		double d = dist(nm1, nm2) * (-cos_n2);
		rect[1] = MyMarker(nm2.x - d * rt[0], nm2.y - d * rt[1], nm2.z - d * rt[2]);
		cout<<"cos_n2 = "<<cos_n2<<endl;
	}
	if((cos_f2 = COS_THETA_UNIT(f2n2, f2f1)) < 0.0)
	{
		double d = dist(fm1, fm2) * (-cos_f2);
		rect[2] = MyMarker(fm2.x + d * rt[0], fm2.y + d * rt[1], fm2.z + d * rt[2]);
		cout<<"cos_f2 = "<<cos_f2<<endl;
	}
	if((cos_f1 = COS_THETA_UNIT(f1n1, f1f2)) < 0.0)
	{
		double d = dist(fm1, fm2) * (-cos_f1);
		rect[3] = MyMarker(fm1.x + d * rt[0], fm1.y + d * rt[1], fm1.z + d * rt[2]);
		cout<<"cos_f1 = "<<cos_f1<<endl;
	}

	// 2. add margin
	double a[3];
	a[0] = rect[3].x - rect[0].x;
	a[1] = rect[3].y - rect[0].y;
	a[2] = rect[3].z - rect[0].z;
	double la = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] /= la; a[1] /= la; a[2] /= la;

	double b[3];
	b[0] = rect[1].x - rect[0].x;
	b[1] = rect[1].y - rect[0].y;
	b[2] = rect[1].z - rect[0].z;
	double lb = sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
	b[0] /= lb; b[1] /= lb; b[2] /= lb;

	double c[3];
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
	double lc = sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
	c[0] /= lc; c[1] /= lc; c[2] /= lc;

	MyMarker o;
	o.x = rect[0].x - margin * a[0] - margin * b[0] - margin * c[0];
	o.y = rect[0].y - margin * a[1] - margin * b[1] - margin * c[1];
	o.z = rect[0].z - margin * a[2] - margin * b[2] - margin * c[2];

	long bsz0 = dist(rect[0], rect[3]) + 1 + 2 * margin + 0.5;
	long bsz1 = dist(rect[0], rect[1]) + 1 + 2 * margin + 0.5;
	long bsz2 = 1 + 2 * margin + 0.5;
	long bsz01 = bsz0 * bsz1;
	long btol_sz = bsz01 * bsz2;
	unsigned char * outimg1d = new unsigned char[btol_sz]; for(long i = 0; i < btol_sz; i++) outimg1d[i] = 0;
	for(long k = 0; k < bsz2; k++)
	{
		for(long j = 0; j < bsz1; j++)
		{
			for(long i = 0; i < bsz0; i++)
			{
				long ii = o.x + i * a[0] + j * b[0] + k * c[0] + 0.5;
				long jj = o.y + i * a[1] + j * b[1] + k * c[1] + 0.5;
				long kk = o.z + i * a[2] + j * b[2] + k * c[2] + 0.5;
				if(ii >= 0 && ii < sz0 && jj >= 0 && jj < sz1 && kk >= 0 && kk < sz2)
				{
					long ind1 = k * bsz01 + j * bsz0 + i;
					long ind2 = kk * sz01 + jj * sz0 + ii;
					outimg1d[ind1] = inimg1d[ind2];
				}
			}
		}
	}

	// 3. get sub_markers and tar_markers
	MyMarker new_nm1(margin + dist(nm1, rect[0]), margin, margin);
	MyMarker new_fm1(margin + dist(fm1, rect[0]), margin, margin);
	MyMarker new_nm2(margin + dist(nm2, rect[1]), margin + dist(rect[0], rect[1]), margin);
	MyMarker new_fm2(margin + dist(fm2, rect[1]), margin + dist(rect[0], rect[1]), margin);
	vector<MyMarker> sub_markers, tar_markers;
	GET_LINE_MARKERS(new_nm1, new_fm1, sub_markers);
	GET_LINE_MARKERS(new_nm2, new_fm2, tar_markers);
	fastmarching_linker(sub_markers, tar_markers, outimg1d, outswc, bsz0, bsz1, bsz2, cnn_type);
	for(int i = 0; i < outswc.size(); i++)
	{
		double x = o.x + outswc[i]->x * a[0] + outswc[i]->y * b[0] + outswc[i]->z * c[0];
		double y = o.y + outswc[i]->x * a[1] + outswc[i]->y * b[1] + outswc[i]->z * c[1];
		double z = o.z + outswc[i]->x * a[2] + outswc[i]->y * b[2] + outswc[i]->z * c[2];
		outswc[i]->x = x;
		outswc[i]->y = y;
		outswc[i]->z = z;
	}
	if(outimg1d){delete [] outimg1d; outimg1d = 0;}
	return true;
}

// draw without bounding box
template<class T> bool fastmarching_drawing1(vector<MyMarker> & near_markers, vector<MyMarker> &far_markers, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	assert(near_markers.size() == far_markers.size() && near_markers.size() >= 2);

	MyMarker near_marker1 = near_markers[0];
	MyMarker far_marker1 = far_markers[0];
	MyMarker near_marker2 = near_markers[1];
	MyMarker far_marker2 = far_markers[1];
	vector<MyMarker> sub_markers, tar_markers;
	GET_LINE_MARKERS(near_marker1, far_marker1, sub_markers);
	GET_LINE_MARKERS(near_marker2, far_marker2, tar_markers);
	vector<MyMarker*> tmp_outswc;

	cout<<"fm-linker between ray 0 and ray 1"<<endl;
	fastmarching_linker(sub_markers, tar_markers, inimg1d, tmp_outswc, sz0, sz1, sz2, cnn_type);
	//reverse(tmp_outswc.begin(), tmp_outswc.end()); // put the tar_marker to the last
	outswc.insert(outswc.end(), tmp_outswc.begin(), tmp_outswc.end());
	sub_markers.clear();
	sub_markers.push_back(*(*(tmp_outswc.rbegin())));
	tar_markers.clear();
	tmp_outswc.clear();

	for(int i = 2; i < near_markers.size(); i++)
	{
		near_marker2 = near_markers[i];
		far_marker2 = far_markers[i];
		GET_LINE_MARKERS(near_marker2, far_marker2, tar_markers);
		cout<<"fm-linker between ray "<<i-1<<" and ray "<<i<<endl;
		fastmarching_linker(sub_markers, tar_markers, inimg1d, tmp_outswc, sz0, sz1, sz2, cnn_type);
		//reverse(tmp_outswc.begin(), tmp_outswc.end()); // put the tar_marker to the last
		(*outswc.rbegin())->parent = (*tmp_outswc.begin());
		outswc.insert(outswc.end(), tmp_outswc.begin(), tmp_outswc.end());

		sub_markers.clear();
		sub_markers.push_back(*(*(tmp_outswc.rbegin())));
		tar_markers.clear();
		tmp_outswc.clear();
	}
	//smooth_curve(outswc, 4);
	return true;
}

// draw with bounding box
template<class T> bool fastmarching_drawing2(vector<MyMarker> & near_markers, vector<MyMarker> &far_markers, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	assert(near_markers.size() == far_markers.size() && near_markers.size() >= 2);

	MyMarker nm1 = near_markers[0];
	MyMarker fm1 = far_markers[0];
	MyMarker nm2 = near_markers[1];
	MyMarker fm2 = far_markers[1];
	vector<MyMarker*> tmp_outswc;

	cout<<"fm-linker between ray 0 and ray 1"<<endl;
	fastmarching_linker(nm1, fm1, nm2, fm2, inimg1d, tmp_outswc, sz0, sz1, sz2, cnn_type);
	outswc.insert(outswc.end(), tmp_outswc.begin(), tmp_outswc.end());
	nm1 = *(*(tmp_outswc.rbegin()));
	fm1 = nm1;
	tmp_outswc.clear();

	for(int i = 2; i < near_markers.size(); i++)
	{
		nm2 = near_markers[i];
		fm2 = far_markers[i];
		cout<<"fm-linker between ray "<<i-1<<" and ray "<<i<<endl;
		fastmarching_linker(nm1, fm1, nm2, fm2, inimg1d, tmp_outswc, sz0, sz1, sz2, cnn_type);
		(*outswc.rbegin())->parent = (*tmp_outswc.begin());
		outswc.insert(outswc.end(), tmp_outswc.begin(), tmp_outswc.end());

		nm1 = *(*(tmp_outswc.rbegin()));
		fm1 = nm1;
		tmp_outswc.clear();
	}
     //	smooth_curve(outswc, 4);
	return true;
}

/******************************************************************************
 * Fast marching based linker, will give out the nearest two markers in two different sets
 *
 * Input :  sub_markers     the source markers with initial phi values
 *          tar_markers     the target markers, will store final phi values and the parent will the changed
 *          inimg1d         original input image
 *
 * Output : par_tree        the parental tree from tar_markers to sub_markers
 *
 * Notice :
 * min_int is set to 0
 * max_int is set to 255 always
 * markers in tar_markers and sub_markers are not included
 * *****************************************************************************/
template<class T> bool fastmarching_linker(map<MyMarker*, double> & sub_markers, map<MyMarker*, double> & tar_markers, T * inimg1d, vector<MyMarker*> & par_tree, int sz0, int sz1, int sz2, int stop_num, int cnn_type = 2)
{
     stop_num = MIN(stop_num, tar_markers.size());
	enum{ALIVE = -1, TRIAL = 0, FAR_ = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
	cout<<"cnn_type = "<<cnn_type<<endl;

	float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
	map<long, MyMarker*> sub_map, tar_map;
	for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++)
	{
		MyMarker * tar_marker = it->first;
		int x = tar_marker->x + 0.5;
		int y = tar_marker->y + 0.5;
		int z = tar_marker->z + 0.5;
		long ind = z*sz01 + y*sz0 + x;
		tar_map[ind] = tar_marker;
	}

	for(map<MyMarker*, double>::iterator it = sub_markers.begin(); it != sub_markers.end(); it++)
	{
		MyMarker * sub_marker = it->first;
		int x = sub_marker->x + 0.5;
		int y = sub_marker->y + 0.5;
		int z = sub_marker->z + 0.5;
		long ind = z*sz01 + y*sz0 + x;
		sub_map[ind] = sub_marker;
	}

	// GI parameter min_int, max_int, li
	/*double max_int = 0; // maximum intensity, used in GI
	double min_int = INF;
	for(long i = 0; i < tol_sz; i++)
	{
		if(inimg1d[i] > max_int) max_int = inimg1d[i];
		if(inimg1d[i] < min_int) min_int = inimg1d[i];
	}
	max_int -= min_int;
	*/
	double min_int = 0;
	double max_int = 255;
	double li = 10;

	// initialization
	char * state = new char[tol_sz];
	for(long i = 0; i < tol_sz; i++) state[i] = FAR_;

	vector<long> submarker_inds;
	for(map<MyMarker*, double>::iterator it = sub_markers.begin(); it != sub_markers.end(); it++)
	{
		MyMarker * sub_marker = it->first;
		int i = sub_marker->x + 0.5;
		int j = sub_marker->y + 0.5;
		int k = sub_marker->z + 0.5;
		if(i < 0 || i >= sz0 || j < 0 || j >= sz1 || k < 0 || k >= sz2) continue;
		long ind = k*sz01 + j*sz0 + i;
		submarker_inds.push_back(ind);
		state[ind] = ALIVE;
		phi[ind] = sub_markers[sub_marker];
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
	double process1 = time_counter*1000.0/tol_sz;
	vector<long> marched_inds;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*1000.0/tol_sz;
		if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		parent[min_ind] = min_elem->prev_ind;
		if(tar_map.find(min_ind) != tar_map.end())
		{
			marched_inds.push_back(min_ind);
			if(marched_inds.size() > stop_num) break;
		}

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

						if(state[index] == FAR_)
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


     // refresh the score values and parent info in tar_markers
     for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++)
     {
          MyMarker * tar_marker = it->first;
          int x = tar_marker->x + 0.5;
          int y = tar_marker->y + 0.5;
          int z = tar_marker->z + 0.5;
          long ind = z*sz01 + y*sz0 + x;
          if(state[ind] != ALIVE || tar_marker != tar_map[ind])
          {
               tar_marker->parent = 0;
               it->second = INF;
               continue;
          }
          it->second = phi[ind];

          // find the path from tar_marker
          // tar_marker is not added
          MyMarker * child_marker = tar_marker;

          ind = parent[ind];
          while(sub_map.find(ind) == sub_map.end())
          {
               int i = ind % sz0;
               int j = ind/sz0 % sz1;
               int k = ind/sz01 % sz2;
               MyMarker * new_marker = new MyMarker(i,j,k);
               child_marker->parent = new_marker;
               new_marker->parent = 0;
               new_marker->radius = 5;//markerRadius(inimg1d, in_sz, *new_marker, thresh);
               par_tree.push_back(new_marker);
               child_marker = new_marker;
               ind = parent[ind];
          }
          MyMarker * sub_marker = sub_map[ind];
          child_marker->parent = sub_marker;
          // sub_marker is not added
          //par_tree.push_back(sub_marker);
     }

     cout<<par_tree.size()<<" markers in par_tree"<<endl;


	if(phi) {delete [] phi; phi = 0;}
	if(parent) {delete [] parent; parent = 0;}
	if(state) {delete [] state; state = 0;}
	return true;
}

// marching with bounding box
// Please make sure
// 1. sub_markers are located between nm1 and fm1
//
template<class T> bool fastmarching_linker(map<MyMarker*, double> & sub_markers, map<MyMarker*, double> & tar_markers, T * inimg1d, vector<MyMarker*> & par_tree, int sz0, int sz1, int sz2, MyMarker nm1, MyMarker fm1, MyMarker nm2, MyMarker fm2, int stop_num, int cnn_type = 2, int margin = 5)
{
	assert(par_tree.empty());
	long sz01 = sz0 * sz1;

	// 1. calc rt
	double rt[3] = {0.0, 0.0, 0.0};
	if(nm1 != fm1)
	{
		double tx1 = fm1.x - nm1.x;
		double ty1 = fm1.y - nm1.y;
		double tz1 = fm1.z - nm1.z;
		double dst1 = sqrt(tx1 * tx1 + ty1 * ty1 + tz1 * tz1);
		rt[0] = tx1 / dst1;
		rt[1] = ty1 / dst1;
		rt[2] = tz1 / dst1;
	}
	else if(nm2 != fm2)
	{
		double tx2 = fm2.x - nm2.x;
		double ty2 = fm2.y - nm2.y;
		double tz2 = fm2.z - nm2.z;
		double dst2 = sqrt(tx2 * tx2 + ty2 * ty2 + tz2 * tz2);
		rt[0] = tx2 / dst2;
		rt[1] = ty2 / dst2;
		rt[2] = tz2 / dst2;
	}
	else
	{
		cerr<<"Error : nm1 == nm2 && fm1 == fm2"<<endl;
		return false;
	}
	// 2. calc different vectors
	double n1n2[3] = {nm2.x - nm1.x, nm2.y - nm1.y, nm2.z - nm1.z};
	MAKE_UNIT(n1n2);
	double n2n1[3] = {-n1n2[0], -n1n2[1], -n1n2[2]};

	double f1f2[3] = {fm2.x - fm1.x, fm2.y - fm1.y, fm2.z - fm1.z};
	MAKE_UNIT(f1f2);
	double f2f1[3] = {-f1f2[0], -f1f2[1], -f1f2[2]};

	double n1f1[3] = {rt[0], rt[1], rt[2]};
	double f1n1[3] = {-rt[0], -rt[1], -rt[2]};

	double n2f2[3] = {rt[0], rt[1], rt[2]};
	double f2n2[3] = {-rt[0], -rt[1], -rt[2]};

	//int margin = 5;

	// 1. get initial rectangel
	MyMarker rect[4] = {nm1, nm2, fm2, fm1};
	double cos_n1, cos_n2, cos_f1, cos_f2;
	if((cos_n1 = COS_THETA_UNIT(n1f1, n1n2)) < 0.0)
	{
		double d = dist(nm1, nm2) * (-cos_n1);
		rect[0] = MyMarker(nm1.x - d * rt[0], nm1.y - d * rt[1], nm1.z - d * rt[2]);
		cout<<"cos_n1 = "<<cos_n1<<endl;
	}
	if((cos_n2 = COS_THETA_UNIT(n2f2, n2n1)) < 0.0)
	{
		double d = dist(nm1, nm2) * (-cos_n2);
		rect[1] = MyMarker(nm2.x - d * rt[0], nm2.y - d * rt[1], nm2.z - d * rt[2]);
		cout<<"cos_n2 = "<<cos_n2<<endl;
	}
	if((cos_f2 = COS_THETA_UNIT(f2n2, f2f1)) < 0.0)
	{
		double d = dist(fm1, fm2) * (-cos_f2);
		rect[2] = MyMarker(fm2.x + d * rt[0], fm2.y + d * rt[1], fm2.z + d * rt[2]);
		cout<<"cos_f2 = "<<cos_f2<<endl;
	}
	if((cos_f1 = COS_THETA_UNIT(f1n1, f1f2)) < 0.0)
	{
		double d = dist(fm1, fm2) * (-cos_f1);
		rect[3] = MyMarker(fm1.x + d * rt[0], fm1.y + d * rt[1], fm1.z + d * rt[2]);
		cout<<"cos_f1 = "<<cos_f1<<endl;
	}

	// 2. add margin
	double a[3];
	a[0] = rect[3].x - rect[0].x;
	a[1] = rect[3].y - rect[0].y;
	a[2] = rect[3].z - rect[0].z;
	double la = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] /= la; a[1] /= la; a[2] /= la;

	double b[3];
	b[0] = rect[1].x - rect[0].x;
	b[1] = rect[1].y - rect[0].y;
	b[2] = rect[1].z - rect[0].z;
	double lb = sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
	b[0] /= lb; b[1] /= lb; b[2] /= lb;

	double c[3];
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
	double lc = sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
	c[0] /= lc; c[1] /= lc; c[2] /= lc;

	MyMarker o;
	o.x = rect[0].x - margin * a[0] - margin * b[0] - margin * c[0];
	o.y = rect[0].y - margin * a[1] - margin * b[1] - margin * c[1];
	o.z = rect[0].z - margin * a[2] - margin * b[2] - margin * c[2];

	long bsz0 = dist(rect[0], rect[3]) + 1 + 2 * margin + 0.5;
	long bsz1 = dist(rect[0], rect[1]) + 1 + 2 * margin + 0.5;
	long bsz2 = 1 + 2 * margin + 0.5;
	long bsz01 = bsz0 * bsz1;
	long btol_sz = bsz01 * bsz2;
	unsigned char * outimg1d = new unsigned char[btol_sz]; for(long i = 0; i < btol_sz; i++) outimg1d[i] = 0;
	for(long k = 0; k < bsz2; k++)
	{
		for(long j = 0; j < bsz1; j++)
		{
			for(long i = 0; i < bsz0; i++)
			{
				long ii = o.x + i * a[0] + j * b[0] + k * c[0] + 0.5;
				long jj = o.y + i * a[1] + j * b[1] + k * c[1] + 0.5;
				long kk = o.z + i * a[2] + j * b[2] + k * c[2] + 0.5;
				if(ii >= 0 && ii < sz0 && jj >= 0 && jj < sz1 && kk >= 0 && kk < sz2)
				{
					long ind1 = k * bsz01 + j * bsz0 + i;
					long ind2 = kk * sz01 + jj * sz0 + ii;
					outimg1d[ind1] = inimg1d[ind2];
				}
			}
		}
	}

	// 3. get new_sub_markers and new_tar_markers
	for(map<MyMarker*, double>::iterator it = sub_markers.begin(); it != sub_markers.end(); it++)
	{
		MyMarker * sub_marker = it->first;
		sub_marker->x = margin + dist(*sub_marker, rect[0]);
		sub_marker->y = margin;
		sub_marker->z = margin;
	}
	double dst_rect01 = dist(rect[0], rect[1]);
	for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++)
	{
		MyMarker * tar_marker = it->first;
		tar_marker->x = margin + dist(*tar_marker, rect[1]);
		tar_marker->y = margin + dst_rect01;
		tar_marker->z = margin;
	}
	fastmarching_linker(sub_markers, tar_markers, outimg1d, par_tree, bsz0, bsz1, bsz2, stop_num, cnn_type);
	for(map<MyMarker*, double>::iterator it = sub_markers.begin(); it != sub_markers.end(); it++)
	{
		MyMarker * sub_marker = it->first;
		double x = o.x + sub_marker->x * a[0] + sub_marker->y * b[0] + sub_marker->z * c[0];
		double y = o.y + sub_marker->x * a[1] + sub_marker->y * b[1] + sub_marker->z * c[1];
		double z = o.z + sub_marker->x * a[2] + sub_marker->y * b[2] + sub_marker->z * c[2];
		sub_marker->x = x;
		sub_marker->y = y;
		sub_marker->z = z;
	}
	for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++)
	{
		MyMarker * tar_marker = it->first;
		double x = o.x + tar_marker->x * a[0] + tar_marker->y * b[0] + tar_marker->z * c[0];
		double y = o.y + tar_marker->x * a[1] + tar_marker->y * b[1] + tar_marker->z * c[1];
		double z = o.z + tar_marker->x * a[2] + tar_marker->y * b[2] + tar_marker->z * c[2];
		tar_marker->x = x;
		tar_marker->y = y;
		tar_marker->z = z;
	}
	for(vector<MyMarker*>::iterator it = par_tree.begin(); it != par_tree.end(); it++)
	{
		MyMarker * marker = *it;
		double x = o.x + marker->x * a[0] + marker->y * b[0] + marker->z * c[0];
		double y = o.y + marker->x * a[1] + marker->y * b[1] + marker->z * c[1];
		double z = o.z + marker->x * a[2] + marker->y * b[2] + marker->z * c[2];
		marker->x = x;
		marker->y = y;
		marker->z = z;
	}
	if(outimg1d){delete [] outimg1d; outimg1d = 0;}
	return true;

}



#ifndef GET_LINE_MARKER_MAP
#define GET_LINE_MARKER_MAP(marker1, marker2, marker_map) \
{\
    double dst = sqrt((marker1.x - marker2.x) * (marker1.x - marker2.x) + \
            (marker1.y - marker2.y) * (marker1.y - marker2.y) + \
            (marker1.z - marker2.z) * (marker1.z - marker2.z));\
    set<MyMarker> unique_markers;\
    if(dst==0.0) \
    {\
        MyMarker * marker = new MyMarker(marker1);\
        marker_map.insert(pair<MyMarker*, float>(marker, INF));\
    }\
    else\
    {\
        double tx = (marker2.x - marker1.x) / dst;\
        double ty = (marker2.y - marker1.y) / dst;\
        double tz = (marker2.z - marker1.z) / dst;\
        for(double r = 0.0; r < dst+1; r++) \
        {\
            int x = marker1.x + tx * r + 0.5;\
            int y = marker1.y + ty * r + 0.5;\
            int z = marker1.z + tz * r + 0.5;\
            if(unique_markers.find(MyMarker(x,y,z)) == unique_markers.end())\
            {\
                MyMarker * marker = new MyMarker(x,y,z);\
                marker_map.insert(pair<MyMarker*, float>(marker, INF));\
                unique_markers.insert(*marker);\
            }\
        }\
    }\
    unique_markers.clear();\
}

#endif



template<class T> bool fastmarching_drawing_dynamic(vector<MyMarker> & near_markers, vector<MyMarker> &far_markers, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2, int margin = 5)
{
     long sz01 = (long)sz0*sz1;
	cout<<"welcome to fastmarching_drawing_dynamicly"<<endl;
	assert(near_markers.size() == far_markers.size() && near_markers.size() >= 2);

	MyMarker near_marker1;// = near_markers[0];
	MyMarker far_marker1;// = far_markers[0];
	MyMarker near_marker2 = near_markers[0];
	MyMarker far_marker2 = far_markers[0];

     set<MyMarker*> start_ray;
	map<MyMarker*, double> sub_markers, tar_markers;
     //	GET_LINE_MARKER_MAP(near_marker1, far_marker1, sub_markers);
	GET_LINE_MARKER_MAP(near_marker2, far_marker2, tar_markers);
	for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++)
     {
          it->second = 0.0;
          start_ray.insert(it->first);
     }
     //	cout<<"fm-linker between ray 0 and ray 1"<<endl;
	vector<MyMarker*> all_markers, par_tree;
	//fastmarching_linker(sub_markers, tar_markers, inimg1d, par_tree, sz0, sz1, sz2, near_marker1, far_marker1, near_marker2, far_marker2, cnn_type);
	//all_markers.insert(all_markers.end(), par_tree.begin(), par_tree.end()); par_tree.clear();
	//for(map<MyMarker*, double>::iterator it = sub_markers.begin(); it != sub_markers.end(); it++) all_markers.push_back(it->first);
	for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++) all_markers.push_back(it->first);

	for(int i = 1; i < near_markers.size(); i++)
	{
          if(near_markers[i].ind(sz0, sz01) == near_marker2.ind(sz0, sz01) || far_markers[i].ind(sz0, sz01) == far_marker2.ind(sz0,sz01)) // omit ray i
          {
               cout<<"ray "<<i<<" is duplicated "<<endl;
               continue;
          }
          else
          {
               cout<<"fm-linker between ray "<<i-1<<" and ray "<<i<<endl;
          }

		near_marker1 = near_marker2;    far_marker1 = far_marker2;
		near_marker2 = near_markers[i]; far_marker2 = far_markers[i];

		sub_markers.clear(); sub_markers = tar_markers;
		tar_markers.clear(); GET_LINE_MARKER_MAP(near_marker2, far_marker2, tar_markers);
          int stop_num = (i == (near_markers.size()-1))? 1 : (tar_markers.size()+1)/2;
		fastmarching_linker(sub_markers, tar_markers, inimg1d, par_tree, sz0, sz1, sz2, near_marker1, far_marker1, near_marker2, far_marker2, stop_num, cnn_type, margin);
		all_markers.insert(all_markers.end(), par_tree.begin(), par_tree.end()); par_tree.clear();
		for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++) all_markers.push_back(it->first);
	}

	// extract the best trajectory
	double min_score = 0;
    	MyMarker * min_marker = 0;
	for(map<MyMarker*, double>::iterator it = tar_markers.begin(); it != tar_markers.end(); it++)
	{
		MyMarker * marker = it->first;
		double score = it->second;
		if(min_marker == 0 || score < min_score)
		{
			min_score = score;
			min_marker = marker;
		}
	}
	MyMarker * p = min_marker;
	MyMarker * new_marker = new MyMarker(p->x, p->y, p->z); outswc.push_back(new_marker);
	MyMarker * child_marker = new_marker;
	p = p->parent;
	while(p)
	{
		new_marker = new MyMarker(p->x, p->y, p->z); outswc.push_back(new_marker);
		child_marker->parent = new_marker;
		child_marker = new_marker;
		if(p->parent == 0) break;
          p = p->parent;
	}


     bool ret = (start_ray.find(p) != start_ray.end());

     // reverse and smooth
    // reverse(outswc.begin(), outswc.end());
     //smooth_curve(outswc, 4);

     for(int i = 0; i < all_markers.size(); i++) delete all_markers[i];
	all_markers.clear();

     return ret;
}


// calculate the bounding box containing all near_markers and far_markers and margin
// then do fastmarching from the first ray to last ray
template<class T> bool fastmarching_drawing_serialbboxes(vector<MyMarker> & near_markers, vector<MyMarker> &far_markers, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2, int margin = 5)
{
	cout<<"welcome to fastmarching_drawing4"<<endl;
	assert(near_markers.size() == far_markers.size());

	MyMarker nm1, nm2, fm1, fm2;
	nm2 = near_markers[0];
	fm2 = far_markers[0];
    V3DLONG sz01 = (V3DLONG)sz0 * sz1;

	vector<MyMarker> all_mask;
    for(V3DLONG m = 1; m < near_markers.size(); m++)
	{
		nm1 = nm2; fm1 = fm2;
		nm2 = near_markers[m]; fm2 = far_markers[m];

		// 1. calc rt
		double rt[3] = {0.0, 0.0, 0.0};
		if(nm1 != fm1)
		{
			double tx1 = fm1.x - nm1.x;
			double ty1 = fm1.y - nm1.y;
			double tz1 = fm1.z - nm1.z;
			double dst1 = sqrt(tx1 * tx1 + ty1 * ty1 + tz1 * tz1);
			rt[0] = tx1 / dst1;
			rt[1] = ty1 / dst1;
			rt[2] = tz1 / dst1;
		}
		else if(nm2 != fm2)
		{
			double tx2 = fm2.x - nm2.x;
			double ty2 = fm2.y - nm2.y;
			double tz2 = fm2.z - nm2.z;
			double dst2 = sqrt(tx2 * tx2 + ty2 * ty2 + tz2 * tz2);
			rt[0] = tx2 / dst2;
			rt[1] = ty2 / dst2;
			rt[2] = tz2 / dst2;
		}
		else
		{
			cerr<<"Error : nm1 == nm2 && fm1 == fm2"<<endl;
			return false;
		}
		// 2. calc different vectors
		double n1n2[3] = {nm2.x - nm1.x, nm2.y - nm1.y, nm2.z - nm1.z};
		MAKE_UNIT(n1n2);
		double n2n1[3] = {-n1n2[0], -n1n2[1], -n1n2[2]};

		double f1f2[3] = {fm2.x - fm1.x, fm2.y - fm1.y, fm2.z - fm1.z};
		MAKE_UNIT(f1f2);
		double f2f1[3] = {-f1f2[0], -f1f2[1], -f1f2[2]};

		double n1f1[3] = {rt[0], rt[1], rt[2]};
		double f1n1[3] = {-rt[0], -rt[1], -rt[2]};

		double n2f2[3] = {rt[0], rt[1], rt[2]};
		double f2n2[3] = {-rt[0], -rt[1], -rt[2]};

		int margin = 5;

		// 1. get initial rectangel
		MyMarker rect[4] = {nm1, nm2, fm2, fm1};
		double cos_n1, cos_n2, cos_f1, cos_f2;
		if((cos_n1 = COS_THETA_UNIT(n1f1, n1n2)) < 0.0)
		{
			double d = dist(nm1, nm2) * (-cos_n1);
			rect[0] = MyMarker(nm1.x - d * rt[0], nm1.y - d * rt[1], nm1.z - d * rt[2]);
		}
		if((cos_n2 = COS_THETA_UNIT(n2f2, n2n1)) < 0.0)
		{
			double d = dist(nm1, nm2) * (-cos_n2);
			rect[1] = MyMarker(nm2.x - d * rt[0], nm2.y - d * rt[1], nm2.z - d * rt[2]);
		}
		if((cos_f2 = COS_THETA_UNIT(f2n2, f2f1)) < 0.0)
		{
			double d = dist(fm1, fm2) * (-cos_f2);
			rect[2] = MyMarker(fm2.x + d * rt[0], fm2.y + d * rt[1], fm2.z + d * rt[2]);
		}
		if((cos_f1 = COS_THETA_UNIT(f1n1, f1f2)) < 0.0)
		{
			double d = dist(fm1, fm2) * (-cos_f1);
			rect[3] = MyMarker(fm1.x + d * rt[0], fm1.y + d * rt[1], fm1.z + d * rt[2]);
		}

		// 2. add margin
		double a[3];
		a[0] = rect[3].x - rect[0].x;
		a[1] = rect[3].y - rect[0].y;
		a[2] = rect[3].z - rect[0].z;
		double la = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
		a[0] /= la; a[1] /= la; a[2] /= la;

		double b[3];
		b[0] = rect[1].x - rect[0].x;
		b[1] = rect[1].y - rect[0].y;
		b[2] = rect[1].z - rect[0].z;
		double lb = sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
		b[0] /= lb; b[1] /= lb; b[2] /= lb;

		double c[3];
		c[0] = a[1] * b[2] - a[2] * b[1];
		c[1] = a[2] * b[0] - a[0] * b[2];
		c[2] = a[0] * b[1] - a[1] * b[0];
		double lc = sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
		c[0] /= lc; c[1] /= lc; c[2] /= lc;

		MyMarker o;
		o.x = rect[0].x - margin * a[0] - margin * b[0] - margin * c[0];
		o.y = rect[0].y - margin * a[1] - margin * b[1] - margin * c[1];
		o.z = rect[0].z - margin * a[2] - margin * b[2] - margin * c[2];

        V3DLONG bsz0 = dist(rect[0], rect[3]) + 1 + 2 * margin + 0.5;
        V3DLONG bsz1 = dist(rect[0], rect[1]) + 1 + 2 * margin + 0.5;
        V3DLONG bsz2 = 1 + 2 * margin + 0.5;
        V3DLONG bsz01 = bsz0 * bsz1;
        for(V3DLONG k = 0; k < bsz2; k++)
		{
            for(V3DLONG j = 0; j < bsz1; j++)
			{
                for(V3DLONG i = 0; i < bsz0; i++)
				{
                    V3DLONG ii = o.x + i * a[0] + j * b[0] + k * c[0] + 0.5;
                    V3DLONG jj = o.y + i * a[1] + j * b[1] + k * c[1] + 0.5;
                    V3DLONG kk = o.z + i * a[2] + j * b[2] + k * c[2] + 0.5;
					if(ii >= 0 && ii < sz0 && jj >= 0 && jj < sz1 && kk >= 0 && kk < sz2)
					{
						all_mask.push_back(MyMarker(ii,jj,kk));
					}
				}
			}
		}
	}
	// prepare boundingbox
    V3DLONG mx = MAX_INT, my = MAX_INT, mz = MAX_INT;
    V3DLONG Mx = 0, My = 0, Mz = 0;
	for(vector<MyMarker>::iterator it = all_mask.begin(); it != all_mask.end(); it++)
	{
		MyMarker marker = *it;
		mx = MIN(mx , marker.x);
		my = MIN(my , marker.y);
		mz = MIN(mz , marker.z);
		Mx = MAX(Mx, marker.x);
		My = MAX(My, marker.y);
		Mz = MAX(Mz, marker.z);
	}

    V3DLONG msz0 = Mx - mx + 1;
    V3DLONG msz1 = My - my + 1;
    V3DLONG msz2 = Mz - mz + 1;
    V3DLONG msz01 = msz0 * msz1;
    V3DLONG mtol_sz = msz2 * msz01;
	unsigned char * mskimg1d = new unsigned char[mtol_sz]; memset(mskimg1d, 0, mtol_sz);
	for(vector<MyMarker>::iterator it = all_mask.begin(); it != all_mask.end(); it++)
	{
		MyMarker marker = *it;
		MyMarker m_marker = MyMarker(marker.x - mx, marker.y - my, marker.z - mz);
		mskimg1d[m_marker.ind(msz0, msz01)] = inimg1d[marker.ind(sz0, sz01)];
	}
	nm1 = near_markers[0]; fm1 = far_markers[0];
	nm2 = *near_markers.rbegin(); fm2 = *far_markers.rbegin();

	nm1 = MyMarker(nm1.x - mx, nm1.y - my, nm1.z - mz);
	nm2 = MyMarker(nm2.x - mx, nm2.y - my, nm2.z - mz);
	fm1 = MyMarker(fm1.x - mx, fm1.y - my, fm1.z - mz);
	fm2 = MyMarker(fm2.x - mx, fm2.y - my, fm2.z - mz);

	vector<MyMarker> sub_markers, tar_markers;
	GET_LINE_MARKERS(nm1, fm1, sub_markers);
	GET_LINE_MARKERS(nm2, fm2, tar_markers);
	fastmarching_linker(sub_markers, tar_markers, mskimg1d, outswc, msz0, msz1, msz2, cnn_type);
    for(V3DLONG i = 0; i < outswc.size(); i++)
	{
		outswc[i]->x += mx;
		outswc[i]->y += my;
		outswc[i]->z += mz;
	}
	return true;

}

// like drawing4, but the bound box is much smaller
template<class T> bool fastmarching_drawing5(vector<MyMarker> & near_markers, vector<MyMarker> &far_markers, T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	assert(near_markers.size() == far_markers.size());

	long sz01 = sz0 * sz1;

	MyMarker nm1, nm2, fm1, fm2;
	nm2 = near_markers[0];
	fm2 = far_markers[0];

	vector<MyMarker> all_mask;  // use vector instead of set to save time
	for(int m = 1; m < near_markers.size(); m++)
	{
		nm1 = nm2; fm1 = fm2;
		nm2 = near_markers[m]; fm2 = far_markers[m];

		// 1. calc rt
		double rt[3] = {0.0, 0.0, 0.0};
		if(nm1 != fm1)
		{
			double tx1 = fm1.x - nm1.x;
			double ty1 = fm1.y - nm1.y;
			double tz1 = fm1.z - nm1.z;
			double dst1 = sqrt(tx1 * tx1 + ty1 * ty1 + tz1 * tz1);
			rt[0] = tx1 / dst1;
			rt[1] = ty1 / dst1;
			rt[2] = tz1 / dst1;
		}
		else if(nm2 != fm2)
		{
			double tx2 = fm2.x - nm2.x;
			double ty2 = fm2.y - nm2.y;
			double tz2 = fm2.z - nm2.z;
			double dst2 = sqrt(tx2 * tx2 + ty2 * ty2 + tz2 * tz2);
			rt[0] = tx2 / dst2;
			rt[1] = ty2 / dst2;
			rt[2] = tz2 / dst2;
		}
		else
		{
			cerr<<"Error : nm1 == nm2 && fm1 == fm2"<<endl;
			return false;
		}
		// 2. calc different vectors
		double n1n2[3] = {nm2.x - nm1.x, nm2.y - nm1.y, nm2.z - nm1.z};
		MAKE_UNIT(n1n2);
		double n2n1[3] = {-n1n2[0], -n1n2[1], -n1n2[2]};

		double f1f2[3] = {fm2.x - fm1.x, fm2.y - fm1.y, fm2.z - fm1.z};
		MAKE_UNIT(f1f2);
		double f2f1[3] = {-f1f2[0], -f1f2[1], -f1f2[2]};

		double n1f1[3] = {rt[0], rt[1], rt[2]};
		double f1n1[3] = {-rt[0], -rt[1], -rt[2]};

		double n2f2[3] = {rt[0], rt[1], rt[2]};
		double f2n2[3] = {-rt[0], -rt[1], -rt[2]};

		int margin = 5;

		// 1. get initial rectangel
		MyMarker rect[4] = {nm1, nm2, fm2, fm1};
		double cos_n1, cos_n2, cos_f1, cos_f2;
		if((cos_n1 = COS_THETA_UNIT(n1f1, n1n2)) < 0.0)
		{
			double d = dist(nm1, nm2) * (-cos_n1);
			rect[0] = MyMarker(nm1.x - d * rt[0], nm1.y - d * rt[1], nm1.z - d * rt[2]);
		}
		if((cos_n2 = COS_THETA_UNIT(n2f2, n2n1)) < 0.0)
		{
			double d = dist(nm1, nm2) * (-cos_n2);
			rect[1] = MyMarker(nm2.x - d * rt[0], nm2.y - d * rt[1], nm2.z - d * rt[2]);
		}
		if((cos_f2 = COS_THETA_UNIT(f2n2, f2f1)) < 0.0)
		{
			double d = dist(fm1, fm2) * (-cos_f2);
			rect[2] = MyMarker(fm2.x + d * rt[0], fm2.y + d * rt[1], fm2.z + d * rt[2]);
		}
		if((cos_f1 = COS_THETA_UNIT(f1n1, f1f2)) < 0.0)
		{
			double d = dist(fm1, fm2) * (-cos_f1);
			rect[3] = MyMarker(fm1.x + d * rt[0], fm1.y + d * rt[1], fm1.z + d * rt[2]);
		}

		// 2. add margin
		double a[3];
		a[0] = rect[3].x - rect[0].x;
		a[1] = rect[3].y - rect[0].y;
		a[2] = rect[3].z - rect[0].z;
		double la = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
		a[0] /= la; a[1] /= la; a[2] /= la;

		double b[3];
		b[0] = rect[1].x - rect[0].x;
		b[1] = rect[1].y - rect[0].y;
		b[2] = rect[1].z - rect[0].z;
		double lb = sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
		b[0] /= lb; b[1] /= lb; b[2] /= lb;

		double c[3]; // c = a x b
		c[0] = a[1] * b[2] - a[2] * b[1];
		c[1] = a[2] * b[0] - a[0] * b[2];
		c[2] = a[0] * b[1] - a[1] * b[0];
		double lc = sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
		c[0] /= lc; c[1] /= lc; c[2] /= lc;

		MyMarker o;
		o.x = rect[0].x - margin * a[0] - margin * b[0] - margin * c[0];
		o.y = rect[0].y - margin * a[1] - margin * b[1] - margin * c[1];
		o.z = rect[0].z - margin * a[2] - margin * b[2] - margin * c[2];

		long bsz0 = dist(rect[0], rect[3]) + 1 + 2 * margin + 0.5;
		long bsz1 = dist(rect[0], rect[1]) + 1 + 2 * margin + 0.5;
		long bsz2 = 1 + 2 * margin + 0.5;
		long bsz01 = bsz0 * bsz1;
		for(long k = 0; k < bsz2; k++)
		{
			for(long j = 0; j < bsz1; j++)
			{
				for(long i = 0; i < bsz0; i++)
				{
					long ii = o.x + i * a[0] + j * b[0] + k * c[0] + 0.5;
					long jj = o.y + i * a[1] + j * b[1] + k * c[1] + 0.5;
					long kk = o.z + i * a[2] + j * b[2] + k * c[2] + 0.5;
					if(ii >= 0 && ii < sz0 && jj >= 0 && jj < sz1 && kk >= 0 && kk < sz2)
					{
						all_mask.push_back(MyMarker(ii,jj,kk));
					}
				}
			}
		}
	}

	// calculate pca for projection points
	vector<MyMarker> prjct3d_markers;
	MyMarker orig_marker;
	orig_marker.x = (near_markers[0].x + far_markers[0].x)/2.0;
	orig_marker.y = (near_markers[0].y + far_markers[0].y)/2.0;
	orig_marker.z = (near_markers[0].z + far_markers[0].z)/2.0;
	prjct3d_markers.push_back(orig_marker);

	// get the center of mass
	MyMarker mid_marker = orig_marker;
	int num_rays = near_markers.size();
	for(int m = 1; m < num_rays; m++)
	{
		MyMarker nm = near_markers[m];
		MyMarker fm = far_markers[m];
		MyMarker prjct3d_marker;
		GET_POINT_TO_LINE_PROJECT(orig_marker, prjct3d_marker, nm, fm);
		prjct3d_markers.push_back(prjct3d_marker);

		mid_marker.x += prjct3d_marker.x;
		mid_marker.y += prjct3d_marker.y;
		mid_marker.z += prjct3d_marker.z;
	}

	mid_marker.x /= num_rays;
	mid_marker.y /= num_rays;
	mid_marker.z /= num_rays;

	vector<MyMarker> prjct2d_markers;
	prjct2d_markers.push_back(MyMarker(0,0,0));
	double dx = mid_marker.x - orig_marker.x;
	double dy = mid_marker.y - orig_marker.y;
	double dz = mid_marker.z - orig_marker.z;
	double dd = sqrt(dx*dx + dy*dy + dz*dz);
	assert(dd != 0.0);
	dx /= dd; dy /= dd; dz /= dd;

	double a[3] = {dx, dy, dz};
	double b[3], c[3];
	c[0] = far_markers[0].x - near_markers[0].x;
	c[1] = far_markers[0].y - near_markers[0].y;
	c[2] = far_markers[0].z - near_markers[0].z;
	double lc = sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
	assert(lc != 0);
	c[0] /= lc; c[1] /= lc; c[2] /= lc;
	// b = c x a
	b[0] = c[1] * a[2] - c[2] * a[1];
	b[1] = c[2] * a[0] - c[0] * a[2];
	b[2] = c[0] * a[1] - c[1] * a[0];

	// get center of mass in new coordinate system

	double avg_x = 0, avg_y = 0, avg_z = 0;

	for(int m = 1; m < prjct3d_markers.size(); m++)
	{
		MyMarker marker = prjct3d_markers[m];
		dx = marker.x - orig_marker.x;
		dy = marker.y - orig_marker.y;
		dz = marker.z - orig_marker.z;
		dd = sqrt(dx*dx + dy*dy + dz*dz);
		assert(dd != 0.0);
		dx /= dd; dy /= dd; dz /= dd;
		double cos_theta_x = dx * a[0] + dy * a[1] + dz * a[2];
		double cos_theta_y = dx * b[0] + dy * b[1] + dz * b[2];
		double x = dd * cos_theta_x;
		double y = dd * cos_theta_y;
		double z = 0.0;
		prjct2d_markers.push_back(MyMarker(x,y,z));
		avg_x += x;
		avg_y += y;
		avg_z += z;
	}
	avg_x /= num_rays; avg_y /= num_rays; avg_z /= num_rays;

	// get the orientation
	double alpha = 0.0, beta = 0.0, gama = 0.0;
	for(int m = 0; m < prjct2d_markers.size(); m++)
	{
		alpha += (prjct2d_markers[m].x - avg_x) * (prjct2d_markers[m].x - avg_x);
		beta +=  2*(prjct2d_markers[m].x - avg_x) * (prjct2d_markers[m].y - avg_y);
		gama +=  (prjct2d_markers[m].y - avg_y) * (prjct2d_markers[m].y - avg_y);
	}
	//alpha /= num_rays; beta /= num_rays; gama /= num_rays;
	double theta = 0.5 * atan(beta/(alpha-gama));
	double cos_theta = cos(theta);
	double sin_theta = sin(theta);

	// create new coordinate system
	double aa[3], bb[3];
	double cc[3] = {c[0], c[1], c[2]};
	aa[0] = cos_theta * a[0] + sin_theta * b[0];
	aa[1] = cos_theta * a[1] + sin_theta * b[1];
	aa[2] = cos_theta * a[2] + sin_theta * b[2];

	bb[0] = cc[1] * aa[2] - cc[2] * aa[1];
	bb[1] = cc[2] * aa[0] - cc[0] * aa[2];
	bb[2] = cc[0] * aa[1] - cc[1] * aa[0];

	vector<MyMarker> new_mask;
	double mx = MAX_DOUBLE, my = MAX_DOUBLE, mz = MAX_DOUBLE;
	double Mx = - MAX_DOUBLE, My = - MAX_DOUBLE, Mz = -MAX_DOUBLE;
	for(int i = 0; i < all_mask.size(); i++)
	{
		MyMarker marker = all_mask[i];
		dx = marker.x - mid_marker.x;
		dy = marker.y - mid_marker.y;
		dz = marker.z - mid_marker.z;
		dd = sqrt(dx*dx + dy*dy + dz*dz);
		assert(dd != 0.0);
		dx /= dd; dy /= dd; dz /= dd;
		double cos_theta_x = dx * aa[0] + dy * aa[1] + dz * aa[2];
		double cos_theta_y = dx * bb[0] + dy * bb[1] + dz * bb[2];
		double cos_theta_z = dx * cc[0] + dy * cc[1] + dz * cc[2];
		double x = dd * cos_theta_x;
		double y = dd * cos_theta_y;
		double z = dd * cos_theta_z;
		mx = MIN(x, mx); my = MIN(y, my); mz = MIN(z, mz);
		Mx = MAX(x, Mx); My = MAX(y, My); Mz = MAX(z, Mz);
		new_mask.push_back(MyMarker(x,y,z));
	}

	long msz0 = Mx - mx + 0.5 + 1;
	long msz1 = My - my + 0.5 + 1;
	long msz2 = Mz - mz + 0.5 + 1;
	long msz01 = msz0 * msz1;
	long mtol_sz = msz2 * msz01;
	unsigned char * outimg1d = new unsigned char[mtol_sz]; memset(outimg1d, 0, mtol_sz);

	// get new index in new coordinate system for all mask_markers
	for(int m = 0; m < all_mask.size(); m++)
	{
		MyMarker marker1 = all_mask[m];
		long ind1 = marker1.ind(sz0, sz01);

		MyMarker marker2 = new_mask[m];
		int x = marker2.x - mx + 0.5;
		int y = marker2.y - my + 0.5;
		int z = marker2.z - mz + 0.5;
		long ind2 = z * msz01 + y * msz0 + x;
		outimg1d[ind2] = inimg1d[ind1];
	}
	nm1 = near_markers[0]; fm1 = far_markers[0];
	nm2 = *near_markers.rbegin(); fm2 = *far_markers.rbegin();

	// convert to index in bounding box
	MyMarker new_nm1, new_fm1, new_nm2, new_fm2;
	orig_marker.x = mid_marker.x + mx * aa[0] + my * bb[0] + mz * cc[0];
	orig_marker.y = mid_marker.y + mx * aa[1] + my * bb[1] + mz * cc[1];
	orig_marker.z = mid_marker.z + mx * aa[2] + my * bb[2] + mz * cc[2];
	GET_POSITION_IN_NEW_COORDINATE(nm1, new_nm1, orig_marker, aa, bb, cc);
	GET_POSITION_IN_NEW_COORDINATE(fm1, new_fm1, orig_marker, aa, bb, cc);
	GET_POSITION_IN_NEW_COORDINATE(nm2, new_nm2, orig_marker, aa, bb, cc);
	GET_POSITION_IN_NEW_COORDINATE(fm2, new_fm2, orig_marker, aa, bb, cc);

	vector<MyMarker> sub_markers, tar_markers;
	GET_LINE_MARKERS(new_nm1, new_fm1, sub_markers);
	GET_LINE_MARKERS(new_nm2, new_fm2, tar_markers);
	fastmarching_linker(sub_markers, tar_markers, outimg1d, outswc, msz0, msz1, msz2, cnn_type);
	for(int i = 0; i < outswc.size(); i++)
	{
		double x = outswc[i]->x;
		double y = outswc[i]->y;
		double z = outswc[i]->z;

		outswc[i]->x = orig_marker.x + x * aa[0] + y * bb[0] + z * cc[0];
		outswc[i]->y = orig_marker.y + x * aa[1] + y * bb[1] + z * cc[1];
		outswc[i]->z = orig_marker.z + x * aa[2] + y * bb[2] + z * cc[2];
	}

	if(outimg1d){delete [] outimg1d; outimg1d = 0;}

	return true;
}


#endif




