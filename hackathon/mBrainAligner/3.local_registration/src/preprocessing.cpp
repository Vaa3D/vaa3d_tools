
#include "preprocessing.h"



bool Calculate_gradient_img(float *&grand_sub, long long * & sz_img_sub, float * & p_img32f_sub255, unsigned char *p_img_sub)

{
	float *p_img32f_gradx = 0, *p_img32f_grady = 0, *p_img32f_gradz = 0;
	long sz_img[4];
	sz_img[0] = sz_img_sub[0]; sz_img[1] = sz_img_sub[1]; sz_img[2] = sz_img_sub[2]; sz_img[3] = sz_img_sub[3];

	//calculate the gradients
	//q_sobel_3D(p_img32f, sz_img, p_img32f_gradx, p_img32f_grady, p_img32f_gradz);
	if (!q_derivatives_3D(p_img32f_sub255, sz_img, 'x', p_img32f_gradx) ||
		!q_derivatives_3D(p_img32f_sub255, sz_img, 'y', p_img32f_grady) ||
		!q_derivatives_3D(p_img32f_sub255, sz_img, 'z', p_img32f_gradz))
	{
		printf("ERROR: q_derivatives_3D() return false!\n");
		return false;
	}
	
	long long l_npixels = sz_img[0] * sz_img[1] * sz_img[2];
	grand_sub = new(std::nothrow) float[l_npixels]();

	for (long i = 0; i < l_npixels; i++)
	{
		grand_sub[i] = pow(p_img32f_gradx[i] * p_img32f_gradx[i] + p_img32f_grady[i] * p_img32f_grady[i] + p_img32f_gradz[i] * p_img32f_gradz[i], 0.5);
		if (p_img_sub[i] == 0)
			grand_sub[i] == 0;
	}

	if (p_img32f_gradx) 			{ delete[]p_img32f_gradx;		p_img32f_gradx = 0; }
	if (p_img32f_grady) 			{ delete[]p_img32f_grady;		p_img32f_grady = 0; }
	if (p_img32f_gradz) 			{ delete[]p_img32f_gradz;		p_img32f_gradz = 0; }

	return true;


}

bool Convert_image_datatype(float * & p_img32f_tar,  float * & p_img32f_sub_bk, float * &grand_tar, float * &grand_sub, unsigned char *&p_img_sub, long long * & sz_img)
{
	long long l_npixels = sz_img[0] * sz_img[1] * sz_img[2];
	p_img32f_tar = new(std::nothrow) float[l_npixels]();
	p_img32f_sub_bk = new(std::nothrow) float[l_npixels]();

	//find the max and min intensity value
	float f_maxint_tar = 0.0, f_maxint_sub = 0.0;
	float f_minint_tar = 10e+10, f_minint_sub = p_img_sub[0];
	for (long long i = 0; i < l_npixels; i++)
	{
		if (grand_tar[i] > f_maxint_tar)	f_maxint_tar = grand_tar[i];
		if (grand_sub[i] > f_maxint_sub)	f_maxint_sub = grand_sub[i];
		if (grand_tar[i] < f_minint_tar)	f_minint_tar = grand_tar[i];
		if (grand_sub[i] < f_minint_sub)	f_minint_sub = grand_sub[i];
	}
	printf("\t\t>>maxint_tar=%.2f, minint_tar=%.2f\n", f_maxint_tar, f_minint_tar);
	printf("\t\t>>maxint_sub=%.2f, minint_sub=%.2f\n", f_maxint_sub, f_minint_sub);
	//convert and scale


	float  ****p_img32f_tar_4d = 0, ****p_img32f_sub_4d = 0;
	new4dpointer(p_img32f_tar_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img32f_tar);
	new4dpointer(p_img32f_sub_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img32f_sub_bk);


	for (int z = 0; z < sz_img[2]; z++)
		for (int y = 0; y < sz_img[1]; y++)
			for (int x = 0; x < sz_img[0]; x++)
			{
				int dex = z*sz_img[1] * sz_img[0] + y*sz_img[0] + x;
				
				p_img32f_tar_4d[0][z][y][x] = (grand_tar[dex] - f_minint_tar) / (f_maxint_tar - f_minint_tar);

				p_img32f_sub_4d[0][z][y][x] = (grand_sub[dex] - f_minint_sub) / (f_maxint_sub - f_minint_sub);
			}

	saveImage("laplalcian_tar.v3draw", (unsigned char *)p_img32f_tar, sz_img, 4);
	saveImage("laplalcian_sub.v3draw", (unsigned char *)p_img32f_sub_bk, sz_img, 4);


	if (grand_tar) 			{ delete[]grand_tar;		grand_tar = 0; }
	if (grand_sub) 			{ delete[]grand_sub;		grand_sub = 0; }


	if (p_img32f_tar_4d)		{ delete4dpointer(p_img32f_tar_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
	if (p_img32f_sub_4d)		{ delete4dpointer(p_img32f_sub_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }
	return true;
}

bool Convert_image255(float *&p_img32f_tar255, float * &p_img32f_sub255,unsigned char * & p_img_sub,
	unsigned char * & p_img_tar, long long & l_npixels)

{
	p_img32f_tar255 = new(std::nothrow) float[l_npixels]();
	p_img32f_sub255 = new(std::nothrow) float[l_npixels]();
	/*p_img32_sub_label = new(std::nothrow) float[l_npixels]();*/



	//find the maximal intensity value
	float d_maxintensity_tar = 0.0, d_maxintensity_sub = 0.0;
	for (long i = 0; i < l_npixels; i++)
	{
		if (p_img_tar[i]>d_maxintensity_tar)
			d_maxintensity_tar = p_img_tar[i];
		if (p_img_sub[i]>d_maxintensity_sub)
			d_maxintensity_sub = p_img_sub[i];
	}


	//convert and rescale
	for (long i = 0; i < l_npixels; i++)
	{

		p_img32f_sub255[i] = p_img_sub[i] / d_maxintensity_sub*255.0;
		p_img32f_tar255[i] = p_img_tar[i] / d_maxintensity_tar*255.0;
	/*	p_img32_sub_label[i] = p_img_sub_label[i];*/

	}


	for (long i = 0; i < l_npixels; i++)
	{
		if (p_img_sub[i] <= 20)
			p_img32f_sub255[i] = 0;
	}


	printf("\t>>d_maxintensity_tar=%.2f\n", d_maxintensity_tar);
	printf("\t>>d_maxintensity_sub=%.2f\n", d_maxintensity_sub);


	return true;
}

