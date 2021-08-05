// q_convolve_kernel.cpp
// by Lei Qu
// 2019-05-07

#include "q_convolve_kernel.h"
#include "math.h"

//Gaussian kernel generator
bool q_kernel_gaussian_1D(const long l_radius,const double sigma,
		vector<double> &vec1D_kernel)
{
	//check paras
	if(l_radius<0)
	{
		printf("ERROR: Invalid input kernel radius!\n");
		return false;
	}
	if(sigma<=0)
	{
		printf("ERROR: Invalid input sigma!\n");
		return false;
	}
	if(vec1D_kernel.size()!=0)
	{
		printf("WARNNING: Output vec1D_kernel is not empty, original data will be cleared!\n");
		vec1D_kernel.clear();
	}

	//Gaussian Kernel size
	long sz_kernel=l_radius*2+1;

	//initial fill kernel
	vec1D_kernel.assign(sz_kernel,0.0);
	double d_kernel_sum=0;
	long x;
	for(long xx=-l_radius;xx<=l_radius;xx++)
	{
		x=xx+l_radius;

		vec1D_kernel[x]=exp(-xx*xx/(2*sigma*sigma));
		d_kernel_sum+=vec1D_kernel[x];
	}

	//normalize kernel
	for(long xx=-l_radius;xx<=l_radius;xx++)
	{
		x=xx+l_radius;
		vec1D_kernel[x]/=d_kernel_sum;
	}

	return true;
}

bool q_kernel_gaussian_3D(const long l_radius_x,const long l_radius_y,const long l_radius_z,const double sigma,
		vector< vector< vector<double> > > &vec3D_kernel)
{
	//check paras
	if(l_radius_x<0 || l_radius_y<0 || l_radius_z<0)
	{
		printf("ERROR: Invalid input kernel radius!\n");
		return false;
	}
	if(sigma<=0)
	{
		printf("ERROR: Invalid input sigma!\n");
		return false;
	}
	if(vec3D_kernel.size()!=0)
	{
		printf("WARNNING: Output vec3D_kernel is not empty, original data will be cleared!\n");
		vec3D_kernel.clear();
	}

	//Gaussian Kernel size
	long sz_kernel[3];//w,h,z
	sz_kernel[0]=l_radius_x*2+1;
	sz_kernel[1]=l_radius_y*2+1;
	sz_kernel[2]=l_radius_z*2+1;

	//initial fill kernel
	vec3D_kernel.assign(sz_kernel[1],vector< vector<double> >(sz_kernel[0],vector<double>(sz_kernel[2],0.0)));
	double d_kernel_sum=0;
	long x,y,z;
	for(long xx=-l_radius_x;xx<=l_radius_x;xx++)
	{
		x=xx+l_radius_x;
		for(long yy=-l_radius_y;yy<=l_radius_y;yy++)
		{
			y=yy+l_radius_y;
			for(long zz=-l_radius_z;zz<=l_radius_z;zz++)
			{
				z=zz+l_radius_z;
//				vec3D_kernel[y][x][z]=1;
				vec3D_kernel[y][x][z]=exp(-(xx*xx+yy*yy+zz*zz)/(2*sigma*sigma));
				d_kernel_sum+=vec3D_kernel[y][x][z];
			}
		}
	}

	//normalize kernel
	for(long xx=-l_radius_x;xx<=l_radius_x;xx++)
	{
		x=xx+l_radius_x;
		for(long yy=-l_radius_y;yy<=l_radius_y;yy++)
		{
			y=yy+l_radius_y;
			for(long zz=-l_radius_z;zz<=l_radius_z;zz++)
			{
				z=zz+l_radius_z;
				vec3D_kernel[y][x][z]/=d_kernel_sum;
			}
		}
	}

	return true;
}

//sobel kernel (for gradient)
bool q_kernel_sobel_3D(const char xyz, vector< vector< vector<double> > > &vec3D_kernel)
{
	//check paras
	if ((!xyz == 'x' || xyz == 'y' || xyz == 'z'))
	{
		printf("ERROR: xyz should be one of xyz!\n");
		return false;
	}
	if (vec3D_kernel.size() != 0)
	{
		printf("WARNNING: Output vec3D_kernel is not empty, original data will be cleared!\n");
		vec3D_kernel.clear();
	}

	vec3D_kernel.assign(3, vector< vector<double> >(3, vector<double>(3, 0.0)));

	if (xyz == 'x')
	{
		for (int row = 0; row < 3; row++)
			for (int zz = 0; zz < 3; zz++)
			{
				vec3D_kernel[row][0][zz] = -1;
				vec3D_kernel[row][2][zz] = 1;
			}
	}
	else if (xyz == 'y')
	{
		for (int col = 0; col < 3; col++)
			for (int zz = 0; zz < 3; zz++)
			{
				vec3D_kernel[0][col][zz] = -1;
				vec3D_kernel[2][col][zz] = 1;
			}
	}
	else
	{
		for (int row = 0; row < 3; row++)
			for (int col = 0; col < 3; col++)
			{
				vec3D_kernel[row][col][0] = -1;
				vec3D_kernel[row][col][2] = 1;
			}
	}

	return true;
}

//laplacian kernel (for edge)
bool q_kernel_laplacian_3D(vector< vector< vector<double> > > &vec3D_kernel)
{
	//check paras
	if (vec3D_kernel.size() != 0)
	{
		printf("WARNNING: Output vec3D_kernel is not empty, original data will be cleared!\n");
		vec3D_kernel.clear();
	}

	vec3D_kernel.assign(3, vector< vector<double> >(3, vector<double>(3, 1.0)));
	vec3D_kernel[1][1][1] = -(3*3*3-1);

	return true;
}