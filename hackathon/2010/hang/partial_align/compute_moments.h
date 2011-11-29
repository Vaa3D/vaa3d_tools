#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <queue>
#include <fstream>

#include "img_definition.h"
//#include "../../../v3d_main/jba/c++/jba_mainfunc.h"
//#include "../../../v3d_main/jba/c++/wkernel.h"

template <class T> bool regularMoment(double & valMoments,T *inimg1d, V3DLONG sz[3], V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, int pp, int qq, int rr)
{
	if (!inimg1d || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0)
		return false;
	
	V3DLONG i,j,k;
	
	T *** data3d = 0; new3dpointer(data3d, sz[0], sz[1], sz[2], inimg1d);
	V3DLONG xmax = sz[0], ymax=sz[1], zmax=sz[2];
	
	V3DLONG ib=x0-r, jb=y0-r, kb=z0-r, ie=x0+r, je=y0+r, ke=z0+r;
	if (ib<0) ib=0;
	if (jb<0) jb=0;
	if (kb<0) kb=0;
	if (ie>xmax-1) ie=xmax-1;
	if (je>ymax-1) je=ymax-1;
	if (ke>zmax-1) ke=zmax-1;
	//  printf("%d %d %d %d %d %d\n", ib,ie,jb,je,kb,ke);
	
	double r2 = r*r;
	double dx,dy,dz;
	double s0=0, s1=0; //, s2=0;
	double pdz, pdy, pdx, psum;
	
	for (k=kb;k<=ke;k++)
	{
		dz = (double(k)-z0)/r;
		pdz = pow(dz,rr);
		for (j=jb;j<=je;j++)
		{
			dy = (double(j)-y0)/r;
			pdy = pow(dy,qq);
			for (i=ib;i<=ie;i++)
			{
				dx = (double(i)-x0)/r;
				if (dx*dx+dy*dy+dz*dz>r2) continue; //so it is spherical
				
				pdx=pow(dx,pp);
				
				psum = pdx*pdy*pdz;
				s1 += data3d[k][j][i] * psum;
				s0 += psum;
				//		s2 += data3d[k][j][i];
			}
		}
	}
	
	valMoments = s1;
	//valMoments = s1/s0;
	//printf("s1=%6.5f\ts0=%6.5f\tv=%6.5f\n", s1, s0, valMoments);
	return true;
}



template <class T> bool regularMoment(double & valMoments, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, int pp, int qq, int rr)
{
	if (!img || !img->valid())
		return false;
	
	V3DLONG i,j,k;
	
	T *** data3d = img->getData3dHandle();
	V3DLONG xmax = img->sz0(), ymax=img->sz1(), zmax=img->sz2();
	
	V3DLONG ib=x0-r, jb=y0-r, kb=z0-r, ie=x0+r, je=y0+r, ke=z0+r;
	if (ib<0) ib=0;
	if (jb<0) jb=0;
	if (kb<0) kb=0;
	if (ie>xmax-1) ie=xmax-1;
	if (je>ymax-1) je=ymax-1;
	if (ke>zmax-1) ke=zmax-1;
	//  printf("%d %d %d %d %d %d\n", ib,ie,jb,je,kb,ke);
	
	double r2 = r*r;
	double dx,dy,dz;
	double s0=0, s1=0; //, s2=0;
	double pdz, pdy, pdx, psum;
	
	for (k=kb;k<=ke;k++)
	{
		dz = (double(k)-z0)/r;
		pdz = pow(dz,rr);
		for (j=jb;j<=je;j++)
		{
			dy = (double(j)-y0)/r;
			pdy = pow(dy,qq);
			for (i=ib;i<=ie;i++)
			{
				dx = (double(i)-x0)/r;
				if (dx*dx+dy*dy+dz*dz>r2) continue; //so it is spherical
				
				pdx=pow(dx,pp);
				
				psum = pdx*pdy*pdz;
				s1 += data3d[k][j][i] * psum;
				s0 += psum;
				//		s2 += data3d[k][j][i];
			}
		}
	}
	
	valMoments = s1;
	//valMoments = s1/s0;
	//printf("s1=%6.5f\ts0=%6.5f\tv=%6.5f\n", s1, s0, valMoments);
	return true;
}

template <class T> bool computeGMI(Vector1DSimple<double> & momentVec, T *inimg1d, V3DLONG sz[3], V3DLONG x0, V3DLONG y0, V3DLONG z0, int r)
{
    if (!(momentVec.valid()) || momentVec.sz0()<5 ||
	    !inimg1d || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0 ||
	    x0<0 || x0>= sz[0] || y0<0 || y0>=sz[1] || z0<0 || z0>=sz[2] ||
		r<0)
		return false;
	
	double c200; regularMoment(c200, inimg1d, sz, x0, y0, z0, r, 2, 0, 0);
	double c020; regularMoment(c020, inimg1d, sz, x0, y0, z0, r, 0, 2, 0);
	double c002; regularMoment(c002, inimg1d, sz, x0, y0, z0, r, 0, 0, 2);
	double c110; regularMoment(c110, inimg1d, sz, x0, y0, z0, r, 1, 1, 0);
	double c101; regularMoment(c101, inimg1d, sz, x0, y0, z0, r, 1, 0, 1);
	double c011; regularMoment(c011, inimg1d, sz, x0, y0, z0, r, 0, 1, 1);
	double c000; regularMoment(c000, inimg1d, sz, x0, y0, z0, r, 0, 0, 0);
	
    double * t = momentVec.getData1dHandle();
	t[0] = inimg1d[z0 * sz[0] * sz[1] + y0 * sz[0] + x0];
	t[1] = c000; //0.0;
	t[2] = c200+c020+c002; //t[2]=pow(t[2]/3, 0.5);
	t[3] = c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110;// t[3]=pow(t[3]/3, 0.25);
	t[4] = c200*c020*c002-c002*c110*c110+2*c110*c101*c011-c020*c101*c101-c200*c011*c011; // t[4]=pow(t[4]/5, 0.125);
	
    return true;
}

template <class T> bool computeGMI(Vector1DSimple<double> & momentVec, Vol3DSimple<T> *data3d, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r)
{
    if (!(momentVec.valid()) || momentVec.sz0()<5 ||
	    !data3d || !data3d->valid() ||
	    x0<0 || x0>=data3d->sz0() || y0<0 || y0>=data3d->sz1() || z0<0 || z0>=data3d->sz2() ||
		r<0)
		return false;
	
	double c200; regularMoment(c200, data3d, x0, y0, z0, r, 2, 0, 0);
	double c020; regularMoment(c020, data3d, x0, y0, z0, r, 0, 2, 0);
	double c002; regularMoment(c002, data3d, x0, y0, z0, r, 0, 0, 2);
	double c110; regularMoment(c110, data3d, x0, y0, z0, r, 1, 1, 0);
	double c101; regularMoment(c101, data3d, x0, y0, z0, r, 1, 0, 1);
	double c011; regularMoment(c011, data3d, x0, y0, z0, r, 0, 1, 1);
	double c000; regularMoment(c000, data3d, x0, y0, z0, r, 0, 0, 0);
	
    double * t = momentVec.getData1dHandle();
	t[0] = data3d->getData3dHandle()[z0][y0][x0];
	t[1] = c000; //0.0;
	t[2] = c200+c020+c002; t[2]=pow(t[2]/3, 0.5);
	t[3] = c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110; t[3]=pow(t[3]/3, 0.25);
	t[4] = c200*c020*c002-c002*c110*c110+2*c110*c101*c011-c020*c101*c101-c200*c011*c011;  t[4]=pow(t[4]/5, 0.125);
	
    return true;
}

/* template <class T> bool regularMoment(double & valMoments, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, int pp, int qq, int rr, const KernelSet * ks)
{
	if (!img || !img->valid())
		return false;
	double *** cur_kernel3d = ks->kernel->getData3dHandle()[rr][qq][pp]->data3d;
	if (!cur_kernel3d)
	{
		fprintf(stderr, "Invalid parameters of the regularMoment() with kernel. __line %d\n", __LINE__);
		return false;
	}
	
	V3DLONG i,j,k;
	
	T *** data3d = img->getData3dHandle();
	V3DLONG xmax = img->sz0(), ymax=img->sz1(), zmax=img->sz2();
	
	V3DLONG ib=x0-r, jb=y0-r, kb=z0-r, ie=x0+r, je=y0+r, ke=z0+r;
	if (ib<0) ib=0;
	if (jb<0) jb=0;
	if (kb<0) kb=0;
	if (ie>xmax-1) ie=xmax-1;
	if (je>ymax-1) je=ymax-1;
	if (ke>zmax-1) ke=zmax-1;
	//  printf("%d %d %d %d %d %d\n", ib,ie,jb,je,kb,ke);
	
	
	double r2 = r*r;
	int dx,dy,dz;
	double s0=0, s1=0, s2=0;
	
	V3DLONG i2,j2,k2;
	V3DLONG k2b = (k<r)?(V3DLONG(r)-k):0;
	V3DLONG j2b = (j<r)?(V3DLONG(r)-j):0;
	V3DLONG i2b = (i<r)?(V3DLONG(r)-i):0;
	
	for (k=kb, k2=k2b;k<=ke;k++, k2++)
	{
		dz = k-z0;
		for (j=jb, j2=j2b;j<=je;j++, j2++)
		{
			dy = j-y0;
			for (i=ib, i2=i2b;i<=ie;i++, i2++)
			{
				dx = i-x0;
				if (dx*dx+dy*dy+dz*dz>r2) continue; //so it is spherical
				
				//	    s1 += data3d[k][j][i] * pow(dx, pp) * pow(dy, qq) * pow(dz, rr);
				s1 += data3d[k][j][i] * cur_kernel3d[k2][j2][i2];
				//	    s0 += cur_kernel3d[k2][j2][i2];
				//		s2 += data3d[k][j][i];
			}
		}
	}
	
	valMoments = s1;
	return true;
}
*/

template <class T> bool regularMoment(double & valMoments, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, int pp, int qq, int rr, const double *** cur_kernel3d)
{
	if (!img || !img->valid())
		return false;
	//double *** cur_kernel3d = ks->kernel->getData3dHandle()[rr][qq][pp]->data3d;
	if (!cur_kernel3d)
	{
		fprintf(stderr, "Invalid parameters of the regularMoment() with kernel. __line %d\n", __LINE__);
		return false;
	}
	
	V3DLONG i,j,k;
	
	T *** data3d = img->getData3dHandle();
	V3DLONG xmax = img->sz0(), ymax=img->sz1(), zmax=img->sz2();
	
	V3DLONG ib=x0-r, jb=y0-r, kb=z0-r, ie=x0+r, je=y0+r, ke=z0+r;
	if (ib<0) ib=0;
	if (jb<0) jb=0;
	if (kb<0) kb=0;
	if (ie>xmax-1) ie=xmax-1;
	if (je>ymax-1) je=ymax-1;
	if (ke>zmax-1) ke=zmax-1;
	//  printf("%d %d %d %d %d %d\n", ib,ie,jb,je,kb,ke);
	
	
	double r2 = r*r;
	int dx,dy,dz;
	double s0=0, s1=0, s2=0;
	
	V3DLONG i2,j2,k2;
	V3DLONG k2b = (k<r)?(V3DLONG(r)-k):0;
	V3DLONG j2b = (j<r)?(V3DLONG(r)-j):0;
	V3DLONG i2b = (i<r)?(V3DLONG(r)-i):0;
	
	for (k=kb, k2=k2b;k<=ke;k++, k2++)
	{
		dz = k-z0;
		for (j=jb, j2=j2b;j<=je;j++, j2++)
		{
			dy = j-y0;
			for (i=ib, i2=i2b;i<=ie;i++, i2++)
			{
				dx = i-x0;
				if (dx*dx+dy*dy+dz*dz>r2) continue; //so it is spherical
				
				//	    s1 += data3d[k][j][i] * pow(dx, pp) * pow(dy, qq) * pow(dz, rr);
				s1 += data3d[k][j][i] * cur_kernel3d[k2][j2][i2];
				//	    s0 += cur_kernel3d[k2][j2][i2];
				//		s2 += data3d[k][j][i];
			}
		}
	}
	
	valMoments = s1;
	return true;
}

/*template <class T> bool computeGMI(Vector1DSimple<double> & momentVec, Vol3DSimple<T> *data3d, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, const KernelSet * ks)
{
    if (!(momentVec.valid()) || momentVec.sz0()<5 ||
	    !data3d || !data3d->valid() ||
	    x0<0 || x0>=data3d->sz0() || y0<0 || y0>=data3d->sz1() || z0<0 || z0>=data3d->sz2() ||
		r<0)
		return false;
	
	double c200; regularMoment(c200, data3d, x0, y0, z0, r, 2, 0, 0, ks);
	double c020; regularMoment(c020, data3d, x0, y0, z0, r, 0, 2, 0, ks);
	double c002; regularMoment(c002, data3d, x0, y0, z0, r, 0, 0, 2, ks);
	double c110; regularMoment(c110, data3d, x0, y0, z0, r, 1, 1, 0, ks);
	double c101; regularMoment(c101, data3d, x0, y0, z0, r, 1, 0, 1, ks);
	double c011; regularMoment(c011, data3d, x0, y0, z0, r, 0, 1, 1, ks);
	double c000; regularMoment(c000, data3d, x0, y0, z0, r, 0, 0, 0, ks);
	
    double * t = momentVec.getData1dHandle();
	t[0] = data3d->getData3dHandle()[z0][y0][x0];
	t[1] = c000;
	t[2] = c200+c020+c002;
	t[3] = c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110;
	t[4] = c200*c020*c002-c002*c110*c110+2*c110*c101*c011-c020*c101*c101-c200*c011*c011;
	
    return true;
}
*/

template <class T> bool computeGMI(Vector1DSimple<double> & momentVec, Vol3DSimple<T> *data3d, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r, const double *** cur_kernel3d)
{
    if (!(momentVec.valid()) || momentVec.sz0()<5 ||
	    !data3d || !data3d->valid() ||
	    x0<0 || x0>=data3d->sz0() || y0<0 || y0>=data3d->sz1() || z0<0 || z0>=data3d->sz2() ||
		r<0)
		return false;
	
	double c200; regularMoment(c200, data3d, x0, y0, z0, r, 2, 0, 0, cur_kernel3d);
	double c020; regularMoment(c020, data3d, x0, y0, z0, r, 0, 2, 0, cur_kernel3d);
	double c002; regularMoment(c002, data3d, x0, y0, z0, r, 0, 0, 2, cur_kernel3d);
	double c110; regularMoment(c110, data3d, x0, y0, z0, r, 1, 1, 0, cur_kernel3d);
	double c101; regularMoment(c101, data3d, x0, y0, z0, r, 1, 0, 1, cur_kernel3d);
	double c011; regularMoment(c011, data3d, x0, y0, z0, r, 0, 1, 1, cur_kernel3d);
	double c000; regularMoment(c000, data3d, x0, y0, z0, r, 0, 0, 0,cur_kernel3d);
	
    double * t = momentVec.getData1dHandle();
	t[0] = data3d->getData3dHandle()[z0][y0][x0];
	t[1] = c000; //0.0;
	t[2] = c200+c020+c002;
	t[3] = c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110;
	t[4] = c200*c020*c002-c002*c110*c110+2*c110*c101*c011-c020*c101*c101-c200*c011*c011;
	
    return true;
}

//compute the average at different radii as a feature vector. 070515

template <class T> bool computeCircularMeanVector(Vector1DSimple<double> & meanVec, Vector1DSimple<double> & cntVec, Vol3DSimple<T> *img, V3DLONG x0, V3DLONG y0, V3DLONG z0, int r)
{
    if (!(meanVec.valid()) || meanVec.sz0()!=r+1 ||
	    !(cntVec.valid()) || cntVec.sz0()!=r+1 ||
	    !img || !img->valid() ||
	    x0<0 || x0>=img->sz0() || y0<0 || y0>=img->sz1() || z0<0 || z0>=img->sz2() ||
		r<0)
		return false;
	
    double * t = meanVec.getData1dHandle();
    double * cnt = cntVec.getData1dHandle();
	V3DLONG i,j,k;
	
	for (i=0;i<=r;i++)
	{
		t[i] = 0;
		cnt[i] = 0;
	}
	
	T *** data3d = img->getData3dHandle();
	V3DLONG xmax = img->sz0(), ymax=img->sz1(), zmax=img->sz2();
	
	V3DLONG ib=x0-r, jb=y0-r, kb=z0-r, ie=x0+r, je=y0+r, ke=z0+r;
	if (ib<0) ib=0;
	if (jb<0) jb=0;
	if (kb<0) kb=0;
	if (ie>xmax-1) ie=xmax-1;
	if (je>ymax-1) je=ymax-1;
	if (ke>zmax-1) ke=zmax-1;
	//  printf("%d %d %d %d %d %d\n", ib,ie,jb,je,kb,ke);
	
	double dx,dy,dz;
	double s0=0, s1=0, s2=0;
	double pdz, pdy, pdx, psum;
	
	for (k=kb;k<=ke;k++)
	{
		dz = (double(k)-z0);
		pdz = dz*dz;
		for (j=jb;j<=je;j++)
		{
			dy = (double(j)-y0);
			pdy = dy*dy;
			for (i=ib;i<=ie;i++)
			{
				dx = (double(i)-x0);
				pdx = dx*dx;
				
				psum = sqrt(pdx+pdy+pdz);
				if (psum>r) continue; //so it is spherical
				
				V3DLONG cur_index=V3DLONG(round(psum));
				if (data3d[k][j][i]>1e-8)
				{
					t[cur_index] += data3d[k][j][i];
					cnt[cur_index] += 1;
				}
			}
		}
	}
	
	const double pi=3.141592635;
	for (i=0;i<=r;i++)
	{
		t[i] = (cnt[i]>0) ? (t[i]/cnt[i]) : 0.0;
		cnt[i] /= 4*pi*r*r;
		//printf("%d\t%5.3f\t%5.3f\n", i, t[i], cnt[i]);
	}
	return true;
}


