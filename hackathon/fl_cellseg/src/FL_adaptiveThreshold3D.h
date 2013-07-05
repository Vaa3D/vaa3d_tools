// 3D adaptive thresholding
// F. Long
// 20080507

#ifndef __ADAPTIVE_THRESHOLD3D__
#define __ADAPTIVE_THRESHOLD3D__

#include "volimg_proc.h"
#include "img_definition.h"

#include "FL_upSample3D.h"
#include "FL_downSample3D.h"
#include "FL_defType.h"

#define CONTRAST 15
//#define COEF 1.5  
//#define COEF 1  
#define COEF 0.8  


template <class T1> bool min_and_max(T1 * vec1d, V3DLONG cnt, T1 & minval, T1 &maxval)
{
	if (!vec1d || cnt<0)
		return false;
	
	minval = maxval = vec1d[0];
	
	for (V3DLONG i=1; i<cnt; i++)
	{
		if (vec1d[i]<minval) minval = vec1d[i];
		else if (vec1d[i]>maxval) maxval = vec1d[i];
	}
	
	return true;	
}

template <class T1, class T2> bool adaptiveThre3d(T1 ***indata3d, T2 *** outdata3d, V3DLONG *sz, V3DLONG * kernelsz, V3DLONG * kernelstp)
{
	
	V3DLONG i,j,k;
	V3DLONG m,n,p;
		
	unsigned V3DLONG len, cnt, cnt0;
	V3DLONG rr[3];
	
	for (i=0;i<3;i++)
	{
	    rr[i] = (2*kernelsz[i]+1);
	}
	len = rr[0]*rr[1]*rr[2];
	
	float *vec1d = new float[len];	
	float meanval, stdval,minval,maxval;
	V3DLONG cntk, cntj, cnti;
	V3DLONG m1, m2, n1, n2, p1, p2;
	
	V3DLONG szsmall[3];
	for (V3DLONG i=0; i<3; i++)
	{
		szsmall[i] = floor(sz[i]/kernelstp[i])+1;
	}
	 
	T2 *bgddatasmall1d = new T2 [szsmall[0]*szsmall[1]*szsmall[2]];
	T2 ***bgddatasmall3d = 0;
	new3dpointer(bgddatasmall3d, szsmall[0], szsmall[1], szsmall[2], bgddatasmall1d);

	// finding background level on sampled grids
	
	cntk=0;
	
	for (k = 0; k<sz[2]; k+=kernelstp[2])
	{
		m1 = k-kernelsz[2]; m2 = k+kernelsz[2];
		cntj=0;

		for (j = 0; j<sz[1]; j+=kernelstp[1])
		{

			n1 = j-kernelsz[1]; n2 = j+kernelsz[1];
		    cnti=0;
			
			for (i = 0; i<sz[0]; i+=kernelstp[0])
			{

				p1 = i-kernelsz[0]; p2 = i+kernelsz[0];
				cnt = 0; 
			
				for (m = m1; m <= m2; m++)
				{
					for (n = n1; n <= n2; n++)
					{				
						for (p = p1 ; p <= p2; p++)
						{   
							if (m>=0 & m<sz[2] & n>=0 & n<sz[1] & p>=0 & p<sz[0])
							{
							    vec1d[cnt] = (float)indata3d[m][n][p];
								cnt++;
							}
						}
					}
				}
				
				mean_and_std(vec1d, (float)cnt, meanval, stdval);		
				min_and_max(vec1d, (float)cnt, minval, maxval);	

				 bgddatasmall3d[cntk][cntj][cnti] = ((maxval-minval)>CONTRAST)?(T2)(meanval+COEF*stdval):255;
				 	
				cnti++;
			}
			cntj++;
		}
		cntk++;
	}
		
	//interpolate

	double dfactor[3];	
	V3DLONG sznew[3];

	for (i=0; i<3; i++)
	{
		dfactor[i] = (double)sz[i]/(double)szsmall[i];
		sznew[i] = V3DLONG(ceil(dfactor[i]*szsmall[i]));
	}
	  
	T2 *bdgdata1d = new T2 [sznew[0]*sznew[1]*sznew[2]];
	T2 ***bdgdata3d = 0;
	new3dpointer(bdgdata3d, sznew[0], sznew[1], sznew[2], bdgdata1d);
	upsample3dvol(bdgdata3d, bgddatasmall3d, szsmall[0], szsmall[1], szsmall[2], dfactor);
	
	//pad margin to make outimg the same size of inimg
	for (k=0; k<sz[2]; k++)
	{
		for (j=0;j<sz[1]; j++)
		{
			for (i=0;i<sz[0];i++)
			{
				outdata3d[k][j][i]=0;
			}
		}
	}

	V3DLONG k1 = sz[2]<sznew[2]?sz[2]:sznew[2];
	V3DLONG j1 = sz[1]<sznew[1]?sz[1]:sznew[1];
	V3DLONG i1 = sz[0]<sznew[0]?sz[0]:sznew[0];

//	for (k=0; k<k1; k++)
//	{
//		for (j=0;j<j1; j++)
//		{
//			for (i=0;i<i1;i++)
//			{
//				outdata3d[k][j][i] = (T2)((indata3d[k][j][i]>bdgdata3d[k][j][i])?255:0);
//			}
//		}
//	}

	for (k=kernelsz[2]; k<k1-kernelsz[2]; k++)
	{
		for (j=kernelsz[1];j<j1-kernelsz[1]; j++)
		{
			for (i=kernelsz[0];i<i1-kernelsz[0];i++)
			{
				outdata3d[k][j][i] = (T2)((indata3d[k][j][i]>bdgdata3d[k][j][i])?255:0);
			}
		}
	}

	//free memory
//	if (bdgdata3d) {delete bdgdata3d; bdgdata3d=0;}
	if (bdgdata3d) {delete3dpointer(bdgdata3d, sznew[0], sznew[1], sznew[2]);}
	if (bdgdata1d) {delete bdgdata1d; bdgdata1d=0;}
//	if (bgddatasmall3d) {delete bgddatasmall3d; bgddatasmall3d=0;}
	if (bgddatasmall3d) {delete3dpointer(bgddatasmall3d, szsmall[0], szsmall[1], szsmall[2]);}
	if (bgddatasmall1d) {delete bgddatasmall1d; bgddatasmall1d=0;}
    if (vec1d) {delete []vec1d; vec1d=0;}
	
	return true;
}


template <class T1, class T2> bool adaptiveThre3d(Vol3DSimple <T1> * inimg, Vol3DSimple <T2> * outimg, V3DLONG * kernelsz, V3DLONG * kernelstp)
{

	// check arguments 
	if (!inimg || !inimg->valid() || !outimg || !outimg->valid())
	{
		printf("Invalid input\n");
		return false;
	}

	V3DLONG sz[3];
	sz[0] = inimg->sz0();  
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2(); 

    for (V3DLONG i=0;i<3;i++)
	{
		if  (kernelsz[i]<=0 || kernelsz[i]>= sz[i] || kernelstp[i]<0 || kernelstp[i]>= sz[i])
		{
			printf("Invalid input\n");
			return false;
		}
	}		
	
	T1 ***indata3d = inimg->getData3dHandle();	
	T2 ***outdata3d = outimg->getData3dHandle();
	
	adaptiveThre3d(indata3d, outdata3d, sz, kernelsz, kernelstp);
	
	return true;
}
	
	


// temporary, don't use it, this template has some problem, 
// the size of the image after downsample and upsample may not be the same as the original image
//template <class T> bool  adaptiveThre3d(Vol3DSimple <T> * inimg, Vol3DSimple <T> * outimg, int kernelsz)
//{
//	if (!inimg || !inimg->valid() || !outimg ||!outimg->valid() || kernelsz<=0)
//		return false;
//	
//	// downsample
//	
//	Vol3DSimple <T> * inimgsmall = 0;
//	
//	if (!downsample3dvol(inimgsmall,inimg, 2)) //downsample3dvol requires outimgtmp to be NULL, thus outimg cannot be used directly
//	{
//	    printf("fail to downsample image \n");
//		return false;
//	}
//	
//	// adaptive thresholding
//	
//    Vol3DSimple <T> *outimgsmall = new Vol3DSimple <T> (inimgsmall);
//	T ***indatasmall3d = inimgsmall->getData3dHandle();
//	T ***outdatasmall3d = outimgsmall->getData3dHandle();
//		
//	V3DLONG i,j,k;
//	V3DLONG m,n,p;
//		
//	V3DLONG *sz = new V3DLONG [3];
//	sz[0] = inimgsmall->sz0();  
//	sz[1] = inimgsmall->sz1();
//	sz[2] = inimgsmall->sz2(); 
//
//	unsigned V3DLONG len, cnt, cnt0;
//	V3DLONG rr=(2*kernelsz+1);
//	len = rr*rr*rr;
//		
//	
//	float *vec1d = new float[len];	
//	float meanval, stdval,minval,maxval;
//	
//	for (k = 0; k<sz[2]; k++)
//	{
//		printf("%d\n", k);   
//		for (j = 0; j<sz[1]; j++)
//		{
//			for (i = 0; i<sz[0]; i++)
//			{
//				
//				cnt = 0; 
//				
//				V3DLONG m1, m2, n1, n2, p1, p2;
//				m1 = k-kernelsz; m2 = k+kernelsz;
//				n1 = j-kernelsz; n2 = j+kernelsz;
//				p1 = i-kernelsz; p2 = i+kernelsz;
//				
//				for (m = m1; m <= m2; m++)
//				{
//					for (n = n1; n <= n2; n++)
//					{				
//						for (p = p1 ; p <= p2; p++)
//						{   
//							if (m>=0 & m<sz[2] & n>=0 & n<sz[1] & p>=0 & p<sz[0])
//							{
//							    vec1d[cnt] = (float)indatasmall3d[m][n][p];
//								cnt++;
//							}
//						}
//					}
//				}
//				
//				mean_and_std(vec1d, (float)cnt, meanval, stdval);		
//				min_and_max(vec1d, (float)cnt, minval, maxval);	
//				
//				if ((maxval-minval)>0) // if contrast small, then the entire region is background
//				{
//				    outdatasmall3d[k][j][i] = (indatasmall3d[k][j][i]>(meanval+2*stdval))?1:0;
////					outdatasmall1d[cnt0] = (indatasmall3d[k][j][i]>(meanval+2*stdval))?1:0;
//				}
//				else
//				{
//					outdatasmall3d[k][j][i] = 0;
////					outdatasmall1d[cnt0] = 0;
//				}
//				
////				cnt0++;
//			}
//		}
//	}
//		
//	printf("******\n");
//	
//	Vol3DSimple <T> * outimgtmp = 0;
//	
//	//upsample
//	if (upsample3dvol(outimgtmp, outimgsmall, 2)==false)
//	{
//		return false;
//    }
//	
//	printf("%d %d %d\n", outimgtmp->sz0(), outimgtmp->sz1(), outimgtmp->sz2());
//	printf("%d %d %d\n", outimg->sz0(), outimg->sz1(), outimg->sz2());
//	 
//	T *outimgdata =  outimg -> getData1dHandle();
//	T *outimgtmpdata =  outimgtmp -> getData1dHandle();
//	
//	for (V3DLONG i=0; i< outimg->sz0()*outimg->sz1()*outimg->sz2(); i++)
//	{
//		outimgdata[i] = outimgtmpdata[i];
//	}
//			
//	if (sz) {delete []sz; sz=0;}
//    if (vec1d) {delete []vec1d; vec1d=0;}
//	if (outimgtmp) {delete []outimgtmp; outimgtmp = 0;}
//	if (outimgsmall) {delete []outimgsmall; outimgsmall = 0;}
//	if (inimgsmall) {delete []inimgsmall; inimgsmall =0;}
//	
//	return true;
//}


#endif
