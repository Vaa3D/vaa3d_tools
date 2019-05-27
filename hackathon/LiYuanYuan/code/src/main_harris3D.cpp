//main_srsreg.cpp

// by Lei Qu@20190425

#include <stdio.h>
#include "stackutil.h"
#include "basic_surf_objs.h"
#include "../../basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions

#include "q_sobel3D.cpp"//template function
#include "q_derivatives3D.h"
#include "q_imgaussian3D.h"
#include "q_EigenVectors3D.h"

class Coord3D_PCM
{
public:
	double x, y, z;
	Coord3D_PCM(double x0, double y0, double z0) { x = x0; y = y0; z = z0; }
	Coord3D_PCM() { x = y = z = 0; }
};

int main(int argc, char *argv[])
{
	//-----------------------------------------------------------------------------------------
	printf("1. Load image. \n");
	QString qs_filename_img_input = "./average_template_25_u8_xpad_s2.v3draw";
	//QString qs_filename_img_input = "./17781_s4_ssd_s2.v3draw";
	unsigned char *p_img_input = 0;
	long long *sz_img_input = 0;
	int datatype_input = 0;
	long l_npixels = 0;
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

	//-----------------------------------------------------------------------------------------
	printf("2. Convert image datatype from uint8 to 32f and rescale to [0~255]. \n");
	float *p_img32f = 0;
	{
		p_img32f = new(std::nothrow) float[l_npixels]();
		if (!p_img32f)
		{
			printf("ERROR: Fail to allocate memory for p_img32f!\n");
			return false;
		}

		//find the maximal intensity value
		float d_maxintensity_input = 0.0;
		for (long i = 0; i<l_npixels; i++)
			if (p_img_input[i]>d_maxintensity_input)
				d_maxintensity_input = p_img_input[i];
		//convert and rescale
		for (long i = 0; i < l_npixels; i++)
			p_img32f[i] = p_img_input[i] / d_maxintensity_input*255.0;
		printf("\t>>d_maxintensity=%.2f\n", d_maxintensity_input);
	}

	//-----------------------------------------------------------------------------------------
	printf("3. Calculate the gradient images. \n");
	float *p_img32f_gradx = 0, *p_img32f_grady = 0, *p_img32f_gradz = 0;
	{
		long sz_img[4];
		sz_img[0] = sz_img_input[0]; sz_img[1] = sz_img_input[1]; sz_img[2] = sz_img_input[2]; sz_img[3] = sz_img_input[3];

		//calculate the gradients
		//q_sobel_3D(p_img32f, sz_img, p_img32f_gradx, p_img32f_grady, p_img32f_gradz);
		if (!q_derivatives_3D(p_img32f, sz_img, 'x', p_img32f_gradx) ||
			!q_derivatives_3D(p_img32f, sz_img, 'y', p_img32f_grady) ||
			!q_derivatives_3D(p_img32f, sz_img, 'z', p_img32f_gradz))
		{
			printf("ERROR: q_derivatives_3D() return false!\n");
			return false;
		}
		saveImage("gradx.v3draw", (unsigned char *)p_img32f_gradx, sz_img_input, 4);
		saveImage("grady.v3draw", (unsigned char *)p_img32f_grady, sz_img_input, 4);
		saveImage("gradz.v3draw", (unsigned char *)p_img32f_gradz, sz_img_input, 4);
	}

	//-----------------------------------------------------------------------------------------
	printf("4. Compute the eigenvectors and values. \n");
	float *p_img32f_eigval3 = 0, *p_img32f_eigval2 = 0, *p_img32f_eigval1 = 0;
	float *p_img32f_eigvec3x = 0, *p_img32f_eigvec3y = 0, *p_img32f_eigvec3z = 0;
	float *p_img32f_eigvec2x = 0, *p_img32f_eigvec2y = 0, *p_img32f_eigvec2z = 0;
	float *p_img32f_eigvec1x = 0, *p_img32f_eigvec1y = 0, *p_img32f_eigvec1z = 0;
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

		//Smooth the structure tensors, for better direction estimation
		{
			float *p_img32f_Ixx_smooth = 0, *p_img32f_Iyy_smooth = 0, *p_img32f_Izz_smooth = 0;
			float *p_img32f_Ixy_smooth = 0, *p_img32f_Ixz_smooth = 0, *p_img32f_Iyz_smooth = 0;
			float sigma = 2, szkernel = 3;
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
			//replace original image pointers
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

		//Compute the eigenvectors and values(eigval3>eigval2>eigval1) of the structure tensors
		if(!q_eigenvectors3D(p_img32f_Ixx,p_img32f_Ixy,p_img32f_Ixz,
							 p_img32f_Iyy,p_img32f_Iyz,p_img32f_Izz,
							 sz_img,
							 p_img32f_eigval3,p_img32f_eigval2,p_img32f_eigval1,
							 p_img32f_eigvec3x,p_img32f_eigvec3y,p_img32f_eigvec3z,
							 p_img32f_eigvec2x,p_img32f_eigvec2y,p_img32f_eigvec2z,
							 p_img32f_eigvec1x,p_img32f_eigvec1y,p_img32f_eigvec1z))
		{
			printf("ERROR: q_eigenvectors3D() return false!\n");
			return false;
		}
		////eigen -> singular
		//for (long i = 0; i < l_npixels; i++)
		//{
		//	p_img32f_eigval3[i] = sqrtf(fabs(p_img32f_eigval3[i]));
		//	p_img32f_eigval2[i] = sqrtf(fabs(p_img32f_eigval2[i]));
		//	p_img32f_eigval1[i] = sqrtf(fabs(p_img32f_eigval1[i]));
		//}
		if (p_img32f_Ixx) 			{ delete[]p_img32f_Ixx;		p_img32f_Ixx = 0; }
		if (p_img32f_Iyy) 			{ delete[]p_img32f_Iyy;		p_img32f_Iyy = 0; }
		if (p_img32f_Izz) 			{ delete[]p_img32f_Izz;		p_img32f_Izz = 0; }
		if (p_img32f_Ixy) 			{ delete[]p_img32f_Ixy;		p_img32f_Ixy = 0; }
		if (p_img32f_Ixz) 			{ delete[]p_img32f_Ixz;		p_img32f_Ixz = 0; }
		if (p_img32f_Iyz) 			{ delete[]p_img32f_Iyz;		p_img32f_Iyz = 0; }

		saveImage("eigval3.v3draw", (unsigned char *)p_img32f_eigval3, sz_img_input, 4);
		saveImage("eigval2.v3draw", (unsigned char *)p_img32f_eigval2, sz_img_input, 4);
		saveImage("eigval1.v3draw", (unsigned char *)p_img32f_eigval1, sz_img_input, 4);
	}

	//-----------------------------------------------------------------------------------------
	printf("5. Compute corner response function R. \n");
	//2D: det(M) - k(trace(M))^2 <==> u1*u2-k(u1+u2)^2, 0.04<=k<=0.06
	//3D: det(M) - k(trace(M))^3 <==> u1*u2*u3-k(u1+u2+u3)^3, ???
	float *p_img32f_R = 0;
	{
		p_img32f_R = new(std::nothrow) float[l_npixels]();
		if (!p_img32f_R)
		{
			printf("ERROR: Fail to allocate memory for corner response image R!\n");
			return false;
		}

		//float k = 0.05;
		float k = 10e-7;
		for (long i = 0; i < l_npixels; i++)
		{
			float tmp = p_img32f_eigval3[i] + p_img32f_eigval2[i];
			p_img32f_R[i] = (p_img32f_eigval3[i] * p_img32f_eigval2[i]) - k*tmp*tmp;
			//float tmp = p_img32f_eigval3[i] + p_img32f_eigval1[i];
			//p_img32f_R[i] = (p_img32f_eigval3[i] * p_img32f_eigval1[i]) - k*tmp*tmp;
		}
		//float k = 10e-5;
		//for (long i = 0; i < l_npixels; i++)
		//{
		//	float tmp = p_img32f_eigval3[i] + p_img32f_eigval2[i] + p_img32f_eigval1[i];
		//	p_img32f_R[i] = (p_img32f_eigval3[i] * p_img32f_eigval2[i] * p_img32f_eigval1[i]) - k*tmp*tmp*tmp;
		//	//p_img32f_R[i] = (p_img32f_eigval3[i] * p_img32f_eigval2[i] * p_img32f_eigval1[i]) - k*tmp*tmp;
		//	//p_img32f_R[i] = (p_img32f_eigval3[i] * p_img32f_eigval1[i]);
		//}

		////normalize R to 0~255
		////find the max&min intensity value
		//float d_maxint = 0.0, d_minint=10e+10;
		//for (long i = 0; i<l_npixels; i++)
		//	if (p_img32f_R[i]>d_maxint)			d_maxint = p_img32f_R[i];
		//	else if (p_img32f_R[i]<d_minint)	d_minint = p_img32f_R[i];
		////convert and rescale
		//for (long i = 0; i < l_npixels; i++)
		//	p_img32f_R[i] = (p_img32f_R[i]-d_minint) / d_maxint*255.0;
		//printf("\t>>intensity_max=%.2f, intensity_min=%.2f\n", d_maxint, d_minint);

		saveImage("R.v3draw", (unsigned char *)p_img32f_R, sz_img_input, 4);
	}

	//{
	//	unsigned char *p_tmp = 0;
	//	p_tmp = new(std::nothrow) unsigned char[l_npixels]();
	//	for (long i = 0; i < l_npixels; i++)
	//		if (p_img32f_R[i]>5) p_tmp[i] = 255;

	//	saveImage("R_thresh.v3draw", (unsigned char *)p_tmp, sz_img_input, 1);
	//}

	//-----------------------------------------------------------------------------------------
	printf("6. NMS. \n");
	{
		vector<Coord3D_PCM> vec_corners;
		unsigned char *p_img_nms = 0;
		p_img_nms = new(std::nothrow) unsigned char[l_npixels]();
		float  ****p_img_R_4d = 0;
		unsigned char ****p_img_nms_4d = 0, ****p_img_input_4d = 0;
		new4dpointer(p_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p_img32f_R);
		new4dpointer(p_img_nms_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p_img_nms);
		new4dpointer(p_img_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p_img_input);

		int radius = 3;//3
		for (long Z = 0; Z < sz_img_input[2]; Z++)
			for (long Y = 0; Y < sz_img_input[1]; Y++)
				for (long X = 0; X < sz_img_input[0]; X++)
				{
					if (p_img_R_4d[0][Z][Y][X] < 10e+1) continue;
					if (p_img_input_4d[0][Z][Y][X] < 5) continue;

					bool b_ismax = true;
					long x_i, y_i, z_i;//corresponding image coordinate of kernel
					for (long xx = -radius; xx <= radius; xx++)
					{
						x_i = X + xx;
						x_i = x_i < 0 ? 0 : x_i;		x_i = x_i >= sz_img_input[0] ? sz_img_input[0] - 1 : x_i;
						for (long yy = -radius; yy <= radius; yy++)
						{
							y_i = Y + yy;
							y_i = y_i < 0 ? 0 : y_i;		y_i = y_i >= sz_img_input[1] ? sz_img_input[1] - 1 : y_i;
							for (long zz = -radius; zz <= radius; zz++)
							{
								z_i = Z + zz;
								z_i = z_i < 0 ? 0 : z_i;		z_i = z_i >= sz_img_input[2] ? sz_img_input[2] - 1 : z_i;

								if (p_img_R_4d[0][Z][Y][X] < p_img_R_4d[0][z_i][y_i][x_i])
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
						p_img_nms_4d[0][Z][Y][X] = 255;
						Coord3D_PCM cornerpos(X, Y, Z);
						vec_corners.push_back(cornerpos);
					}
				}
		printf("\t[%d] corner detected after NMS.\n", vec_corners.size());

		saveImage("R_nms.v3draw", (unsigned char *)p_img_nms, sz_img_input, 1);
		QList<ImageMarker> ql_marker;
		for (unsigned long i = 0; i < vec_corners.size(); i++)
		{
			ImageMarker marker;
			marker.radius = 5;
			marker.x = vec_corners[i].x*2; marker.y = vec_corners[i].y*2; marker.z = vec_corners[i].z*2;
			ql_marker.push_back(marker);
		}
		writeMarker_file("harris3D_corners.marker", ql_marker);

		if (p_img_input_4d)		{ delete4dpointer(p_img_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
		if (p_img_R_4d)			{ delete4dpointer(p_img_R_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
		if (p_img_nms_4d)		{ delete4dpointer(p_img_nms_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
		if (p_img_nms) 			{ delete[]p_img_nms;			p_img_nms = 0; }
	}

	//-----------------------------------------------------------------------------------------
	printf("*. free memory. \n");
	if (p_img32f_R) 			{ delete[]p_img32f_R;			p_img32f_R = 0; }
	if (p_img32f_eigval3) 		{ delete[]p_img32f_eigval3;		p_img32f_eigval3 = 0; }
	if (p_img32f_eigval2) 		{ delete[]p_img32f_eigval2;		p_img32f_eigval2 = 0; }
	if (p_img32f_eigval1) 		{ delete[]p_img32f_eigval1;		p_img32f_eigval1 = 0; }
	if (p_img32f_eigvec3x) 		{ delete[]p_img32f_eigvec3x;	p_img32f_eigvec3x = 0; }
	if (p_img32f_eigvec3y) 		{ delete[]p_img32f_eigvec3y;	p_img32f_eigvec3y = 0; }
	if (p_img32f_eigvec3z) 		{ delete[]p_img32f_eigvec3z;	p_img32f_eigvec3z = 0; }
	if (p_img32f_eigvec2x) 		{ delete[]p_img32f_eigvec2x;	p_img32f_eigvec2x = 0; }
	if (p_img32f_eigvec2y) 		{ delete[]p_img32f_eigvec2y;	p_img32f_eigvec2y = 0; }
	if (p_img32f_eigvec2z) 		{ delete[]p_img32f_eigvec2z;	p_img32f_eigvec2z = 0; }
	if (p_img32f_eigvec1x) 		{ delete[]p_img32f_eigvec1x;	p_img32f_eigvec1x = 0; }
	if (p_img32f_eigvec1y) 		{ delete[]p_img32f_eigvec1y;	p_img32f_eigvec1y = 0; }
	if (p_img32f_eigvec1z) 		{ delete[]p_img32f_eigvec1z;	p_img32f_eigvec1z = 0; }
	if (p_img32f_gradx) 		{ delete[]p_img32f_gradx;		p_img32f_gradx = 0; }
	if (p_img32f_grady) 		{ delete[]p_img32f_grady;		p_img32f_grady = 0; }
	if (p_img32f_gradz) 		{ delete[]p_img32f_gradz;		p_img32f_gradz = 0; }
	if (p_img32f) 				{ delete[]p_img32f;				p_img32f = 0; }
	if (p_img_input) 			{ delete[]p_img_input;			p_img_input = 0; }
	if (sz_img_input) 			{ delete[]sz_img_input;			sz_img_input = 0; }

	printf("Program exit success.\n");
	return true;
}

//{
//	unsigned char *p_img_tmp = new(std::nothrow) unsigned char[l_npixels]();
//	for (long i = 0; i < l_npixels; i++)
//		p_img_tmp[i] = (unsigned int)(p_img32f_gradx[i]);
//	saveImage("gradx.v3draw", p_img_tmp, sz_img_input, 1);
//	delete[]p_img_tmp;
//}
