// 3D distance transform
// F. Long
// 20080822

#ifndef __DISTANCE_TRANSFORM3D__
#define __DISTANCE_TRANSFORM3D__

#include "volimg_proc.h"
#include "img_definition.h"

#include "FL_bwdist.h"

// inimg: input image
// distimg: distance transform of the input image
// indeximg: index of the region for each pixel, to which the distance is shortest
// tag: 1 if computing the distance transform of non-zero values, 0 if compute the distance transform of zero values

template <class T> bool distTrans3d(Vol3DSimple <T> *inimg, Vol3DSimple <float> *distimg, Vol3DSimple <V3DLONG> *indeximg, unsigned char tag)
{	
	if (!inimg || !inimg->valid() || !distimg || !distimg->valid() || !indeximg || !indeximg->valid())
	{	printf("invalid image in distance transform \n");
		return false;
	}
	
	V3DLONG *sz = new V3DLONG [3];
	sz[0] = inimg->sz0();
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2();
	
	V3DLONG len = sz[0]*sz[1]*sz[2];

	T *p = inimg->getData1dHandle();
	float *data1d = distimg->getData1dHandle();	
	V3DLONG *pix_index = indeximg->getData1dHandle();
	
	V3DLONG k;
		
	for (k=0;k<len;k++)
	{
		data1d[k] = (float) p[k];	
	}
	
	dt3d_binary(data1d, pix_index, sz, tag); // compute the distance transform for foreground (non-zero) pixels	if tag = 1
	
	return true;
}

template <class T1, class T2> bool distTrans3d(Vol3DSimple <T1> *inimg, Vol3DSimple <T2> *distimg, Vol3DSimple <V3DLONG> *indeximg, unsigned char tag)
{	
	if (!inimg || !inimg->valid() || !distimg || !distimg->valid() || !indeximg || !indeximg->valid())
	{	printf("invalid image in distance transform \n");
		return false;
	}
	
	V3DLONG *sz = new V3DLONG [3];
	sz[0] = inimg->sz0();
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2();
	
	T1 *indata1d = inimg->getData1dHandle();
	T2 *outdata1d = distimg->getData1dHandle();	
	V3DLONG *pix_index = indeximg->getData1dHandle();
	
	dt3d_binary(indata1d, outdata1d, pix_index, sz, tag); // compute the distance transform for foreground (non-zero) pixels	if tag = 1
	
	return true;
}


#endif
	
