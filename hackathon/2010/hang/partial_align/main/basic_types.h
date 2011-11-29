#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__
#include "v3d_basicdatatype.h"
#include <string>
#include <vector>
using namespace std;
struct MarkerType
{
    V3DLONG x, y, z;      // point coordinates
	//double value;

    MarkerType() {x=y=z=0;}
	MarkerType(V3DLONG xx, V3DLONG yy, V3DLONG zz): x(xx), y(yy), z(zz){}
	//MarkerType(V3DLONG xx, V3DLONG yy, V3DLONG zz, V3DLONG _value): x(xx), y(yy), z(zz), value(_value){}
};

struct FeatureType
{
	V3DLONG x;
	V3DLONG y;
	V3DLONG z;
	vector<double> descriptor;
};

vector<MarkerType> readMarker_file(const string filename);
bool writeMarker_file(const string filename, const vector<MarkerType> & vecMarker);

vector<FeatureType> readFeature_file(const string filename);
bool writeFeature_file(const string filename, const vector<FeatureType> & vecFeature);

#endif
