// compute region properties: area, pixelIdxList, centroid
// F. Long
// 20081027

#ifndef __REGION_PROPS__
#define __REGION_PROPS__

#include "volimg_proc.h"

class Props
{
public:

	V3DLONG area;
	V3DLONG *pixelIdxList;
	V3DLONG *centroid;

public:
	Props()
	{
		area = 0;
		pixelIdxList = 0;
		centroid = 0;
	}
		
	// 2d
	template <class T> V3DLONG getArea(T **img, int rgnidx, const V3DLONG *sz);
	template <class T> V3DLONG * getPixelIdxList(T **img, int rgnidx, const V3DLONG *sz);
	template <class T> V3DLONG * getCentroid(T **img, int rgnidx, const V3DLONG *sz);
	
	//3d
	template <class T> V3DLONG getArea(T ***img, int rgnidx, const V3DLONG *sz);
	template <class T> V3DLONG * getPixelIdxList(T ***img, int rgnidx, const V3DLONG *sz);
	template <class T> V3DLONG * getCentroid(T ***img, int rgnidx, const V3DLONG *sz);

	~Props()
	{
		if (pixelIdxList) {delete [] pixelIdxList; pixelIdxList = 0;}
		if (centroid) {delete [] centroid; centroid = 0;}		
	}
	
};

//Props::Props()
//{
//	area = 0;
//	pixelIdxList = 0;
//	centroid = 0;
//}

template <class T> V3DLONG Props::getArea(T **img, int rgnidx, const V3DLONG *sz)
{
	area = 0;
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (img[j][i] == rgnidx)
			area++;
	}
	
	return area;
}

template <class T> V3DLONG * Props::getPixelIdxList(T **img, int rgnidx, const V3DLONG *sz)
{
	Props:getArea(img, rgnidx, sz);
	pixelIdxList = new V3DLONG [area];
	
	int count = 0;
	
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (img[j][i] == rgnidx)
		{	count ++;
			pixelIdxList[count] = j*sz[0] + i;
		}
	}
	
	return pixelIdxList;
	
}

template <class T> V3DLONG * Props::getCentroid(T **img, int rgnidx, const V3DLONG *sz)
{	
	centroid = new V3DLONG [2];
	centroid[0] = 0;
	centroid[1] = 0;
	
	int count = 0;
	
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (img[j][i] == rgnidx)
		{	count ++;
			centroid[0] = centroid[0] + i;
			centroid[1] = centroid[1] + j;
		}
	}
	centroid[0] = centroid[0]/count;
	centroid[1] = centroid[1]/count;
	
	return centroid;	 	
}

template <class T> V3DLONG Props::getArea(T ***img, int rgnidx, const V3DLONG *sz)
{
	area = 0;
	for (int k=0; k<sz[2]; k++)
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (img[k][j][i] == rgnidx)
			area++;
	}
	
	return area;
}

template <class T> V3DLONG * Props::getPixelIdxList(T ***img, int rgnidx, const V3DLONG *sz)
{
	Props:getArea(img, rgnidx, sz);
	pixelIdxList = new V3DLONG [area];
	
	int count = 0;
	
	for (int k=0; k<sz[2]; k++)
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (img[k][j][i] == rgnidx)
		{	count ++;
			pixelIdxList[count] = k*sz[1]*sz[0] + j*sz[0] + i;
		}
	}

	return pixelIdxList;

}

template <class T> V3DLONG * Props::getCentroid(T ***img, int rgnidx, const V3DLONG *sz)
{
	centroid = new V3DLONG [3];
	
	for (int i=0; i<2; i++)
		centroid[i] = 0;
	
	int count = 0;
	
	for (int k=0; k<sz[2]; k++)
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (img[k][j][i] == rgnidx)
		{	count ++;
			centroid[0] = centroid[0] + i;
			centroid[1] = centroid[1] + j;
			centroid[2] = centroid[2] + k;
			
		}
	}
	
	for (int i=0; i<2; i++)
		centroid[i] = centroid[i]/count;
		
	return centroid;

}


//Props::~Props()
//{
//	if (pixelIdxList) {delete [] pixelIdxList; pixelIdxList = 0;}
//	if (centroid) {delete [] centroid; centroid = 0;}
//	
//}

template <class T> Props *  regionProps(T **labelimage, const V3DLONG *sz)
{
	Props * rgnProps;
	T maxval;
		
	img2d_max(maxval, labelimage, sz[0], sz[1]);
	
	rgnProps = new Props [maxval];
	
	// find all label values, indicaated by tag
	unsigned char *tag = new unsigned char [maxval];
	for (int cnt =0; cnt<maxval; cnt++)
	{
		tag[cnt] = 0;
	}
	
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (labelimage[j][i]>0)
			tag[labelimage[j][i]] = 1;
	}	
	
	// process each region
	for (int cnt =0; cnt<maxval; cnt++)
	{
		if (tag[cnt]>0)
		{
			rgnProps[cnt].area = getArea(labelimage, cnt, sz);
			rgnProps[cnt].pixelIdxList = getPixelIdxList(labelimage, cnt, sz);
			rgnProps[cnt].centroid = getCentroid(labelimage, cnt, sz);
			
		}
	}
	
	return rgnProps;
}

template <class T> Props * regionProps(T ***labelimage, const V3DLONG *sz)
{
	Props * rgnProps;
	T maxval;
	
	vol3d_max(maxval, labelimage, sz[0], sz[1], sz[2]);
		
	rgnProps = new Props [maxval];
	
	// find all label values, indicaated by tag
	unsigned char *tag = new unsigned char [maxval];
	
	for (int cnt =0; cnt<maxval; cnt++)
	{
		tag[cnt] = 0;
	}

	for (int k=0; k<sz[2]; k++)		
	for (int j=0; j<sz[1]; j++)
	for (int i=0; i<sz[0]; i++)
	{
		if (labelimage[k][j][i]>0)
			tag[labelimage[k][j][i]] = 1;
	}	
	
	// process each region
	for (int cnt =0; cnt<maxval; cnt++)
	{
		if (tag[cnt]>0)
		{
			rgnProps[cnt].area = getArea(labelimage, cnt, sz);
			rgnProps[cnt].pixelIdxList = getPixelIdxList(labelimage, cnt, sz);
			rgnProps[cnt].centroid = getCentroid(labelimage, cnt, sz);

			
		}
	}
	
	return rgnProps;
}


#endif 

