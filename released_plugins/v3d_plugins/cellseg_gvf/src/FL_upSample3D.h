// upsample a 3D volume by a factor along each dimension
// F. Long
// 20080507
// 20080830 updated

#ifndef __UP_SAMPLE3D__
#define __UP_SAMPLE3D__

#include "volimg_proc.h"
#include "img_definition.h"
#include "stackutil.h"

#include "FL_interpolateCoordLinear3D.h"
#include "FL_interpolateCoordCubic3D.h"
#include "FL_coordDefinition.h"
#include "FL_defType.h"

// 20120403
template <class T> bool upsample3dvol(T *outdata, T *indata, V3DLONG *szout, V3DLONG *szin, double *dfactor)
{ 
    
    szout[0] = (V3DLONG)(ceil(dfactor[0]*szin[0]));
    szout[1] = (V3DLONG)(ceil(dfactor[1]*szin[1]));
    szout[2] = (V3DLONG)(ceil(dfactor[2]*szin[2]));
    
	
	V3DLONG totallen = szout[0] * szout[1] * szout[2]; 
	Coord3D * c = new Coord3D [totallen];
	T * v = new T [totallen];
	
	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		return false;
	}
	
    //computation
	V3DLONG i,j,k, ind;
    ind=0;
	
	for (k=0;k<szout[2];k++)
	{
		for (j=0;j<szout[1];j++)
		{
			for (i=0;i<szout[0];i++)
			{
				c[ind].x = double(i)/dfactor[0];
//#ifndef POSITIVE_Y_COORDINATE
//				c[ind].y = double(szout[1]-1-j)/dfactor[1];
//#else
				c[ind].y = double(j)/dfactor[1]; 
//#endif
				c[ind].z = double(k)/dfactor[2];
				v[ind] = -1; //set as a special value 
				ind++;
			}
		}
	}
	
	interpolate_coord_linear(v, c, totallen, indata, szin[0], szin[1], szin[2], 0, szin[0]-1, 0, szin[1]-1, 0, szin[2]-1);	
	
    ind=0;
//    long szout_xy = szout[0]*szout[1];
//    
//    
//	for (k=0;k<szout[2];k++)
//	{
//        long tmp1 = k*szout_xy;
//		for (j=0;j<szout[1];j++)
//		{
//            long tmp2 = tmp1 + j*szout[0];
//			for (i=0;i<szout[0];i++)
//			{
//                outdata[tmp2+i] = (T)(v[ind++]);
//			}
//		}
//	}
    

	for (i=0; i<totallen; i++)
        outdata[i] = (T)(v[i]);
    
    // free temporary memory
	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}


template <class T> bool upsample3dvol(T ***outdata, T ***indata, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, double *dfactor)
{ 

	V3DLONG tsz0o = (V3DLONG)(ceil(dfactor[0]*tsz0)), tsz1o = (V3DLONG)(ceil(dfactor[1]*tsz1)), tsz2o = (V3DLONG)(ceil(dfactor[2]*tsz2));
	
	V3DLONG totallen = tsz0o * tsz1o * tsz2o; 
	Coord3D * c = new Coord3D [totallen];
	T * v = new T [totallen];
	
	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		return false;
	}
	
    //computation
	V3DLONG i,j,k, ind;
    ind=0;
	
	for (k=0;k<tsz2o;k++)
	{
		for (j=0;j<tsz1o;j++)
		{
			for (i=0;i<tsz0o;i++)
			{
				c[ind].x = double(i)/dfactor[0];
//#ifndef POSITIVE_Y_COORDINATE
//				c[ind].y = double(tsz1o-1-j)/dfactor[1];
//#else
				c[ind].y = double(j)/dfactor[1]; 
//#endif
				c[ind].z = double(k)/dfactor[2];
				v[ind] = -1; //set as a special value 
				ind++;
			}
		}
	}
	
	interpolate_coord_linear(v, c, totallen, indata, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);	
//    interpolate_coord_cubic(v, c, totallen, indata, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);
    
    ind=0;
	for (k=0;k<tsz2o;k++)
	{
		for (j=0;j<tsz1o;j++)
		{
			for (i=0;i<tsz0o;i++)
			{
				outdata[k][j][i] = (T)(v[ind++]);
			}
		}
	}

	
    // free temporary memory
	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}


template <class T> bool upsample3dvol(Vol3DSimple<T> * & outimg, Vol3DSimple<T> * inimg, double *dfactor) 
{
    //verify the input parameters
	
    if (outimg || !inimg || !inimg->valid() || dfactor[0]<1 || dfactor[1]<1 || dfactor[2]<1) 
	   {fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__); return false;}
	

	T *** timg3din = inimg->getData3dHandle();
	V3DLONG tsz0 = inimg->sz0();
	V3DLONG tsz1 = inimg->sz1();
	V3DLONG tsz2 = inimg->sz2(); 

	V3DLONG tsz0o = V3DLONG(ceil(dfactor[0]*tsz0)), tsz1o = V3DLONG(ceil(dfactor[1]*tsz1)), tsz2o = V3DLONG(ceil(dfactor[2]*tsz2));
	
    outimg = new Vol3DSimple<T> (tsz0o, tsz1o, tsz2o);
	if (!outimg || !outimg->valid())
	{
		fprintf(stderr, "Fail to allocate memory in upsample3dvol(). [%d]\n", __LINE__);
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}
	
	T *** timg3dout = outimg->getData3dHandle();
	upsample3dvol(timg3dout, timg3din, tsz0, tsz1, tsz2, dfactor);
	
	return true;
}	

#endif
