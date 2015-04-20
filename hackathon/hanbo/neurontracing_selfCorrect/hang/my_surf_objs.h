// OCT 11, modification :
// 1.add degree as union type in MyMarker, 
// 2. MyMarker is alias as MyNode
// 3. just disable __SET_MARKER_DEGREE__ if you don't like these change
#ifndef __MY_SURF_OBJS_H__
#define __MY_SURF_OBJS_H__
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#define __SET_MARKER_DEGREE__

using namespace std;

#define MARKER_BASE 1.0 // the basic offset of marker is 1.0, the marker coordinate will be converted when read and save


struct MyPoint
{
	int x;
	int y;
	int z;
	bool operator<(const MyPoint & other) const{
		if(z > other.z) return false;
		if(z < other.z) return true;
		if(y > other.y) return false;
		if(y < other.y) return true;
		if(x > other.x) return false;
		if(x < other.x) return true;
		return false;
	}
	MyPoint()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	MyPoint(int _x, int _y, int _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

struct MYXYZ {
   double x,y,z;
};

// root node with parent 0
struct MyMarker
{
	double x;
	double y;
	double z;
	union
	{
#ifdef __SET_MARKER_DEGREE__
		double degree;
#endif
		double radius;
	};
	int type;
	MyMarker* parent;
	MyMarker(){x=y=z=radius=0.0; type = 3; parent=0;}
	MyMarker(double _x, double _y, double _z) {x = _x; y = _y; z = _z; radius = 0.0; type = 3; parent = 0;}
	MyMarker(const MyMarker & v){x=v.x; y=v.y; z=v.z; radius = v.radius; type = v.type; parent = v.parent;}
	MyMarker(const MyPoint & v){x=v.x; y=v.y; z=v.z; radius = 0.0; type = 3; parent = 0;}
	MyMarker(const MYXYZ & v){x=v.x; y=v.y; z=v.z; radius = 0.0; type = 3; parent = 0;}

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

	long long ind(long long sz0, long long sz01)
	{
		return ((long long)(z+0.5) * sz01 + (long long)(y+0.5)*sz0 + (long long)(x+0.5));
	}
};

typedef MyMarker MyNode;

#define MidMarker(m1, m2) MyMarker(((m1).x + (m2).x)/2.0,((m1).y + (m2).y)/2.0,((m1).z + (m2).z)/2.0)


vector<MyMarker> readMarker_file(string marker_file);
bool readMarker_file(string marker_file, vector<MyMarker*> &markers);
bool saveMarker_file(string marker_file, vector<MyMarker> & out_markers);
bool saveMarker_file(string marker_file, vector<MyMarker*> & out_markers);
bool saveMarker_score(string marker_file, vector<MyMarker*> & out_markers, map<MyMarker*, double> & score_map);

vector<MyMarker*> readSWC_file(string swc_file);
bool saveSWC_file(string swc_file, vector<MyMarker*> & out_markers);
bool saveSWC_score(string swc_file, vector<MyMarker*> & out_markers, map<MyMarker*, double> & score_map);

double dist(MyMarker a, MyMarker b);

#endif
