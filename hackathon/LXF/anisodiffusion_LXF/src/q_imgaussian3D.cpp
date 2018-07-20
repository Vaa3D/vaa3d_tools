
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "string.h"
#include <vector>

#include "q_imgaussian3D.h"

#ifndef min
#define min(a,b)        ((a) < (b) ? (a): (b))
#endif
#ifndef max
#define max(a,b)        ((a) > (b) ? (a): (b))
#endif
#define clamp(a, b1, b2) min(max(a, b1), b2);


void imfilter1D_float(float *I,int lengthI,float *H,int lengthH, float *J)
{
    int x, i, index, offset;
    int b2, offset2;
    if(lengthI==1)  
    { 
        J[0]=I[0];
    }
    else
    {
        offset=(lengthH-1)/2;
        for(x=0; x<min(offset,lengthI); x++) {
            J[x]=0;
            b2=lengthI-1; offset2=x-offset;
            for(i=0; i<lengthH; i++) {
                index=clamp(i+offset2, 0, b2); J[x]+=I[index]*H[i];
            }
        }
       
        for(x=offset; x<(lengthI-offset); x++) {
            J[x]=0;
            b2=lengthI-1; offset2=x-offset;
            for(i=0; i<lengthH; i++) {
                index=i+offset2; J[x]+=I[index]*H[i];
            }
        }
       
         b2=lengthI-1; 
         for(x=max(lengthI-offset,offset); x<lengthI; x++) {
              J[x]=0;
              offset2=x-offset;
              for(i=0; i<lengthH; i++) {
                  index=clamp(i+offset2, 0, b2); J[x]+=I[index]*H[i];
             }
         }
       
    }
}

void imfilter2D_float(float *I,int * sizeI,float *H,int lengthH, float *J)
{
    int y, x, i, y2;
    float *Irow, *Crow;
    int index=0, line=0;
    float *RCache;
    int *nCache;
    int hks, offset, offset2;
    RCache=(float *)malloc(lengthH*sizeI[0]*sizeof(float));
    for(i=0; i<lengthH*sizeI[0]; i++) { RCache[i]=0; }
    nCache=(int *)malloc(lengthH*sizeof(int));
    for(i=0; i<lengthH; i++) { nCache[i]=0; }
    hks=((lengthH-1)/2);
    for(y=0; y<min(hks,sizeI[1]); y++) {
        Irow=&I[index];
        Crow=&RCache[line*sizeI[0]];
        imfilter1D_float(Irow, sizeI[0], H, lengthH, Crow);
        index+=sizeI[0];
        if(y!=(sizeI[1]-1))
        {
            line++; if(line>(lengthH-1)) { line=0; }
        }
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }
    for(y2=y; y2<hks; y2++) {
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }
            
    for(y=hks; y<(sizeI[1]-1); y++) {
        Irow=&I[index];
        Crow=&RCache[line*sizeI[0]];
        imfilter1D_float(Irow, sizeI[0], H, lengthH, Crow);
        offset=(y-hks)*sizeI[0]; offset2=nCache[0]*sizeI[0];
        for(x=0; x<sizeI[0]; x++) { J[offset+x]=RCache[offset2+x]*H[0]; }
        for(i=1; i<lengthH; i++) {
            offset2=nCache[i]*sizeI[0];
            for(x=0; x<sizeI[0]; x++) { J[offset+x]+=RCache[offset2+x]*H[i]; }
        }
        index+=sizeI[0];
        line++; if(line>(lengthH-1)) { line=0; }
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }

    for(y=max(sizeI[1]-1,hks); y<sizeI[1]; y++) {
        Irow=&I[index];
        Crow=&RCache[line*sizeI[0]];
        imfilter1D_float(Irow, sizeI[0], H, lengthH, Crow);
        offset=(y-hks)*sizeI[0]; offset2=nCache[0]*sizeI[0];
        for(x=0; x<sizeI[0]; x++) { J[offset+x]=RCache[offset2+x]*H[0]; }
        for(i=1; i<lengthH; i++) {
            offset2=nCache[i]*sizeI[0];
            for(x=0; x<sizeI[0]; x++) { J[offset+x]+=RCache[offset2+x]*H[i]; }
        }
        index+=sizeI[0];
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }

    for(y=max(sizeI[1],hks); y<(sizeI[1]+hks); y++) {
        offset=(y-hks)*sizeI[0]; offset2=nCache[0]*sizeI[0];
        for(x=0; x<sizeI[0]; x++) { J[offset+x]=RCache[offset2+x]*H[0]; }
        for(i=1; i<lengthH; i++) {
            offset2=nCache[i]*sizeI[0];
            for(x=0; x<sizeI[0]; x++) { J[offset+x]+=RCache[offset2+x]*H[i]; }
        }
        index+=sizeI[0];
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }

    free(RCache);
	free(nCache);
}

void imfilter3D_float(float *I,int * sizeI,float *H,int lengthH, float *J)
{
    int z, j, i, z2;
    float *Islice, *Cslice;
    int index=0, line=0;
    float *SCache;
    int *nCache;
    int hks, offset, offset2;
    int nslice;
    nslice=sizeI[0]*sizeI[1];
    SCache=(float *)malloc(lengthH*nslice*sizeof(float));
	for(i=0; i<nslice; i++) { SCache[i]=0; }
    nCache=(int *)malloc(lengthH*sizeof(int));
    for(i=0; i<lengthH; i++) { nCache[i]=0; }
    hks=((lengthH-1)/2);
    for(z=0; z<min(hks,sizeI[2]); z++) {
        Islice=&I[index];
        Cslice=&SCache[line*nslice];
        imfilter2D_float(Islice, sizeI, H, lengthH, Cslice);
        index+=nslice;
        if(z!=(sizeI[2]-1))
        {
            line++; if(line>(lengthH-1)) { line=0; }
        }
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }
    for(z2=z; z2<hks; z2++) {
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }
    for(z=hks; z<(sizeI[2]-1); z++) {
        Islice=&I[index];
        Cslice=&SCache[line*nslice];
        imfilter2D_float(Islice, sizeI, H, lengthH, Cslice);
        offset=(z-hks)*nslice; offset2=nCache[0]*nslice;
        for(j=0; j<nslice; j++) { J[offset+j]=SCache[offset2+j]*H[0]; }
        for(i=1; i<lengthH; i++) {
            offset2=nCache[i]*nslice;
            for(j=0; j<nslice; j++) { J[offset+j]+=SCache[offset2+j]*H[i]; }
        }
        index+=nslice;
        line++; if(line>(lengthH-1)) { line=0; }
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }
    for(z=max(sizeI[2]-1,hks); z<sizeI[2]; z++) {
        Islice=&I[index];
        Cslice=&SCache[line*nslice];
        imfilter2D_float(Islice, sizeI, H, lengthH, Cslice);
        offset=(z-hks)*nslice; offset2=nCache[0]*nslice;
        for(j=0; j<nslice; j++) { J[offset+j]=SCache[offset2+j]*H[0]; }
        for(i=1; i<lengthH; i++) {
            offset2=nCache[i]*nslice;
            for(j=0; j<nslice; j++) { J[offset+j]+=SCache[offset2+j]*H[i]; }
        }
        index+=nslice;
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }
    for(z=max(sizeI[2],hks); z<(sizeI[2]+hks); z++) {
        offset=(z-hks)*nslice; offset2=nCache[0]*nslice;
        for(j=0; j<nslice; j++) { J[offset+j]=SCache[offset2+j]*H[0]; }
        for(i=1; i<lengthH; i++) {
            offset2=nCache[i]*nslice;
            for(j=0; j<nslice; j++) { J[offset+j]+=SCache[offset2+j]*H[i]; }
        }
        index+=nslice;
        for(i=0; i<(lengthH-1); i++) { nCache[i]=nCache[i+1]; } nCache[lengthH-1]=line;
    }

    free(SCache);
	free(nCache);
}

void GaussianFiltering3D_float(float *I, float *J,int *dimsI,float sigma,float kernel_size)
{
	int kernel_length,i;
    float x, *H, totalH=0;
	
	/* Construct the 1D gaussian kernel */
	if(kernel_size<1) { kernel_size=1; }
    kernel_length=(int)(2*ceil(kernel_size/2)+1);
	H = (float *)malloc(kernel_length*sizeof(float));
	x=-ceil(kernel_size/2);
	for (i=0; i<kernel_length; i++) { H[i]=exp(-((x*x)/(2*(sigma*sigma)))); totalH+=H[i]; x++; }
	for (i=0; i<kernel_length; i++) { H[i]/=totalH; }
	
	/* Do the filtering */
	imfilter3D_float(I, dimsI, H, kernel_length, J);
    /* Clear memory gaussian kernel */
	free(H);
}


bool q_imgaussian3D(float *p_img32f,long sz_img[4],float sigma,float szkernel,float *&p_img32f_output)
{
	//check paras
	if(p_img32f==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(sz_img[3]>1)
	{
		printf("ERROR: Input image should be single channel!\n");
		return false;
	}
	if(sigma<0 || szkernel<=0)
	{
		printf("ERROR: Invalid sigma or szkernel!\n");
		return false;
	}
	if(p_img32f_output)
	{
		printf("WARNNING: Output image pointer is not NULL, original data will be cleared!\n");
		if(p_img32f_output) 	{delete []p_img32f_output;		p_img32f_output=0;}
	}

	//allocate memory
	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
	p_img32f_output=new(std::nothrow) float[l_npixels]();
	if(!p_img32f_output)
	{
		printf("ERROR: Fail to allocate memory for output image!\n");
		return false;
	}

	//do gaussian smooth
	int dimsI[3];
	dimsI[0]=sz_img[0];//ncol-width
	dimsI[1]=sz_img[1];//nrow-height
	dimsI[2]=sz_img[2];//ndep-z
	GaussianFiltering3D_float(p_img32f,p_img32f_output,dimsI,sigma,szkernel);

	return true;
}
