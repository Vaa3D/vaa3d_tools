



//itk headers
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "itkConnectedThresholdImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryThinningImageFilter3D.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <queue>
#include <ctime>

using namespace std;




//This method is from adaptive thresholdoing plugin. Add a binarization process for some images in BigNeuron gold standard set. 11/2015
void BinaryProcess(unsigned char *apsInput, unsigned char * aspOutput, long iImageWidth, long iImageHeight, long iImageLayer, long h, long d)
{
	long i, j, k, n, count;
	double t, temp;

	long mCount = iImageHeight * iImageWidth;
	for (i = 0; i<iImageLayer; i++)
	{
		for (j = 0; j<iImageHeight; j++)
		{
			for (k = 0; k<iImageWidth; k++)
			{
				long curpos = i * mCount + j*iImageWidth + k;
				long curpos1 = i* mCount + j*iImageWidth;
				long curpos2 = j* iImageWidth + k;
				temp = 0;
				count = 0;
				for (n = 1; n <= d; n++)
				{
					if (k>h*n) { temp += apsInput[curpos1 + k - (h*n)]; count++; }
					if (k + (h*n)< iImageWidth) { temp += apsInput[curpos1 + k + (h*n)]; count++; }
					if (j>h*n) { temp += apsInput[i* mCount + (j - (h*n))*iImageWidth + k]; count++; }//
					if (j + (h*n)<iImageHeight) { temp += apsInput[i* mCount + (j + (h*n))*iImageWidth + k]; count++; }//
					if (i>(h*n)) { temp += apsInput[(i - (h*n))* mCount + curpos2]; count++; }//
					if (i + (h*n)< iImageLayer) { temp += apsInput[(i + (h*n))* mCount + j* iImageWidth + k]; count++; }
				}
				t = apsInput[curpos] - temp / (count);
				aspOutput[curpos] = (t > 0) ? t : 0;
			}
		}
	}
}

//v3d preprocessing method for histogram equalization
bool hist_eq_uint8(unsigned char * data1d, long len)
{
	if (!data1d || len <= 0)
	{
		printf("The input parameters are invalid in hist_eq_uint8().\n");
		return false;
	}

	long NBIN = 256;
	long *h = new long[NBIN];
	double *c = new double[NBIN];
	if (!h)
	{
		printf("Fail to allocate memory in hist_eq_uint8().\n");
		return false;
	}

	long i;

	for (i = 0; i<NBIN; i++) h[i] = 0;

	for (i = 0; i<len; i++)
	{
		h[data1d[i]]++;
	}

	c[0] = h[0];
	for (i = 1; i<NBIN; i++) c[i] = c[i - 1] + h[i];
	for (i = 0; i<NBIN; i++) { c[i] /= c[NBIN - 1]; c[i] *= (NBIN - 1); }

	for (i = 0; i<len; i++)
	{
		data1d[i] = c[data1d[i]];
	}

	if (c) { delete[]c; c = 0; }
	if (h) { delete[]h; h = 0; }
	return true;
}

bool hist_eq_range_uint8(unsigned char * data1d, long len, unsigned char lowerbound, unsigned char upperbound) //only eq the [lowerbound,upperbound]
{
	if (!data1d || len <= 0)
	{
		printf("The input parameters are invalid in hist_eq_uint8().\n");
		return false;
	}

	if (lowerbound>upperbound)
	{
		unsigned char tmp = lowerbound; lowerbound = upperbound; upperbound = tmp;
	}

	long NBIN = 256;
	long *h = new long[NBIN];
	double *c = new double[NBIN];
	if (!h)
	{
		printf("Fail to allocate memory in hist_eq_uint8().\n");
		return false;
	}

	long i;

	for (i = 0; i<NBIN; i++) h[i] = 0;

	for (i = 0; i<len; i++)
	{
		h[data1d[i]]++;
	}

	c[lowerbound] = h[lowerbound];
	for (i = lowerbound + 1; i <= upperbound; i++) c[i] = c[i - 1] + h[i];
	double range = upperbound - lowerbound;
	for (i = lowerbound; i <= upperbound; i++) { c[i] /= c[upperbound]; c[i] *= range; c[i] += lowerbound; }

	for (i = 0; i<len; i++)
	{
		if (data1d[i] >= lowerbound && data1d[i] <= upperbound)
			data1d[i] = c[data1d[i]];
	}

	if (c) { delete[]c; c = 0; }
	if (h) { delete[]h; h = 0; }
	return true;
}

//in_sz = x, y, z, c
//char * & outimg     =    reference to the pointer that points to a character memory location
//c = channel number
//Wx = window x-dim radius
//Wy = window y-dim radius
//Wz = window z-dim radius
//(window size is 2*radius+1)
/*
Using median filter of radius 1 has quick process time, leaves some specks but doesn't cut 	away at thin structures.
Using median filter of radius 2 has decent process time, rids of almost all specks but cuts 	away at some thin structures.
Using median filter of radius 3 is just ridiculously slow and cuts away at a lot of the 	structures.
Repitition of 2 times seems to be the most rewarding. 3 times is not to costly to do, just to be safe (although it may be best to just do it a single time because the rewards of repitition are minimal
*/
void median_filter(unsigned char * data1d, long *in_sz, unsigned int Wx, unsigned int Wy, unsigned int Wz, unsigned int c, unsigned char * &outimg)
{

	long N = in_sz[0];
	long M = in_sz[1];
	long P = in_sz[2];
	long sc = in_sz[3];
	long pagesz = N*M*P;

	unsigned char * arr, tmp;
	int ii, jj;
	int size = (2 * Wx + 1)*(2 * Wy + 1)*(2 * Wz + 1);
	arr = new unsigned char[size];

	//filtering
	long offsetc = (c - 1)*pagesz;

	//declare temporary pointer
	unsigned char *pImage = new unsigned char[pagesz];
	if (!pImage)
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		for (long i = 0; i<pagesz; i++)
			pImage[i] = 0;
	}

	//Median Filtering
	for (long iz = 0; iz < P; iz++)
	{
		printf("\r median filter : %d %% completed ", ((iz + 1) * 100) / P); fflush(stdout);
		long offsetk = iz*M*N;
		for (long iy = 0; iy < M; iy++)
		{
			long offsetj = iy*N;
			for (long ix = 0; ix < N; ix++)
			{

				long xb = ix - Wx; if (xb<0) xb = 0;
				long xe = ix + Wx; if (xe >= N - 1) xe = N - 1;
				long yb = iy - Wy; if (yb<0) yb = 0;
				long ye = iy + Wy; if (ye >= M - 1) ye = M - 1;
				long zb = iz - Wz; if (zb<0) zb = 0;
				long ze = iz + Wz; if (ze >= P - 1) ze = P - 1;
				ii = 0;

				for (long k = zb; k <= ze; k++)
				{
					long offsetkl = k*M*N;
					for (long j = yb; j <= ye; j++)
					{
						long offsetjl = j*N;
						for (long i = xb; i <= xe; i++)
						{
							unsigned char dataval = data1d[offsetc + offsetkl + offsetjl + i];
							arr[ii] = dataval;
							if (ii>0)
							{
								jj = ii;
								while (jj > 0 && arr[jj - 1]>arr[jj])
								{
									tmp = arr[jj];
									arr[jj] = arr[jj - 1];
									arr[jj - 1] = tmp;
									jj--;
								}
							}
							ii++;
						}
					}
				}


				//set value
				long index_pim = offsetk + offsetj + ix;
				pImage[index_pim] = arr[int(0.5*ii) + 1];
			}
		}
	}
	printf("\n");

	outimg = pImage;
	delete[] arr;

}




/*data1d = p4DImage->getRawData();
	N = p4DImage->getXDim();
	M = p4DImage->getYDim();
	P = p4DImage->getZDim();
	sc = p4DImage->getCDim();*/
unsigned char * Preprocess(unsigned char* data1d, long N, long M, long P, long sc, int& threshold)
{
	long c;
	long in_sz[4];
	
	bool ok1;

	if (sc == 1)
	{
		c = 1;
		ok1 = true;
	}

	if (!ok1)
		return NULL;

	in_sz[0] = N;
	in_sz[1] = M;
	in_sz[2] = P;
	in_sz[3] = sc;

	
	if (threshold == 0 || threshold == 998)
	{
		 if (threshold == 998)
		 {
	      unsigned char * outimg = 0;
		  median_filter(data1d, in_sz, 1, 1, 1, sc, outimg);
		  delete[] data1d;
		  data1d = outimg;
	   } 


		//adaptive thresholding is not suitable for image with big soma since it will leave hole in the middle
		long h = 5, d = 3;
		unsigned char *pData = NULL;
		try
		{
			pData = (unsigned char *)(new unsigned char[N*M*P]);
		}
		catch (...)
		{
			//v3d_msg("Fail to allocate memory when applying adaptive thresholding for simple axis anlyzer.", 0);
			if (pData) { delete[]pData; pData = NULL; }
			return NULL;
		}
		BinaryProcess(data1d, pData, N, M, P, h, d);
		delete[] data1d;
		data1d = pData;
		//v3d_msg("Done thresholding", 0);

		//stat of thresholed forground pixels to see if making sense
		long num = 0;
		for (int iz = 0; iz<P; iz++) {
			int offsetk = iz*M*N;
			for (int iy = 0; iy < M; iy++) {
				int offsetj = iy*N;
				for (int ix = 0; ix < N; ix++) {
					int datavalue = data1d[offsetk + offsetj + ix];
					if (datavalue > 0) num++;
				}
			}
		}
		//v3d_msg(QString("Total percentage of foreground pixels after adaptive thresholding:%1").arg(((float)num) / (N*M*P)), 0);

		//enhance the histogram before apply binarization.
		//This avoids have broken holes in the connected structure.
		hist_eq_range_uint8(data1d, N*M*P, 10, 255);
		//v3d_msg(QString("Histogram equalization done"), 0);
	}

	//iterative histogram-based thresholding 
	if (threshold == 999 || threshold == 0 || threshold == 998)
	{
		if (threshold != 998 )
		{
	     unsigned char * outimg = 0;
		  median_filter(data1d, in_sz, 1, 1, 1, sc, outimg);
		  delete[] data1d;
		  data1d = outimg;
	   } 

		long num = 0;
		long sum = 0;
		for (int iz = 0; iz < P; iz++)
		{
			int offsetk = iz*M*N;
			for (int iy = 0; iy < M; iy++)
			{
				int offsetj = iy*N;
				for (int ix = 0; ix < N; ix++)
				{

					int PixelValue = data1d[offsetk + offsetj + ix];

						num++;
						sum += PixelValue;
						if (num < 10)
							cout << ix << ":" << iy << ":" << iz << " ";

				}
			}
		}
		int meanv = sum / num;
		long sum1, num1, sum2, num2;
		int iterationnum = 2;// find the valley in gray level historgram, keep small to avoid under segmentation
		for (int it = 0; it < iterationnum; it++)
		{
			sum1 = 0, num1 = 0;
			sum2 = 0, num2 = 0;
			for (long iz = 0; iz < P; iz++)
			{
				long offsetk = iz*M*N;
				for (long iy = 0; iy < M; iy++)
				{
					long offsetj = iy*N;
					for (long ix = 0; ix < N; ix++)
					{
						long idx = offsetk + offsetj + ix;
						if (data1d[idx] > meanv)
						{
							sum1 += data1d[idx]; num1++;
						}
						else
						{
							sum2 += data1d[idx]; num2++;
						}
					}
				}
			}
			//adjust new threshold
			//change
			if (num1 != 0 || num2 != 0){
				meanv = (int)(0.5*(sum1 / num1 + sum2 / num2));
			}
			else{
				cout << "Issue with thresholding. Set the threshold to 0" << endl;
				meanv = 0;
			}
		}
		//meanv = meanv - 10; //adjust low due to dark image to avoid undertrace
		std::cout << "\nmean adjusted:" << meanv << std::endl;

		//v3d_msg(QString("Total foreground points:%1").arg(sum1), 0);
		//v3d_msg(QString("Total percentage of foreground points after binarization:%1").arg(((float)num1) / (N*M*P)), 0);

		threshold = meanv;
	}//end of thresholding!!

	//testing
	return data1d;
}



