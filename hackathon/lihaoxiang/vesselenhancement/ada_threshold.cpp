/*
 *  ada_threshold.cpp
 *
 *  Created by Yang, Jinzhu and Hanchuan Peng, on 11/22/10.
 *  Add dofunc() interface by Jianlong Zhou, 2012-04-18.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdlib.h>
//#include "newmat.h"
#include "ada_threshold.h"
#include "v3d_message.h"
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"
#include "stackutil.h"

#define INF 100000
using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(threshold, ThPlugin);

template <class T> T abs_lambda(T num);

template <class T> bool swapthree(T& dummya, T& dummyb, T& dummyc);

template <class T> void gaussian_filter(T* data1d, V3DLONG *in_sz, unsigned int Wx, unsigned int Wy, unsigned int Wz, unsigned int c, double sigma, float* &outimg);

template <class T>
void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d, string path1, string path2)
{
	
	V3DLONG ii,i,j,k,n;
	double sum_response,response;
	float filtered,sum_filtered;
	sum_filtered = 0;
	
	
	V3DLONG mCount = iImageHeight * iImageWidth;  //mCount为一层的像素点数
	V3DLONG mNumber = iImageHeight * iImageWidth * iImageLayer;
	V3DLONG size_image[4];

	aspOutput = new T [mNumber];

    size_image[0] = iImageWidth;
    size_image[1] = iImageHeight;
    size_image[2] = iImageLayer;
    size_image[3] = 1;

	unsigned int wx = 1, wy = 1, wz = 1, c = 1;
	double sigma1 = 1, sigma2 = 2;
    float * filtered_one=0;

	gaussian_filter(apsInput, size_image, wx, wy, wz, c, sigma1, filtered_one);


	double fxx, fyy, fzz, fxy, fyz, fzx;
	double lambda1, lambda2, lambda3;
	double x[4],y[4],z[4],xy[4],yz[4],zx[4];
	double xvalue;
	double max3, min3, max2, min2, maxresponse, mean2, mean3,meaninput,sumxx,sumyy,sumzz,sumxy,sumyz,sumzx;
	double value_90,suml3,suml2;
	value_90 = 0;
	max3 = 0;
	min3 = 0;
	max2 = 0;
	min2 = 0;
	maxresponse = 0;
	meaninput = 0;
	sumxx = 0;
	sumyy = 0;
	sumzz = 0;
	sumxy = 0;
	sumyz = 0;
	sumzx = 0;
	suml3 = 0;
	suml2 = 0;


	

	for (i = 0; i<iImageLayer; i++)              //i为层数
	{
		for (j = 0; j<iImageHeight; j++)         //j为行数
		{
			for (k = 0; k<iImageWidth; k++)		 //k为列数
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
				V3DLONG curpos1 = i* mCount + j*iImageWidth;       //curpos1代表该层该行第一个像素序号
				V3DLONG curpos2 = j* iImageWidth + k;              //curpos2代表像素在该层内的序号
				
				meaninput = meaninput + apsInput[curpos];
			
			
				if (i <= 1 || j <= 1 || k <= 1 || i >= iImageLayer - 2 || j >= iImageHeight - 2 || k >= iImageWidth - 2)
							{
								aspOutput[curpos] = 0;
							}
							else
							{   
								//求Hessian矩阵
								
								x[0] = apsInput[curpos - 2];
								x[1] = apsInput[curpos - 1];
								x[2] = apsInput[curpos + 1];
								x[3] = apsInput[curpos + 2];

								y[0] = apsInput[curpos - 2 * iImageWidth];
								y[1] = apsInput[curpos - 1 * iImageWidth];
								y[2] = apsInput[curpos + 1 * iImageWidth];
								y[3] = apsInput[curpos + 2 * iImageWidth];

								z[0] = apsInput[curpos - 2 * mCount];
								z[1] = apsInput[curpos - 1 * mCount];
								z[2] = apsInput[curpos + 1 * mCount];
								z[3] = apsInput[curpos + 2 * mCount];

								xy[0] = apsInput[curpos - 1 - 1 * iImageWidth];
								xy[1] = apsInput[curpos + 1 - 1 * iImageWidth];
								xy[2] = apsInput[curpos - 1 + 1 * iImageWidth];
								xy[3] = apsInput[curpos + 1 + 1 * iImageWidth];

								yz[0] = apsInput[curpos - 1 * iImageWidth - 1 * mCount];
								yz[1] = apsInput[curpos + 1 * iImageWidth - 1 * mCount];
								yz[2] = apsInput[curpos - 1 * iImageWidth + 1 * mCount];
								yz[3] = apsInput[curpos + 1 * iImageWidth + 1 * mCount];

								zx[0] = apsInput[curpos - 1 * mCount - 1];
								zx[1] = apsInput[curpos + 1 * mCount - 1];
								zx[2] = apsInput[curpos - 1 * mCount + 1];
								zx[3] = apsInput[curpos + 1 * mCount + 1];

								xvalue = apsInput[curpos];
								xvalue = xvalue / 843;

								if (xvalue > 1)
								{xvalue = 1;}
								value_90 = value_90 + xvalue;

								for (ii = 0; ii < 4; ii++)
								{
									x[ii] = x[ii] / 843;
									if (x[ii] > 1)
									{x[ii] = 1;}
									else{}

									y[ii] = y[ii] / 843;
									if (y[ii] > 1)
									{y[ii] = 1;}
									else{}

									z[ii] = z[ii] / 843;
									if (z[ii] > 1)
									{z[ii] = 1;}
									else{}

									xy[ii] = xy[ii] / 843;
									if (xy[ii] > 1)
									{xy[ii] = 1;}
									else{ }

									yz[ii] = yz[ii] / 843;
									if (yz[ii] > 1)
									{yz[ii] = 1;}
									else{}

									zx[ii] = zx[ii] / 843;
									if (zx[ii] > 1)
									{zx[ii] = 1;}
									else{}
								}

								//fxx = 0.25*(x[0] - 4 * x[1] + 6 * xvalue - 4 * x[2] + x[3]);
								//fyy = 0.25*(y[0] - 4 * y[1] + 6 * xvalue - 4 * y[2] + y[3]);
								//fzz = 0.25*(z[0] - 4 * z[1] + 6 * xvalue - 4 * z[2] + z[3]);

								//fxy = 0.25*(xy[0] + xy[1] + xy[2] + xy[3] - 2 * x[1] - 2 * x[2] - 2 * y[1] - 2 * y[2] + 4 * xvalue);
								//fyz = 0.25*(yz[0] + yz[1] + yz[2] + yz[3] - 2 * z[1] - 2 * z[2] - 2 * y[1] - 2 * y[2] + 4 * xvalue);
								//fzx = 0.25*(zx[0] + zx[1] + zx[2] + zx[3] - 2 * x[1] - 2 * x[2] - 2 * z[1] - 2 * z[2] + 4 * xvalue);

								fxx = 0.25*(x[0] + x[3] - 2 * xvalue);
								fyy = 0.25*(y[0] + y[3] - 2 * xvalue);
								fzz = 0.25*(z[0] + z[3] - 2 * xvalue);

								fxy = 0.25*(xy[0] - xy[1] - xy[2] + xy[3]);
								fyz = 0.25*(yz[0] - yz[1] - yz[2] + yz[3]);
								fzx = 0.25*(zx[0] - zx[1] - zx[2] + zx[3]);

								sumxx += fxx;
								sumyy += fyy;
								sumzz += fzz;
								sumxy += fxy;
								sumyz += fyz;
								sumzx += fzx;
							
								SymmetricMatrix Cov_Matrix(3);
								Cov_Matrix.Row(1) << fxx;
								Cov_Matrix.Row(2) << fxy << fyy;
								Cov_Matrix.Row(3) << fzx << fyz << fzz;

								DiagonalMatrix DD;
								EigenValues(Cov_Matrix, DD);
								lambda1 = DD(1);
								lambda2 = DD(2);
								lambda3 = DD(3);
								swapthree(lambda3, lambda2, lambda1);

								if (abs_lambda(lambda3) < 0.0001 || lambda2>10000)
								{lambda3 = 0;}
								if (abs_lambda(lambda2) < 0.0001 || lambda2>10000)
								{lambda2 = 0;}

								if (lambda3 < 0 && lambda3 >= -0.5861)
								{
									lambda3 = -0.5861;
								}
								else
								{

								}

								suml2 += lambda2;
								suml3 += lambda3;

								response = (abs_lambda(lambda2)*abs_lambda(lambda2)*abs_lambda(lambda3 - lambda2) * 27) / (pow((2 * abs_lambda(lambda2) + abs_lambda(lambda3 - lambda2)), 3));
						
								if (lambda2 < lambda3 / 2)
								{
									response = 1;
								}
								else
								{

								}

								if (lambda2 >= 0 || lambda3 >= 0)
								{
									response = 0;
								}
								
								if (maxresponse > response)
								{

								}
								else
								{
									maxresponse = response;
								}

								/*response = double(filtered_one[curpos]);*/
								//aspOutput[curpos] = 1321*response;
								//value_90 += response;
								filtered = filtered_one[curpos];
                                sum_filtered += filtered;
								//response = double(filtered);
                                aspOutput[curpos] = filtered;
							}

			}
		}
	}
	//infile.close();
	//infile2.close();
	//aspOutput = filtered_one;
	meaninput = meaninput / mNumber;

	//cout << "maxresponse:" << maxresponse << endl;

	cout << "width:" << iImageWidth << endl;
	cout << "height:" << iImageHeight << endl;
	cout << "layer:" << iImageLayer << endl;

	//cout << "max3:" << max3 << endl;
	//cout << "min3:" << min3 << endl;
	//cout << "max2:" << max2 << endl;
	//cout << "min2:" << min2 << endl;
	//cout << "mean2:" << mean2 << endl;
	//cout << "mean3:" << mean3 << endl;
	  cout << "meaninput:" << meaninput << endl;
	  cout << "hxx" << sumxx << endl;
	  cout << "hyy" << sumyy << endl;
	  cout << "hzz" << sumzz << endl;
	  cout << "hxy" << sumxy << endl;
	  cout << "hyz" << sumyz << endl;
	  cout << "hzx" << sumzx << endl;
	  cout << "value_90:" << value_90 << endl;
	  cout << "sum_filtered:" << sum_filtered << endl;
	  cout << "sum_lambda2" << suml2 << endl;
	  cout << "sum_lambda3" << suml3 << endl;


}

template <class T> 
T abs_lambda(T num)
{
	return (num<0) ? -num : num;
}

template <class T> 
bool swapthree(T& dummya, T& dummyb, T& dummyc)
{

	if ((abs_lambda(dummya) >= abs_lambda(dummyb)) && (abs_lambda(dummyb) >= abs_lambda(dummyc))) //
	{

	}
	else if ((abs_lambda(dummya) >= abs_lambda(dummyc)) && (abs_lambda(dummyc) >= abs_lambda(dummyb))) //
	{
		T temp = dummyb;
		dummyb = dummyc;
		dummyc = temp;
	}
	else if ((abs_lambda(dummyb) >= abs_lambda(dummya)) && (abs_lambda(dummya) >= abs_lambda(dummyc))) //
	{
		T temp = dummya;
		dummya = dummyb;
		dummyb = temp;
	}
	else if ((abs_lambda(dummyb) >= abs_lambda(dummyc)) && (abs_lambda(dummyc) >= abs_lambda(dummya))) //
	{
		T temp = dummya;
		dummya = dummyb;
		dummyb = dummyc;
		dummyc = temp;
	}
	else if ((abs_lambda(dummyc) >= abs_lambda(dummya)) && (abs_lambda(dummya) >= abs_lambda(dummyb))) //
	{
		T temp = dummya;
		dummya = dummyc;
		dummyc = dummyb;
		dummyb = temp;
	}
	else if ((abs_lambda(dummyc) >= abs_lambda(dummyb)) && (abs_lambda(dummyb) >= abs_lambda(dummya))) //
	{
		T temp = dummyc;
		dummyc = dummya;
		dummya = temp;
	}
	else
	{
		return false;
	}

	return true;
}


template <class T> void gaussian_filter(T* data1d,
	V3DLONG *in_sz,
	unsigned int Wx,
	unsigned int Wy,
	unsigned int Wz,
	unsigned int c,
	double sigma,
	float* &outimg)
{
    if (!data1d || !in_sz || in_sz[0] <= 0 || in_sz[1] <= 0 || in_sz[2] <= 0 || in_sz[3] <= 0 || outimg)
    {
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

	if (outimg)
	{
		v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
		delete[]outimg;
		outimg = 0;
	}

	// for filter kernel
	double sigma_s2 = 0.5 / (sigma*sigma); // 1/(2*sigma*sigma)
	double pi_sigma = 1.0 / (sqrt(2 * 3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)

	float min_val = INF, max_val = 0;


	V3DLONG N = in_sz[0];
	V3DLONG M = in_sz[1];
	V3DLONG P = in_sz[2];
	V3DLONG sc = in_sz[3];
	V3DLONG pagesz = N*M*P;

	//filtering
	V3DLONG offset_init = (c - 1)*pagesz;

	//declare temporary pointer
	float *pImage = new float[pagesz];
	if (!pImage)
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		for (V3DLONG i = 0; i<pagesz; i++)
			pImage[i] = data1d[i + offset_init];  //first channel data (red in V3D, green in ImageJ)
	}
	//Filtering
	//
	//   Filtering along x
	if (N<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		float  *WeightsX = 0;
		WeightsX = new float[Wx];
		if (!WeightsX)
			return;

		float Half = (float)(Wx - 1) / 2.0;

		// Gaussian filter equation:
		// http://en.wikipedia.org/wiki/Gaussian_blur
		//   for (unsigned int Weight = 0; Weight < Half; ++Weight)
		//   {
		//        const float  x = Half* float (Weight) / float (Half);
		//         WeightsX[(int)Half - Weight] = WeightsX[(int)Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); // Corresponding symmetric WeightsX
		//    }

		for (unsigned int Weight = 0; Weight <= Half; ++Weight)
		{
			const float  x = float(Weight) - Half;
			WeightsX[Weight] = WeightsX[Wx - Weight - 1] = pi_sigma * exp(-(x * x *sigma_s2)); // Corresponding symmetric WeightsX
		}


		double k = 0.;
		for (unsigned int Weight = 0; Weight < Wx; ++Weight)
			k += WeightsX[Weight];

		for (unsigned int Weight = 0; Weight < Wx; ++Weight)
			WeightsX[Weight] /= k;

		printf("\n x dierction");

		for (unsigned int Weight = 0; Weight < Wx; ++Weight)
			printf("/n%f", WeightsX[Weight]);

		//   Allocate 1-D extension array
		float  *extension_bufferX = 0;
		extension_bufferX = new float[N + (Wx << 1)];

		unsigned int offset = Wx >> 1;

		//	along x
		const float  *extStop = extension_bufferX + N + offset;

		for (V3DLONG iz = 0; iz < P; iz++)
		{
			for (V3DLONG iy = 0; iy < M; iy++)
			{
				float  *extIter = extension_bufferX + Wx;
				for (V3DLONG ix = 0; ix < N; ix++)
				{
					*(extIter++) = pImage[iz*M*N + iy*N + ix];
				}

				//   Extend image
				const float  *const stop_line = extension_bufferX - 1;
				float  *extLeft = extension_bufferX + Wx - 1;
				const float  *arrLeft = extLeft + 2;
				float  *extRight = extLeft + N + 1;
				const float  *arrRight = extRight - 2;

				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);

				}

				//	Filtering
				extIter = extension_bufferX + offset;

				float  *resIter = &(pImage[iz*M*N + iy*N]);

				while (extIter < extStop)
				{
					double sum = 0.;
					const float  *weightIter = WeightsX;
					const float  *const End = WeightsX + Wx;
					const float * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * float(*(arrIter++));
					extIter++;
					*(resIter++) = sum;

					//for rescale
					if (max_val<*arrIter) max_val = *arrIter;
					if (min_val>*arrIter) min_val = *arrIter;


				}

			}
		}
		//de-alloc
		if (WeightsX) { delete[]WeightsX; WeightsX = 0; }
		if (extension_bufferX) { delete[]extension_bufferX; extension_bufferX = 0; }

	}

	//   Filtering along y
	if (M<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		float  *WeightsY = 0;
		WeightsY = new float[Wy];
		if (!WeightsY)
			return;

		float Half = (float)(Wy - 1) / 2.0;

		// Gaussian filter equation:
		// http://en.wikipedia.org/wiki/Gaussian_blur
		/* for (unsigned int Weight = 0; Weight < Half; ++Weight)
		{
		const float  y = Half* float (Weight) / float (Half);
		WeightsY[(int)Half - Weight] = WeightsY[(int)Half + Weight] = pi_sigma * exp(-y * y *sigma_s2); // Corresponding symmetric WeightsY
		}*/

		for (unsigned int Weight = 0; Weight <= Half; ++Weight)
		{
			const float  y = float(Weight) - Half;
			WeightsY[Weight] = WeightsY[Wy - Weight - 1] = pi_sigma * exp(-(y * y *sigma_s2)); // Corresponding symmetric WeightsY
		}


		double k = 0.;
		for (unsigned int Weight = 0; Weight < Wy; ++Weight)
			k += WeightsY[Weight];

		for (unsigned int Weight = 0; Weight < Wy; ++Weight)
			WeightsY[Weight] /= k;

		//	along y
		float  *extension_bufferY = 0;
		extension_bufferY = new float[M + (Wy << 1)];

		unsigned int offset = Wy >> 1;
		const float *extStop = extension_bufferY + M + offset;

		for (V3DLONG iz = 0; iz < P; iz++)
		{
			for (V3DLONG ix = 0; ix < N; ix++)
			{
				float  *extIter = extension_bufferY + Wy;
				for (V3DLONG iy = 0; iy < M; iy++)
				{
					*(extIter++) = pImage[iz*M*N + iy*N + ix];
				}

				//   Extend image
				const float  *const stop_line = extension_bufferY - 1;
				float  *extLeft = extension_bufferY + Wy - 1;
				const float  *arrLeft = extLeft + 2;
				float  *extRight = extLeft + M + 1;
				const float  *arrRight = extRight - 2;

				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}

				//	Filtering
				extIter = extension_bufferY + offset;

				float  *resIter = &(pImage[iz*M*N + ix]);

				while (extIter < extStop)
				{
					double sum = 0.;
					const float  *weightIter = WeightsY;
					const float  *const End = WeightsY + Wy;
					const float * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * float(*(arrIter++));
					extIter++;
					*resIter = sum;
					resIter += N;

					//for rescale
					if (max_val<*arrIter) max_val = *arrIter;
					if (min_val>*arrIter) min_val = *arrIter;


				}

			}
		}

		//de-alloc
		if (WeightsY) { delete[]WeightsY; WeightsY = 0; }
		if (extension_bufferY) { delete[]extension_bufferY; extension_bufferY = 0; }


	}

	//  Filtering  along z
	if (P<2)
	{
		//do nothing
	}
	else
	{
		//create Gaussian kernel
		float  *WeightsZ = 0;
		WeightsZ = new float[Wz];
		if (!WeightsZ)
			return;

		float Half = (float)(Wz - 1) / 2.0;

		/* for (unsigned int Weight = 1; Weight < Half; ++Weight)
		{
		const float  z = Half * float (Weight) / Half;
		WeightsZ[(int)Half - Weight] = WeightsZ[(int)Half + Weight] = pi_sigma * exp(-z * z * sigma_s2) ; // Corresponding symmetric WeightsZ
		}*/

		for (unsigned int Weight = 0; Weight <= Half; ++Weight)
		{
			const float  z = float(Weight) - Half;
			WeightsZ[Weight] = WeightsZ[Wz - Weight - 1] = pi_sigma * exp(-(z * z *sigma_s2)); // Corresponding symmetric WeightsZ
		}


		double k = 0.;
		for (unsigned int Weight = 0; Weight < Wz; ++Weight)
			k += WeightsZ[Weight];

		for (unsigned int Weight = 0; Weight < Wz; ++Weight)
			WeightsZ[Weight] /= k;

		//	along z
		float  *extension_bufferZ = 0;
		extension_bufferZ = new float[P + (Wz << 1)];

		unsigned int offset = Wz >> 1;
		const float *extStop = extension_bufferZ + P + offset;

		for (V3DLONG iy = 0; iy < M; iy++)
		{
			for (V3DLONG ix = 0; ix < N; ix++)
			{

				float  *extIter = extension_bufferZ + Wz;
				for (V3DLONG iz = 0; iz < P; iz++)
				{
					*(extIter++) = pImage[iz*M*N + iy*N + ix];
				}

				//   Extend image
				const float  *const stop_line = extension_bufferZ - 1;
				float  *extLeft = extension_bufferZ + Wz - 1;
				const float  *arrLeft = extLeft + 2;
				float  *extRight = extLeft + P + 1;
				const float  *arrRight = extRight - 2;

				while (extLeft > stop_line)
				{
					*(extLeft--) = *(arrLeft++);
					*(extRight++) = *(arrRight--);
				}

				//	Filtering
				extIter = extension_bufferZ + offset;

				float  *resIter = &(pImage[iy*N + ix]);

				while (extIter < extStop)
				{
					double sum = 0.;
					const float  *weightIter = WeightsZ;
					const float  *const End = WeightsZ + Wz;
					const float * arrIter = extIter;
					while (weightIter < End)
						sum += *(weightIter++) * float(*(arrIter++));
					extIter++;
					*resIter = sum;
					resIter += M*N;

					//for rescale
					if (max_val<*arrIter) max_val = *arrIter;
					if (min_val>*arrIter) min_val = *arrIter;

				}

			}
		}

		//de-alloc
		if (WeightsZ) { delete[]WeightsZ; WeightsZ = 0; }
		if (extension_bufferZ) { delete[]extension_bufferZ; extension_bufferZ = 0; }


	}

	outimg = pImage;


	return;
}


void thimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code);
bool thimg(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

//plugin funcs
const QString title = "adaptive threshold transform";
QStringList ThPlugin::menulist() const
{
     return QStringList()
          << tr("3D (w/o parameters)")
          << tr("3D (set parameters)")
          << tr("Help");
}

void ThPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("3D (w/o parameters)"))
	{
          thimg(callback, parent,1 );
     }
	else if (menu_name == tr("3D (set parameters)"))
	{
		thimg(callback, parent, 2 );
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Simple adaptive thresholding: for each voxel, compute a threshold which is the average intensity of its neighboring voxels and then subtract the threshold from the current voxel's intensity value. If the result is <0, then set it as 0. The neighborhood is defined along 6 axial directions in 3D, with N samples of each direction (N -- the 'number of sampling points' in the parameter setting dialog), and M voxels between every nearest pair of samples (M -- the 'sampling interval' in the parameter setting dialog).");
		return;
	}

}

QStringList ThPlugin::funclist() const
{
	return QStringList()
		<<tr("adath")
		<<tr("help");
}


bool ThPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("adath"))
	{
        return thimg(callback, input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x threshold -f adath -i <inimg_file> -o <outimg_file> -p <h> <d>"<<endl;
		cout<<endl;
		cout<<"h       sampling interval, default 5,"<<endl;
		cout<<"d       number of sampling points, default 3,"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x threshold -f adath -i input.raw -o output.raw -p 5 3"<<endl;
		cout<<endl;
		return true;
	}
}

bool thimg(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Gaussian filter"<<endl;
	if (input.size()<1 || output.size() != 1) return false;

	V3DLONG h = 5, d = 3;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) h = atoi(paras.at(0));
          if(paras.size() >= 2) d = atoi(paras.at(1));
	}

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
	cout<<"h = "<<h<<endl;
    cout<<"d = "<<d<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

    Image4DSimple *subject = callback.loadImage(inimg_file);
    if(!subject || !subject->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (subject) {delete subject; subject=0;}
         return false;
    }

     clock_t start_t = clock(); // record time point
     // =====================================================================>>>>>>>>>
     V3DLONG sz0 = subject->getXDim();
     V3DLONG sz1 = subject->getYDim();
     V3DLONG sz2 = subject->getZDim();
     V3DLONG sz3 = subject->getCDim();       //sz3为通道数
     V3DLONG pagesz_sub = sz0*sz1*sz2;

	//----------------------------------------------------------------------------------------------------------------------------------
	V3DLONG channelsz = sz0*sz1*sz2;
	void *pData=NULL;

	V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
	string path1 = "D:/v3d/vaa3d_tools/released_plugins/v3d_plugins/haoxiang/matlabcode/response.txt";
	string path2 = "D:/v3d/vaa3d_tools/released_plugins/v3d_plugins/haoxiang/lambda3.txt";
        switch (subject->getDatatype())
		{
            case V3D_UINT8:
				try
				{
					pData = (void *)(new unsigned char [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.",0);
					if (pData) {delete []pData; pData=0;}
					return false;
				}

				{
                    unsigned char * pSubtmp_uint8 = subject->getRawData();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_uint8 + ich*channelsz, (unsigned char *)pData + ich*channelsz, sz0, sz1, sz2, h, d, path1, path2 );
				}
				break;

            case V3D_UINT16:
				try
				{
					pData = (void *)(new short int [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.",0);
					if (pData) {delete []pData; pData=0;}
					return false;
				}

				{
                    short int * pSubtmp_uint16 = (short int *)subject->getRawData();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_uint16 + ich*channelsz, (short int *)pData + ich*channelsz, sz0, sz1, sz2, h, d, path1, path2 );
				}

				break;

            case V3D_FLOAT32:
				try
				{
					pData = (void *)(new float [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.", 0);
					if (pData) {delete []pData; pData=0;}
					return false;
				}

				{
                    float * pSubtmp_float32 = (float *)subject->getRawData();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_float32 + ich*channelsz, (float *)pData + ich*channelsz, sz0, sz1, sz2, h, d, path1, path2 );
				}

				break;

			default:
				break;
		}

	//----------------------------------------------------------------------------------------------------------------------------------

	clock_t end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
	//cout << "hahahahahahahahahahahahaahahhahahahahahahaahhahaahh" << endl << "hahahahahahahahahahahahahahahahaha" << endl;

     // =====================================================================<<<<<<<<<
    Image4DSimple outimg;
    outimg.setData((unsigned char *)pData, sz0, sz1, sz2, sz3, subject->getDatatype());

    callback.saveImage(&outimg, outimg_file);
    v3d_msg("Finish saving output file.",0);

     if(subject) {delete []subject; subject=0;}

     return true;
}





void thimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	V3DLONG h;
	V3DLONG d;
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}

	if (method_code == 1)   //获取处理模式，1为默认参数，2为自定参数
	{
		h = 5;
		d = 3;
	}
	else
	{
		if( method_code == 2)
		{
			AdaTDialog dialog(callback, parent);
			if (dialog.exec()!=QDialog::Accepted)
			return;
			else
			{
				h = dialog.Ddistance->text().toLong()-1;
				d = dialog.Dnumber->text().toLong()-1;
				printf("d% h,d% d \n ",h,d);
			}
		}
	}

	clock_t start_t = clock(); // record time point

	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);

	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	Image4DProxy<Image4DSimple> pSub(subject);

	V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
	V3DLONG sz3 = subject->getCDim();
	V3DLONG pagesz_sub = sz0*sz1*sz2;

	//----------------------------------------------------------------------------------------------------------------------------------
	V3DLONG channelsz = sz0*sz1*sz2;
	void *pData=NULL;


	string path1 = "D:/v3d/vaa3d_tools/released_plugins/v3d_plugins/haoxiang/matlabcode/response.txt";
	string path2 = "D:/v3d/vaa3d_tools/released_plugins/v3d_plugins/haoxiang/lambda3.txt";

	V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
		switch (subject->getDatatype())
		{
			case V3D_UINT8:
				try
				{
					pData = (void *)(new unsigned char [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.");
					if (pData) {delete []pData; pData=0;}
					return;
				}

				{
					unsigned char * pSubtmp_uint8 = pSub.begin();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_uint8+ich*channelsz, (unsigned char *)pData+ich*channelsz, sz0, sz1, sz2, h, d, path1, path2 );
				}
				break;

			case V3D_UINT16:
				try
				{
					pData = (void *)(new short int [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.");
					if (pData) {delete []pData; pData=0;}
					return;
				}

				{
					short int * pSubtmp_uint16 = (short int *)pSub.begin();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_uint16+ich*channelsz, (short int *)pData+ich*channelsz, sz0, sz1, sz2, h, d, path1, path2 );
				}

				break;

			case V3D_FLOAT32:
				try
				{
					pData = (void *)(new float [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.");
					if (pData) {delete []pData; pData=0;}
					return;
				}

				{
					float * pSubtmp_float32 = (float *)pSub.begin();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_float32+ich*channelsz, (float *)pData+ich*channelsz, sz0, sz1, sz2, h, d, path1, path2 );
				}

				break;

			default:
				break;
		}

	//----------------------------------------------------------------------------------------------------------------------------------

	clock_t end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);

	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)pData, sz0, sz1, sz2, sz3, subject->getDatatype());

	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();

	callback.setImage(newwin, &p4DImage);      //设置图像数据
	callback.setImageName(newwin, QString("thresholded image"));  //设置窗口名称
	callback.updateImageWindow(newwin);        //更新窗口显示
}

void AdaTDialog::update()
{
	//get current data
	Dn = Dnumber->text().toLong()-1;
	Dh = Ddistance->text().toLong()-1;
		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
}
