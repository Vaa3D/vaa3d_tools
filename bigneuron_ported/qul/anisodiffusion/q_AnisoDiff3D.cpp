// q_AnisoDiff3D.cpp
// by Lei Qu
// 2015-03-16
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <vector>

#include "stackutil.h"
#include "q_AnisoDiff3D.h"
#include "q_derivatives3D.h"
#include "q_imgaussian3D.h"
#include "q_EigenVectors3D.h"


bool q_AnisoDiff3D(const double *p_img64f_input,const V3DLONG sz_img_input[4],double *&p_img64f_output)
{
	//check paras
	if(p_img64f_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 || sz_img_input[2]<=0 || sz_img_input[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(sz_img_input[3]>1)
	{
		printf("ERROR: Input image should be single channel!\n");
		return false;
	}
	if(p_img64f_output)
	{
		printf("WARNNING: Output image pointer is not NULL, original data will be cleared!\n");
		if(p_img64f_output) 	{delete []p_img64f_output;		p_img64f_output=0;}
	}

	long l_npixels=sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3];
	long sz_img[4];
	sz_img[0]=sz_img_input[0];sz_img[1]=sz_img_input[1];sz_img[2]=sz_img_input[2];sz_img[3]=sz_img_input[3];

	//allocate memory for output image and initialize
	p_img64f_output=new(std::nothrow) double[l_npixels]();
	if(!p_img64f_output)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_output!\n");
		return false;
	}
	for(long i=0;i<l_npixels;i++)
		p_img64f_output[i]=p_img64f_input[i];

	//do diffusion
	clock_t iterstart;
	for(unsigned int iter=0;iter<15;iter++)
	{
		iterstart=clock();

		//------------------------------------------------------------------------------------------------------------------------------------
		//Calculate the gradient images.
		double *p_img64f_gradx=0,*p_img64f_grady=0,*p_img64f_gradz=0;
		{
			//Reduce the effect of inter-neuron intensity variation, for better neuron direction estimation
			double *p_img64f_tmp=0;
			p_img64f_tmp=new(std::nothrow) double[l_npixels]();
			if(!p_img64f_tmp)
			{
				printf("ERROR: Fail to allocate memory for p_img64f_tmp!\n");
				if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
				return false;
			}

			double d_foregound_thresh=2.0;
			for(long i=0;i<l_npixels;i++)
				if(p_img64f_output[i]>d_foregound_thresh)
					p_img64f_tmp[i]=255.0;
			//calculate the gradients
			if(!q_derivatives_3D(p_img64f_tmp,sz_img,'x',p_img64f_gradx) ||
				!q_derivatives_3D(p_img64f_tmp,sz_img,'y',p_img64f_grady) ||
				!q_derivatives_3D(p_img64f_tmp,sz_img,'z',p_img64f_gradz))
			{
				printf("ERROR: q_derivatives_3D() return false!\n");
				if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
				if(p_img64f_tmp) 			{delete []p_img64f_tmp;		p_img64f_tmp=0;}
				if(p_img64f_gradx) 			{delete []p_img64f_gradx;	p_img64f_gradx=0;}
				if(p_img64f_grady) 			{delete []p_img64f_grady;	p_img64f_grady=0;}
				if(p_img64f_gradz) 			{delete []p_img64f_gradz;	p_img64f_gradz=0;}
				return false;
			}
			//free memory
			if(p_img64f_tmp) 			{delete []p_img64f_tmp;		p_img64f_tmp=0;}
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		//Construct the 3D structure tensors
		double *p_img64f_Ixx=0,*p_img64f_Iyy=0,*p_img64f_Izz=0;
		double *p_img64f_Ixy=0,*p_img64f_Ixz=0,*p_img64f_Iyz=0;
		p_img64f_Ixx=new(std::nothrow) double[l_npixels]();
		p_img64f_Iyy=new(std::nothrow) double[l_npixels]();
		p_img64f_Izz=new(std::nothrow) double[l_npixels]();
		p_img64f_Ixy=new(std::nothrow) double[l_npixels]();
		p_img64f_Ixz=new(std::nothrow) double[l_npixels]();
		p_img64f_Iyz=new(std::nothrow) double[l_npixels]();
		if(!p_img64f_Ixx || !p_img64f_Iyy || !p_img64f_Izz|| !p_img64f_Ixy|| !p_img64f_Ixz|| !p_img64f_Iyz)
		{
			printf("ERROR: Fail to allocate memory for structure tensor images!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_gradx) 			{delete []p_img64f_gradx;	p_img64f_gradx=0;}
			if(p_img64f_grady) 			{delete []p_img64f_grady;	p_img64f_grady=0;}
			if(p_img64f_gradz) 			{delete []p_img64f_gradz;	p_img64f_gradz=0;}
			if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
			if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
			if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
			if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
			if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
			if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}
			return false;
		}
		for(long i=0;i<l_npixels;i++)
		{
			p_img64f_Ixx[i]=p_img64f_gradx[i]*p_img64f_gradx[i];
			p_img64f_Iyy[i]=p_img64f_grady[i]*p_img64f_grady[i];
			p_img64f_Izz[i]=p_img64f_gradz[i]*p_img64f_gradz[i];
			p_img64f_Ixy[i]=p_img64f_gradx[i]*p_img64f_grady[i];
			p_img64f_Ixz[i]=p_img64f_gradx[i]*p_img64f_gradz[i];
			p_img64f_Iyz[i]=p_img64f_grady[i]*p_img64f_gradz[i];
		}
		//free memory
		if(p_img64f_gradx) 			{delete []p_img64f_gradx;	p_img64f_gradx=0;}
		if(p_img64f_grady) 			{delete []p_img64f_grady;	p_img64f_grady=0;}
		if(p_img64f_gradz) 			{delete []p_img64f_gradz;	p_img64f_gradz=0;}

		//Smooth the structure tensors, for better neuron direction estimation
		double *p_img64f_Ixx_smooth=0,*p_img64f_Iyy_smooth=0,*p_img64f_Izz_smooth=0;
		double *p_img64f_Ixy_smooth=0,*p_img64f_Ixz_smooth=0,*p_img64f_Iyz_smooth=0;
		double sigma=3,szkernel=sigma*4;
		if(!q_imgaussian3D(p_img64f_Ixx,sz_img,sigma,szkernel,p_img64f_Ixx_smooth))
		{
			printf("ERROR: q_imgaussian3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
			if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
			if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
			if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
			if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
			if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}
			if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
			if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
			if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
			if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
			if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
			if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}
			return false;
		}
		if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
		if(!q_imgaussian3D(p_img64f_Iyy,sz_img,sigma,szkernel,p_img64f_Iyy_smooth))
		{
			printf("ERROR: q_imgaussian3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
			if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
			if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
			if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
			if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
			if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}
			if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
			if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
			if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
			if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
			if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
			if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}
			return false;
		}
		if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
		if(!q_imgaussian3D(p_img64f_Izz,sz_img,sigma,szkernel,p_img64f_Izz_smooth))
		{
			printf("ERROR: q_imgaussian3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
			if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
			if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
			if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
			if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
			if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}
			if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
			if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
			if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
			if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
			if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
			if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}
			return false;
		}
		if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
		if(!q_imgaussian3D(p_img64f_Ixy,sz_img,sigma,szkernel,p_img64f_Ixy_smooth))
		{
			printf("ERROR: q_imgaussian3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
			if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
			if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
			if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
			if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
			if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}
			if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
			if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
			if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
			if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
			if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
			if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}
			return false;
		}
		if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
		if(!q_imgaussian3D(p_img64f_Ixz,sz_img,sigma,szkernel,p_img64f_Ixz_smooth))
		{
			printf("ERROR: q_imgaussian3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
			if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
			if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
			if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
			if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
			if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}
			if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
			if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
			if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
			if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
			if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
			if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}
			return false;
		}
		if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
		if(!q_imgaussian3D(p_img64f_Iyz,sz_img,sigma,szkernel,p_img64f_Iyz_smooth))
		{
			printf("ERROR: q_imgaussian3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Ixx) 			{delete []p_img64f_Ixx;		p_img64f_Ixx=0;}
			if(p_img64f_Iyy) 			{delete []p_img64f_Iyy;		p_img64f_Iyy=0;}
			if(p_img64f_Izz) 			{delete []p_img64f_Izz;		p_img64f_Izz=0;}
			if(p_img64f_Ixy) 			{delete []p_img64f_Ixy;		p_img64f_Ixy=0;}
			if(p_img64f_Ixz) 			{delete []p_img64f_Ixz;		p_img64f_Ixz=0;}
			if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}
			if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
			if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
			if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
			if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
			if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
			if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}
			return false;
		}
		if(p_img64f_Iyz) 			{delete []p_img64f_Iyz;		p_img64f_Iyz=0;}

		//Compute the eigenvectors and values(eigval3>eigval2>eigval1) of the structure tensors
		double *p_img64f_eigval3=0,*p_img64f_eigval2=0,*p_img64f_eigval1=0;
		double *p_img64f_eigvec3x=0,*p_img64f_eigvec3y=0,*p_img64f_eigvec3z=0;
		double *p_img64f_eigvec2x=0,*p_img64f_eigvec2y=0,*p_img64f_eigvec2z=0;
		double *p_img64f_eigvec1x=0,*p_img64f_eigvec1y=0,*p_img64f_eigvec1z=0;
		if(!q_eigenvectors3D(p_img64f_Ixx_smooth,p_img64f_Ixy_smooth,p_img64f_Ixz_smooth,
							 p_img64f_Iyy_smooth,p_img64f_Iyz_smooth,p_img64f_Izz_smooth,
							 sz_img,
							 p_img64f_eigval3,p_img64f_eigval2,p_img64f_eigval1,
							 p_img64f_eigvec3x,p_img64f_eigvec3y,p_img64f_eigvec3z,
							 p_img64f_eigvec2x,p_img64f_eigvec2y,p_img64f_eigvec2z,
							 p_img64f_eigvec1x,p_img64f_eigvec1y,p_img64f_eigvec1z))
		{
			printf("ERROR: q_eigenvectors3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
			if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
			if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
			if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
			if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
			if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}
			if(p_img64f_eigval3) 		{delete []p_img64f_eigval3;		p_img64f_eigval3=0;}
			if(p_img64f_eigval2) 		{delete []p_img64f_eigval2;		p_img64f_eigval2=0;}
			if(p_img64f_eigval1) 		{delete []p_img64f_eigval1;		p_img64f_eigval1=0;}
			if(p_img64f_eigvec3x) 		{delete []p_img64f_eigvec3x;	p_img64f_eigvec3x=0;}
			if(p_img64f_eigvec3y) 		{delete []p_img64f_eigvec3y;	p_img64f_eigvec3y=0;}
			if(p_img64f_eigvec3z) 		{delete []p_img64f_eigvec3z;	p_img64f_eigvec3z=0;}
			if(p_img64f_eigvec2x) 		{delete []p_img64f_eigvec2x;	p_img64f_eigvec2x=0;}
			if(p_img64f_eigvec2y) 		{delete []p_img64f_eigvec2y;	p_img64f_eigvec2y=0;}
			if(p_img64f_eigvec2z) 		{delete []p_img64f_eigvec2z;	p_img64f_eigvec2z=0;}
			if(p_img64f_eigvec1x) 		{delete []p_img64f_eigvec1x;	p_img64f_eigvec1x=0;}
			if(p_img64f_eigvec1y) 		{delete []p_img64f_eigvec1y;	p_img64f_eigvec1y=0;}
			if(p_img64f_eigvec1z) 		{delete []p_img64f_eigvec1z;	p_img64f_eigvec1z=0;}
			return false;
		}
			
		//free memory
		if(p_img64f_Ixx_smooth) 	{delete []p_img64f_Ixx_smooth;	p_img64f_Ixx_smooth=0;}
		if(p_img64f_Iyy_smooth) 	{delete []p_img64f_Iyy_smooth;	p_img64f_Iyy_smooth=0;}
		if(p_img64f_Izz_smooth) 	{delete []p_img64f_Izz_smooth;	p_img64f_Izz_smooth=0;}
		if(p_img64f_Ixy_smooth) 	{delete []p_img64f_Ixy_smooth;	p_img64f_Ixy_smooth=0;}
		if(p_img64f_Ixz_smooth) 	{delete []p_img64f_Ixz_smooth;	p_img64f_Ixz_smooth=0;}
		if(p_img64f_Iyz_smooth) 	{delete []p_img64f_Iyz_smooth;	p_img64f_Iyz_smooth=0;}

		//------------------------------------------------------------------------------------------------------------------------------------
		//Construct Diffusion Tensor
		//diffusion along direction (v1) with minimal eigenvalue (mu1) - neuron direction
		//D=lamda1*v1*v1'+lamda2*v2*v2'+lamda1*v3*v3'
		double *p_img64f_Dxx=0,*p_img64f_Dyy=0,*p_img64f_Dzz=0;
		double *p_img64f_Dxy=0,*p_img64f_Dxz=0,*p_img64f_Dyz=0;
		p_img64f_Dxx=new(std::nothrow) double[l_npixels]();	p_img64f_Dyy=new(std::nothrow) double[l_npixels]();	p_img64f_Dzz=new(std::nothrow) double[l_npixels]();
		p_img64f_Dxy=new(std::nothrow) double[l_npixels]();	p_img64f_Dxz=new(std::nothrow) double[l_npixels]();	p_img64f_Dyz=new(std::nothrow) double[l_npixels]();
		if(!p_img64f_Dxx || !p_img64f_Dyy || !p_img64f_Dzz|| !p_img64f_Dxy|| !p_img64f_Dxz|| !p_img64f_Dyz)
		{
			printf("ERROR: Fail to allocate memory for diffusion tensor images!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_eigval3) 		{delete []p_img64f_eigval3;		p_img64f_eigval3=0;}
			if(p_img64f_eigval2) 		{delete []p_img64f_eigval2;		p_img64f_eigval2=0;}
			if(p_img64f_eigval1) 		{delete []p_img64f_eigval1;		p_img64f_eigval1=0;}
			if(p_img64f_eigvec3x) 		{delete []p_img64f_eigvec3x;	p_img64f_eigvec3x=0;}
			if(p_img64f_eigvec3y) 		{delete []p_img64f_eigvec3y;	p_img64f_eigvec3y=0;}
			if(p_img64f_eigvec3z) 		{delete []p_img64f_eigvec3z;	p_img64f_eigvec3z=0;}
			if(p_img64f_eigvec2x) 		{delete []p_img64f_eigvec2x;	p_img64f_eigvec2x=0;}
			if(p_img64f_eigvec2y) 		{delete []p_img64f_eigvec2y;	p_img64f_eigvec2y=0;}
			if(p_img64f_eigvec2z) 		{delete []p_img64f_eigvec2z;	p_img64f_eigvec2z=0;}
			if(p_img64f_eigvec1x) 		{delete []p_img64f_eigvec1x;	p_img64f_eigvec1x=0;}
			if(p_img64f_eigvec1y) 		{delete []p_img64f_eigvec1y;	p_img64f_eigvec1y=0;}
			if(p_img64f_eigvec1z) 		{delete []p_img64f_eigvec1z;	p_img64f_eigvec1z=0;}
			if(p_img64f_Dxx) 			{delete []p_img64f_Dxx;		p_img64f_Dxx=0;}
			if(p_img64f_Dyy) 			{delete []p_img64f_Dyy;		p_img64f_Dyy=0;}
			if(p_img64f_Dzz) 			{delete []p_img64f_Dzz;		p_img64f_Dzz=0;}
			if(p_img64f_Dxy) 			{delete []p_img64f_Dxy;		p_img64f_Dxy=0;}
			if(p_img64f_Dxz) 			{delete []p_img64f_Dxz;		p_img64f_Dxz=0;}
			if(p_img64f_Dyz) 			{delete []p_img64f_Dyz;		p_img64f_Dyz=0;}
			return false;
		}

		double alpha=1e-10,C=1e-10;
		for(long i=0;i<l_npixels;i++)
		{
			//calculate diffusion strength along different directions
			double di=p_img64f_eigval3[i]-p_img64f_eigval1[i];
			double di2=di*di;
			if(di2<1e-15) di2=1e-15;
			double lamda1=alpha+(1-alpha)*exp(-C/di2); 
			double lamda2=alpha;
			double lamda3=alpha;
			//construct diffusion tensor
			p_img64f_Dxx[i]=lamda1*p_img64f_eigvec1x[i]*p_img64f_eigvec1x[i]+
				lamda2*p_img64f_eigvec2x[i]*p_img64f_eigvec2x[i]+
				lamda3*p_img64f_eigvec3x[i]*p_img64f_eigvec3x[i];
			p_img64f_Dyy[i]=lamda1*p_img64f_eigvec1y[i]*p_img64f_eigvec1y[i]+
				lamda2*p_img64f_eigvec2y[i]*p_img64f_eigvec2y[i]+
				lamda3*p_img64f_eigvec3y[i]*p_img64f_eigvec3y[i];
			p_img64f_Dzz[i]=lamda1*p_img64f_eigvec1z[i]*p_img64f_eigvec1z[i]+
				lamda2*p_img64f_eigvec2z[i]*p_img64f_eigvec2z[i]+
				lamda3*p_img64f_eigvec3z[i]*p_img64f_eigvec3z[i];
			p_img64f_Dxy[i]=lamda1*p_img64f_eigvec1x[i]*p_img64f_eigvec1y[i]+
				lamda2*p_img64f_eigvec2x[i]*p_img64f_eigvec2y[i]+
				lamda3*p_img64f_eigvec3x[i]*p_img64f_eigvec3y[i];
			p_img64f_Dxz[i]=lamda1*p_img64f_eigvec1x[i]*p_img64f_eigvec1z[i]+
				lamda2*p_img64f_eigvec2x[i]*p_img64f_eigvec2z[i]+
				lamda3*p_img64f_eigvec3x[i]*p_img64f_eigvec3z[i];
			p_img64f_Dyz[i]=lamda1*p_img64f_eigvec1y[i]*p_img64f_eigvec1z[i]+
				lamda2*p_img64f_eigvec2y[i]*p_img64f_eigvec2z[i]+
				lamda3*p_img64f_eigvec3y[i]*p_img64f_eigvec3z[i];
		}
		//free memory
		if(p_img64f_eigval3) 		{delete []p_img64f_eigval3;		p_img64f_eigval3=0;}
		if(p_img64f_eigval2) 		{delete []p_img64f_eigval2;		p_img64f_eigval2=0;}
		if(p_img64f_eigval1) 		{delete []p_img64f_eigval1;		p_img64f_eigval1=0;}
		if(p_img64f_eigvec3x) 		{delete []p_img64f_eigvec3x;	p_img64f_eigvec3x=0;}
		if(p_img64f_eigvec3y) 		{delete []p_img64f_eigvec3y;	p_img64f_eigvec3y=0;}
		if(p_img64f_eigvec3z) 		{delete []p_img64f_eigvec3z;	p_img64f_eigvec3z=0;}
		if(p_img64f_eigvec2x) 		{delete []p_img64f_eigvec2x;	p_img64f_eigvec2x=0;}
		if(p_img64f_eigvec2y) 		{delete []p_img64f_eigvec2y;	p_img64f_eigvec2y=0;}
		if(p_img64f_eigvec2z) 		{delete []p_img64f_eigvec2z;	p_img64f_eigvec2z=0;}
		if(p_img64f_eigvec1x) 		{delete []p_img64f_eigvec1x;	p_img64f_eigvec1x=0;}
		if(p_img64f_eigvec1y) 		{delete []p_img64f_eigvec1y;	p_img64f_eigvec1y=0;}
		if(p_img64f_eigvec1z) 		{delete []p_img64f_eigvec1z;	p_img64f_eigvec1z=0;}

		//------------------------------------------------------------------------------------------------------------------------------------
		//Do diffusion: dI=div(D*grad(I))
		//calculate the gradients on original image
		if(!q_derivatives_3D(p_img64f_output,sz_img,'x',p_img64f_gradx) ||
		   !q_derivatives_3D(p_img64f_output,sz_img,'y',p_img64f_grady) ||
		   !q_derivatives_3D(p_img64f_output,sz_img,'z',p_img64f_gradz))
		{
			printf("ERROR: q_eigenvectors3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_Dxx) 			{delete []p_img64f_Dxx;		p_img64f_Dxx=0;}
			if(p_img64f_Dyy) 			{delete []p_img64f_Dyy;		p_img64f_Dyy=0;}
			if(p_img64f_Dzz) 			{delete []p_img64f_Dzz;		p_img64f_Dzz=0;}
			if(p_img64f_Dxy) 			{delete []p_img64f_Dxy;		p_img64f_Dxy=0;}
			if(p_img64f_Dxz) 			{delete []p_img64f_Dxz;		p_img64f_Dxz=0;}
			if(p_img64f_Dyz) 			{delete []p_img64f_Dyz;		p_img64f_Dyz=0;}
			if(p_img64f_gradx) 			{delete []p_img64f_gradx;	p_img64f_gradx=0;}
			if(p_img64f_grady) 			{delete []p_img64f_grady;	p_img64f_grady=0;}
			if(p_img64f_gradz) 			{delete []p_img64f_gradz;	p_img64f_gradz=0;}
			return false;
		}

		//calculate the flux components: D*grad(I)
		double *p_img64f_j1=0,*p_img64f_j2=0,*p_img64f_j3=0;
		p_img64f_j1=new(std::nothrow) double[l_npixels](); p_img64f_j2=new(std::nothrow) double[l_npixels](); p_img64f_j3=new(std::nothrow) double[l_npixels]();
		for(long i=0;i<l_npixels;i++)
		{
			p_img64f_j1[i]=p_img64f_Dxx[i]*p_img64f_gradx[i]+p_img64f_Dxy[i]*p_img64f_grady[i]+p_img64f_Dxz[i]*p_img64f_gradz[i];
			p_img64f_j2[i]=p_img64f_Dxy[i]*p_img64f_gradx[i]+p_img64f_Dyy[i]*p_img64f_grady[i]+p_img64f_Dyz[i]*p_img64f_gradz[i];
			p_img64f_j3[i]=p_img64f_Dxz[i]*p_img64f_gradx[i]+p_img64f_Dyz[i]*p_img64f_grady[i]+p_img64f_Dzz[i]*p_img64f_gradz[i];
		}
		if(p_img64f_Dxx) 			{delete []p_img64f_Dxx;		p_img64f_Dxx=0;}
		if(p_img64f_Dyy) 			{delete []p_img64f_Dyy;		p_img64f_Dyy=0;}
		if(p_img64f_Dzz) 			{delete []p_img64f_Dzz;		p_img64f_Dzz=0;}
		if(p_img64f_Dxy) 			{delete []p_img64f_Dxy;		p_img64f_Dxy=0;}
		if(p_img64f_Dxz) 			{delete []p_img64f_Dxz;		p_img64f_Dxz=0;}
		if(p_img64f_Dyz) 			{delete []p_img64f_Dyz;		p_img64f_Dyz=0;}
		if(p_img64f_gradx) 			{delete []p_img64f_gradx;	p_img64f_gradx=0;}
		if(p_img64f_grady) 			{delete []p_img64f_grady;	p_img64f_grady=0;}
		if(p_img64f_gradz) 			{delete []p_img64f_gradz;	p_img64f_gradz=0;}

		//calculate increment of flux
		double *p_img64f_j1x=0,*p_img64f_j2y=0,*p_img64f_j3z=0;
		if(!q_derivatives_3D(p_img64f_j1,sz_img,'x',p_img64f_j1x) ||
		   !q_derivatives_3D(p_img64f_j2,sz_img,'y',p_img64f_j2y) ||
		   !q_derivatives_3D(p_img64f_j3,sz_img,'z',p_img64f_j3z))
		{
			printf("ERROR: q_derivatives_3D() return false!\n");
			if(p_img64f_output) 		{delete []p_img64f_output;	p_img64f_output=0;}
			if(p_img64f_gradx) 			{delete []p_img64f_gradx;	p_img64f_gradx=0;}
			if(p_img64f_grady) 			{delete []p_img64f_grady;	p_img64f_grady=0;}
			if(p_img64f_gradz) 			{delete []p_img64f_gradz;	p_img64f_gradz=0;}
			if(p_img64f_j1) 			{delete []p_img64f_j1;		p_img64f_j1=0;}
			if(p_img64f_j2) 			{delete []p_img64f_j2;		p_img64f_j2=0;}
			if(p_img64f_j3) 			{delete []p_img64f_j3;		p_img64f_j3=0;}
			return false;
		}
		if(p_img64f_j1) 			{delete []p_img64f_j1;		p_img64f_j1=0;}
		if(p_img64f_j2) 			{delete []p_img64f_j2;		p_img64f_j2=0;}
		if(p_img64f_j3) 			{delete []p_img64f_j3;		p_img64f_j3=0;}

		//update in an explicit way
		for(long i=0;i<l_npixels;i++)
		{
			//calculate increment div(D*grad(I))
			double du=p_img64f_j1x[i]+p_img64f_j2y[i]+p_img64f_j3z[i];
			//guarantee no signal lost
			if(du<=0) du=0;
			p_img64f_output[i]+=du;
			if(p_img64f_output[i]>255.0) p_img64f_output[i]=255.0;
		}
		if(p_img64f_j1x) 			{delete []p_img64f_j1x;		p_img64f_j1x=0;}
		if(p_img64f_j2y) 			{delete []p_img64f_j2y;		p_img64f_j2y=0;}
		if(p_img64f_j3z) 			{delete []p_img64f_j3z;		p_img64f_j3z=0;}

		printf(">>iter=%3d, timeconsume=%.3f s\n",iter,((float)(clock()-iterstart))/CLOCKS_PER_SEC);
	}

	//{
	//	unsigned char *p_img_tmp=new(std::nothrow) unsigned char[l_npixels]();
	//	for(long i=0;i<l_npixels;i++)
	//		p_img_tmp[i]=(unsigned int)(p_img64f_output[i]);
	//	saveImage("d:/SVN/Vaa3D_source_code/v3d_external/released_plugins/v3d_plugins/anisodiffusion/output.raw",p_img_tmp,sz_img_input,1);
	//	delete []p_img_tmp;
	//}


	return true;
}