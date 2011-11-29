#ifndef EDGE_DETECTION_H_H
#define EDGE_DETECTION_H_H

#include <iostream>
#include <vector>
#include <cmath>

#include "basic_memory.cpp"
#include "v3d_basicdatatype.h"

//#include "utils.h"
using namespace std;

template <class T> int computeEdgeGrid(V3DLONG * (&grids)[3], V3DLONG &gridnum, V3DLONG gsz[3], T * inimg1d, V3DLONG sz[3], double sigma, int r, double theta);

template <class T1, class T2> bool computeGaussian(T2 * &outimg1d, T1 * inimg1d,V3DLONG sz[3], double sigma, int r);
template <class T1, class T2> bool computeThreshold(T2 * &outimg1d, T1 * inimg1d,V3DLONG sz[3], double theta);
template <class T> bool computeThreshold(T * &inimg1d,V3DLONG sz[3], double theta);
template <class T1, class T2> bool computeGradience(T2 * &outimg1d, T1 * inimg1d, V3DLONG sz[3], double theta);
template <class T1, class T2> int computeEdgeGrid(V3DLONG * (&grids)[3], V3DLONG &gridnum, V3DLONG gsz[3], T2 * inimg1d, T1 * eimg1d, V3DLONG sz[3], double theta);

template <class T> int computeEdgeGrid(V3DLONG * (&grids)[3], V3DLONG &gridnum, V3DLONG gsz[3], T * inimg1d, V3DLONG sz[3], double sigma, int r, double theta)
{
	if(!inimg1d || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0 || gsz[0] <= 0 || gsz[1] <= 0 || gsz[2] <= 0 || sigma <= 0.0 || r % 2 != 1 || theta <= 0.0) return -1;

	double * outimg1d = 0;
	double * eimg1d = 0;
	if(computeGaussian(outimg1d, inimg1d, sz, sigma, r) == -1) return -1;
	//if(computeThreshold(outimg1d, sz, theta) == -1) return -1;
	if(computeGradience(eimg1d, outimg1d, sz, theta) == -1) return -1;
	if(computeEdgeGrid(grids, gridnum, gsz, inimg1d, eimg1d, sz, theta) == -1) return -1;
	
	if(outimg1d) {delete outimg1d; outimg1d = 0;}
	if(eimg1d) {delete eimg1d; eimg1d = 0;}
	return 1;
}

template <class T1, class T2> int computeEdgeGrid(V3DLONG * (&grids)[3], V3DLONG &gridsnum, V3DLONG gsz[3], T2 * inimg1d, T1 * eimg1d, V3DLONG sz[3], double theta)
{
	if(!eimg1d || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0 || gsz[0] <= 0 || gsz[1] <= 0 || gsz[2] <= 0) return -1;

	gridsnum = gsz[0] * gsz[1] * gsz[2];

	T1 *** eimg3d = 0;
	T2 *** inimg3d = 0;
	try
	{
		grids[0] =  new V3DLONG[gridsnum];
		grids[1] =  new V3DLONG[gridsnum];
		grids[2] =  new V3DLONG[gridsnum];
		new3dpointer(eimg3d, sz[0], sz[1], sz[2], eimg1d);
		new3dpointer(inimg3d, sz[0], sz[1], sz[2], inimg1d);
	}
	catch(...)
	{
		if(grids[0]) {delete[] grids[0]; grids[0] = 0;}
		if(grids[1]) {delete[] grids[1]; grids[1] = 0;}
		if(grids[2]) {delete[] grids[2]; grids[2] = 0;}
		if(eimg3d) delete3dpointer(eimg3d, sz[0], sz[1], sz[2]);
		if(inimg3d) delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
		return 0;
	}

	V3DLONG dx = sz[0] / gsz[0];
	V3DLONG dy = sz[1] / gsz[1];
	V3DLONG dz = sz[2] / gsz[2];
	
	cout<<"dx: "<<dx<<" dy: "<<dy<<" dz: "<<dz<<endl;

	V3DLONG i,j,k;
	V3DLONG ii,jj,kk;
	V3DLONG bx, by, bz, ex, ey, ez;
	V3DLONG mx, my, mz;

	int count=0;
	for(kk = 0; kk < gsz[2]; kk++)
	{
		bz = kk * dz;
		ez = bz + dz;
		if(kk == gsz[2] - 1) ez = sz[2];

		for(jj = 0; jj < gsz[1]; jj++)
		{
			by = jj * dy;
			ey = by + dy;
			if(jj == gsz[1] - 1) ey = sz[1];

			for(ii = 0; ii < gsz[0]; ii++)
			{
				bx = ii * dx;
				ex = bx + dx;
				if(ii == gsz[0] - 1) ex = sz[0];

				mx = bx; my = by; mz = bz;
				for(k = bz; k < ez; k++)
				{
					for(j = by; j < ey; j++)
					{
						for(i = bx; i < ex; i++)
						{
							if(eimg3d[k][j][i] > eimg3d[mz][my][mx])
							{
								mz = k;
								my = j;
								mx = i;
							}
						}
					}
				}
				if(eimg3d[mz][my][mx] >= 1.0 && inimg3d[mz][my][mx] >= theta)
				{
					grids[2][count] = mz;// (bz + ez) /2; // mz;
					grids[1][count] = my;//(by + ey) /2; // my;
					grids[0][count] = mx;//(bx + ex) /2; // mx;
					count++;
				}
			}
		}
	}
	gridsnum = count;

	if(eimg3d) delete3dpointer(eimg3d, sz[0], sz[1], sz[2]);
	if(inimg3d) delete3dpointer(eimg3d, sz[0], sz[1], sz[2]);

	return 1;
}

template <class T1, class T2> bool computeGradience(T2 * &outimg1d, T1 * inimg1d, V3DLONG sz[3], double theta)
{
	if(!inimg1d) return false;

	V3DLONG i,j,k;

	T1 *** inimg3d = 0;
	T2 *** outimg3d = 0;

	try
	{
		outimg1d = new T2[sz[0]*sz[1]*sz[2]];
		new3dpointer(outimg3d, sz[0], sz[1], sz[2], outimg1d);
		new3dpointer(inimg3d, sz[0], sz[1], sz[2], inimg1d);
	}
	catch(...)
	{
		if(outimg1d) {delete[] outimg1d; outimg1d = 0;}
		if(outimg3d) delete3dpointer(outimg3d, sz[0], sz[1], sz[2]);
		if(inimg3d)  delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
		return false;
	}


	T2 dx, dy, dz;
	for(k = 0; k < sz[2]; k++)
	{
		for(j = 0; j < sz[1]; j++)
		{
			for(i = 0; i < sz[0]; i++)
			{
				if(inimg3d[k][j][i] < theta){outimg3d[k][j][i] = (T2) 0.0; continue;}
				dx = 0; dy = 0; dz = 0;

				int count = 0;
				if(k - 1 >= 0) {dz += inimg3d[k][j][i] - inimg3d[k-1][j][i]; count++;}
				if(k + 1 < sz[2]) {dz += inimg3d[k+1][j][i] - inimg3d[k][j][i]; count++;}
				dz /= count;

				count = 0;
				if(j - 1 >= 0) {dy += inimg3d[k][j][i] - inimg3d[k][j-1][i]; count++;}
				if(j + 1 < sz[1]) {dy += inimg3d[k][j+1][i] - inimg3d[k][j][i]; count++;}
				dy /= count;

				count = 0;
				if(i - 1 >= 0) {dx += inimg3d[k][j][i] - inimg3d[k][j][i-1]; count++;}
				if(i + 1 < sz[0]) {dx += inimg3d[k][j][i+1] - inimg3d[k][j][i]; count++;}
				dx /= count;

				outimg3d[k][j][i] =(T2) sqrt(dx*dx + dy*dy + dz*dz);
			}
		}
	}

	if(outimg3d) delete3dpointer(outimg3d, sz[0], sz[1], sz[2]);
	if(inimg3d)  delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
	return true;
}

template <class T1, class T2> bool computeThreshold(T2 * &outimg1d, T1 * inimg1d,V3DLONG sz[3], double theta)
{
	if(outimg1d != 0 || inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG size = sz[0]*sz[1]*sz[2];
	try
	{
		outimg1d = new T2[size];
	}
	catch(...)
	{
		if(outimg1d) {delete outimg1d; outimg1d = 0;}
		return false;
	}
	V3DLONG i;
	for(i = 0; i < size; i++) outimg1d[i] = (inimg1d[i] < theta) ? 0 : inimg1d[i];
}

template <class T> bool computeThreshold(T * &inimg1d,V3DLONG sz[3], double theta)
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG size = sz[0]*sz[1]*sz[2];
	V3DLONG i;
	for(i = 0; i < size; i++) inimg1d[i] = (inimg1d[i] < theta) ? 0 : inimg1d[i];
}

template <class T1, class T2> bool computeGaussian(T2 * &outimg1d, T1 * inimg1d,V3DLONG sz[3], double sigma, int r)
{
	if(!inimg1d || sigma < 0.0 || r%2 != 1) return false;
	V3DLONG i, j, k;
	V3DLONG ii, jj, kk;
	V3DLONG size = r*r*r;

	T1 *** indata3d = 0 ;
	T2 *** outdata3d = 0;

	double * gauss1d = 0;
	double *** gauss3d = 0; 

	try
	{
		outimg1d = new T2[sz[0]*sz[1]*sz[2]];
		gauss1d = new double[size];

		new3dpointer(indata3d, sz[0], sz[1], sz[2], inimg1d);
		new3dpointer(outdata3d, sz[0], sz[1], sz[2], outimg1d);
		new3dpointer(gauss3d, r, r, r, gauss1d);
	}
	catch(...)
	{
		cerr<<"Not enough memory!"<<endl;
		if(outimg1d) {delete[] outimg1d; outimg1d = 0;}
		if(gauss1d)  {delete[] gauss1d; gauss1d = 0;}
		if(indata3d) delete3dpointer(indata3d, sz[0],sz[1],sz[2]);
		if(outdata3d) delete3dpointer(outdata3d, sz[0],sz[1],sz[2]);
		if(gauss3d) delete3dpointer(gauss3d, r, r, r);
		return false;
	}

	int rr = r/2;
	double PI = 3.1415926;
	double A = 1.0; //1.0/(2.0*PI*sigma*sigma);
	double B = -1.0/(2.0*sigma*sigma);

	double C = 0.0;
	for(k = 0; k < r; k++)
	{
		for(j = 0; j < r; j++)
		{
			for(i = 0; i < r; i++)
			{
				if(k <= rr && j <= rr && i <= rr){
					double D = (k - rr) * (k - rr) + (j - rr)*(j - rr) + (i - rr)*(i - rr); 
					gauss3d[k][j][i] = A * exp(B * D); 
				}
				else
				{
					V3DLONG k2 = k , j2 = j, i2 = i;
					if(k > rr) k2 = r - k - 1;
					if(j > rr) j2 = r - j - 1;
					if(i > rr) i2 = r - i - 1;
					gauss3d[k][j][i] = gauss3d[k2][j2][i2];
				}
				C += gauss3d[k][j][i];
			}
		}
	}

	// uniform 
	try
	{
		for(i = 0; i < size; i++) 
		{
			gauss1d[i] /= C;
		}
	}
	catch(...)
	{
		cout<<"divided by zero in gausian matrix"<<endl;
	}

	// main loop
	for(k = 0; k < sz[2]; k++)
	{
		for(j = 0; j < sz[1]; j++)
		{
			for(i = 0; i < sz[0]; i++)
			{
				double sumi = 0.0;
				double sumg = 1.0;
				for(kk = k - rr; kk <= k + rr; kk++)
				{
					for(jj = j - rr; jj <= j + rr; jj++)
					{
						for(ii = i - rr; ii <= i + rr; ii++)
						{
							if( ii >= 0 && jj >= 0 && kk >= 0 &&
									ii < sz[0] && jj < sz[1] && kk < sz[2])
								sumi += indata3d[kk][jj][ii] * gauss3d[kk - k + rr][jj - j + rr][ii - i + rr];
							else
							{
								sumg -= gauss3d[kk - k + rr][jj - j + rr][ii - i + rr];
							}
						}
					}
				}
				outdata3d[k][j][i] =(T2)(sumi /sumg);
			}
		}
	}

	if(gauss1d)  {delete[] gauss1d; gauss1d = 0;}
	if(indata3d) delete3dpointer(indata3d, sz[0],sz[1],sz[2]);
	if(outdata3d) delete3dpointer(outdata3d, sz[0],sz[1],sz[2]);
	if(gauss3d) delete3dpointer(gauss3d, r, r, r);
	return true;
}

#endif
