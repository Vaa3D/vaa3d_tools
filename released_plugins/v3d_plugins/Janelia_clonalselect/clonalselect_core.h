/* clonalselect_core.h
 * 2013-01-13: create this program by Yang Yu
 */


#ifndef __CLONALSELECT_CORE_H__
#define __CLONALSELECT_CORE_H__

//
#include <cmath>
#include <ctime>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>

using namespace std;
using std::ifstream;

//
/// basic funcs copied from y_img.h
//

// delete 1d pointer
template <class T>
void y_del(T *&p)
{
    if(p) {delete []p; p=NULL;}
    return;
}

// delete 2 1d pointers
template<class T1, class T2>
void y_del2(T1 *&p1, T2 *&p2)
{
    y_del<T1>(p1);
    y_del<T2>(p2);
    return;
}

// new 1d pointer
template<class T, class Tidx>
void y_new(T *&p, Tidx N)
{
    //
    y_del<T>(p);

    //
    try
    {
        p = new T [N];
        for(Tidx i=0; i<N; i++) p[i] = 0;
    }
    catch(...)
    {
        cout<<"Attempt to allocate memory failed!"<<endl;
        y_del<T>(p);
        return;
    }
    return;
}

//
/// point clouds class
//

/** Adjusted from pcl PCD_V6 data structure
 *
 * # Point Clouds Data version 1.0
 * FIELDS x y z v
 * TYPE 2 2 2 1
 * DIMENSIONS 1024 512 218
 * POINTS ...
 * MAXVALUE ...
 * DATA ascii/binary
 *
 */

#define PCD_COMMENT (" Point Clouds Data version 1.0 ")
enum DataSize {DSUINT8=1, DSUINT16=2, DSFLOAT32=4};
enum StoreType {ASCII=0, BINARY=1};

//
template<class Tdata, class Tidx>
class Point
{
public:
    Point(){}
    ~Point(){}

public:
    Tidx x, y, z;
    Tdata v;
};

template<class Tdata, class Tidx>
class PCDHeader
{
public:
    PCDHeader(){}
    ~PCDHeader(){}

public:
    Tidx width, height, depth;
    DataSize x, y, z, v;
    Tidx len;
    Tdata maxv;
    StoreType st;
};

//
class PointClouds
{
public:
    PointClouds();
    ~PointClouds();

public:
    int read(string fn);
    int write(string fn);

    int convert(unsigned char* p, long sx, long sy, long sz);

public:
    PCDHeader<unsigned short, long> pcdheadinfo;
    vector< Point<unsigned short, unsigned short> > points;
    double thresh;
};


#endif // __CLONALSELECT_CORE_H__



