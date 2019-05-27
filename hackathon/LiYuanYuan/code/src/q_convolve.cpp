// q_convolve.cpp
// by Lei Qu
// 2010-02-24

//#include "q_convolve.h"
#include <vector>
using namespace std;
#include "math.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions

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

bool q_kernel_gaussian(const long l_radius_x,const long l_radius_y,const long l_radius_z,const double sigma,
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

//sobel kernel generator (for gradient)
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


//image convolution: output = input * kernel
template<class T>
bool q_convolve_3D(T *&p_img, const long sz_img[4],
	const vector< vector< vector<double> > > &vec3D_kernel)
{
	//check paras
	if (p_img == 0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if (sz_img[0] <= 0 || sz_img[1] <= 0 || sz_img[2] <= 0 || sz_img[3] <= 0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if (vec3D_kernel.size() == 0)
	{
		printf("ERROR: Invalid input kernel!\n");
		return false;
	}

	long sz_kernel[3];//w,h,z
	sz_kernel[0] = vec3D_kernel[0].size();
	sz_kernel[1] = vec3D_kernel.size();
	sz_kernel[2] = vec3D_kernel[0][0].size();
	long radius_x, radius_y, radius_z;
	radius_x = (sz_kernel[0] - 1) / 2;
	radius_y = (sz_kernel[1] - 1) / 2;
	radius_z = (sz_kernel[2] - 1) / 2;

	//allocate memory
	long l_npixels = sz_img[0] * sz_img[1] * sz_img[2] * sz_img[3];

	T *p_img_tmp = 0;
	p_img_tmp = new T[l_npixels]();
	if (!p_img_tmp)
	{
		printf("ERROR: Fail to allocate memory for output image!\n");
		return false;
	}

	T ****p_img_4d = 0, ****p_img_tmp_4d = 0;
	if (!new4dpointer(p_img_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img) ||
		!new4dpointer(p_img_tmp_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img_tmp))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img_tmp) 		{ delete[]p_img_tmp;		p_img_tmp = 0; }
		if (p_img_4d)		{ delete4dpointer(p_img_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
		if (p_img_tmp_4d) 	{ delete4dpointer(p_img_tmp_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
		return false;
	}

	//convolve
	for (long Z = 0; Z < sz_img[2]; Z++)
		for (long Y = 0; Y < sz_img[1]; Y++)
			for (long X = 0; X < sz_img[0]; X++)
			{
				//convolve one pixel
				long x_k, y_k, z_k;//kernel element coordinate
				long x_i, y_i, z_i;//corresponding image coordinate of kernel
				for (long xx = -radius_x; xx <= radius_x; xx++)
				{
					x_k = xx + radius_x;
					x_i = X + xx;
					x_i = x_i < 0 ? 0 : x_i;		x_i = x_i >= sz_img[0] ? sz_img[0] - 1 : x_i;
					for (long yy = -radius_y; yy <= radius_y; yy++)
					{
						y_k = yy + radius_y;
						y_i = Y + yy;
						y_i = y_i < 0 ? 0 : y_i;		y_i = y_i >= sz_img[1] ? sz_img[1] - 1 : y_i;
						for (long zz = -radius_z; zz <= radius_z; zz++)
						{
							z_k = zz + radius_z;
							z_i = Z + zz;
							z_i = z_i < 0 ? 0 : z_i;		z_i = z_i >= sz_img[2] ? sz_img[2] - 1 : z_i;

							for (long C = 0; C < sz_img[3]; C++)
								p_img_tmp_4d[C][Z][Y][X] += p_img_4d[C][z_i][y_i][x_i] * vec3D_kernel[y_k][x_k][z_k];
						}
					}
				}

			}

	//change pointer
	if (p_img) 		{ delete[]p_img;		p_img = 0; }
	p_img = p_img_tmp;

	//free memory
	if (p_img_4d)		{ delete4dpointer(p_img_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
	if (p_img_tmp_4d) 	{ delete4dpointer(p_img_tmp_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }

	return true;
}

//fast image convolution
//	use a 1D kernel to convolve image along XYZ to simulation a classic 3D kernel
template<class T>
bool q_convolve_3D_fast(T *&p_img,const long sz_img[4],
		const vector<double> &vec1D_kernel)
{
	//check paras
	if(p_img==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(vec1D_kernel.size()==0)
	{
		printf("ERROR: Invalid input kernel!\n");
		return false;
	}

	long radius=(vec1D_kernel.size()-1)/2;

	//allocate memory
	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

	T *p_img_tmp1=new double[l_npixels]();
	T *p_img_tmp2=new double[l_npixels]();
	if(!p_img_tmp1 || !p_img_tmp2)
	{
		printf("ERROR: Fail to allocate memory for output image!\n");
		return false;
	}

	T ****p_img_4d=0,****p_img_tmp1_4d=0,****p_img_tmp2_4d=0;
	if(!new4dpointer(p_img_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img) ||
	   !new4dpointer(p_img_tmp1_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img_tmp1) ||
	   !new4dpointer(p_img_tmp2_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3],p_img_tmp2))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img_tmp1) 		{delete []p_img_tmp1;		p_img_tmp1=0;}
		if(p_img_4d)			{delete4dpointer(p_img_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img_tmp1_4d) 	{delete4dpointer(p_img_tmp1_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		if(p_img_tmp2_4d) 	{delete4dpointer(p_img_tmp2_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
		return false;
	}

	//convolve along X
	for(long i=0;i<l_npixels;i++) p_img_tmp1[i]=0.0;	//in some machine, array was not initilized to 0! don't know why
	for(long X=0;X<sz_img[0];X++)
	{
		long x_k;//kernel element coordinate
		long x_i;//corresponding image coordinate of kernel
		for(long xx=-radius;xx<=radius;xx++)
		{
			x_k=xx+radius;
			x_i=X+xx;
			x_i = x_i<0 ? 0:x_i;		x_i = x_i>=sz_img[0] ? sz_img[0]-1:x_i;

			for(long Z=0;Z<sz_img[2];Z++)
				for(long Y=0;Y<sz_img[1];Y++)
					for(long C=0;C<sz_img[3];C++)
						p_img_tmp1_4d[C][Z][Y][X]+=p_img_4d[C][Z][Y][x_i]*vec1D_kernel[x_k];
		}
	}
	//convolve along Y
	for(long i=0;i<l_npixels;i++) p_img_tmp2[i]=0.0;	//in some machine, array was not initilized to 0! don't know why
	for(long Y=0;Y<sz_img[1];Y++)
	{
		long y_k;//kernel element coordinate
		long y_i;//corresponding image coordinate of kernel
		for(long yy=-radius;yy<=radius;yy++)
		{
			y_k=yy+radius;
			y_i=Y+yy;
			y_i = y_i<0 ? 0:y_i;		y_i = y_i>=sz_img[1] ? sz_img[1]-1:y_i;

			for(long Z=0;Z<sz_img[2];Z++)
				for(long X=0;X<sz_img[0];X++)
					for(long C=0;C<sz_img[3];C++)
						p_img_tmp2_4d[C][Z][Y][X]+=p_img_tmp1_4d[C][Z][y_i][X]*vec1D_kernel[y_k];
		}
	}
	//convolve along Z
	for(long i=0;i<l_npixels;i++) p_img_tmp1[i]=0.0;
	for(long Z=0;Z<sz_img[2];Z++)
	{
		long z_k;//kernel element coordinate
		long z_i;//corresponding image coordinate of kernel
		for(long zz=-radius;zz<=radius;zz++)
		{
			z_k=zz+radius;
			z_i=Z+zz;
			z_i = z_i<0 ? 0:z_i;		z_i = z_i>=sz_img[2] ? sz_img[2]-1:z_i;

			for(long Y=0;Y<sz_img[1];Y++)
				for(long X=0;X<sz_img[0];X++)
					for(long C=0;C<sz_img[3];C++)
						p_img_tmp1_4d[C][Z][Y][X]+=p_img_tmp2_4d[C][z_i][Y][X]*vec1D_kernel[z_k];
		}
	}
						
	//change pointer
	if(p_img) 		{delete []p_img;		p_img=0;}
	p_img=p_img_tmp1;
	p_img_tmp1=0;

	//free memory
	if(p_img_tmp1) 		{delete []p_img_tmp1;		p_img_tmp1=0;}
	if(p_img_tmp2) 		{delete []p_img_tmp2;		p_img_tmp2=0;}
	if(p_img_4d)		{delete4dpointer(p_img_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img_tmp1_4d) 	{delete4dpointer(p_img_tmp1_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}
	if(p_img_tmp2_4d) 	{delete4dpointer(p_img_tmp2_4d,sz_img[0],sz_img[1],sz_img[2],sz_img[3]);}

	return true;
}
