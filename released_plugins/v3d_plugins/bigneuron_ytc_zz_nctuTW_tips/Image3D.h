#ifndef _GEO_H
#include "Geo.h"
#endif

#ifndef _IPT_H
#include "IPT.h"
#endif

#ifndef UNION_FIND_H
#include "UnionFind.h"
#endif

#ifndef _IMAGE3D_H
#define _IMAGE3D_H

#define PI = 3.14159265358979323846


#include <vector>
#include <cmath>
#include <queue>
#include <list>

//Image3D-1
void BranchPoint2D_Iter2D( unsigned char* const inBuf, const int width, const int height, const int zSize, std::list<VectorCordinate >& Bps2D );
void Endpoint3D_Iter2D( unsigned char* const inBuf, const int width, const int height, const int zSize, std::vector<VectorCordinate >& endP );
void Binarize(unsigned char* inBuf, const int width, const int height, const int zSize);


//Image3D
void Endpoint3D( unsigned char* const inBuf, const int width, const int height, const int zSize, std::vector<VectorCordinate >& endP );
bool Endpoint3D( unsigned char* const inBuf, const int x, const int y, const int z, const int width, const int height, const int zSize );
void MaxCC_3D26( unsigned char* const inBuf, const int width, const int height, const int zSize, std::vector<unsigned int>& MaxCC26 );
void FindCCs(unsigned char* ImgBuf, const IPTHeader header, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs);
void FindCCs(unsigned char* ImgBuf, const int width, const int height, const int zSize, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs);
void FindCCs(unsigned char* ImgBuf, const int width, const int height, const int zSize, const int CC_THRESHOLD, const int GREY_RADIUS, std::vector< std::vector<int> >& CCs);
void FindCC2Ds(unsigned char* ImgBuf, const IPTHeader header, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs );
void FindCC2Ds(unsigned char* ImgBuf, const IPTHeader header, std::vector< std::vector<int> >& CCs, const int CC_THRESHOLD, const unsigned char GRAY_THRESHOLD );
void FindCC2Ds(unsigned char* ImgBuf, const int width, const int height, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs );
void FindCC2D4s(unsigned char* ImgBuf, const int width, const int height, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs );
bool SimplePoint(unsigned char* inBuf, const int x, const int y, const int z, const int width, const int height, const int zSize);
void BinaryDilation2D(unsigned char *imgBuf, const int width, const int height, const int R = 1 );
void GradientField2D(VectorCordinate*** Field, unsigned char* inBuf, const int width, const int height, const int zSize);
void GradientField3D(VectorCordinate*** Field, unsigned char* inBuf, const int width, const int height, const int zSize);
void GradientField2D_V2(VectorCordinate*** Field, unsigned char* inBuf, const int width, const int height, const int zSize);

void Skeleton2D( const unsigned char *input, unsigned char  *output, const int Xsize, const int Ysize );
void Skeletonization3D_Slicewise( unsigned char* inBuf, unsigned char* skeleBuf, const int width, const int height, const int zSize );

template<typename T>
void Binarize3D_sliceWise( T* ImgBuf, const int width, const int height, const int zSize, const int threshold )
{
	const int Area = width * height;
	for(int z=0; z<zSize; ++z)
	{
		for(int i=0; i<Area; ++i)
			if( ImgBuf[ z * Area + i ] < threshold )
				ImgBuf[ z * Area + i ] = 0;
	}

	return;
}

// This thinning function is modified from Richard Lin's code project IPToolBox //
void thinning_1frT( const unsigned char *input, unsigned char  *output, const int Xsize, const int Ysize );

void MedianFilter3D(unsigned char* ImgBuf, const int width, const int height, const int zSize, const int Radius = 1);
//BilateralFilter.h
//coded by Li Tsong-tse
//November, 6, 2008
void bilateralfilter(unsigned char *in, unsigned char *out, long height, long width, int domain=0, int range=0, int mask=5);

template <typename T>
void Image2DCCs( T* buf, const int width, const int height, const int THRESHOLD = 9)
{
	typedef unsigned long UL;

	const int Area = width * height;

	MyUnionFind3D UF;
	UF.UnionFindInitialize( Area );

	bool *visited = new bool [ Area ];
	for(int i=0; i<Area; ++i)
		visited[i] = false;

	for(int y=0; y<height; ++y)
	{
		for(int x=0; x<width; ++x)
		{
			UL pos1 = y * width + x;
			if( buf[ pos1 ] <= 0 )
				continue;

			for(int my = -1; my<= 1; ++my)
			{
				if( y + my < 0 || y + my >= height )
					continue;

				for(int mx = -1; mx <= 1; ++mx)
				{
					if( x+mx < 0 || x+mx >= width )
						continue;

					UL pos2 = (y+my) * width + (x+mx);

					if( buf[ pos2 ] > 0 )
					{
						UL id1 = UF.Find( pos1 );
						UL id2 = UF.Find( pos2 );
						if( id1 != id2 )
							UF.Union(id2, id1);
					}
				}
			}
		}
	}

	for(int i=0; i<Area; ++i)
		if( buf[i] > 0 )
			UF.Find(i);

	for(int i=0; i<Area; ++i)
	{
		if( buf[i] <= 0 )
			continue;

		UF.Size[i] = UF.Size[ UF.Find(i) ];
		if( UF.Size[i] < THRESHOLD )
			buf[i] = 0;
	}

	delete [] visited;
}

template <typename T>
void Image3DCCs( T* buf, const int width, const int height, const int zSize, const int THRESHOLD = 27)
{
	const int Area = width * height;
	const int Vol = Area * zSize;

	typedef unsigned long UL;

	MyUnionFind3D UF;
	UF.UnionFindInitialize( Vol );

	bool *visited = new bool [ Vol ];
	for(int i=0; i<Vol; ++i)
		visited[i] = false;

	for(int z=0; z<zSize; ++z)
	{
		for(int y=0; y<height; ++y)
		{
			for(int x=0; x<width; ++x)
			{
				UL pos1 = z * Area + y * width + x;
				if( buf[ pos1 ] <= 0 )
					continue;
				for(int mz = -1; mz<= 1; ++mz)
				{
					if( z + mz < 0 || z + mz >= zSize )
						continue;

					for(int my = -1; my<= 1; ++my)
					{
						if( y + my < 0 || y + my >= height )
							continue;

						for(int mx = -1; mx <= 1; ++mx)
						{
							if( x+mx < 0 || x+mx >= width )
								continue;

							UL pos2 = (z+mz) * Area + (y+my) * width + (x+mx);

							if( buf[ pos2 ] > 0 )
							{
								UL id1 = UF.Find( pos1 );
								UL id2 = UF.Find( pos2 );
								if( id1 != id2 )
									UF.Union(id2, id1);
							}
						}
					}
				}
			}
		}
	}

	for(int i=0; i<Vol; ++i)
		if( buf[i] > 0 )
			UF.Find(i);

	for(int i=0; i<Vol; ++i)
	{
		if( buf[i] <= 0 )
			continue;

		UF.Size[i] = UF.Size[ UF.Find(i) ];
		if( UF.Size[i] < THRESHOLD )
			buf[i] = 0;
	}

	delete [] visited;
}

#endif
