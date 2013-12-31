// curve/surface evolution according to the level set evolution equation in Chunming Li et al's paper: 
//      "Level Set Evolution Without Reinitialization: A New Variational Formulation"
//       in Proceedings CVPR'2005 

//latest 200910107

#ifndef __EVOLVE_CPP__
#define __EVOLVE_CPP__

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "img_definition.h"
//#include "stackutil.h"
//#include "basic_memory.h"


//compute 2D gradient 
template <class T> void gradient(T **image, float **fx, float **fy, const V3DLONG *sz)
{
	int x,y;

	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)
	{
		fx[y][x] = 0;
		fy[y][x] = 0;
	}

	// y dimension
	
	for(y=1;y<sz[1]-1;y++)
	for(x=0;x<sz[0];x++)	
	{
		fy[y][x] = ((float)image[y+1][x] - (float)image[y-1][x])/2;
	}

	//deal with margin 	
	for(x=0;x<sz[0];x++)	
	{
		fy[0][x] = (float)image[1][x] - (float)image[0][x]; 
		fy[sz[1]-1][x] = (float)image[sz[1]-1][x] - (float)image[sz[1]-2][x];
	}
	
	// x dimension
	for(y=0;y<sz[1];y++)
	for(x=1;x<sz[0]-1;x++)	
	{
		fx[y][x] = ((float)image[y][x+1] - (float)image[y][x-1])/2; 
	}
	
	//deal with margin
	for(y=0;y<sz[1];y++)	
	{
		fx[y][0] = (float)image[y][1] - (float)image[y][0]; 
		fx[y][sz[0]-1] = (float)image[y][sz[0]-1] - (float)image[y][sz[0]-2];
	}			
							
	return;
}


//compute 3D gradient
template <class T> void gradient(T ***image, float ***fx, float ***fy, float ***fz, const V3DLONG *sz)
{
	int x,y,z;

	for(z=0;z<sz[2];z++)
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)
	{
		fx[z][y][x] = 0;
		fy[z][y][x] = 0;
		fz[z][y][x] = 0;
	}

	// z dimension
	
	for(z=1;z<sz[2]-1;z++)
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)	
	{
		fz[z][y][x] = ((float)image[z+1][y][x] - (float)image[z-1][y][x])/2;
	}

	//deal with margin
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)	
	{
		fz[0][y][x] = (float)image[1][y][x] - (float)image[0][y][x]; 
		fz[sz[2]-1][y][x] = (float)image[sz[2]-1][y][x] - (float)image[sz[2]-2][y][x];
	}
	
	// y dimension
	for(y=1;y<sz[1]-1;y++)
	for(z=0;z<sz[2];z++)
	for(x=0;x<sz[0];x++)	
	{
		fy[z][y][x] = ((float)image[z][y+1][x] - (float)image[z][y-1][x])/2;
	}
	
	//deal with margin
	for(z=0;z<sz[2];z++)
	for(x=0;x<sz[0];x++)	
	{
		fy[z][0][x] = (float)image[z][1][x] - (float)image[z][0][x]; 
		fy[z][sz[1]-1][x] = (float)image[z][sz[1]-1][x] - (float)image[z][sz[1]-2][x];
	}
		
	// x dimension
	for(x=1;x<sz[0]-1;x++)
	for(z=0;z<sz[2];z++)
	for(y=0;y<sz[1];y++)	
	{
		fx[z][y][x] = ((float)image[z][y][x+1] - (float)image[z][y][x-1])/2;
	}
	
	//deal with margin
	for(z=0;z<sz[2];z++)
	for(y=0;y<sz[1];y++)	
	{
		fx[z][y][0] = (float)image[z][y][1] - (float)image[z][y][0]; 
		fx[z][y][sz[0]-1] = (float)image[z][y][sz[0]-1] - (float)image[z][y][sz[0]-2];
	}
							
	return;
		
}

//Normalize 2D gradient 
void gradientNoramlize(float **fx, float **fy, const V3DLONG *sz)
{
	int x,y;
	float tmp;
		
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)	
	{
		tmp = sqrt(fx[y][x]*fx[y][x] + fy[y][x]*fy[y][x] + 1e-10);
		fx[y][x] = fx[y][x]/tmp;
		fy[y][x] = fy[y][x]/tmp;
		
	}
}


//Normalize 3D gradient 
void gradientNoramlize(float ***fx, float ***fy, float ***fz, const V3DLONG *sz)
{
	int x,y,z;
	float tmp;
		
	for(z=0;z<sz[2];z++)
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)	
	{
		tmp = sqrt(fx[z][y][x]*fx[z][y][x] + fy[z][y][x]*fy[z][y][x]  + fz[z][y][x]*fz[z][y][x]+ 1e-10);
		fx[z][y][x] = fx[z][y][x]/tmp;
		fy[z][y][x] = fy[z][y][x]/tmp;
		fz[z][y][x] = fz[z][y][x]/tmp;
		
	}
}

//compute 2D laplacian
template <class T> void laplacian(T **image, float **lap, const V3DLONG *sz)
{
	int x,y;
	float **laptmp = 0, *laptmp1d=0;
   laptmp1d = new float [sz[0]*sz[1]];
   if (laptmp1d)
	   new2dpointer(laptmp, sz[0], sz[1], laptmp1d);
	
	// initializing lap and laptmp to 0 decreases speed,  not necessary here, but safe
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)
	{
		lap[y][x] = 0;
		laptmp[y][x] = 0;
	}

	// y dimension
	
	for(y=1;y<sz[1]-1;y++)
	for(x=0;x<sz[0];x++)	
	{
		lap[y][x] = ((float)image[y+1][x] + (float)image[y-1][x])/2 - (float)image[y][x]; 
	}
		
	// extrapolation
	for (x=0;x<sz[0]; x++)
		lap[0][x] = 2*(float)lap[1][x] - (float)lap[2][x];
		
	for (x=0;x<sz[0]; x++)
		lap[sz[1]-1][x] = -(float)lap[sz[1]-3][x] + 2*(float)lap[sz[1]-2][x];
		

	// x dimension

	for(y=0;y<sz[1];y++)		
	for(x=1;x<sz[0]-1;x++)
	{
		laptmp[y][x] = ((float)image[y][x+1] + (float)image[y][x-1])/2 - (float)image[y][x]; 
	}
		
	// extrapolation		
	for (y=0;y<sz[1]; y++)
		laptmp[y][0] = 2*(float)laptmp[y][1] - (float)laptmp[y][2];
		
	for (y=0;y<sz[1]; y++)
		laptmp[y][sz[0]-1] = -(float)laptmp[y][sz[0]-3] + 2*(float)laptmp[y][sz[0]-2];

	// combine x and y dimensions
	for(y=0;y<sz[1];y++)		
	for(x=0;x<sz[0];x++)
	{
		lap[y][x] = (lap[y][x]+laptmp[y][x])/2; 
	}


   if (laptmp) delete2dpointer(laptmp, sz[0], sz[1]);
   if (laptmp1d) {delete []laptmp1d; laptmp1d=0;}
			
		
}


//compute 3D laplacian
template <class T> void laplacian(T ***image, float ***lap, const V3DLONG *sz)
{
	int x,y, z;
	float ***laptmp = 0, *laptmp1d=0;
   laptmp1d = new float [sz[0]*sz[1]*sz[2]];
   if (laptmp1d)
	   new3dpointer(laptmp, sz[0], sz[1], sz[2], laptmp1d);

	// initializing lap and laptmp to 0 decreases speed, not necessary here, but safe	   
	for(z=0;z<sz[2];z++)
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)
	{
		lap[z][y][x] = 0;
		laptmp[z][y][x] = 0;		
	}

	// z dimension
	for(z=1;z<sz[2]-1;z++)	
	for(y=0;y<sz[1];y++)
	for(x=0;x<sz[0];x++)	
	{
		lap[z][y][x] = ((float)image[z+1][y][x] + (float)image[z-1][y][x])/2 - (float)image[z][y][x]; 
	}
		
	// extrapolation
	for (y=0;y<sz[1];y++)
	for (x=0;x<sz[0]; x++)
		lap[0][y][x] = 2*(float)lap[1][y][x] - (float)lap[2][y][x];

	for (y=0;y<sz[1];y++)		
	for (x=0;x<sz[0]; x++)
		lap[sz[2]-1][y][x] = -(float)lap[sz[2]-3][y][x] + 2*(float)lap[sz[2]-2][y][x];
		
	// y dimension
	for(z=0;z<sz[2];z++)	
	for(y=1;y<sz[1]-1;y++)
	for(x=0;x<sz[0];x++)	
	{
		laptmp[z][y][x] = ((float)image[z][y+1][x] + (float)image[z][y-1][x])/2 - (float)image[z][y][x]; 
	}
		
	// extrapolation
	for (z=0;z<sz[2];z++)
	for (x=0;x<sz[0];x++)
		laptmp[z][0][x] = 2*(float)laptmp[z][1][x] - (float)laptmp[z][2][x];

	for (z=0;z<sz[2];z++)
	for (x=0;x<sz[0];x++)
		laptmp[z][sz[1]-1][x] = -(float)laptmp[z][sz[1]-3][x] + 2*(float)laptmp[z][sz[1]-2][x];

	// combine y and z dimension
	for(z=0;z<sz[2];z++)
	for(y=0;y<sz[1];y++)		
	for(x=0;x<sz[0];x++)
	{
		lap[z][y][x]+=laptmp[z][y][x]; 
	}
	
	// x dimension
	for(z=0;z<sz[2];z++)	
	for(y=0;y<sz[1];y++)
	for(x=1;x<sz[0]-1;x++)	
	{
		laptmp[z][y][x] = ((float)image[z][y][x+1] + (float)image[z][y][x-1])/2 - (float)image[z][y][x]; 
	}

	// extrapolation
	for (z=0;z<sz[2];z++)
	for (y=0;y<sz[1];y++)
		laptmp[z][y][0] = 2*(float)laptmp[z][y][1] - (float)laptmp[z][y][2];

	for (z=0;z<sz[2];z++)
	for (y=0;y<sz[1];y++)
		laptmp[z][y][sz[0]-1] = -(float)laptmp[z][y][sz[0]-3] + 2*(float)laptmp[z][y][sz[0]-2];
		
	// combine x,y, and z dimension
	for(z=0;z<sz[2];z++)
	for(y=0;y<sz[1];y++)		
	for(x=0;x<sz[0];x++)
	{
		lap[z][y][x]=(lap[z][y][x] + laptmp[z][y][x])/3; 
	}

   if (laptmp) delete3dpointer(laptmp, sz[0], sz[1], sz[2]);
   if (laptmp1d) {delete []laptmp1d; laptmp1d=0;}
			
//	for(z=1;z<sz[2]-1;z++)
//	for(y=1;y<sz[1]-1;y++)
//	for(x=1;x<sz[0]-1;x++)	
//	{
////		lap[z][y][x] = image[z][y][x+1] + image[z][y][x-1] + image[z][y+1][x] + image[z][y-1][x] + image[z+1][y][x] + image[z-1][y][x] - 6*image[z][y][x]; 
//		lap[z][y][x] = (image[z][y][x+1] + image[z][y][x-1] + image[z][y+1][x] + image[z][y-1][x] + image[z+1][y][x] + image[z-1][y][x])/6 - image[z][y][x]; 
//
//	}
		
}


//generate a smeared 2D dirac function, allocate memory of f2d before calling Dirac
// f2d is the output

//float **Dirac(float **x, float sigma, const V3DLONG *sz)
void Dirac(float **x, float sigma, const V3DLONG *sz, float **f2d)
{

//	float *f1d, **f2d;
	int i,j;
	
//	//allocate memory for f2d
//   V3DLONG tmplen = sz[0]*sz[1];
//   if (tmplen>0 && sz[0]>0 && sz[1]>0)
//   {
//	   f1d = new float [tmplen];
//	   if (f1d)
//	   {
//		   f2d = 0;
//		   new2dpointer(f2d, sz[0], sz[1], f1d);
//		}
//	}
			   
	//compute f2d	
	for(j=0; j<sz[1];j++)
	for(i=0; i<sz[0]; i++)
	{		
		if ((x[j][i]<=sigma) && (x[j][i]>=-sigma))
		{
			f2d[j][i]=(1.0/2.0/sigma)*(1+cos(3.1415926*x[j][i]/sigma)); // note if write 1/2, will cause serious problems			
		}
		else
		{
			f2d[j][i] = 0;
		}
		
	}
//	return f2d;
}

//generate a smeared 3D dirac function, not ready yet
void Dirac(float ***x, float sigma, const V3DLONG *sz, float ***f3d)
{

//	float *f1d=0, ***f3d = 0;
	int i,j,k;
	
//	//allocate memory for f3d
//   V3DLONG tmplen = sz[0]*sz[1]*sz[2];
//   if (tmplen>0 && sz[0]>0 && sz[1]>0 && sz[2]>0)
//   {
//	   f1d = new float [tmplen];
//	   if (f1d)
//		   new3dpointer(f3d, sz[0], sz[1], sz[2], f1d);
//	}
			   
	//compute f3d
	for(k=0; k<sz[2]; k++)
	for(j=0; j<sz[1]; j++)
	for(i=0; i<sz[0]; i++)
	{
		if ((x[k][j][i]<=sigma) && (x[k][j][i]>=-sigma))
		{
			f3d[k][j][i]=(1.0/2.0/sigma)*(1+cos(3.1415926*x[k][j][i]/sigma));
		}
		else
		{
			f3d[k][j][i] = 0;
		}
	}
//	return f3d;
}

//compute 2D curvature, allocate memory of K before calling curvatureCentral
// K is the output

//float **curvatureCentral(float **nx, float **ny, const V3DLONG *sz, float **K)
void curvatureCentral(float **nx, float **ny, const V3DLONG *sz, float **K)
{

	float **nxx=0,**nxy=0, **nyx=0, **nyy=0;
	float *nxx1d=0, *nxy1d=0, *nyx1d=0, *nyy1d=0;
		
//	float **nxx,**nxy, **nyx, **nyy, **K;
//	float *nxx1d, *nxy1d, *nyx1d, *nyy1d, *K1d;


	//allocat memory for nxx, nxy, nyx, nyy
   V3DLONG tmplen = sz[0]*sz[1];
   if (tmplen>0 && sz[0]>0 && sz[1]>0)
   {
	   nxx1d = new float [tmplen];
	   if (nxx1d)
		   new2dpointer(nxx, sz[0], sz[1], nxx1d);
		
	   nxy1d = new float [tmplen];
	   if (nxy1d)
		   new2dpointer(nxy, sz[0], sz[1], nxy1d);

	   nyx1d = new float [tmplen];
	   if (nyx1d)
		   new2dpointer(nyx, sz[0], sz[1], nyx1d);

	   nyy1d = new float [tmplen];
	   if (nyy1d)
		   new2dpointer(nyy, sz[0], sz[1], nyy1d);
		
//		K1d = new float [tmplen];
//		if (K1d)
//		{
//			K = 0;
//			new2dpointer(K, sz[0], sz[1], K1d);
//		}
		
	}
	
	
	gradient(nx, nxx, nxy, sz);  
	gradient(ny, nyx, nyy, sz);

	for (int j=0; j<sz[1]; j++)	
	for (int i=0; i<sz[0]; i++)
	{
		K[j][i] =nxx[j][i]+nyy[j][i];
	}
	
	// delete 1d and 2d pointers

   if (nxx) delete2dpointer(nxx, sz[0], sz[1]);
   if (nxx1d) {delete []nxx1d; nxx1d=0;}

   if (nxy) delete2dpointer(nxy, sz[0], sz[1]);
   if (nxy1d) {delete []nxy1d; nxy1d=0;}

   if (nyx) delete2dpointer(nyx, sz[0], sz[1]);
   if (nyx1d) {delete []nyx1d; nyx1d=0;}

   if (nyy) delete2dpointer(nyy, sz[0], sz[1]);
   if (nyy1d) {delete []nyy1d; nyy1d=0;}	
	
//	return K;
}


//compute 3D curvature, allocate memory of K before calling the function
// K is the output
float ***curvatureCentral(float ***nx, float ***ny, float ***nz, const V3DLONG *sz, float ***K)
{
	float ***nxx=0,***nxy=0, ***nxz=0, ***nyx=0, ***nyy=0, ***nyz=0, ***nzx=0, ***nzy=0, ***nzz=0;
	float *nxx1d=0, *nxy1d=0, *nxz1d=0, *nyx1d=0, *nyy1d=0, *nyz1d=0, *nzx1d=0, *nzy1d=0, *nzz1d=0;

	//allocat memory for nxx, nxy, nxz, nyx, nyy, nyz, nzx, nzy, nzz;
   V3DLONG tmplen = sz[0]*sz[1]*sz[2];
   if (tmplen>0 && sz[0]>0 && sz[1]>0 && sz[2]>0)
   {
	   nxx1d = new float [tmplen];
	   if (nxx1d)
		   new3dpointer(nxx, sz[0], sz[1], sz[2], nxx1d);
		
	   nxy1d = new float [tmplen];
	   if (nxy1d)
		   new3dpointer(nxy, sz[0], sz[1], sz[2], nxy1d);

	   nxz1d = new float [tmplen];
	   if (nxz1d)
		   new3dpointer(nxz, sz[0], sz[1], sz[2], nxz1d);

	   nyx1d = new float [tmplen];
	   if (nyx1d)
		   new3dpointer(nyx, sz[0], sz[1], sz[2], nyx1d);

	   nyy1d = new float [tmplen];
	   if (nyy1d)
		   new3dpointer(nyy, sz[0], sz[1], sz[2], nyy1d);

	   nyz1d = new float [tmplen];
	   if (nyz1d)
		   new3dpointer(nyz, sz[0], sz[1], sz[2], nyz1d);

	   nzx1d = new float [tmplen];
	   if (nzx1d)
		   new3dpointer(nzx, sz[0], sz[1], sz[2], nzx1d);

	   nzy1d = new float [tmplen];
	   if (nzy1d)
		   new3dpointer(nzy, sz[0], sz[1], sz[2], nzy1d);

	   nzz1d = new float [tmplen];
	   if (nzz1d)
		   new3dpointer(nzz, sz[0], sz[1], sz[2],  nzz1d);
		
	}
	
	
	gradient(nx, nxx, nxy, nxz, sz);  
	gradient(ny, nyx, nyy, nyz, sz);
	gradient(nz, nzx, nzy, nzz, sz);
	
	for (int k=0; k<sz[2]; k++) 
	for (int j=0; j<sz[1]; j++)	
	for (int i=0; i<sz[0]; i++)
	{
		K[k][j][i] =nxx[k][j][i]+nyy[k][j][i] + nzz[k][j][i];
	}
	
	// delete 1d and 2d pointers

   if (nxx) delete3dpointer(nxx, sz[0], sz[1], sz[2]);
   if (nxx1d) {delete []nxx1d; nxx1d=0;}

   if (nxy) delete3dpointer(nxy, sz[0], sz[1], sz[2]);
   if (nxy1d) {delete []nxy1d; nxy1d=0;}
   
   if (nxz) delete3dpointer(nxz, sz[0], sz[1], sz[2]);
   if (nxz1d) {delete []nxz1d; nxz1d=0;}   

   if (nyx) delete3dpointer(nyx, sz[0], sz[1], sz[2]);
   if (nyx1d) {delete []nyx1d; nyx1d=0;}

   if (nyy) delete3dpointer(nyy, sz[0], sz[1], sz[2]);
   if (nyy1d) {delete []nyy1d; nyy1d=0;}
   
   if (nyz) delete3dpointer(nyz, sz[0], sz[1], sz[2]);
   if (nyz1d) {delete []nyz1d; nyz1d=0;}	
		
   if (nzx) delete3dpointer(nzx, sz[0], sz[1], sz[2]);
   if (nzx1d) {delete []nzx1d; nzx1d=0;}

   if (nzy) delete3dpointer(nzy, sz[0], sz[1], sz[2]);
   if (nzy1d) {delete []nzy1d; nzy1d=0;}
   
   if (nzz) delete3dpointer(nzz, sz[0], sz[1], sz[2]);
   if (nzz1d) {delete []nzz1d; nzz1d=0;}	
	
//	return K;
   return K; //I un-comment this as MSVC compiler force to return the value check. 2010-05-20 by PHC
}


//Make a 2D function satisfy Neumann boundary condition
//float ** NeumannBoundCond(float **f, const V3DLONG *sz)
//void NeumannBoundCond(float ** &f, const V3DLONG *sz)
void NeumannBoundCond(float ** f, const V3DLONG *sz)
{

	float **g=0, *g1d=0;
	V3DLONG i,j;
	
	//allocate memory for g
   V3DLONG tmplen = sz[0]*sz[1];
   if (tmplen>0 && sz[0]>0 && sz[1]>0)
   {
	   g1d = new float [tmplen];
	   if (g1d)
		   new2dpointer(g, sz[0], sz[1], g1d);
	}
	
	for(j=0; j<sz[1]; j++)
	for(i=0; i<sz[0]; i++)
	{
		g[j][i] = f[j][i];
	}

	g[0][0] = g[2][2];
	g[0][sz[0]-1] = g[2][sz[0]-3];
	g[sz[1]-1][0] = g[sz[1]-3][2];
	g[sz[1]-1][sz[0]-1] = g[sz[1]-3][sz[0]-3];
	
	for(i=1; i<sz[0]-1; i++)
	{
		g[0][i] = g[2][i];
		g[sz[1]-1][i] = g[sz[1]-3][i]; 
	}
	
	for(j=1; j<sz[1]-1; j++)
	{
		g[j][0] = g[j][2];
		g[j][sz[0]-1] = g[j][sz[0]-3]; 
	}

	for(j=0; j<sz[1]; j++)
	for(i=0; i<sz[0]; i++)
	{
		f[j][i] = g[j][i];
	}

	if (g) delete2dpointer(g, sz[0], sz[1]);
	if (g1d) {delete[]g1d; g1d = 0;}
	
//    if (f)
//	{
//       float * f1d = f[0];
//	   delete2dpointer(f, sz[0], sz[1]);
//	   delete []f1d;
//	}
//   f = g;	
//	//return g;
}


//Make a 3D function satisfy Neumann boundary condition
void NeumannBoundCond(float ***f, const V3DLONG *sz)
{

	float ***g=0, *g1d=0;
	V3DLONG i,j,k;
	
	//allocate memory for g
   V3DLONG tmplen = sz[0]*sz[1]*sz[2];
   if (tmplen>0 && sz[0]>0 && sz[1]>0 && sz[2]>0)
   {
	   g1d = new float [tmplen];
	   if (g1d)
		   new3dpointer(g, sz[0], sz[1], sz[2], g1d);
	}
	
	for(k=0; k<sz[2]; k++)
	for(j=0; j<sz[1]; j++)
	for(i=0; i<sz[0]; i++)
	{
		g[k][j][i] = f[k][j][i];
	}

	// sweep along z dimension	
	for (k=0; k<sz[2]; k++)
	{
		g[k][0][0] = g[k][2][2];
		g[k][0][sz[0]-1] = g[k][2][sz[0]-3];
		g[k][sz[1]-1][0] = g[k][sz[1]-3][2];
		g[k][sz[1]-1][sz[0]-1] = g[k][sz[1]-3][sz[0]-3];
		
		for(i=1; i<sz[0]-1; i++)
		{
			g[k][0][i] = g[k][2][i];
			g[k][sz[1]-1][i] = g[k][sz[1]-3][i]; 
		}
		
		for(j=1; j<sz[1]-1; j++)
		{
			g[k][j][0] = g[k][j][2];
			g[k][j][sz[0]-1] = g[k][j][sz[0]-3]; 
		}
	}
	
	//sweep along y dimension
	for (j=0; j<sz[1]; j++)
	{
		g[0][j][0] = g[2][j][2];
		g[0][j][sz[0]-1] = g[2][j][sz[0]-3];
		g[sz[2]-1][j][0] = g[sz[2]-3][j][2];
		g[sz[2]-1][j][sz[0]-1] = g[sz[2]-3][j][sz[0]-3];
		
		for(i=1; i<sz[0]-1; i++)
		{
			g[0][j][i] = g[2][j][i];
			g[sz[2]-1][j][i] = g[sz[2]-3][j][i]; 
		}
		
		for(k=1; k<sz[2]-1; k++)
		{
			g[k][j][0] = g[k][j][2];
			g[k][j][sz[0]-1] = g[k][j][sz[0]-3]; 
		}
	}
	
	// every point on the 6 faces of the cube has been replaced by an inner point,
	// no need to sweep along x dimension any more

	for(k=0; k<sz[2]; k++)
	for(j=0; j<sz[1]; j++)
	for(i=0; i<sz[0]; i++)
	{
		f[k][j][i] = g[k][j][i];
	}

	if (g) delete3dpointer(g, sz[0], sz[1], sz[2]);
	if (g1d) {delete[]g1d; g1d = 0;}

	
//	return g;
}

// 2d level set, with replelling force between neighboring regions
// input and out share the same memory u
void evloveLevelSet(float **u, float **uneighbor, float **g, float *para, const int numIter, const V3DLONG *sz)
{
//	float **vx=0, **vy=0, **ux=0, **uy=0, **lap=0, **u=0;
//	float *vx1d, *vy1d, *ux1d, *uy1d, *lap1d, *u1d;
//	float **diracU=0, **K=0; 
//	float *diracU1d, *K1d;
	
	float **vx=0, **vy=0, **ux=0, **uy=0, **lap=0;
	float *vx1d=0, *vy1d=0, *ux1d=0, *uy1d=0, *lap1d=0;
	float **diracU=0, **K=0; 
	float *diracU1d=0, *K1d=0;
	
	float lamda = para[0], mu =para[1], alf=para[2], epsilon=para[3], delt=para[4], gama=para[5];
	V3DLONG i,j;
	float weightedLengthTerm, penalizingTerm, repellTerm, weightedAreaTerm;

	// allocate memory for vx, vy, diracU, K
   V3DLONG tmplen = sz[0]*sz[1];	
   if (tmplen>0 && sz[0]>0 && sz[1]>0)
   {
	   vx1d = new float [tmplen];
	   if (vx1d)
		   new2dpointer(vx, sz[0], sz[1], vx1d);

	   vy1d = new float [tmplen];
	   if (vy1d)
		   new2dpointer(vy, sz[0], sz[1], vy1d);

	   ux1d = new float [tmplen];
	   if (ux1d)
		   new2dpointer(ux, sz[0], sz[1], ux1d);

	   uy1d = new float [tmplen];
	   if (uy1d)
		   new2dpointer(uy, sz[0], sz[1], uy1d);

	   lap1d = new float [tmplen];
	   if (lap1d)
		   new2dpointer(lap, sz[0], sz[1], lap1d);
		 
	   diracU1d = new float [tmplen];
	   if (diracU1d)
		   new2dpointer(diracU, sz[0], sz[1], diracU1d);
		
	   K1d = new float [tmplen];
	   if (K1d)
		   new2dpointer(K, sz[0], sz[1], K1d);

//	   u1d = new float [tmplen];
//	   if (u1d)	   
//		   new2dpointer(u, sz[0], sz[1], u1d);
		
	}
	
	
//	for (j=0; j<sz[1]; j++)
//	for (i=0; i<sz[0]; i++)
//	{
//		u[j][i] = u0[j][i];
//	}
	
	gradient(g,vx,vy,sz);

	for (int k=0; k<numIter; k++)
	{
		//u = NeumannBoundCond(u, sz);
		NeumannBoundCond(u, sz);		
		gradient(u, ux, uy, sz); 				
		gradientNoramlize(ux,uy, sz);				
//		diracU = Dirac(u, epsilon, sz);		
//		K=curvatureCentral(ux,uy,sz);
		Dirac(u, epsilon, sz, diracU);		
		curvatureCentral(ux,uy,sz, K);

		laplacian(u, lap, sz);
		
		// computer forces
		for(j=0;j<sz[1];j++)
		for(i=0;i<sz[0];i++)
		{
			weightedLengthTerm = lamda*diracU[j][i]*(vx[j][i]*ux[j][i] + vy[j][i]*uy[j][i] + g[j][i]*K[j][i]);
			penalizingTerm=mu*(4*lap[j][i]-K[j][i]);
			weightedAreaTerm=alf*diracU[j][i]*g[j][i];
			repellTerm = - gama* uneighbor[j][i];
			u[j][i]=u[j][i]+delt*(weightedLengthTerm + weightedAreaTerm + penalizingTerm + repellTerm);  // update the level set function    
		}
	}


	// delete pointer, free memory
	
   if (vx) delete2dpointer(vx, sz[0], sz[1]);
   if (vx1d) {delete []vx1d; vx1d=0;}

   if (vy) delete2dpointer(vy, sz[0], sz[1]);
   if (vy1d) {delete []vy1d; vy1d=0;}

   if (ux) delete2dpointer(ux, sz[0], sz[1]);
   if (ux1d) {delete []ux1d; ux1d=0;}

   if (uy) delete2dpointer(uy, sz[0], sz[1]);
   if (uy1d) {delete []uy1d; uy1d=0;}

   if (lap) delete2dpointer(lap, sz[0], sz[1]);
   if (lap1d) {delete []lap1d; lap1d=0;}   

   if (diracU) delete2dpointer(diracU, sz[0], sz[1]);
   if (diracU1d) {delete []diracU1d; diracU1d=0;}  
   
   if (K) delete2dpointer(K, sz[0], sz[1]);
   if (K1d) {delete []K1d; K1d=0;}  
   
}

//2d level set without considering  repelling force between neighboring regions

void evloveLevelSet(float **u, float **g, float *para, const int numIter, const V3DLONG *sz)
{


//	float **vx=0, **vy=0, **ux=0, **uy=0, **lap=0, **u=0;
//	float *vx1d, *vy1d, *ux1d, *uy1d, *lap1d, *u1d;
//	float **diracU=0, **K=0; 
//	float *diracU1d, *K1d; 

	float **vx=0, **vy=0, **ux=0, **uy=0, **lap=0;
	float *vx1d=0, *vy1d=0, *ux1d=0, *uy1d=0, *lap1d=0;
	float **diracU=0, **K=0; 
	float *diracU1d=0, *K1d=0; 
	
	float lamda = para[0], mu =para[1], alf=para[2], epsilon=para[3], delt=para[4];
	V3DLONG i,j;
	float weightedLengthTerm, penalizingTerm, weightedAreaTerm;
	
	
	// allocate memory for vx, vy, ux, uy, diracU, K
   V3DLONG tmplen = sz[0]*sz[1];
   if (tmplen>0 && sz[0]>0 && sz[1]>0)
   {
	   vx1d = new float [tmplen];
	   if (vx1d)
		   new2dpointer(vx, sz[0], sz[1], vx1d);

	   vy1d = new float [tmplen];
	   if (vy1d)
		   new2dpointer(vy, sz[0], sz[1], vy1d);

	   ux1d = new float [tmplen];
	   if (ux1d)
		   new2dpointer(ux, sz[0], sz[1], ux1d);

	   uy1d = new float [tmplen];
	   if (uy1d)
		   new2dpointer(uy, sz[0], sz[1], uy1d);

	   lap1d = new float [tmplen];
	   if (lap1d)
		   new2dpointer(lap, sz[0], sz[1], lap1d);
		 
	   diracU1d = new float [tmplen];
	   if (diracU1d)
		   new2dpointer(diracU, sz[0], sz[1], diracU1d);

		
	   K1d = new float [tmplen];
	   if (K1d)
		   new2dpointer(K, sz[0], sz[1], K1d);

//	   u1d = new float [tmplen];
//	   if (u1d)
//		   new2dpointer(u, sz[0], sz[1], u1d);
		
	}
	
	
//	for (j=0; j<sz[1]; j++)
//	for (i=0; i<sz[0]; i++)
//	{
//		u[j][i] = u0[j][i];
//	}
	
	gradient(g,vx,vy,sz);

	for (int k=0; k<numIter; k++)
	{
		//u = NeumannBoundCond(u, sz);
		NeumannBoundCond(u, sz);		
		gradient(u, ux, uy, sz); 
				
		gradientNoramlize(ux,uy, sz);				
//		diracU = Dirac(u, epsilon, sz);		
//		K=curvatureCentral(ux,uy,sz);
		Dirac(u, epsilon, sz, diracU);		
		curvatureCentral(ux,uy,sz, K);
		laplacian(u, lap, sz);
		
		// computer forces
		
		for(j=0;j<sz[1];j++)
		for(i=0;i<sz[0];i++)
		{
			weightedLengthTerm = lamda*diracU[j][i]*(vx[j][i]*ux[j][i] + vy[j][i]*uy[j][i] + g[j][i]*K[j][i]);
			penalizingTerm=mu*(4*lap[j][i]-K[j][i]);								
			weightedAreaTerm=alf*diracU[j][i]*g[j][i];
												
			u[j][i]=u[j][i]+delt*(weightedLengthTerm + weightedAreaTerm + penalizingTerm);  // update the level set function    

		}
		
	}

	// delete pointer, free memory
	
   if (vx) delete2dpointer(vx, sz[0], sz[1]);
   if (vx1d) {delete []vx1d; vx1d=0;}

   if (vy) delete2dpointer(vy, sz[0], sz[1]);
   if (vy1d) {delete []vy1d; vy1d=0;}

   if (ux) delete2dpointer(ux, sz[0], sz[1]);
   if (ux1d) {delete []ux1d; ux1d=0;}

   if (uy) delete2dpointer(uy, sz[0], sz[1]);
   if (uy1d) {delete []uy1d; uy1d=0;}

   if (lap) delete2dpointer(lap, sz[0], sz[1]);
   if (lap1d) {delete []lap1d; lap1d=0;}   
    
   if (diracU) delete2dpointer(diracU, sz[0], sz[1]);
   if (diracU1d) {delete []diracU1d; diracU1d=0;}  
   
   if (K) delete2dpointer(K, sz[0], sz[1]);
   if (K1d) {delete []K1d; K1d=0;}  
	
//   if (u0) 
//   {
//       float * u01d = u0[0];
//	   delete2dpointer(u0, sz[0], sz[1]);
//	   delete []u01d;
//	}
//   u0 = u;	
	return;
}


// 3d level set, with repelling force between neighboring regions
// input and out share the same memory u
void evloveLevelSet(float ***u, float ***uneighbor, float ***g, float *para, const int numIter, const V3DLONG *sz)
{

	float ***vx=0, ***vy=0, ***vz=0, ***ux=0, ***uy=0, ***uz=0, ***lap=0;
	float *vx1d=0, *vy1d=0, *vz1d=0, *ux1d=0, *uy1d=0, *uz1d=0, *lap1d=0;	
	float ***diracU=0, ***K=0;
	float *diracU1d=0, *K1d=0;

	float lamda = para[0], mu =para[1], alf=para[2], epsilon=para[3], delt=para[4], gama=para[5];
	V3DLONG i,j,k;
	float weightedLengthTerm, penalizingTerm, repellTerm, weightedAreaTerm;
	
	
	// allocate memory for vx, vy, vz, diracU, K
	
   V3DLONG tmplen = sz[0]*sz[1]*sz[2];
   if (tmplen>0 && sz[0]>0 && sz[1]>0 && sz[2]>0)
   {
	   vx1d = new float [tmplen];
	   if (vx1d)
		   new3dpointer(vx, sz[0], sz[1], sz[2], vx1d);

	   vy1d = new float [tmplen];
	   if (vy1d)
		   new3dpointer(vy, sz[0], sz[1], sz[2], vy1d);
	
	   vz1d = new float [tmplen];
	   if (vz1d)
		   new3dpointer(vz, sz[0], sz[1], sz[2], vz1d);
				   
	   ux1d = new float [tmplen];
	   if (ux1d)
		   new3dpointer(ux, sz[0], sz[1], sz[2], ux1d);

	   uy1d = new float [tmplen];
	   if (uy1d)
		   new3dpointer(uy, sz[0], sz[1], sz[2], uy1d);

	   uz1d = new float [tmplen];
	   if (uz1d)
		   new3dpointer(uz, sz[0], sz[1], sz[2], uz1d);
		
	   diracU1d = new float [tmplen];
	   if (diracU1d)
		   new3dpointer(diracU, sz[0], sz[1], sz[2], diracU1d);
		
	   K1d = new float [tmplen];
	   if (K1d)
		   new3dpointer(K, sz[0], sz[1], sz[2], K1d);

	}
	
//	for (k=0;k<sz[2];k++)
//	for (j=0; j<sz[1]; j++)
//	for (i=0; i<sz[0]; i++)
//	{
//		u[k][j][i] = u0[k][j][i];
//	}
	
	gradient(g,vx,vy,vz,sz);
	 
	for (k=0; k<numIter; k++)
	{
		NeumannBoundCond(u, sz); // 3D Neumann Boundary Conditions not clear, just do nothing		
		gradient(u, ux, uy, uz, sz); 
		gradientNoramlize(ux,uy,uz, sz);				
		Dirac(u, epsilon, sz, diracU);		
		curvatureCentral(ux,uy,uz,sz,K);
		laplacian(u, lap, sz);
		
		// computer forces
		for(k=0;k<sz[2];k++)
		for(j=0;j<sz[1];j++)
		for(i=0;i<sz[0];i++)
		{
			weightedLengthTerm = lamda*diracU[k][j][i]*(vx[k][j][i]*ux[k][j][i] + vy[k][j][i]*uy[k][j][i] + vz[k][j][i]*uz[k][j][i]+g[k][j][i]*K[k][j][i]);
			penalizingTerm=mu*(4*lap[k][j][i]-K[k][j][i]);
			weightedAreaTerm=alf*diracU[k][j][i]*g[k][j][i];
			repellTerm = - gama* uneighbor[k][j][i];
			u[k][j][i]=u[k][j][i]+delt*(weightedLengthTerm + weightedAreaTerm + penalizingTerm + repellTerm);  // update the level set function    
		}
	}



	// delete pointer, free memory	
   if (vx) delete3dpointer(vx, sz[0], sz[1], sz[2]);
   if (vx1d) {delete []vx1d; vx1d=0;}

   if (vy) delete3dpointer(vy, sz[0], sz[1], sz[2]);
   if (vy1d) {delete []vy1d; vy1d=0;}

   if (vz) delete3dpointer(vz, sz[0], sz[1], sz[2]);
   if (vz1d) {delete []vz1d; vz1d=0;}

   if (ux) delete3dpointer(ux, sz[0], sz[1], sz[2]);
   if (ux1d) {delete []ux1d; ux1d=0;}

   if (uy) delete3dpointer(uy, sz[0], sz[1], sz[2]);
   if (uy1d) {delete []uy1d; uy1d=0;}

   if (uz) delete3dpointer(uz, sz[0], sz[1], sz[2]);
   if (uz1d) {delete []uz1d; uz1d=0;}

   if (lap) delete3dpointer(lap, sz[0], sz[1], sz[2]);
   if (lap1d) {delete []lap1d; lap1d=0;}

   if (diracU) delete3dpointer(diracU, sz[0], sz[1], sz[2]);
   if (diracU1d) {delete []diracU1d; diracU1d=0;}

   if (K) delete3dpointer(K, sz[0], sz[1], sz[2]);
   if (K1d) {delete []K1d; K1d=0;}   
	
//	return u;
}

// 3d level set, without considering repelling force between neighboring regions
void evloveLevelSet(float ***u, float ***g, float *para, const int numIter, const V3DLONG *sz)
{

	float ***vx=0, ***vy=0, ***vz=0, ***ux=0, ***uy=0, ***uz=0, ***lap=0;
	float *vx1d=0, *vy1d=0, *vz1d=0, *ux1d=0, *uy1d=0, *uz1d=0, *lap1d=0;	
	float ***diracU=0, ***K=0;
	float *diracU1d=0, *K1d=0;

	float lamda = para[0], mu =para[1], alf=para[2], epsilon=para[3], delt=para[4], gama=para[5];
	V3DLONG i,j,k;
	float weightedLengthTerm, penalizingTerm, weightedAreaTerm;
	
	
	// allocate memory for vx, vy, vz, ux, uy, uz, diracU, K	
   V3DLONG tmplen = sz[0]*sz[1]*sz[2];
   if (tmplen>0 && sz[0]>0 && sz[1]>0 && sz[2]>0)
   {
	   vx1d = new float [tmplen];
	   if (vx1d)
		   new3dpointer(vx, sz[0], sz[1], sz[2], vx1d);

	   vy1d = new float [tmplen];
	   if (vy1d)
		   new3dpointer(vy, sz[0], sz[1], sz[2], vy1d);
	
	   vz1d = new float [tmplen];
	   if (vz1d)
		   new3dpointer(vz, sz[0], sz[1], sz[2], vz1d);
				   
	   ux1d = new float [tmplen];
	   if (ux1d)
		   new3dpointer(ux, sz[0], sz[1], sz[2], ux1d);

	   uy1d = new float [tmplen];
	   if (uy1d)
		   new3dpointer(uy, sz[0], sz[1], sz[2], uy1d);

	   uz1d = new float [tmplen];
	   if (uz1d)
		   new3dpointer(uz, sz[0], sz[1], sz[2], uz1d);

	   lap1d = new float [tmplen];
	   if (lap1d)
		   new3dpointer(lap, sz[0], sz[1], sz[2], lap1d);
		
	   diracU1d = new float [tmplen];
	   if (diracU1d)
		   new3dpointer(diracU, sz[0], sz[1], sz[2], diracU1d);
		
	   K1d = new float [tmplen];
	   if (K1d)
		   new3dpointer(K, sz[0], sz[1], sz[2], K1d);

	}
	
//	for (k=0;k<sz[2];k++)
//	for (j=0; j<sz[1]; j++)
//	for (i=0; i<sz[0]; i++)
//	{
//		u[k][j][i] = u0[k][j][i];
//	}
	
	gradient(g,vx,vy,vz,sz);
	 
	for (k=0; k<numIter; k++)
	{
		NeumannBoundCond(u, sz);		
		gradient(u, ux, uy, uz, sz); 
		
		gradientNoramlize(ux,uy,uz, sz);				
		Dirac(u, epsilon, sz, diracU);		
		curvatureCentral(ux,uy,uz,sz, K);
		laplacian(u, lap, sz);
		
		// computer forces
		for(k=0;k<sz[2];k++)
		for(j=0;j<sz[1];j++)
		for(i=0;i<sz[0];i++)
		{
			weightedLengthTerm = lamda*diracU[k][j][i]*(vx[k][j][i]*ux[k][j][i] + vy[k][j][i]*uy[k][j][i] + vz[k][j][i]*uz[k][j][i] + g[k][j][i]*K[k][j][i]);
			penalizingTerm=mu*(4*lap[k][j][i]-K[k][j][i]);
			weightedAreaTerm=alf*diracU[k][j][i]*g[k][j][i];
			
			u[k][j][i]=u[k][j][i]+delt*(weightedLengthTerm + weightedAreaTerm + penalizingTerm);  // update the level set function    
			
		}

	}

	// delete pointer, free memory	
   if (vx) delete3dpointer(vx, sz[0], sz[1], sz[2]);
   if (vx1d) {delete []vx1d; vx1d=0;}

   if (vy) delete3dpointer(vy, sz[0], sz[1], sz[2]);
   if (vy1d) {delete []vy1d; vy1d=0;}

   if (vz) delete3dpointer(vz, sz[0], sz[1], sz[2]);
   if (vz1d) {delete []vz1d; vz1d=0;}

   if (ux) delete3dpointer(ux, sz[0], sz[1], sz[2]);
   if (ux1d) {delete []ux1d; ux1d=0;}

   if (uy) delete3dpointer(uy, sz[0], sz[1], sz[2]);
   if (uy1d) {delete []uy1d; uy1d=0;}
   
   if (uz) delete3dpointer(uz, sz[0], sz[1], sz[2]);
   if (uz1d) {delete []uz1d; uz1d=0;}

	if (lap) delete3dpointer(lap, sz[0], sz[1], sz[2]);
   if (lap1d) {delete []lap1d; lap1d=0;}  

   if (diracU) delete3dpointer(diracU, sz[0], sz[1], sz[2]);
   if (diracU1d) {delete []diracU1d; diracU1d=0;}

   if (K) delete3dpointer(K, sz[0], sz[1], sz[2]);
   if (K1d) {delete []K1d; K1d=0;}   
	
//	return u;
}

#endif

