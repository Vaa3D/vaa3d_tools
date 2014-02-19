// 3D filtering
// F. Long
// 20080507
// 20080826
    
#ifndef __FILTER3D__
#define __FILTER3D__

#include "volimg_proc.h"
#include "img_definition.h"
#include "stackutil.h"

#include <math.h>
#include "pbetai.cpp"
#include "FL_sort.h"


// gaussfilt3d_img operates on class Vol3DSimple
template <class T> bool gaussfilt3d(Vol3DSimple <T> * img, int kernelsz, float sigma)
{
	if (!img || !img->valid() || kernelsz<=0 ||sigma<=0)
		return false;

    Vol3DSimple <T> *inimg = new Vol3DSimple <T> (img); 

	T ***indata3d = inimg->getData3dHandle();
	T ***outdata3d = img->getData3dHandle();

	V3DLONG *sz = new V3DLONG [3];
	sz[0] = inimg->sz0();  
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2(); 
		
	gaussfilt3d(indata3d, outdata3d, sz, kernelsz, sigma);
	
	if (sz) {delete []sz; sz=0;}
	return true;
	
}
	
template <class T1, class T2> bool gaussfilt3d(T1 ***indata3d, T2 ***outdata3d, V3DLONG *sz, int kernelsz, float sigma)
{
	V3DLONG i,j,k;
	V3DLONG m,n,p;

	unsigned V3DLONG len, cnt;
	V3DLONG rr=(2*kernelsz+1);
	len = rr*rr*rr;
	
	float *gf1d = new float [len];
	float ***gf3d=0;
	
	new3dpointer(gf3d, rr, rr, rr, gf1d);
	
	if (!gf3d)
	{
		printf("Fail to allocate memory in gaussfilt3d().\n");
		return false;
	}
	
	float sigma2 = 2*sigma*sigma;
	float sumval = 0;	
	
   // compute the template
   for (k=0; k<rr; k++)
   { 
		V3DLONG k1 = k-kernelsz;
		
		for (j=0; j<rr; j++)
		{
			V3DLONG j1 = j-kernelsz;
			
			for (i=0; i<rr; i++)
			{
				V3DLONG i1 = i-kernelsz;
				gf3d[k][j][i] = exp((-k1*k1)/sigma2)*exp((-j1*j1)/sigma2)*exp((-i1*i1)/sigma);
				sumval = sumval + gf3d[k][j][i];
			}
		}
	}
	
  for (k=0; k<rr; k++)
   { 
		for (j=0; j<rr; j++)
		{
			for (i=0; i<rr; i++)
			{
				gf3d[k][j][i] = gf3d[k][j][i]/sumval;
			}
		}
	}
	
	
	for (k = 0; k<sz[2]; k++)
	{
		for (j = 0; j<sz[1]; j++)
		{
			for (i = 0; i<sz[0]; i++)
			{
				V3DLONG m1, m2, n1, n2, p1, p2;
				m1 = k-kernelsz; m2 = k+kernelsz;
				n1 = j-kernelsz; n2 = j+kernelsz;
				p1 = i-kernelsz; p2 = i+kernelsz;
				float sumval=0;
				
				for (m = m1; m <= m2; m++)
				{
					for (n = n1; n <= n2; n++)
					{				
						for (p = p1 ; p <= p2; p++)
						{   
							if (m<0 || m>=sz[2] || n<0 || n>=sz[1] || p<0 || p>=sz[0])
							{
								sumval = sumval + 0.0;
							}
							else
							{
								sumval = sumval + (float)indata3d[m][n][p]*gf3d[m-m1][n-n1][p-p1];
							}
						}
					}
				}
							
				outdata3d[k][j][i] = sumval;
			}
		}
	}
	

	if (gf1d) {delete []gf1d; gf1d=0;}
	if (gf3d) {delete3dpointer(gf3d, rr, rr, rr);}
	
	return true;
}

// medfilt3d_img operates on class Vol3DSimple
template <class T> bool medfilt3d(Vol3DSimple <T> * img, int kernelsz)
{
	if (!img || !img->valid() || kernelsz<=0)
		return false;

    Vol3DSimple <T> *inimg = new Vol3DSimple <T> (img); 
	
	T ***indata3d = inimg->getData3dHandle();
	T ***outdata3d = img->getData3dHandle();
	
	V3DLONG *sz = new V3DLONG [3];
	sz[0] = inimg->sz0();  
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2(); 
	
	medfilt3d(indata3d, outdata3d, sz, kernelsz);
		
	if (sz) {delete []sz; sz=0;}
	return true;
}


template <class T1, class T2> bool medfilt3d(T1 ***indata3d, T2 ***outdata3d, V3DLONG *sz, int kernelsz)
{
	V3DLONG i,j,k;
	V3DLONG m,n,p;

	//unsigned 
	V3DLONG len, cnt;
	V3DLONG rr=(2*kernelsz+1);
	len = rr*rr*rr;
    V3DLONG midlen = (len-1)/2; //this is not exactly the midpoint, but should be fine for now. Noted by PHC 20130719
	
    float *vec1d=0;
    try
    {
     printf("len=%ld\n", len); 
     vec1d = new float [len];
    }
    catch (...)
    {
        printf("Fail to allocate memory in medfilt3d().\n");
        if (vec1d) {delete []vec1d; vec1d=0;}
	return false;
    }
	
	for (k = 0; k<sz[2]; k++)
	{
		printf("z=%d \n", k);   
		for (j = 0; j<sz[1]; j++)
		{
			for (i = 0; i<sz[0]; i++)
			{
				cnt = 0; 
				for (m = k-kernelsz; m <= k+kernelsz; m++)
				{
					for (n = j-kernelsz; n <= j+kernelsz; n++)
					{				
						for (p = i-kernelsz; p <= i+kernelsz; p++)
						{   
							if (m<0 || m>=sz[2] || n<0 || n>=sz[1] || p<0 || p>=sz[0])
							{
								vec1d[cnt] = 0.0;
							}
							else
							{
								vec1d[cnt] = (float)indata3d[m][n][p];
							}
							cnt++;
						}
					}
				}
				
				//sort
				sort2(len, vec1d); // call sort2 of numerical recipe  //here is the problem!!
				
				outdata3d[k][j][i] = vec1d[midlen];
			}
		}
	}
	
	printf("\nMedian filtering done\n");
	
	if (vec1d) {delete []vec1d; vec1d=0;}
	
	printf("Median free\n");

	return true;
}

#endif

