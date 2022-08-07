//histeq.cpp
//by Hanchuan Peng
//2006-2011

#include "histeq.h"

#include <stdio.h>

bool hist_eq_uint8(unsigned char * data1d, V3DLONG len)
{
	if (!data1d || len<=0)
	{
		printf("The input parameters are invalid in hist_eq_uint8().\n");
		return false;
	}
	
	V3DLONG NBIN=256;
	V3DLONG *h = new V3DLONG [NBIN]; 
	double *c = new double [NBIN]; 
	if (!h)
	{
		printf("Fail to allocate memory in hist_eq_uint8().\n");
		return false;
	}
	
	V3DLONG i;
	
	for (i=0;i<NBIN;i++) h[i]=0;
	
	for (i=0;i<len; i++)
	{
		h[data1d[i]]++;
	}
	
	c[0]=h[0];
	for (i=1;i<NBIN;i++) c[i] = c[i-1]+h[i];
	for (i=0;i<NBIN;i++) {c[i] /= c[NBIN-1]; c[i] *= (NBIN-1);}
	
	for (i=0;i<len;i++)
	{
		data1d[i] = c[data1d[i]];
	}
	
	if (c) {delete []c; c=0;}
	if (h) {delete []h; h=0;}
	return true;
}

bool hist_eq_range_uint8(unsigned char * data1d, V3DLONG len, unsigned char lowerbound, unsigned char upperbound) //only eq the [lowerbound,upperbound]
{
	if (!data1d || len<=0)
	{
		printf("The input parameters are invalid in hist_eq_uint8().\n");
		return false;
	}
	
	if (lowerbound>upperbound) 
	{
		unsigned char tmp=lowerbound; lowerbound=upperbound; upperbound=tmp;
	}
	
	V3DLONG NBIN=256;
	V3DLONG *h = new V3DLONG [NBIN]; 
	double *c = new double [NBIN]; 
	if (!h)
	{
		printf("Fail to allocate memory in hist_eq_uint8().\n");
		return false;
	}
	
	V3DLONG i;
	
	for (i=0;i<NBIN;i++) h[i]=0;
	
	for (i=0;i<len; i++)
	{
		h[data1d[i]]++;
	}
	
	c[lowerbound]=h[lowerbound];
	for (i=lowerbound+1;i<=upperbound;i++) c[i] = c[i-1]+h[i];
	double range = upperbound-lowerbound;
	for (i=lowerbound;i<=upperbound;i++) {c[i] /= c[upperbound]; c[i] *= range; c[i] += lowerbound;}
	
	for (i=0;i<len;i++)
	{
		if (data1d[i]>=lowerbound && data1d[i]<=upperbound)
			data1d[i] = c[data1d[i]];
	}
	
	if (c) {delete []c; c=0;}
	if (h) {delete []h; h=0;}
	return true;
}

