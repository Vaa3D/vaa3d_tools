//cell segmentation
//by Fuhui Long
//2008-03-07


#ifndef __FL_CELLSEGMENT3D_CPP__
#define __FL_CELLSEGMENT3D_CPP__

#include <math.h>

#include "volimg_proc.h"
#include "img_definition.h"
#include "stackutil.h"

#include "FL_cellSegmentation3D.h"
#include "FL_defType.h"
#include "FL_volimgProcLib.h"

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER) && (_WIN64)
#include "vcdiff.h"
#else
#endif


template <class T> void switchpointer(T * &ptr1, T * &ptr2)
{
	T *ptr_tmp;
	ptr_tmp = ptr1;
	ptr1 = ptr2;
	ptr2 = ptr_tmp;
}

template <class T> void switchpointer(T *** &ptr1, T *** &ptr2)
{
	T ***ptr_tmp;
	ptr_tmp = ptr1;
	ptr1 = ptr2;
	ptr2 = ptr_tmp;
}

template <class T> void switchpointer(T *** &ptr1, T *** &ptr2, T * &ptr3, T * &ptr4, unsigned char & stpcounter)
{
	stpcounter++;
	switchpointer(ptr1, ptr2);
	switchpointer(ptr3, ptr4);
}

bool FL_cellseg(Vol3DSimple <unsigned char> *img3d, Vol3DSimple <unsigned short int> *outimg3d, const segParameter & segpara)
{
	if (!img3d || !outimg3d) //should add valid() judgement later
	{
		printf("Invalid parameters to FL_cellseg();\n");
		return false;
	}
	
	unsigned char *data1d = img3d->getData1dHandle();
	V3DLONG data_sz[3];
	data_sz[0] = img3d->sz0(); data_sz[1] = img3d->sz1(); data_sz[2] = img3d->sz2();
	V3DLONG len = data_sz[0] * data_sz[1]*data_sz[2];
	
	Vol3DSimple <unsigned short int> *tmpimg3d = new Vol3DSimple <unsigned short int> (img3d);
	unsigned short int ***tmpdata3d = (unsigned short int ***) tmpimg3d->getData3dHandle();
	unsigned short int ***outdata3d = outimg3d->getData3dHandle();
	unsigned short int *tmpdata1d = (unsigned short int *) tmpimg3d->getData1dHandle();
	unsigned short int *outdata1d = outimg3d->getData1dHandle();
	
	unsigned char stpcounter = 0;
	unsigned char switchtag = 0; 
		
	/* ---------------- */
    /* Median filtering */
	/* ---------------- */	

	printf("Median filtering ...\n");
	medfilt3d(tmpdata3d, outdata3d, data_sz, segpara.medianFiltWid);
	switchtag = 1; // input and output data are different, thus need to switch
	
	/*--------------------*/
	/* Gaussian filtering */
	/*--------------------*/

	printf("Gaussian filtering ...\n");
	
	if (switchtag ==1)
	{
		switchpointer(tmpdata3d, outdata3d, tmpdata1d, outdata1d, stpcounter);
		printf("%d\n", stpcounter);
	}
	gaussfilt3d(tmpdata3d, outdata3d, data_sz, segpara.gaussinFiltWid, segpara.gaussinFiltSigma);

	switchtag = 1;
	
	/*---------------*/
	/* thresholding  */
	/*---------------*/

	if (switchtag ==1)
	{
		switchpointer(tmpdata3d, outdata3d, tmpdata1d, outdata1d, stpcounter);
		printf("%d\n", stpcounter);
	}
	
	if (segpara.adpatThreMethod == 0) // method 0: local adaptive thresholding 
	{
		printf("Adaptive thresholding ...\n");
		
		V3DLONG kernelsz[3], kernelstp[3];
		
		kernelsz[0] = kernelsz[1] = kernelsz[2] = segpara.adaptThreWid;
		kernelstp[0] = kernelstp[1] = kernelstp[2] = segpara.adaptThreStp;
		
		adaptiveThre3d(tmpdata3d, outdata3d, data_sz, kernelsz, kernelstp);	
	}
	else
	{
		if (segpara.adpatThreMethod == 1) // method 1: global thresholding in 3D, Otsu's method 
		{	
			printf("Global optimum thresholding in 3D ...\n");

			float *hist = 0;
			V3DLONG thre;

			hist = histogram(data1d, len, 255);
			thre = otsu_thre(hist, 256);
			printf("threshold value:%d\n", thre);

			if (hist)  {delete []hist; hist=0;}

			// global thresholding

			globalThresh(tmpdata1d, outdata1d, len, thre); 
		}
		else 
		{
			if (segpara.adpatThreMethod == 2) // method 2: global threshold in each 2D slice, fix problematic threshold on the top and bottom slices
			{
				printf("Global optimum thresholding in each 2D slices...\n");

				float *hist = 0;
				V3DLONG *pp = new V3DLONG [data_sz[2]]; //initial threshold of each 2D slice
				V3DLONG *qq = new V3DLONG [data_sz[2]]; //fixed threshold of each 2D slice

				V3DLONG i,j,k;
				V3DLONG len1 = data_sz[0]*data_sz[1];
				
				// compute the threshold of each 2D slice using Otsu's method
				printf("threshold value: ");				
				for (i=0; i<data_sz[2]; i++)
				{
					hist = histogram(tmpdata1d+i*len1, len1, 255);
					pp[i] = otsu_thre(hist, 256);
					qq[i] = pp[i];
					printf("%d ", pp[i]);
				}
				printf("\n");

				// fix problematic thresholds on the top and bottom slices that have sharp jump of values
				V3DLONG n_num=0, p_num=0;
				
				for (i=0; i<data_sz[2]-1; i++)
				{
					if ((pp[i+1] - pp[i]) <0)
					{
						n_num++;
					}
					else
					{
						p_num++;
					}
				}

				printf("n_num = %d,  p_num = %d\n", n_num, p_num);
				
				//unsigned char *tag = new unsigned char[data_sz[2]]; //changed by PHC, 2010-05-20
				//for (i=0; i<data_sz[2]-1; i++)	tag[i]= 0;  //the tag variable was not used at all. thus commented. by PHC, 2010-05-20
				// if (tag) {delete []tag; tag=0;} //added on 2010-05-20 in case forget to delete it later. by PHC

				V3DLONG starti, endi;
				V3DLONG startmax = -999, endmax = -999;
				
				
				for (i=0; i<(data_sz[2]-1)/3; i++)
				{	
					V3DLONG tmp = abs(double(pp[i+1]-pp[i])); //2010-05-20. by PHC. force convert from V3DLONG to double for MSVC complier
					if (tmp>startmax) 
					{
						startmax = tmp;
						starti = i;
					}
				}

				if (p_num>n_num)
				{
                    for (i=(V3DLONG)(double(data_sz[2]-1)*2/3 + 0.5); i<data_sz[2]-1; i++)
					{	
						V3DLONG tmp = pp[i+1]-pp[i];
						if ((abs(double(tmp))>endmax)&(tmp<0))  //2010-05-20. by PHC. force convert from V3DLONG to double for MSVC complier
						{
							endmax = abs(double(tmp));  //2010-05-20. by PHC. force convert from V3DLONG to double for MSVC complier
							endi = i;
						}
					}
				}
				else
				{
                    for (i=(V3DLONG)(double(data_sz[2]-1)*2/3 + 0.5); i<data_sz[2]-1; i++)
					{	
						V3DLONG tmp = pp[i+1]-pp[i];
						if ((abs(double(tmp))>endmax)&(tmp>0))  //2010-05-20. by PHC. force convert from V3DLONG to double for MSVC complier
						{
							endmax = abs(double(tmp));  //2010-05-20. by PHC. force convert from V3DLONG to double for MSVC complier
							endi = i;
						}
					}
					
				}
				

				printf("starti = %d, endi = %d \n", starti, endi);
				
				double mean, std;
				double *ttt = new double [len];
				for (i=0; i<len; i++)
				{
					ttt[i] = (double)tmpdata1d[i];
				}

				mean_and_std(ttt, len, mean, std);
				printf("%f, %f\n", mean, std);
				 
				if (ttt) {delete[] ttt; ttt = 0;}
				
				V3DLONG lowestThre = (V3DLONG) (mean+std);
				
				for (i=0;i<data_sz[2]; i++)
				{
					if (qq[i]<lowestThre)
						qq[i] = lowestThre;
				};
												
				if (p_num>n_num) // intensity increases with larger z
				{
					//extrapolate thresolds for the first several slices
					
					double stp = abs((double(pp[starti+3])-double(pp[starti+1]))/2);	
				
					for (i=0; i<=starti; i++)
					{					

						qq[i] = qq[starti+1]-stp*(starti+1-i);
						if (qq[i] < lowestThre)
						{
							qq[i] = lowestThre; // lowestThre is assumed to be the minimum intensity of meaningful cell
						}
					}

					//extrapolate thresolds for the last several slices
					stp = abs((double(pp[endi])-double(pp[endi-2]))/2);
				
					for (i=endi+1; i<data_sz[2]; i++)
					{
						qq[i] = qq[endi]+stp*(i-endi);
					}
				}
				else //intensity decreases with larger z
				{
					//extrapolate thresolds for the first several slices
				
					double stp = abs((double(pp[starti+3])-double(pp[starti+1]))/2);	
				
					for (i=0; i<=starti; i++)
					{					

						qq[i] = qq[starti+1]+stp*(starti+1-i);

					}

					//extrapolate thresolds for the last several slices
					stp = abs((double(pp[endi])-double(pp[endi-2]))/2);
				
					for (i=endi+1; i<data_sz[2]; i++)
					{
						qq[i] = qq[endi]+stp*(i-endi);
						if (qq[i] < lowestThre)
						{
							qq[i] = lowestThre; // lowestThre is assumed to be the minimum intensity of meaningful cell
						}						
					}
				}

				printf("threshold value: ");				

				for (i=0; i<data_sz[2]; i++)
				{
					printf("%d ", qq[i]);					
				}
				
				// thresholding
				for (i=0; i<data_sz[2]; i++)
				{
					globalThresh(tmpdata1d+i*len1, outdata1d+i*len1, len1, qq[i]);
				}				

				if (hist)  {delete []hist; hist=0;}
				if (pp) {delete []pp; pp=0;}
				if (qq) {delete []qq; qq=0;}
			
			}
		}
	}
	
	switchtag = 1;	
	
	/*---------------*/
	/* segmentation  */
	/*---------------*/

	printf("Segmentation...\n");		

	if (segpara.segMethod == 1) //distance transform + shape-based watershed  
	{

		if (switchtag ==1)
		{
			switchpointer(tmpdata3d, outdata3d, tmpdata1d, outdata1d, stpcounter);
			printf("%d\n", stpcounter);
		}
			
		// distance transform     
		printf("distance transform ...\n");		

		V3DLONG *pixindex = new V3DLONG [len];
		float *tmpdata1d2 = new float [len];
		
		dt3d_binary(tmpdata1d, tmpdata1d2, pixindex, data_sz, 1); // compute the distance transform for foreground (non-zero) pixels the last argument is 1

		// reverse distance map for non-zeros
		float maxval = -999;
		for (V3DLONG i=0; i<len; i++)
		{
			if (tmpdata1d2[i]>maxval)
				maxval = tmpdata1d2[i];
		}
		
		for (V3DLONG i=0; i<len; i++)
		{	
			if (tmpdata1d2[i] !=0)
				tmpdata1d2[i] = (maxval+10) - sqrt(tmpdata1d2[i]);
		}

		if (pixindex) {delete []pixindex; pixindex =0;}
		
		// watershed   
		printf("watershed segmentation....\n");
		float *tmpdata1d3 = 0; // allocate memory inside watershed_vs

		watershed_vs(tmpdata1d2, tmpdata1d3, data_sz, 3, 26);
		
		for (V3DLONG i=0; i<len; i++)
		{	
			outdata1d[i] = (unsigned short int)tmpdata1d3[i];
		}
		
		if (tmpdata1d2) {delete []tmpdata1d2; tmpdata1d2 =0;}
		if (tmpdata1d3) {delete []tmpdata1d3; tmpdata1d3 =0;}
		
	}
	else //intensity-based watershed on thresholded image
	{
		// note do not switch pointer for clarity
		// reverse intensity map for non-zeros
		float maxval = -999;
		for (V3DLONG i=0; i<len; i++)
		{
			if (tmpdata1d[i]>maxval) //tmpdata1d is the input to thresholding step
				maxval = tmpdata1d[i];
		}
		
		for (V3DLONG i=0; i<len; i++)
		{	
			if (outdata1d[i] ==0) //outdata1d is the output of the thresholding step
				tmpdata1d[i] =0;
			else
				tmpdata1d[i] = (maxval+10) - sqrt(double(tmpdata1d[i]));  //2010-05-20. by PHC. force convert from V3DLONG to double for MSVC complier
		}

		// watershed   
		printf("watershed segmentation....\n");
		float *tmpdata1d2 = 0; // allocate memory inside watershed_vs

		watershed_vs(tmpdata1d, tmpdata1d2, data_sz, 3, 26);
		
		for (V3DLONG i=0; i<len; i++)
		{	
			outdata1d[i] = (unsigned short int)tmpdata1d2[i];
		}
		
		if (tmpdata1d2) {delete []tmpdata1d2; tmpdata1d2 =0;}

	}
	
	switchtag = 1;

	/*----------------------------------*/
	/* copy data to outimg3d if necessary
	/*----------------------------------*/
	
	if (stpcounter%2 == 1)
	{
		for (int i=0;i<len; i++)
		{
			tmpdata1d[i] = outdata1d[i];
		}
	}
	
	if (tmpimg3d) {delete tmpimg3d; tmpimg3d=0;}

	return true;
}


#endif
