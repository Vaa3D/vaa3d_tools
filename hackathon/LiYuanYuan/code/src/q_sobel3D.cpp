// q_sobel_3D.cpp
// by Lei Qu
// 2019-04-26

#ifndef Q_SOBEL_3D_CPP_
#define Q_SOBEL_3D_CPP_

#include <vector>
using namespace std;
#include "math.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions

template<class T>
bool q_sobel_3D(T *&p_img_input, const long *sz_img,
	T *&p_img_dx, T *&p_img_dy, T *&p_img_dz)
{
	//check paras
	if(p_img_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img[0]<=0 || sz_img[1]<=0 || sz_img[2]<=0 || sz_img[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if (p_img_dx) 	{ delete[]p_img_dx;		p_img_dx = 0; }
	if (p_img_dy) 	{ delete[]p_img_dy;		p_img_dy = 0; }
	if (p_img_dz) 	{ delete[]p_img_dz;		p_img_dz = 0; }

	vector< vector< vector<double> > > vec3D_kernel_x, vec3D_kernel_y, vec3D_kernel_z;
	vec3D_kernel_x.assign(3, vector< vector<double> >(3, vector<double>(3, 0.0)));
	vec3D_kernel_y.assign(3, vector< vector<double> >(3, vector<double>(3, 0.0)));
	vec3D_kernel_z.assign(3, vector< vector<double> >(3, vector<double>(3, 0.0)));
	for (int m = 0; m < 3; m++)
		for (int n = 0; n < 3; n++)
		{
			vec3D_kernel_x[m][0][n] = -1;
			vec3D_kernel_x[m][2][n] = 1;
			vec3D_kernel_y[0][m][n] = -1;
			vec3D_kernel_y[2][m][n] = 1;
			vec3D_kernel_z[m][n][0] = -1;
			vec3D_kernel_z[m][n][2] = 1;
		}


	//allocate memory
	long l_npixels = sz_img[0] * sz_img[1] * sz_img[2] * sz_img[3];
	p_img_dx = new T[l_npixels]();
	p_img_dy = new T[l_npixels]();
	p_img_dz = new T[l_npixels]();
	if (!p_img_dx || !p_img_dy || !p_img_dz)
	{
		printf("ERROR: Fail to allocate memory for output image!\n");
		return false;
	}
	T ****p_img_4d = 0, ****p_img_dx_4d = 0, ****p_img_dy_4d = 0, ****p_img_dz_4d = 0;
	if (!new4dpointer(p_img_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img_input) ||
		!new4dpointer(p_img_dx_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img_dx) ||
		!new4dpointer(p_img_dy_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img_dy) ||
		!new4dpointer(p_img_dz_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img_dz))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img_dx) 		{ delete[]p_img_dx;		p_img_dx = 0; }
		if (p_img_dy) 		{ delete[]p_img_dy;		p_img_dy = 0; }
		if (p_img_dz) 		{ delete[]p_img_dz;		p_img_dz = 0; }
		if (p_img_4d)		{ delete4dpointer(p_img_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
		if (p_img_dx_4d) 	{ delete4dpointer(p_img_dx_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
		if (p_img_dy_4d) 	{ delete4dpointer(p_img_dy_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
		if (p_img_dz_4d) 	{ delete4dpointer(p_img_dz_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
		return false;
	}

	//convolve
	int radius_x = 1, radius_y = 1, radius_z = 1;
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
							{
								p_img_dx_4d[C][Z][Y][X] += p_img_4d[C][z_i][y_i][x_i] * vec3D_kernel_x[y_k][x_k][z_k];
								p_img_dy_4d[C][Z][Y][X] += p_img_4d[C][z_i][y_i][x_i] * vec3D_kernel_y[y_k][x_k][z_k];
								p_img_dz_4d[C][Z][Y][X] += p_img_4d[C][z_i][y_i][x_i] * vec3D_kernel_z[y_k][x_k][z_k];
							}
						}
					}
				}

			}

	//free memory
	if (p_img_4d)		{ delete4dpointer(p_img_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
	if (p_img_dx_4d) 	{ delete4dpointer(p_img_dx_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
	if (p_img_dy_4d) 	{ delete4dpointer(p_img_dy_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
	if (p_img_dz_4d) 	{ delete4dpointer(p_img_dz_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }

	return true;
}

#endif