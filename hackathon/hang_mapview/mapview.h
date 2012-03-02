#ifndef __MAPVIEW_H__
#define __MAPVIEW_H__

#include <string>
#include "stackutil.h"
using namespace std;

struct ImageMapView
{
	V3DLONG L, M, N;                   // the image size 2^L * 2^M * 2^N
	V3DLONG l, m, n;                   // the initial block size 2^l * 2^m * 2^n, the initial tilling size 2^(L-l) * 2^(M-m) * 2^(N-n)
	string prefix;

	ImageMapView();
	void setPara(string _prefix, V3DLONG _L, V3DLONG _M, V3DLONG _N, V3DLONG _l, V3DLONG _m, V3DLONG _n);
	void getImage(V3DLONG level, unsigned char * & outimg1d, V3DLONG x0, V3DLONG y0, V3DLONG z0, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);

private :
	void getImageSize(V3DLONG level, V3DLONG & ts0, V3DLONG & ts1, V3DLONG & ts2, V3DLONG &bs0, V3DLONG &bs1, V3DLONG &bs2);

};

#endif
