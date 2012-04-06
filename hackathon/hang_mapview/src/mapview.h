#ifndef __MAPVIEW_H__
#define __MAPVIEW_H__

#include <string>
#include "stackutil.h"
using namespace std;

struct ImageMapView
{
	V3DLONG L, M, N;                   // the block number in X, Y, Z dimension
	V3DLONG l, m, n;                   // the block size 
	V3DLONG channel;
	string prefix;

	ImageMapView();
	void setPara(string _prefix, V3DLONG _L, V3DLONG _M, V3DLONG _N, V3DLONG _l, V3DLONG _m, V3DLONG _n);
	void getImage(V3DLONG level, unsigned char * & outimg1d, V3DLONG x0, V3DLONG y0, V3DLONG z0, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);

	void getImageSize(V3DLONG level, V3DLONG & ts0, V3DLONG & ts1, V3DLONG & ts2, V3DLONG &bs0, V3DLONG &bs1, V3DLONG &bs2);

};

bool raw_split(const char * infile, const char * outpref, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);

bool createMapViewFiles(string prefix, V3DLONG ts0, V3DLONG ts1, V3DLONG ts2);

#endif
