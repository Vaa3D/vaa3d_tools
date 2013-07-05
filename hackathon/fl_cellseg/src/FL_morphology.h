/* morphology operations
 * F. Long
  define structural elements and morphology operations, dilate, erode, open, close, and fill
 * F. Long
 * combine FL_morphology_old.h and FL_morphology_old.cpp, for template, interface and implementation need to be in the same file
 */

#ifndef __FL_MORPHOLOGY_H__
#define __FL_MORPHOLOGY_H__

#include "FL_neighborhood.h"
#include "FL_neighborhoodWalker.h"


class Strel
{
	V3DLONG *its_offset;
	V3DLONG its_num_neighbors;
	V3DLONG its_num_dims;
	V3DLONG *its_array_sz;
    
public:
    
	Strel();
	void printStrelInfo();
	void createDiskStrel(const V3DLONG radius);
	void createBallStrel(const V3DLONG radius);	
	void creatCuboidStrel(const V3DLONG *cuboid_sz);
	void creatRectangleStrel(const V3DLONG *rectangle_sz);	
	Neighborhood * convertStrelToNeighbor();
	
	~Strel();
};




template <class T> void bwDilate(Vol3DSimple <T> *img3d,  Strel *se);
template <class T> void bwDilate(Image2DSimple <T> *img2d,  Strel *se);

template <class T> void bwErode(Vol3DSimple <T> *img3d,  Strel *se);
template <class T> void bwErode(Image2DSimple <T> *img2d,  Strel *se);

template <class T> void bwClose(Vol3DSimple<T> *img3d, Strel *se);
template <class T> void bwClose(Image2DSimple<T> *img2d, Strel *se);

template <class T> void bwOpen(Vol3DSimple<T> *img3d, Strel *se);
template <class T> void bwOpen(Image2DSimple<T> *img2d, Strel *se);

template <class T> void bwErode(T *indata,  V3DLONG len0, V3DLONG len1, V3DLONG len2, Strel *se);

// FL, 20120328 added for simple call
template <class T> void bwDilate(T *img3d,  long *sz, Strel *se);
template <class T> void bwErode(T *img3d,  long *sz, Strel *se);
template <class T> void bwOpen(T *img3d, long *sz, Strel *se);


// binary dilate of 3d image
// input: img3d, se
// output: img3d

template <class T> void bwDilate(Vol3DSimple <T> *img3d,  Strel *se)
{
    Neighborhood *nh = new Neighborhood;
    NeighborhoodWalker *nh_walker = new NeighborhoodWalker;
	V3DLONG num_elements = 1;
	V3DLONG i,j;
	V3DLONG indata_dims = 3;
	
	
	T *indata = img3d->getData1dHandle();
	T *outdata;
	
	V3DLONG *indata_sz = new V3DLONG [3];
	
	indata_sz[0]= img3d->sz0();
	indata_sz[1]= img3d->sz1();
	indata_sz[2]= img3d->sz2();

	nh = se->convertStrelToNeighbor();
    nh_walker = new NeighborhoodWalker(nh, indata_sz, indata_dims, 0U);

	for (i=0; i<indata_dims; i++)
		num_elements *= indata_sz[i];
		
	outdata = new T [num_elements];
	
    for (i = 0; i < num_elements; i++)
    {   
//		outdata[i] = 0;
		
        if (indata[i])
        {
            nh_walker->setWalkerLocation(i);
            while (nh_walker->getNextInboundsNeighbor(&j, NULL))
            {
                outdata[j] = 1;
            }
        }
    }

    for (i = 0; i < num_elements; i++)
    {   
		indata[i] = outdata[i];
	}
	
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
	if (outdata) {delete [] outdata; outdata = 0;}	
	if (indata_sz) {delete [] indata_sz; indata_sz = 0;}	
	
}

// FL , 20120328 add, in order for jba_analysis_pipeline.cpp to call

template <class T> void bwDilate(T *img3d,  long *sz, Strel *se)
{
//    Neighborhood *nh = new Neighborhood;
    Neighborhood *nh = 0;
    
//    NeighborhoodWalker *nh_walker = new NeighborhoodWalker;
    NeighborhoodWalker *nh_walker = 0;
    
	V3DLONG num_elements = 1;
	V3DLONG i,j;
	V3DLONG indata_dims = 3;
	
	
	T *indata = img3d;
	T *outdata;
	
    se->printStrelInfo();
	nh = se->convertStrelToNeighbor();
    

    nh_walker = new NeighborhoodWalker(nh, sz, indata_dims, 0U);
    
	for (i=0; i<indata_dims; i++)
		num_elements *= sz[i];
    
	outdata = new T [num_elements];
	
    for (i = 0; i < num_elements; i++)
    {   
        //		outdata[i] = 0;
		
        if (indata[i])
        {
            nh_walker->setWalkerLocation(i);
            while (nh_walker->getNextInboundsNeighbor(&j, NULL))
            {
                outdata[j] = 1;
            }
        }
    }
    
    for (i = 0; i < num_elements; i++)
    {   
		indata[i] = outdata[i];
	}
	
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
	if (outdata) {delete [] outdata; outdata = 0;}	
	
}



// binary dilate of 2d image
// input: img2d, se
// output: img2d

template <class T> void bwDilate(Image2DSimple <T> *img2d,  Strel *se)
{
    Neighborhood *nh = new Neighborhood;
    NeighborhoodWalker *nh_walker = new NeighborhoodWalker;
	V3DLONG num_elements = 1;
	V3DLONG i,j;
	V3DLONG indata_dims = 2;
	
	T *indata = img2d->getData1dHandle();
	V3DLONG *indata_sz = new V3DLONG [2];
	T *outdata;
	
	indata_sz[0]= img2d->sz0();
	indata_sz[1]= img2d->sz1();

	
	nh = se->convertStrelToNeighbor();
    nh_walker = new NeighborhoodWalker(nh, indata_sz, indata_dims, 0U);

	for (i=0; i<indata_dims; i++)
		num_elements *= indata_sz[i];
				
	outdata = new T [num_elements];

//	int cnt = 0;
//    for (i = 0; i < num_elements; i++)
//    {   
//		if (indata[i] >0)
//			cnt ++;
//	}
//	printf("%d\n", cnt);
	
    for (i = 0; i < num_elements; i++)
    {   
		outdata[i] = 0;
	}
	
//	int maxvaltmp = -9999;
    for (i = 0; i < num_elements; i++)
    {   
//		outdata[i] = 0;
		
        if (indata[i])
        {
            nh_walker->setWalkerLocation(i);
            while (nh_walker->getNextInboundsNeighbor(&j, NULL))
            {
                outdata[j] = 1;
//				printf("%d, %d, %d\n", i,j, abs(i-j));
//				maxvaltmp = (maxvaltmp>abs(i-j))?maxvaltmp: abs(i-j);
            }
        }
    }

//	printf("%d\n", maxvaltmp);
	
    for (i = 0; i < num_elements; i++)
    {   
		indata[i] = outdata[i];
	}

//	cnt = 0;
//    for (i = 0; i < num_elements; i++)
//    {   
//		if (indata[i] >0)
//			cnt ++;
//	}
//	printf("%d\n", cnt);
	
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
	if (outdata) {delete [] outdata; outdata = 0;}	
	if (indata_sz) {delete [] indata_sz; indata_sz = 0;}	
	
}


// binary erode
// input: img3d, se
// output: img3d

template <class T> void bwErode(Vol3DSimple <T> *img3d,  Strel *se)
{
    Neighborhood *nh = new Neighborhood;
    NeighborhoodWalker *nh_walker = new NeighborhoodWalker;
	V3DLONG num_elements;
	V3DLONG i,j;
	V3DLONG indata_dims = 3;

	
	T *indata = img3d->getData1dHandle();
	V3DLONG *indata_sz = new V3DLONG [3];
	T *outdata;

	indata_sz[0]= img3d->sz0();
	indata_sz[1]= img3d->sz1();
	indata_sz[2]= img3d->sz2();
	
	nh = se->convertStrelToNeighbor();
    nh_walker = new NeighborhoodWalker(nh, indata_sz, indata_dims, 0U);

	num_elements = img3d->sz0() * img3d->sz1() * img3d->sz2();
		
	outdata = new T [num_elements];
	
    for (i = 0; i < num_elements; i++)
    {   
		outdata[i] = 1;
		
		nh_walker->setWalkerLocation(i);
		while (nh_walker->getNextInboundsNeighbor(&j, NULL))
		{
			if (indata[j]==0)
			{
				outdata[i] = 0; break;
			}
		}
    }

    for (i = 0; i < num_elements; i++)
    {   
		indata[i] = outdata[i];
	}
	
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
	if (outdata) {delete [] outdata; outdata = 0;}	
	if (indata_sz) {delete [] indata_sz; indata_sz = 0;}	
	
}


// indata is 3d
template <class T> void bwErode(T *img3d, long *sz, Strel *se)
{
    Neighborhood *nh = new Neighborhood;
    NeighborhoodWalker *nh_walker = new NeighborhoodWalker;
	V3DLONG len;
	V3DLONG i,j;
    
//	V3DLONG indata_dims = 3;	
//	V3DLONG *indata_sz = new V3DLONG [indata_dims];
//	indata_sz[0]= len0;
//	indata_sz[1]= len1;
//	indata_sz[2]= len2;

	
	nh = se->convertStrelToNeighbor();
    nh_walker = new NeighborhoodWalker(nh, sz, 3, 0U);
	
	len = sz[0]*sz[1]*sz[2];
	
	T *outdata = new T [len];
	
    for (i = 0; i < len; i++)
    {   
		outdata[i] = 1;
		
		nh_walker->setWalkerLocation(i);
		while (nh_walker->getNextInboundsNeighbor(&j, NULL))
		{
			if (img3d[j]==0)
			{
				outdata[i] = 0; break;
			}
		}
    }
	
    for (i = 0; i < len; i++)
    {   
		img3d[i] = outdata[i];
	}
	
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
	if (outdata) {delete [] outdata; outdata = 0;}	
	
}


// indata is 3d
template <class T> void bwErode(T *indata,  V3DLONG len0, V3DLONG len1, V3DLONG len2, Strel *se)
{
    Neighborhood *nh = new Neighborhood;
    NeighborhoodWalker *nh_walker = new NeighborhoodWalker;
	V3DLONG num_elements;
	V3DLONG i,j;
	V3DLONG indata_dims = 3;	
	
	V3DLONG *indata_sz = new V3DLONG [indata_dims];
	T *outdata;
	
	indata_sz[0]= len0;
	indata_sz[1]= len1;
	indata_sz[2]= len2;
	
	nh = se->convertStrelToNeighbor();
    nh_walker = new NeighborhoodWalker(nh, indata_sz, indata_dims, 0U);
	
	num_elements = len0 * len1 * len2;
	
	outdata = new T [num_elements];
	
    for (i = 0; i < num_elements; i++)
    {   
		outdata[i] = 1;
		
		nh_walker->setWalkerLocation(i);
		while (nh_walker->getNextInboundsNeighbor(&j, NULL))
		{
			if (indata[j]==0)
			{
				outdata[i] = 0; break;
			}
		}
    }
	
    for (i = 0; i < num_elements; i++)
    {   
		indata[i] = outdata[i];
	}
	
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
	if (outdata) {delete [] outdata; outdata = 0;}	
	if (indata_sz) {delete [] indata_sz; indata_sz = 0;}	
	
}


// binary erode
// input: img2d, se
// output: img2d

template <class T> void bwErode(Image2DSimple <T> *img2d,  Strel *se)
{
    Neighborhood *nh = new Neighborhood;
    NeighborhoodWalker *nh_walker = new NeighborhoodWalker;
	V3DLONG num_elements;
	V3DLONG i,j;
	V3DLONG indata_dims = 2;

	
	T *indata = img2d->getData1dHandle();
	V3DLONG *indata_sz = new V3DLONG [2];
	T *outdata;

	indata_sz[0]= img2d->sz0();
	indata_sz[1]= img2d->sz1();

	nh = se->convertStrelToNeighbor();
    nh_walker = new NeighborhoodWalker(nh, indata_sz, indata_dims, 0U);

	num_elements = img2d->sz0() * img2d->sz1();
		
	outdata = new T [num_elements];
	
    for (i = 0; i < num_elements; i++)
    {   
		outdata[i] = 1;
		
		nh_walker->setWalkerLocation(i);
		while (nh_walker->getNextInboundsNeighbor(&j, NULL))
		{
			if (indata[j]==0)
			{
				outdata[j] = 0; break;
			}
		}
    }

    for (i = 0; i < num_elements; i++)
    {   
		indata[i] = outdata[i];
	}
	
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
	if (outdata) {delete [] outdata; outdata = 0;}	
	if (indata_sz) {delete [] indata_sz; indata_sz = 0;}	
	
}



template <class T> void bwOpen(T *img3d, long *sz, Strel *se)
{
	
	bwErode(img3d, sz, se);
	bwDilate(img3d, sz, se);	
}


template <class T> void bwOpen(Vol3DSimple<T> *img3d, Strel *se)
{
	
	bwErode(img3d, se);
	bwDilate(img3d, se);	
}

template <class T> void bwOpen(Image2DSimple<T> *img2d, Strel *se)
{	
	bwErode(img2d, se);
	bwDilate(img2d, se);	
}


// binary close for 2d image (i.e, first dilate then erode, using the same se)
// input: img3d, se
// output: img3d

template <class T> void bwClose(Vol3DSimple<T> *img3d, Strel *se)
{
	
	bwDilate(img3d, se);
	bwErode(img3d, se);	
}

// binary close for 2d image (i.e, first dilate then erode, using the same se)
// input: img2d, se
// output: img2d

template <class T> void bwClose(Image2DSimple<T> *img2d, Strel *se)
{	
	bwDilate(img2d, se);
	bwErode(img2d, se);	
}



#endif

