
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include <vector>

#include "q_derivatives3D.h"

#ifndef min
#define min(a,b)        ((a) < (b) ? (a): (b))
#endif
#ifndef max
#define max(a,b)        ((a) > (b) ? (a): (b))
#endif
#define absd(a) ((a)>(-a)?(a):(-a))

//sizeI=[ncol,nrow,ndep]
void gradient3Dx_float(const float *I,const int *sizeI,float *&Ix)
{
    int x,y,z;
    int xp, xn, yp, yn;
    int i;
    int indexn, indexc, indexp;
    float *Irow, *Islices;
    int nSlice;
    int offsetz,offset_slice;
    int slice_select=0, slice_select_p1=0, slice_select_p2=0;
    
    const float smoothfilter[3]={0.187500,0.625000,0.187500};
    const float derivafilter[3]={-0.5,0,0.5};
    
    nSlice=sizeI[0]*sizeI[1];							//pixel num per slice
    Islices=(float *)malloc(4*nSlice*sizeof(float));
    Irow=(float *)malloc(sizeI[0]*sizeof(float));
    
    for(z=0; z<sizeI[2]; z++)
    {
        offsetz=nSlice*z;
        offset_slice=nSlice*slice_select;
        
        for(y=0; y<sizeI[1]; y++)
        {
          /* Smooth y - direction  */
            yn=max(y-1,0);
            yp=min(y+1,sizeI[1]-1);
            
            indexn=yn*sizeI[0]+offsetz; 
            indexc=y*sizeI[0]+offsetz;
            indexp=yp*sizeI[0]+offsetz;
            
            for(x=0; x<sizeI[0]; x++)
            {
                Irow[x] =smoothfilter[0]*I[indexn+x];
                Irow[x]+=smoothfilter[1]*I[indexc+x];
                Irow[x]+=smoothfilter[2]*I[indexp+x];
            }

            indexc=y*sizeI[0]+offset_slice;
          /*  Gradient in x - direction  */
            for(x=0; x<sizeI[0]; x++)
            {
                xn=max(x-1,0); xp=min(x+1,sizeI[0]-1);
                Islices[indexc+x]=derivafilter[0]*Irow[xn]+derivafilter[1]*Irow[x]+derivafilter[2]*Irow[xp];
            }
        }
       
        /* Smooth in z - direction  */
        if(z==1) /* Forward          */
        {
            indexn=slice_select_p1*nSlice; indexc=slice_select_p1*nSlice; indexp=slice_select*nSlice;
            for(i=0; i<nSlice; i++) 
            { 
                Ix[i]=smoothfilter[0]*Islices[i+indexn]+smoothfilter[1]*Islices[i+indexc]+smoothfilter[2]*Islices[i+indexp];
            }
        }
        else if(z>1) /* Central  */
        {
            indexn=slice_select_p2*nSlice; indexc=slice_select_p1*nSlice; indexp=slice_select*nSlice;
            offsetz=nSlice*(z-1);
            for(i=0; i<nSlice; i++) 
            { 
                Ix[offsetz+i]=smoothfilter[0]*Islices[i+indexn]+smoothfilter[1]*Islices[i+indexc]+smoothfilter[2]*Islices[i+indexp];
            }
        }
        
        if(z==(sizeI[2]-1)) /* Backward  */
        {
            indexn=slice_select_p1*nSlice; indexc=slice_select*nSlice; indexp=slice_select*nSlice;
            offsetz=nSlice*z;
            for(i=0; i<nSlice; i++) 
            { 
                Ix[offsetz+i]=smoothfilter[0]*Islices[i+indexn]+smoothfilter[1]*Islices[i+indexc]+smoothfilter[2]*Islices[i+indexp];
            }
        }
       
        slice_select_p2=slice_select_p1; slice_select_p1=slice_select; slice_select++; if(slice_select>3) { slice_select=0; }

    }
    free(Irow);
    free(Islices);
}

void gradient3Dy_float(const float *I,const int *sizeI,float *&Iy)
{
    int x,y,z;
    int xp, xn, yp, yn;
    int i;
    int indexn, indexc, indexp;
    float *Irow, *Islices;
    int nSlice;
    int offsetz,offset_slice;
    int slice_select=0, slice_select_p1=0, slice_select_p2=0;
    
    const float smoothfilter[3]={0.187500,0.625000,0.187500};
    const float derivafilter[3]={-0.5,0,0.5};
    
    nSlice=sizeI[0]*sizeI[1];
    Islices=(float *)malloc(4*nSlice*sizeof(float));
    Irow=(float *)malloc(sizeI[0]*sizeof(float));
    
    for(z=0; z<sizeI[2]; z++)
    {
        offsetz=nSlice*z;
        offset_slice=nSlice*slice_select;
        
        for(y=0; y<sizeI[1]; y++)
        {
          /* Smooth y - direction  */
            yn=max(y-1,0);
            yp=min(y+1,sizeI[1]-1);
            
            indexn=yn*sizeI[0]+offsetz; 
            indexc=y*sizeI[0]+offsetz;
            indexp=yp*sizeI[0]+offsetz;
            
            for(x=0; x<sizeI[0]; x++)
            {
                Irow[x] =derivafilter[0]*I[indexn+x];
                Irow[x]+=derivafilter[1]*I[indexc+x];
                Irow[x]+=derivafilter[2]*I[indexp+x];
            }

            indexc=y*sizeI[0]+offset_slice;
          /*  Gradient in x - direction  */
            for(x=0; x<sizeI[0]; x++)
            {
                xn=max(x-1,0); xp=min(x+1,sizeI[0]-1);
                Islices[indexc+x]=smoothfilter[0]*Irow[xn]+smoothfilter[1]*Irow[x]+smoothfilter[2]*Irow[xp];
            }
        }
       
        /* Smooth in z - direction  */
        if(z==1) /* Forward          */
        {
            indexn=slice_select_p1*nSlice; indexc=slice_select_p1*nSlice; indexp=slice_select*nSlice;
            for(i=0; i<nSlice; i++) 
            { 
                Iy[i]=smoothfilter[0]*Islices[i+indexn]+smoothfilter[1]*Islices[i+indexc]+smoothfilter[2]*Islices[i+indexp];
            }
        }
        else if(z>1) /* Central  */
        {
            indexn=slice_select_p2*nSlice; indexc=slice_select_p1*nSlice; indexp=slice_select*nSlice;
            offsetz=nSlice*(z-1);
            for(i=0; i<nSlice; i++) 
            { 
                Iy[offsetz+i]=smoothfilter[0]*Islices[i+indexn]+smoothfilter[1]*Islices[i+indexc]+smoothfilter[2]*Islices[i+indexp];
            }
        }
        
        if(z==(sizeI[2]-1)) /* Backward  */
        {
            indexn=slice_select_p1*nSlice; indexc=slice_select*nSlice; indexp=slice_select*nSlice;
            offsetz=nSlice*z;
            for(i=0; i<nSlice; i++) 
            { 
                Iy[offsetz+i]=smoothfilter[0]*Islices[i+indexn]+smoothfilter[1]*Islices[i+indexc]+smoothfilter[2]*Islices[i+indexp];
            }
        }
       
        slice_select_p2=slice_select_p1; slice_select_p1=slice_select; slice_select++; if(slice_select>3) { slice_select=0; }
    }
    free(Irow);
    free(Islices);
}

void gradient3Dz_float(const float *I,const int *sizeI,float *&Iz)
{
    int x,y,z;
    int xp, xn, yp, yn;
    int i;
    int indexn, indexc, indexp;
    float *Irow, *Islices;
    int nSlice;
    int offsetz,offset_slice;
    int slice_select=0, slice_select_p1=0, slice_select_p2=0;
    
    const float smoothfilter[3]={0.187500,0.625000,0.187500};
    const float derivafilter[3]={-0.5,0,0.5};
    
    nSlice=sizeI[0]*sizeI[1];
    Islices=(float *)malloc(4*nSlice*sizeof(float));
    Irow=(float *)malloc(sizeI[0]*sizeof(float));
    
    for(z=0; z<sizeI[2]; z++)
    {
        offsetz=nSlice*z;
        offset_slice=nSlice*slice_select;
        
        for(y=0; y<sizeI[1]; y++)
        {
          /* Smooth y - direction  */
            yn=max(y-1,0);
            yp=min(y+1,sizeI[1]-1);
            
            indexn=yn*sizeI[0]+offsetz; 
            indexc=y*sizeI[0]+offsetz;
            indexp=yp*sizeI[0]+offsetz;
            
            for(x=0; x<sizeI[0]; x++)
            {
                Irow[x] =smoothfilter[0]*I[indexn+x];
                Irow[x]+=smoothfilter[1]*I[indexc+x];
                Irow[x]+=smoothfilter[2]*I[indexp+x];
            }

            indexc=y*sizeI[0]+offset_slice;
          /*  Gradient in x - direction  */
            for(x=0; x<sizeI[0]; x++)
            {
                xn=max(x-1,0); xp=min(x+1,sizeI[0]-1);
                Islices[indexc+x]=smoothfilter[0]*Irow[xn]+smoothfilter[1]*Irow[x]+smoothfilter[2]*Irow[xp];
            }
        }
       
        /* Smooth in z - direction  */
        if(z==1) /* Forward          */
        {
            indexn=slice_select_p1*nSlice; indexc=slice_select_p1*nSlice; indexp=slice_select*nSlice;
            for(i=0; i<nSlice; i++) 
            { 
                Iz[i]=derivafilter[0]*Islices[i+indexn]+derivafilter[1]*Islices[i+indexc]+derivafilter[2]*Islices[i+indexp];
            }
        }
        else if(z>1) /* Central  */
        {
            indexn=slice_select_p2*nSlice; indexc=slice_select_p1*nSlice; indexp=slice_select*nSlice;
            offsetz=nSlice*(z-1);
            for(i=0; i<nSlice; i++) 
            { 
                Iz[offsetz+i]=derivafilter[0]*Islices[i+indexn]+derivafilter[1]*Islices[i+indexc]+derivafilter[2]*Islices[i+indexp];
            }
        }
        
        if(z==(sizeI[2]-1)) /* Backward  */
        {
            indexn=slice_select_p1*nSlice; indexc=slice_select*nSlice; indexp=slice_select*nSlice;
            offsetz=nSlice*z;
            for(i=0; i<nSlice; i++) 
            { 
                Iz[offsetz+i]=derivafilter[0]*Islices[i+indexn]+derivafilter[1]*Islices[i+indexc]+derivafilter[2]*Islices[i+indexp];
            }
        }
       
        slice_select_p2=slice_select_p1; slice_select_p1=slice_select; slice_select++; if(slice_select>3) { slice_select=0; }

    }
    free(Irow);
    free(Islices);
}

bool q_derivatives_3D(const float *p_img32f,const long sz_img[4],const char xyz,float *&p_img32f_output)
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
	if(xyz!='x' && xyz!='y' && xyz!='z')
	{
		printf("ERROR: Para xyz should be x, y or z!\n");
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

	//calculate the derivatives
	int dimsI[3];
	dimsI[0]=sz_img[0];//ncol-width
	dimsI[1]=sz_img[1];//nrow-height
	dimsI[2]=sz_img[2];//ndep-z
    switch (xyz)
    {
     case 'x': case 'X':
         gradient3Dx_float(p_img32f, dimsI, p_img32f_output);
       break;
     case 'y': case 'Y':
         gradient3Dy_float(p_img32f, dimsI, p_img32f_output);
       break;
     case 'z': case 'Z':
         gradient3Dz_float(p_img32f, dimsI, p_img32f_output);
       break;
     default:
       printf("\n  option not defined");
       break;
    }

	return true;
}

