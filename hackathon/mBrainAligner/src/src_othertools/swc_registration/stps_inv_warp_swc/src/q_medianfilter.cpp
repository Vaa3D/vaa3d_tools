// q_medianfilter.cpp


#ifndef __Q_MEDIANFILTER_CPP__
#define __Q_MEDIANFILTER_CPP__

#include <math.h>
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"
#include "../../v3d_main/basic_c_fun/v3d_basicdatatype.h"

template <class T> void median_filter(T* data1d,
	long long  *in_sz,
	unsigned int Wx,
	unsigned int Wy,
	unsigned int Wz,
	unsigned int c,
	T* &outimg)
{

	long long  N = in_sz[0];
	long long  M = in_sz[1];
	long long  P = in_sz[2];
	long long  sc = in_sz[3];
	long long  pagesz = N*M*P;

	T *arr, tmp;
	int ii, jj;
	int size = (2 * Wx + 1)*(2 * Wy + 1)*(2 * Wz + 1);
	arr = new T[size];

	//filtering
	long long  offsetc = (c - 1)*pagesz;

	//declare temporary pointer
	T *pImage = new T[pagesz]();
	if (!pImage)
	{
		printf("Fail to allocate memory.\n");
		return;
	}

	//Median Filtering
	for (long long  iz = 0; iz < P; iz++)
	{
		printf("\r median filter : %d %% completed ", ((iz + 1) * 100) / P); fflush(stdout);
		long long  offsetk = iz*M*N;
		for (long long  iy = 0; iy < M; iy++)
		{
			long long  offsetj = iy*N;
			for (long long  ix = 0; ix < N; ix++)
			{

				long long  xb = ix - Wx; if (xb < 0) xb = 0;
				long long  xe = ix + Wx; if (xe >= N - 1) xe = N - 1;
				long long  yb = iy - Wy; if (yb < 0) yb = 0;
				long long  ye = iy + Wy; if (ye >= M - 1) ye = M - 1;
				long long  zb = iz - Wz; if (zb < 0) zb = 0;
				long long  ze = iz + Wz; if (ze >= P - 1) ze = P - 1;
				ii = 0;

				for (long long  k = zb; k <= ze; k++)
				{
					long long  offsetkl = k*M*N;
					for (long long  j = yb; j <= ye; j++)
					{
						long long  offsetjl = j*N;
						for (long long  i = xb; i <= xe; i++)
						{
							T dataval = data1d[offsetc + offsetkl + offsetjl + i];
							arr[ii] = dataval;
							if (ii > 0)
							{
								jj = ii;
								while (jj > 0 && arr[jj - 1] > arr[jj])
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
				long long  index_pim = offsetk + offsetj + ix;
				pImage[index_pim] = arr[int(0.5*ii) + 1];
			}
		}
	}
	printf("\n");

	outimg = pImage;
	delete[] arr;
}

#endif
