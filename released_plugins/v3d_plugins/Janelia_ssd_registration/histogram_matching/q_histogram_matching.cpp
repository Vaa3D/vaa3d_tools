// q_histogram_matching.cpp
// by Lei Qu
// 2010-06-09
#include <stdio.h>
#include <math.h>

#include "q_histogram_matching.h"

// match the histogram of subject image to that of target
// note:
//		both target and subject image should be 8bits and 1 channel
//		target and subject image may have difference size
bool q_histogram_matching_1c(const unsigned char *p_img8u1c_tar,const long *sz_img8u1c_tar,
						     const unsigned char *p_img8u1c_sub,const long *sz_img8u1c_sub,
						     unsigned char *&p_img8u1c_sub2tar)
{
	//check paras
	if(p_img8u1c_tar==0 || p_img8u1c_sub==0)
	{
		printf("ERROR: Invalid input image pointer (target or subject)!\n");
		return false;
	}
	if(sz_img8u1c_tar[0]<=0 || sz_img8u1c_tar[1]<=0 || sz_img8u1c_tar[2]<=0 || sz_img8u1c_tar[3]!=1 ||
	   sz_img8u1c_sub[0]<=0 || sz_img8u1c_sub[1]<=0 || sz_img8u1c_sub[2]<=0 || sz_img8u1c_sub[3]!=1)
	{
		printf("ERROR: Invalid input image size or channel number!\n");
		return false;
	}
	if(p_img8u1c_sub2tar)
	{
		printf("WARNNING: Output image pointer is not empty, original data will be deleted!\n");
		if(p_img8u1c_sub2tar) 	{delete []p_img8u1c_sub2tar;		p_img8u1c_sub2tar=0;}
	}

	long l_npixels_tar,l_npixels_sub;
	l_npixels_tar=sz_img8u1c_tar[0]*sz_img8u1c_tar[1]*sz_img8u1c_tar[2];
	l_npixels_sub=sz_img8u1c_sub[0]*sz_img8u1c_sub[1]*sz_img8u1c_sub[2];

	//compute the histogram
	double d_hist_tar[256]={0},d_hist_sub[256]={0};
	for(long i=0;i<l_npixels_tar;i++)
		d_hist_tar[p_img8u1c_tar[i]]++;
	for(long i=0;i<l_npixels_sub;i++)
		d_hist_sub[p_img8u1c_sub[i]]++;

	//compute the histogram accumulation
	double d_histaccum_tar[256]={0},d_histaccum_sub[256]={0};
	d_histaccum_tar[0]=d_hist_tar[0];
	d_histaccum_sub[0]=d_hist_sub[0];
	for(long i=1;i<256;i++)
	{
		d_histaccum_tar[i]=d_histaccum_tar[i-1]+d_hist_tar[i];
		d_histaccum_sub[i]=d_histaccum_sub[i-1]+d_hist_sub[i];
	}

	//normalize histogram accumulation
	for(long i=0;i<256;i++)
	{
		d_histaccum_tar[i]/=l_npixels_tar;
		d_histaccum_sub[i]/=l_npixels_sub;
	}

	//build look up table (lazy guy's work)
	int LUT[256]={0};
	for(long i=0;i<256;i++)
	{
		double mindiff=1;
		for(long j=0;j<256;j++)
		{
			double absdiff=fabs(d_histaccum_sub[i]-d_histaccum_tar[j]);
			if(mindiff>absdiff)
			{
				mindiff=absdiff;
				LUT[i]=j;
			}
		}
		printf("\t[sub]:%d -> [tar]:%d\n",i,LUT[i]);
	}

	//generate output image
	p_img8u1c_sub2tar=new unsigned char[l_npixels_sub]();
	if(!p_img8u1c_sub2tar)
	{
		printf("ERROR: Fail to allocate memory for p_img8u1c_sub2tar!\n");
		return false;
	}
	for(long i=0;i<l_npixels_sub;i++)
		p_img8u1c_sub2tar[i]=LUT[p_img8u1c_sub[i]];

	return true;
}

