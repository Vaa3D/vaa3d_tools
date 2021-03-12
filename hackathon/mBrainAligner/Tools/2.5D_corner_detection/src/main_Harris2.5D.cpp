#include <stdio.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <numeric>
#include <vector>
#include "basic_memory.cpp"
#include "q_derivatives3D.h"
#include "q_imgaussian3D.h"
#include "q_EigenVectors3D.h"
#include "basic_surf_objs.h"
#include "stackutil.h"

using namespace std;

class point3D64F
{
public:
	double x, y, z;
	point3D64F(double x0, double y0, double z0) { x = x0; y = y0; z = z0; }
	point3D64F() { x = y = z = 0; }
};

#define absf(a) ((a)>(-a)?(a):(-a))

int main()
{
	printf("1. Load image. \n");
	QString qs_filename_img_input = "./img/label_image.v3draw";
	unsigned char *p_img_input = 0;
	long long *sz_img_input = 0;
	int datatype_input = 0;
	long l_npixels = 0;
	int nms2d_w = 7;
	int nms3d_w = 31;
	string save_marker_path = "./result/2.5D_corner.marker";

	if (!qs_filename_img_input.isNull())
	{
		if (!loadImage((char *)qPrintable(qs_filename_img_input), p_img_input, sz_img_input, datatype_input))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_input));
			return false;
		}
		printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_img_input));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]);
		printf("\t\tdatatype: %d\n", datatype_input);
		if (datatype_input != 1)
		{
			printf("ERROR: Input image datatype is not UINT8.\n");
			return false;
		}
	}
	l_npixels = sz_img_input[0] * sz_img_input[1] * sz_img_input[2] * sz_img_input[3];

	/*---------------------------------------------------------------------------------------*/
	printf("2. Convert image datatype from uint8 to 32f and rescale to [0~255]. \n");
	float *p_img32f_tar = 0;
	{
		p_img32f_tar = new(std::nothrow) float[l_npixels]();
		if (!p_img32f_tar)
		{
			printf("ERROR: Fail to allocate memory for p_img32f!\n");
			return false;
		}
		for (long i = 0; i < l_npixels; i++)
		{
			p_img32f_tar[i] = (int)p_img_input[i];
		}
		float d_maxintensity_tar = 0.0;
		for (long i = 0; i < l_npixels; i++)
		{
			if (p_img_input[i] > d_maxintensity_tar)
				d_maxintensity_tar = p_img_input[i];
		}
		for (long i = 0; i < l_npixels; i++)
		{
			p_img32f_tar[i] = p_img_input[i] / d_maxintensity_tar * 255.0;
		}

		printf("\t>>tar_maxintensity=%.2f\n", d_maxintensity_tar);
	}
	/*---------------------------------------------------------------------------------------*/
	printf("3. Calculate gradients in three directions on the original image. \n");
	float *p_img32f_gradx = 0, *p_img32f_grady = 0, *p_img32f_gradz = 0;
	{
		long sz_img[4];
		sz_img[0] = sz_img_input[0]; sz_img[1] = sz_img_input[1]; sz_img[2] = sz_img_input[2]; sz_img[3] = sz_img_input[3];
		//q_sobel_3D(p_img32f, sz_img, p_img32f_gradx, p_img32f_grady, p_img32f_gradz);
		if (!q_derivatives_3D(p_img32f_tar, sz_img, 'x', p_img32f_gradx) ||
			!q_derivatives_3D(p_img32f_tar, sz_img, 'y', p_img32f_grady) ||
			!q_derivatives_3D(p_img32f_tar, sz_img, 'z', p_img32f_gradz))
		{
			printf("ERROR: q_derivatives_3D() return false!\n");
			return false;
		}
	}
	if (p_img32f_tar) 				{ delete[]p_img32f_tar;				p_img32f_tar = 0; }
	/*---------------------------------------------------------------------------------------*/
	printf("4. Compute the 2D eigenvectors and values. \n");
	float *p1_img32f_eigval2 = 0, *p1_img32f_eigval1 = 0;
	float *p2_img32f_eigval2 = 0, *p2_img32f_eigval1 = 0;
	float *p3_img32f_eigval2 = 0, *p3_img32f_eigval1 = 0;
	float *p1_img32f_eigvec2x = 0, *p1_img32f_eigvec2y = 0;
	float *p1_img32f_eigvec1x = 0, *p1_img32f_eigvec1y = 0;
	float *p2_img32f_eigvec2x = 0, *p2_img32f_eigvec2z = 0;
	float *p2_img32f_eigvec1x = 0, *p2_img32f_eigvec1z = 0;
	float *p3_img32f_eigvec2y = 0, *p3_img32f_eigvec2z = 0;
	float *p3_img32f_eigvec1y = 0, *p3_img32f_eigvec1z = 0;
	{
		long sz_img[4];
		sz_img[0] = sz_img_input[0]; sz_img[1] = sz_img_input[1]; sz_img[2] = sz_img_input[2]; sz_img[3] = sz_img_input[3];

		float *p_img32f_Ixx = 0, *p_img32f_Iyy = 0, *p_img32f_Izz = 0;
		float *p_img32f_Ixy = 0, *p_img32f_Ixz = 0, *p_img32f_Iyz = 0;
		p_img32f_Ixx = new(std::nothrow) float[l_npixels]();
		p_img32f_Iyy = new(std::nothrow) float[l_npixels]();
		p_img32f_Izz = new(std::nothrow) float[l_npixels]();
		p_img32f_Ixy = new(std::nothrow) float[l_npixels]();
		p_img32f_Ixz = new(std::nothrow) float[l_npixels]();
		p_img32f_Iyz = new(std::nothrow) float[l_npixels]();
		if (!p_img32f_Ixx || !p_img32f_Iyy || !p_img32f_Izz || !p_img32f_Ixy || !p_img32f_Ixz || !p_img32f_Iyz)
		{
			printf("ERROR: Fail to allocate memory for structure tensor images!\n");
			return false;
		}
		for (long i = 0; i < l_npixels; i++)
		{
			p_img32f_Ixx[i] = p_img32f_gradx[i] * p_img32f_gradx[i];
			p_img32f_Iyy[i] = p_img32f_grady[i] * p_img32f_grady[i];
			p_img32f_Izz[i] = p_img32f_gradz[i] * p_img32f_gradz[i];
			p_img32f_Ixy[i] = p_img32f_gradx[i] * p_img32f_grady[i];
			p_img32f_Ixz[i] = p_img32f_gradx[i] * p_img32f_gradz[i];
			p_img32f_Iyz[i] = p_img32f_grady[i] * p_img32f_gradz[i];
		}
		if (p_img32f_gradx) 			{ delete[]p_img32f_gradx;	p_img32f_gradx = 0; }
		if (p_img32f_grady) 			{ delete[]p_img32f_grady;	p_img32f_grady = 0; }
		if (p_img32f_gradz) 			{ delete[]p_img32f_gradz;	p_img32f_gradz = 0; }
		/*Smooth the structure tensors, for better direction estimation*/
		{
			float *p_img32f_Ixx_smooth = 0, *p_img32f_Iyy_smooth = 0, *p_img32f_Izz_smooth = 0;
			float *p_img32f_Ixy_smooth = 0, *p_img32f_Ixz_smooth = 0, *p_img32f_Iyz_smooth = 0;
			float sigma = 1.3, szkernel = 1;
			if (!q_imgaussian3D(p_img32f_Ixx, sz_img, sigma, szkernel, p_img32f_Ixx_smooth) ||
				!q_imgaussian3D(p_img32f_Iyy, sz_img, sigma, szkernel, p_img32f_Iyy_smooth) ||
				!q_imgaussian3D(p_img32f_Izz, sz_img, sigma, szkernel, p_img32f_Izz_smooth) ||
				!q_imgaussian3D(p_img32f_Ixy, sz_img, sigma, szkernel, p_img32f_Ixy_smooth) ||
				!q_imgaussian3D(p_img32f_Ixz, sz_img, sigma, szkernel, p_img32f_Ixz_smooth) ||
				!q_imgaussian3D(p_img32f_Iyz, sz_img, sigma, szkernel, p_img32f_Iyz_smooth))
			{
				printf("ERROR: q_imgaussian3D() return false!\n");
				return false;
			}
			if (p_img32f_Ixx) 			{ delete[]p_img32f_Ixx;		p_img32f_Ixx = 0; }
			if (p_img32f_Iyy) 			{ delete[]p_img32f_Iyy;		p_img32f_Iyy = 0; }
			if (p_img32f_Izz) 			{ delete[]p_img32f_Izz;		p_img32f_Izz = 0; }
			if (p_img32f_Ixy) 			{ delete[]p_img32f_Ixy;		p_img32f_Ixy = 0; }
			if (p_img32f_Ixz) 			{ delete[]p_img32f_Ixz;		p_img32f_Ixz = 0; }
			if (p_img32f_Iyz) 			{ delete[]p_img32f_Iyz;		p_img32f_Iyz = 0; }
			p_img32f_Ixx = p_img32f_Ixx_smooth;	p_img32f_Iyy = p_img32f_Iyy_smooth;	p_img32f_Izz = p_img32f_Izz_smooth;
			p_img32f_Ixy = p_img32f_Ixy_smooth;	p_img32f_Ixz = p_img32f_Ixz_smooth;	p_img32f_Iyz = p_img32f_Iyz_smooth;
			p_img32f_Ixx_smooth = p_img32f_Iyy_smooth = p_img32f_Izz_smooth = 0;
			p_img32f_Ixy_smooth = p_img32f_Ixz_smooth = p_img32f_Iyz_smooth = 0;
		}
		/*Compute the eigenvectors and values(eigval2>eigval1) of the structure tensors*/
		printf("\t4.1. Calculate the eigenvectors and eigenvalues of the structure tensor in xy. \n");
		if (!l_eigenvectors2D(p_img32f_Ixx, p_img32f_Ixy, p_img32f_Iyy,
			sz_img,
			p1_img32f_eigval2, p1_img32f_eigval1,
			p1_img32f_eigvec2x, p1_img32f_eigvec2y,
			p1_img32f_eigvec1x, p1_img32f_eigvec1y))
		{
			printf("ERROR: l_eigenvectors2D() return false!\n");
			return false;
		}
		printf("\t4.2. Calculate the eigenvectors and eigenvalues of the structure tensor in xz. \n");
		if (!l_eigenvectors2D(p_img32f_Ixx, p_img32f_Ixz, p_img32f_Izz,
			sz_img,
			p2_img32f_eigval2, p2_img32f_eigval1,
			p2_img32f_eigvec2x, p2_img32f_eigvec2z,
			p2_img32f_eigvec1x, p2_img32f_eigvec1z))
		{
			printf("ERROR: l_eigenvectors2D() return false!\n");
			return false;
		}
		printf("\t4.3. Calculate the eigenvectors and eigenvalues of the structure tensor in yz. \n");
		if (!l_eigenvectors2D(p_img32f_Iyy, p_img32f_Iyz, p_img32f_Izz,
			sz_img,
			p3_img32f_eigval2, p3_img32f_eigval1,
			p3_img32f_eigvec2y, p3_img32f_eigvec2z,
			p3_img32f_eigvec1y, p3_img32f_eigvec1z))
		{
			printf("ERROR: l_eigenvectors2D() return false!\n");
			return false;
		}
		if (p_img32f_Ixx) 			    { delete[]p_img32f_Ixx;		p_img32f_Ixx = 0; }
		if (p_img32f_Iyy) 			    { delete[]p_img32f_Iyy;		p_img32f_Iyy = 0; }
		if (p_img32f_Izz) 			    { delete[]p_img32f_Izz;		p_img32f_Izz = 0; }
		if (p_img32f_Ixy) 			    { delete[]p_img32f_Ixy;		p_img32f_Ixy = 0; }
		if (p_img32f_Ixz) 			    { delete[]p_img32f_Ixz;		p_img32f_Ixz = 0; }
		if (p_img32f_Iyz) 			    { delete[]p_img32f_Iyz;		p_img32f_Iyz = 0; }
		if (p1_img32f_eigvec2x) 		{ delete[]p1_img32f_eigvec2x;	p1_img32f_eigvec2x = 0; }
		if (p1_img32f_eigvec2y) 		{ delete[]p1_img32f_eigvec2y;	p1_img32f_eigvec2y = 0; }
		if (p2_img32f_eigvec2x) 		{ delete[]p2_img32f_eigvec2x;	p2_img32f_eigvec2x = 0; }
		if (p2_img32f_eigvec2z) 		{ delete[]p2_img32f_eigvec2z;	p2_img32f_eigvec2z = 0; }
		if (p3_img32f_eigvec2y) 		{ delete[]p3_img32f_eigvec2y;	p3_img32f_eigvec2y = 0; }
		if (p3_img32f_eigvec2z) 		{ delete[]p3_img32f_eigvec2z;	p3_img32f_eigvec2z = 0; }
		if (p1_img32f_eigvec1x) 		{ delete[]p1_img32f_eigvec1x;	p1_img32f_eigvec1x = 0; }
		if (p1_img32f_eigvec1y) 		{ delete[]p1_img32f_eigvec1y;	p1_img32f_eigvec1y = 0; }
		if (p2_img32f_eigvec1x) 		{ delete[]p2_img32f_eigvec1x;	p2_img32f_eigvec1x = 0; }
		if (p2_img32f_eigvec1z) 		{ delete[]p2_img32f_eigvec1z;	p2_img32f_eigvec1z = 0; }
		if (p3_img32f_eigvec1y) 		{ delete[]p3_img32f_eigvec1y;	p3_img32f_eigvec1y = 0; }
		if (p3_img32f_eigvec1z) 		{ delete[]p3_img32f_eigvec1z;	p3_img32f_eigvec1z = 0; }
	}
	/*--------------------------------------------------------------------------------------*/
	printf("5. Compute corner response function R. \n");
	float *p1_img32f_R = 0, *p2_img32f_R = 0, *p3_img32f_R = 0;
	{
		p1_img32f_R = new(std::nothrow) float[l_npixels]();
		p2_img32f_R = new(std::nothrow) float[l_npixels]();
		p3_img32f_R = new(std::nothrow) float[l_npixels]();
		if (!p1_img32f_R || !p2_img32f_R || !p3_img32f_R)
		{
			printf("ERROR: Fail to allocate memory for corner response image R!\n");
			return false;
		}
		float k = 0.04;
		for (long i = 0; i < l_npixels; i++)
		{
			float tmp = p1_img32f_eigval2[i] + p1_img32f_eigval1[i];
			float det = p1_img32f_eigval2[i] * p1_img32f_eigval1[i];
			p1_img32f_R[i] = det - tmp * tmp * k;
		}
		for (long i = 0; i < l_npixels; i++)
		{
			float tmp = p2_img32f_eigval2[i] + p2_img32f_eigval1[i];
			float det = p2_img32f_eigval2[i] * p2_img32f_eigval1[i];
			p2_img32f_R[i] = det - tmp * tmp * k;
		}
		for (long i = 0; i < l_npixels; i++)
		{
			float tmp = p3_img32f_eigval2[i] + p3_img32f_eigval1[i];
			float det = p3_img32f_eigval2[i] * p3_img32f_eigval1[i];
			p3_img32f_R[i] = det - tmp * tmp * k;
		}
	}
	/*----------------------------------------------------------------*/
	vector<point3D64F> vec_corners;
	float ****p1_img_R_4d = 0; float  ****p2_img_R_4d = 0; float  ****p3_img_R_4d = 0; 
	unsigned char ****p1_img_4d = 0, ****p2_img_4d = 0, ****p3_img_4d = 0, ****p_img_input_4d = 0;
	unsigned char *p1_img = new unsigned char[l_npixels]();
	unsigned char *p2_img = new unsigned char[l_npixels]();
	unsigned char *p3_img = new unsigned char[l_npixels]();
	new4dpointer(p1_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p1_img32f_R);
	new4dpointer(p2_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p2_img32f_R);
	new4dpointer(p3_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p3_img32f_R);
	new4dpointer(p1_img_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p1_img);
	new4dpointer(p2_img_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p2_img);
	new4dpointer(p3_img_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p3_img);
	new4dpointer(p_img_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p_img_input);
	printf("6. Get the first candidate point. \n");
	{
		/*-----------------------XY_axis-------------------------------------*/
		int radius = (nms2d_w - 1) / 2;
		int num = 0;
		for (long Z = 0; Z < sz_img_input[2]; Z++)
			for (long Y = 0; Y < sz_img_input[1]; Y++)
				for (long X = 0; X < sz_img_input[0]; X++)
				{
					if (abs(p1_img_R_4d[0][Z][Y][X]) < 0.1)
						continue;
					bool b_ismax = true;
					long x_i, y_i;
					for (long xx = -radius; xx <= radius; xx++)
					{
						x_i = X + xx;
						x_i = x_i < 0 ? 0 : x_i;		x_i = x_i >= sz_img_input[0] ? sz_img_input[0] - 1 : x_i;
						for (long yy = -radius; yy <= radius; yy++)
						{
							y_i = Y + yy;
							y_i = y_i < 0 ? 0 : y_i;		y_i = y_i >= sz_img_input[1] ? sz_img_input[1] - 1 : y_i;
							if (x_i == X && y_i == Y)
								continue;
							if (p1_img_R_4d[0][Z][Y][X] <= p1_img_R_4d[0][Z][y_i][x_i])
							{
								b_ismax = false; break;
							}
						}
						if (!b_ismax) break;
					}
					if (b_ismax)
					{
						p1_img_4d[0][Z][Y][X] = 255;
						num++;
					}
				}
		printf("\t>>The number of first candidate points on the XY view: %d\n", num);
		/*-----------------------XZ_axis-------------------------------------*/
		num = 0;
		for (long Y = 0; Y < sz_img_input[1]; Y++)
			for (long Z = 0; Z < sz_img_input[2]; Z++)
				for (long X = 0; X < sz_img_input[0]; X++)
				{
					if (abs(p2_img_R_4d[0][Z][Y][X]) < 0.1)
						continue;
					bool b_ismax = true;
					long x_i, z_i;
					for (long xx = -radius; xx <= radius; xx++)
					{
						x_i = X + xx;
						x_i = x_i < 0 ? 0 : x_i;		x_i = x_i >= sz_img_input[0] ? sz_img_input[0] - 1 : x_i;
						for (long zz = -radius; zz <= radius; zz++)
						{
							z_i = Z + zz;
							z_i = z_i < 0 ? 0 : z_i;		z_i = z_i >= sz_img_input[2] ? sz_img_input[2] - 1 : z_i;
							if (x_i == X && z_i == Z)
								continue;
							if (p2_img_R_4d[0][Z][Y][X] <= p2_img_R_4d[0][z_i][Y][x_i])
							{
								b_ismax = false; break;
							}
						}
						if (!b_ismax) break;
					}
					if (b_ismax)
					{
						p2_img_4d[0][Z][Y][X] = 255;
						num++;
					}
				}
		printf("\t>>The number of first candidate points on the XZ view: %d\n", num);
		/*-----------------------YZ_axis-------------------------------------*/
		num = 0;
		for (long X = 0; X < sz_img_input[0]; X++)
			for (long Z = 0; Z < sz_img_input[2]; Z++)
				for (long Y = 0; Y < sz_img_input[1]; Y++)
				{
					if (abs(p3_img_R_4d[0][Z][Y][X]) < 0.1)
						continue;
					bool b_ismax = true;
					long y_i, z_i;
					for (long yy = -radius; yy <= radius; yy++)
					{
						y_i = Y + yy;
						y_i = y_i < 0 ? 0 : y_i;		y_i = y_i >= sz_img_input[1] ? sz_img_input[1] - 1 : y_i;
						for (long zz = -radius; zz <= radius; zz++)
						{
							z_i = Z + zz;
							z_i = z_i < 0 ? 0 : z_i;		z_i = z_i >= sz_img_input[2] ? sz_img_input[2] - 1 : z_i;
							if (y_i == Y && z_i == Z)
								continue;
							if (p3_img_R_4d[0][Z][Y][X] <= p3_img_R_4d[0][z_i][y_i][X])
							{
								b_ismax = false; break;
							}
						}
						if (!b_ismax) break;
					}
					if (b_ismax)
					{
						p3_img_4d[0][Z][Y][X] = 255;
						num++;
					}
				}
		printf("\t>>The number of first candidate points on the YZ view: %d\n", num);
	}
	if (p_img_input_4d)		{ delete4dpointer(p_img_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p_img_input) 			{ delete[]p_img_input;			p_img_input = 0; }
	/*---------------------------------------------------------------------------------------*/
	float ****p_img_R_4d = 0;
	float *p_R_img = new float[l_npixels]();
	new4dpointer(p_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p_R_img);
	unsigned char *nms_img = 0;
	nms_img = new unsigned char[l_npixels]();
	unsigned char  ****nms_input_4d = 0;
	new4dpointer(nms_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], nms_img);
	printf("7. Get the second candidate point. \n");
	{
		int num = 0;
		for (long Z = 0; Z < sz_img_input[2]; Z++)
			for (long Y = 0; Y < sz_img_input[1]; Y++)
				for (long X = 0; X < sz_img_input[0]; X++)
				{
					float r_value = 0;
					if ((int)p1_img_4d[0][Z][Y][X] == 255 && (int)p2_img_4d[0][Z][Y][X] == 255 && (int)p3_img_4d[0][Z][Y][X] == 255)
					{
						p_img_R_4d[0][Z][Y][X] = p1_img_R_4d[0][Z][Y][X] + p2_img_R_4d[0][Z][Y][X] + p3_img_R_4d[0][Z][Y][X];
						num++;
					}
					else
					{
						if ((int)p1_img_4d[0][Z][Y][X] == 255 && (int)p2_img_4d[0][Z][Y][X] == 255)
						{
							if (r_value < (p1_img_R_4d[0][Z][Y][X] + p2_img_R_4d[0][Z][Y][X]))
								r_value = p1_img_R_4d[0][Z][Y][X] + p2_img_R_4d[0][Z][Y][X];
						}
						if ((int)p1_img_4d[0][Z][Y][X] == 255 && (int)p3_img_4d[0][Z][Y][X] == 255)  
						{
							if (r_value < (p1_img_R_4d[0][Z][Y][X] + p3_img_R_4d[0][Z][Y][X]))
								r_value = p1_img_R_4d[0][Z][Y][X] + p3_img_R_4d[0][Z][Y][X];
						}
						if ((int)p2_img_4d[0][Z][Y][X] == 255 && (int)p3_img_4d[0][Z][Y][X] == 255)
						{
							if (r_value < (p2_img_R_4d[0][Z][Y][X] + p3_img_R_4d[0][Z][Y][X]))
								r_value = p2_img_R_4d[0][Z][Y][X] + p3_img_R_4d[0][Z][Y][X];
						}
						if (r_value > 0.0)
						{
							p_img_R_4d[0][Z][Y][X] = r_value;
							num++;
						}
					}
				}
		printf("\t>>The number of second candidate points: %d\n", num);
	}
	if (p1_img_R_4d)		{ delete4dpointer(p1_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p2_img_R_4d)		{ delete4dpointer(p2_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p3_img_R_4d)		{ delete4dpointer(p3_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p1_img_4d)		    { delete4dpointer(p1_img_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p2_img_4d)		    { delete4dpointer(p2_img_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p3_img_4d)		    { delete4dpointer(p3_img_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p1_img) 			{ delete[]p1_img;			p1_img = 0; }
	if (p2_img) 			{ delete[]p2_img;			p2_img = 0; }
	if (p3_img) 			{ delete[]p3_img;			p3_img = 0; }
	if (p1_img32f_R) 			{ delete[]p1_img32f_R;			p1_img32f_R = 0; }
	if (p2_img32f_R) 			{ delete[]p2_img32f_R;			p2_img32f_R = 0; }
	if (p3_img32f_R) 			{ delete[]p3_img32f_R;			p3_img32f_R = 0; }
	if (p1_img32f_eigval2) 		{ delete[]p1_img32f_eigval2;		p1_img32f_eigval2 = 0; }
	if (p1_img32f_eigval1) 		{ delete[]p1_img32f_eigval1;		p1_img32f_eigval1 = 0; }
	if (p2_img32f_eigval2) 		{ delete[]p2_img32f_eigval2;		p2_img32f_eigval2 = 0; }
	if (p2_img32f_eigval1) 		{ delete[]p2_img32f_eigval1;		p2_img32f_eigval1 = 0; }
	if (p3_img32f_eigval2) 		{ delete[]p3_img32f_eigval2;		p3_img32f_eigval2 = 0; }
	if (p3_img32f_eigval1) 		{ delete[]p3_img32f_eigval1;		p3_img32f_eigval1 = 0; }
	/*---------------------------------------------------------------------------------------*/
	int nms3d_r = (nms3d_w - 1) / 2;
	printf("8. Get the final marker point. \n");
	{
		for (long Z = 0; Z < sz_img_input[2]; Z++)
			for (long Y = 0; Y < sz_img_input[1]; Y++)
				for (long X = 0; X < sz_img_input[0]; X++)
				{
					if (p_img_R_4d[0][Z][Y][X] == 0.0)
						continue;
					bool b_ismax = true;
					long x_i, y_i, z_i;
					for (long xx = -nms3d_r; xx <= nms3d_r; xx++)
					{
						x_i = X + xx;
						x_i = x_i < 0 ? 0 : x_i;		x_i = x_i >= sz_img_input[0] ? sz_img_input[0] - 1 : x_i;
						for (long yy = -nms3d_r; yy <= nms3d_r; yy++)
						{
							y_i = Y + yy;
							y_i = y_i < 0 ? 0 : y_i;		y_i = y_i >= sz_img_input[1] ? sz_img_input[1] - 1 : y_i;
							for (long zz = -nms3d_r; zz <= nms3d_r; zz++)
							{
								z_i = Z + zz;
								z_i = z_i < 0 ? 0 : z_i;		z_i = z_i >= sz_img_input[2] ? sz_img_input[2] - 1 : z_i;
								if (x_i == X && y_i == Y && z_i == Z)
									continue;
								if (p_img_R_4d[0][Z][Y][X] <= p_img_R_4d[0][z_i][y_i][x_i])
								{
									b_ismax = false; break;
								}
							}
							if (!b_ismax) break;
						}
						if (!b_ismax) break;
					}
					if (b_ismax)
					{
						point3D64F cornerpos(X, Y, Z);
						vec_corners.push_back(cornerpos);
					}
				}
		printf("\t\t[%d] the number of points after the NMS.\n", vec_corners.size());
		QList<ImageMarker> ql_marker;
		for (unsigned long i = 0; i < vec_corners.size(); i++)
		{
			ImageMarker marker;
			marker.radius = 2;
			marker.color.r = 0;
			marker.color.g = 255;
			marker.color.b = 0;
			marker.shape = 1;
			marker.x = vec_corners[i].x; marker.y = vec_corners[i].y; marker.z = vec_corners[i].z;
			ql_marker.push_back(marker);
		}
		char filename[2000];
		sprintf(filename, save_marker_path.c_str());
		writeMarker_file(filename, ql_marker);
	}
	/*-----------------------------------------------------------------------------*/
	printf("*. free memory. \n");
	if (sz_img_input) 			{ delete[]sz_img_input;			sz_img_input = 0; }
	if (p_img_R_4d)		        { delete4dpointer(p_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p_R_img) 			    { delete[]p_R_img;			p_R_img = 0; }
	printf("Program exit success.\n");
	return 0;
}
