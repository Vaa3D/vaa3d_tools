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
#define MAX(a, b) ((a)>(b)?(a):(b))
#define filter1_90 550	
#define filter2_90 380
#define sigma1fang 1
#define sigma2fang 4
#define hc_value 0.9999
using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(threshold, ThPlugin);

template <class T> T abs_lambda(T num);

template <class T> bool swapthree(T& dummya, T& dummyb, T& dummyc);

template <class T> void gaussian_filter(T* data1d, V3DLONG *in_sz, unsigned int Wx, unsigned int Wy, unsigned int Wz, unsigned int c, double sigma, float* &outimg);

template <class T>
void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer,
	V3DLONG h, V3DLONG d)
{
	
	V3DLONG ii,i,j,jj,k,n;
	double sum_response,response,response2;
	float filtered,sum_filtered;
	sum_filtered = 0;

	//T * input_new = new T;
	////input_new = apsInput;
	//input_new = 0;
	//T * input_new = 0;
	//for (jj = 0; jj < iImageHeight * iImageWidth * iImageLayer; jj++)
	//{
	//	input_new[jj] = apsInput[jj]/843;
	//	if (input_new[jj]>1)
	//	{
	//		input_new[jj] = 1;
	//	}
	//}

	V3DLONG mCount = iImageHeight * iImageWidth;  //mCount为一层的像素点数
	V3DLONG mNumber = iImageHeight * iImageWidth * iImageLayer;
	V3DLONG size_image[4];

	//aspOutput = new T [mNumber];

    size_image[0] = iImageWidth;
    size_image[1] = iImageHeight;
    size_image[2] = iImageLayer;
    size_image[3] = 1;

	unsigned int wx1 = 6, wy1 = 6, wz1 = 6, wx2 = 12, wy2 = 12, wz2 = 12, c = 1;
	double sigma1 = 1, sigma2 = 2;
	float * filtered_one = 0;
	float * filtered_two = 0;
	T * output_buff = new T [mNumber];
	T * output_buff2 = new T [mNumber];

	//float *pImage = new float[pagesz]
	gaussian_filter(apsInput, size_image, wx1, wy1, wz1, c, sigma1, filtered_one);
	gaussian_filter(apsInput, size_image, wx2, wy2, wz2, c, sigma2, filtered_two);

	double fxx, fyy, fzz, fxy, fyz, fzx;
	double lambda1, lambda2, lambda3;
	double x[4],y[4],z[4],xy[4],yz[4],zx[4],pengzhang[8];
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
								output_buff[curpos] = 0;
							}
							else
							{   
								
								x[0] = filtered_one[curpos - 2];
								x[1] = filtered_one[curpos - 1];
								x[2] = filtered_one[curpos + 1];
								x[3] = filtered_one[curpos + 2];

								y[0] = filtered_one[curpos - 2 * iImageWidth];
								y[1] = filtered_one[curpos - 1 * iImageWidth];
								y[2] = filtered_one[curpos + 1 * iImageWidth];
								y[3] = filtered_one[curpos + 2 * iImageWidth];

								z[0] = filtered_one[curpos - 2 * mCount];
								z[1] = filtered_one[curpos - 1 * mCount];
								z[2] = filtered_one[curpos + 1 * mCount];
								z[3] = filtered_one[curpos + 2 * mCount];

								xy[0] = filtered_one[curpos - 1 - 1 * iImageWidth];
								xy[1] = filtered_one[curpos + 1 - 1 * iImageWidth];
								xy[2] = filtered_one[curpos - 1 + 1 * iImageWidth];
								xy[3] = filtered_one[curpos + 1 + 1 * iImageWidth];

								yz[0] = filtered_one[curpos - 1 * iImageWidth - 1 * mCount];
								yz[1] = filtered_one[curpos + 1 * iImageWidth - 1 * mCount];
								yz[2] = filtered_one[curpos - 1 * iImageWidth + 1 * mCount];
								yz[3] = filtered_one[curpos + 1 * iImageWidth + 1 * mCount];

								zx[0] = filtered_one[curpos - 1 * mCount - 1];
								zx[1] = filtered_one[curpos + 1 * mCount - 1];
								zx[2] = filtered_one[curpos - 1 * mCount + 1];
								zx[3] = filtered_one[curpos + 1 * mCount + 1];

								xvalue = filtered_one[curpos];
								xvalue = xvalue / filter1_90;

								if (xvalue > 1)
								{xvalue = 1;}
								value_90 = value_90 + xvalue;

								for (ii = 0; ii < 4; ii++)
								{
									x[ii] = x[ii] / filter1_90;
									if (x[ii] > 1)
									{x[ii] = 1;}
									else{}

									y[ii] = y[ii] / filter1_90;
									if (y[ii] > 1)
									{y[ii] = 1;}
									else{}

									z[ii] = z[ii] / filter1_90;
									if (z[ii] > 1)
									{z[ii] = 1;}
									else{}

									xy[ii] = xy[ii] / filter1_90;
									if (xy[ii] > 1)
									{xy[ii] = 1;}
									else{ }

									yz[ii] = yz[ii] / filter1_90;
									if (yz[ii] > 1)
									{yz[ii] = 1;}
									else{}

									zx[ii] = zx[ii] / filter1_90;
									if (zx[ii] > 1)
									{zx[ii] = 1;}
									else{}
								}

								fxx = sigma1fang*0.25*(x[0] + x[3] - 2 * xvalue);
								fyy = sigma1fang*0.25*(y[0] + y[3] - 2 * xvalue);
								fzz = sigma1fang*0.25*(z[0] + z[3] - 2 * xvalue);

								fxy = sigma1fang*0.25*(xy[0] - xy[1] - xy[2] + xy[3]);
								fyz = sigma1fang*0.25*(yz[0] - yz[1] - yz[2] + yz[3]);
								fzx = sigma1fang*0.25*(zx[0] - zx[1] - zx[2] + zx[3]);

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
								{lambda3 = -0.5861;}
								suml2 += lambda2;
								suml3 += lambda3;
								response = (abs_lambda(lambda2)*abs_lambda(lambda2)*abs_lambda(lambda3 - lambda2) * 27) / (pow((2 * abs_lambda(lambda2) + abs_lambda(lambda3 - lambda2)), 3));
						
								if (lambda2 < lambda3 / 2)
								{response = 1;}
								
								if (lambda2 >= 0 || lambda3 >= 0)
								{response = 0;}
								
								if (maxresponse <= response)
								{maxresponse = response;}

								/*response = double(filtered_one[curpos]);*/
								//aspOutput[curpos] = 1321*response;
								//value_90 += response;
								//filtered = filtered_one[curpos];
                                //sum_filtered += filtered;
								//response = double(filtered);
								//aspOutput[curpos] = 1321 * response;
								

								x[0] = filtered_two[curpos - 2];
								x[1] = filtered_two[curpos - 1];
								x[2] = filtered_two[curpos + 1];
								x[3] = filtered_two[curpos + 2];

								y[0] = filtered_two[curpos - 2 * iImageWidth];
								y[1] = filtered_two[curpos - 1 * iImageWidth];
								y[2] = filtered_two[curpos + 1 * iImageWidth];
								y[3] = filtered_two[curpos + 2 * iImageWidth];

								z[0] = filtered_two[curpos - 2 * mCount];
								z[1] = filtered_two[curpos - 1 * mCount];
								z[2] = filtered_two[curpos + 1 * mCount];
								z[3] = filtered_two[curpos + 2 * mCount];

								xy[0] = filtered_two[curpos - 1 - 1 * iImageWidth];
								xy[1] = filtered_two[curpos + 1 - 1 * iImageWidth];
								xy[2] = filtered_two[curpos - 1 + 1 * iImageWidth];
								xy[3] = filtered_two[curpos + 1 + 1 * iImageWidth];

								yz[0] = filtered_two[curpos - 1 * iImageWidth - 1 * mCount];
								yz[1] = filtered_two[curpos + 1 * iImageWidth - 1 * mCount];
								yz[2] = filtered_two[curpos - 1 * iImageWidth + 1 * mCount];
								yz[3] = filtered_two[curpos + 1 * iImageWidth + 1 * mCount];

								zx[0] = filtered_two[curpos - 1 * mCount - 1];
								zx[1] = filtered_two[curpos + 1 * mCount - 1];
								zx[2] = filtered_two[curpos - 1 * mCount + 1];
								zx[3] = filtered_two[curpos + 1 * mCount + 1];

								xvalue = filtered_two[curpos];
								xvalue = xvalue / filter2_90;

								if (xvalue > 1)
								{
									xvalue = 1;
								}
								value_90 = value_90 + xvalue;

								for (ii = 0; ii < 4; ii++)
								{
									x[ii] = x[ii] / filter2_90;
									if (x[ii] > 1)
									{
										x[ii] = 1;
									}
									else{}

									y[ii] = y[ii] / filter2_90;
									if (y[ii] > 1)
									{
										y[ii] = 1;
									}
									else{}

									z[ii] = z[ii] / filter2_90;
									if (z[ii] > 1)
									{
										z[ii] = 1;
									}
									else{}

									xy[ii] = xy[ii] / filter2_90;
									if (xy[ii] > 1)
									{
										xy[ii] = 1;
									}
									else{}

									yz[ii] = yz[ii] / filter2_90;
									if (yz[ii] > 1)
									{
										yz[ii] = 1;
									}
									else{}

									zx[ii] = zx[ii] / filter2_90;
									if (zx[ii] > 1)
									{
										zx[ii] = 1;
									}
									else{}
								}

								fxx = sigma2fang*(x[0] + x[3] - 2 * xvalue);
								fyy = sigma2fang*(y[0] + y[3] - 2 * xvalue);
								fzz = sigma2fang*(z[0] + z[3] - 2 * xvalue);

								fxy = sigma2fang*(xy[0] - xy[1] - xy[2] + xy[3]);
								fyz = sigma2fang*(yz[0] - yz[1] - yz[2] + yz[3]);
								fzx = sigma2fang*(zx[0] - zx[1] - zx[2] + zx[3]);

								sumxx += fxx;
								sumyy += fyy;
								sumzz += fzz;
								sumxy += fxy;
								sumyz += fyz;
								sumzx += fzx;

								SymmetricMatrix Cov_Matrix2(3);
								Cov_Matrix2.Row(1) << fxx;
								Cov_Matrix2.Row(2) << fxy << fyy;
								Cov_Matrix2.Row(3) << fzx << fyz << fzz;

								DiagonalMatrix DD2;
								EigenValues(Cov_Matrix2, DD2);
								lambda1 = DD2(1);
								lambda2 = DD2(2);
								lambda3 = DD2(3);
								swapthree(lambda3, lambda2, lambda1);

								if (abs_lambda(lambda3) < 0.0001 || lambda2>10000)
								{
									lambda3 = 0;
								}
								if (abs_lambda(lambda2) < 0.0001 || lambda2>10000)
								{
									lambda2 = 0;
								}

								if (lambda3 < 0 && lambda3 >= -0.5861)
								{
									lambda3 = -0.5861;
								}
								suml2 += lambda2;
								suml3 += lambda3;
								
								response2 = (abs_lambda(lambda2)*abs_lambda(lambda2)*abs_lambda(lambda3 - lambda2) * 27) / (pow((2 * abs_lambda(lambda2) + abs_lambda(lambda3 - lambda2)), 3));
								
								if (lambda2 < lambda3 / 2)
								{
									response2 = 1;
								}
							
									if (lambda2 >= 0 || lambda3 >= 0)
									{
										response2 = 0;
									}
									
									/*if (k <= 30 || k >= iImageWidth - 30 || j <= 30)
									{
										aspOutput[curpos] = MAX(response, response2)*0.25*apsInput[curpos];
									}
									else if (apsInput[curpos]>400)
									{			
											aspOutput[curpos] = apsInput[curpos];
									}
									else
									{
										aspOutput[curpos] = MAX(response, response2)*apsInput[curpos];
									}*/
									output_buff[curpos] = 1321 * MAX(response, response2);
									if (k <= 40)
									{
										output_buff[curpos] = 1321 * MAX(response, response2)*log2(k/40+1);
										//output_buff[curpos] = 0;
									}
									if (k >= iImageWidth - 41)
									{
										output_buff[curpos] = 1321 * MAX(response, response2)*log2((iImageWidth - 1 - k) / 40 + 1);
										//output_buff[curpos] = 0;
									}

									if (j <= 70)
									{
										if (j <= 40)
										{
											output_buff[curpos] = 1321 * MAX(response, response2)*log2(j / 70 + 1);
										}
										else
										{
											if (i <= (0.0222*j*j - 4.443*j + 202.2))
											{
												output_buff[curpos] = 1321 * MAX(response, response2)*log2(j / 70 + 1);
											}
											else
											{
												output_buff[curpos] = 1321 * MAX(response, response2);
											}
										}
										
									}
									
							
							}
			}
		}
	}


	for (jj = 0; jj < mNumber; jj++)                 //分割结果二值化
	{
		if (output_buff[jj] > 1321 * 0.99)
		{
			output_buff[jj] = 1;
		}
		else
		{
			output_buff[jj] = 0;
		}
	}

	//闭操作，先膨胀再腐蚀
	//第一次膨胀
	//for (i = 0; i < iImageLayer; i++)              //i为层数
	//{
	//	for (j = 0; j < iImageHeight; j++)         //j为行数
	//	{
	//		for (k = 0; k < iImageWidth; k++)		 //k为列数
	//		{
	//			V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
	//			output_buff2[curpos] = 0;
	//			if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
	//			{
	//				output_buff2[curpos] = 0;
	//			}
	//			else
	//			{
	//				pengzhang[0] = output_buff[curpos - 1 - iImageWidth];
	//				pengzhang[1] = output_buff[curpos - iImageWidth];
	//				pengzhang[2] = output_buff[curpos + 1 - iImageWidth];
	//				pengzhang[3] = output_buff[curpos - 1];
	//				pengzhang[4] = output_buff[curpos + 1];
	//				pengzhang[5] = output_buff[curpos - 1 + iImageWidth];
	//				pengzhang[6] = output_buff[curpos + iImageWidth];
	//				pengzhang[7] = output_buff[curpos + 1 + iImageWidth];
	//				for (jj = 0; jj < 8; jj++)
	//				{
	//					if (pengzhang[jj] == 1)
	//					{
	//						output_buff2[curpos] = 1;
	//					}
	//					if (output_buff[jj] == 1)
	//					{
	//						output_buff2[curpos] = 1;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	////第二次膨胀
	//for (i = 0; i < iImageLayer; i++)              //i为层数
	//{
	//	for (j = 0; j < iImageHeight; j++)         //j为行数
	//	{
	//		for (k = 0; k < iImageWidth; k++)		 //k为列数
	//		{
	//			V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
	//			output_buff[curpos] = 0;
	//			if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
	//			{
	//				output_buff[curpos] = 0;
	//			}
	//			else
	//			{
	//				pengzhang[0] = output_buff2[curpos - 1 - iImageWidth];
	//				pengzhang[1] = output_buff2[curpos - iImageWidth];
	//				pengzhang[2] = output_buff2[curpos + 1 - iImageWidth];
	//				pengzhang[3] = output_buff2[curpos - 1];
	//				pengzhang[4] = output_buff2[curpos + 1];
	//				pengzhang[5] = output_buff2[curpos - 1 + iImageWidth];
	//				pengzhang[6] = output_buff2[curpos + iImageWidth];
	//				pengzhang[7] = output_buff2[curpos + 1 + iImageWidth];
	//			}
	//		}
	//	}
	//}

	////第一次腐蚀
	//for (i = 0; i < iImageLayer; i++)              //i为层数
	//{
	//	for (j = 0; j < iImageHeight; j++)         //j为行数
	//	{
	//		for (k = 0; k < iImageWidth; k++)		 //k为列数
	//		{
	//			V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
	//			output_buff2[curpos] = 1;
	//			if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
	//			{
	//				output_buff2[curpos] = 0;
	//			}
	//			else
	//			{
	//				pengzhang[0] = output_buff[curpos - 1 - iImageWidth];
	//				pengzhang[1] = output_buff[curpos - iImageWidth];
	//				pengzhang[2] = output_buff[curpos + 1 - iImageWidth];
	//				pengzhang[3] = output_buff[curpos - 1];
	//				pengzhang[4] = output_buff[curpos + 1];
	//				pengzhang[5] = output_buff[curpos - 1 + iImageWidth];
	//				pengzhang[6] = output_buff[curpos + iImageWidth];
	//				pengzhang[7] = output_buff[curpos + 1 + iImageWidth];
	//				for (jj = 0; jj < 8; jj++)
	//				{
	//					if (pengzhang[jj] == 0)
	//					{
	//						output_buff2[curpos] = 0;
	//					}
	//				
	//				}
	//			}
	//		}
	//	}
	//}
	//

	////第二次腐蚀
	//for (i = 0; i < iImageLayer; i++)              //i为层数
	//{
	//	for (j = 0; j < iImageHeight; j++)         //j为行数
	//	{
	//		for (k = 0; k < iImageWidth; k++)		 //k为列数
	//		{
	//			V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
	//			output_buff[curpos] = 1;
	//			if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
	//			{
	//				output_buff[curpos] = 0;
	//			}
	//			else
	//			{
	//				pengzhang[0] = output_buff2[curpos - 1 - iImageWidth];
	//				pengzhang[1] = output_buff2[curpos - iImageWidth];
	//				pengzhang[2] = output_buff2[curpos + 1 - iImageWidth];
	//				pengzhang[3] = output_buff2[curpos - 1];
	//				pengzhang[4] = output_buff2[curpos + 1];
	//				pengzhang[5] = output_buff2[curpos - 1 + iImageWidth];
	//				pengzhang[6] = output_buff2[curpos + iImageWidth];
	//				pengzhang[7] = output_buff2[curpos + 1 + iImageWidth];
	//				for (jj = 0; jj < 8; jj++)
	//				{
	//					if (pengzhang[jj] == 0)
	//					{
	//						output_buff[curpos] = 0;
	//					}
	//					
	//				}
	//			}
	//		}
	//	}
	//}

//闭操作，先膨胀后腐蚀
pengzhang1(output_buff, output_buff2, iImageLayer, iImageWidth, iImageHeight);
pengzhang1(output_buff2, output_buff, iImageLayer, iImageWidth, iImageHeight);
pengzhang1(output_buff, output_buff2, iImageLayer, iImageWidth, iImageHeight);
fushi1(output_buff2, output_buff, iImageLayer, iImageWidth, iImageHeight);
fushi1(output_buff, output_buff2, iImageLayer, iImageWidth, iImageHeight);
fushi1(output_buff2, output_buff, iImageLayer, iImageWidth, iImageHeight);

//开操作，先腐蚀后膨胀
fushi2(output_buff, output_buff2, iImageLayer, iImageWidth, iImageHeight);
pengzhang1(output_buff2, output_buff, iImageLayer, iImageWidth, iImageHeight);

	for (jj = 0; jj < mNumber; jj++)
	{
		aspOutput[jj] = output_buff[jj];
	}

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
void pengzhang1(T * input, T * output, V3DLONG iImageLayer, V3DLONG iImageWidth, V3DLONG iImageHeight)
{
	V3DLONG mCount = iImageHeight*iImageWidth;
	V3DLONG i, j, k, jj;
	double pengzhang[8];

	for (i = 0; i < iImageLayer; i++)              //i为层数
	{
		for (j = 0; j < iImageHeight; j++)         //j为行数
		{
			for (k = 0; k < iImageWidth; k++)		 //k为列数
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
				output[curpos] = 0;

				if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
				{
					output[curpos] = 0;
				}
				else
				{
					pengzhang[0] = input[curpos - 1 - iImageWidth];
					pengzhang[1] = input[curpos - iImageWidth];
					pengzhang[2] = input[curpos + 1 - iImageWidth];
					pengzhang[3] = input[curpos - 1];
					pengzhang[4] = input[curpos + 1];
					pengzhang[5] = input[curpos - 1 + iImageWidth];
					pengzhang[6] = input[curpos + iImageWidth];
					pengzhang[7] = input[curpos + 1 + iImageWidth];

					for (jj = 0; jj < 8; jj++)
					{
						if (pengzhang[jj] == 1)
						{
							output[curpos] = 1;
						}
						if (input[jj] == 1)
						{
							output[curpos] = 1;
						}
					}

				}
			}
		}
	}
}

template <class T>
void fushi1(T * input, T * output, V3DLONG iImageLayer, V3DLONG iImageWidth, V3DLONG iImageHeight)
{
	V3DLONG mCount = iImageHeight*iImageWidth;
	V3DLONG i, j, k, jj;
	double pengzhang[8];

	for (i = 0; i < iImageLayer; i++)              //i为层数
	{
		for (j = 0; j < iImageHeight; j++)         //j为行数
		{
			for (k = 0; k < iImageWidth; k++)		 //k为列数
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
				output[curpos] = 1;

				if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
				{
					output[curpos] = 0;
				}
				else
				{
					pengzhang[0] = input[curpos - 1 - iImageWidth];
					pengzhang[1] = input[curpos - iImageWidth];
					pengzhang[2] = input[curpos + 1 - iImageWidth];
					pengzhang[3] = input[curpos - 1];
					pengzhang[4] = input[curpos + 1];
					pengzhang[5] = input[curpos - 1 + iImageWidth];
					pengzhang[6] = input[curpos + iImageWidth];
					pengzhang[7] = input[curpos + 1 + iImageWidth];

					for (jj = 0; jj < 8; jj++)
					{
						if (pengzhang[jj] == 0)
						{
							output[curpos] = 0;
						}
					}

				}
			}
		}
	}
}

template <class T>
void pengzhang2(T * input, T * output, V3DLONG iImageLayer, V3DLONG iImageWidth, V3DLONG iImageHeight)
{
	V3DLONG mCount = iImageHeight*iImageWidth;
	V3DLONG i, j, k, jj;
	double pengzhang[4];

	for (i = 0; i < iImageLayer; i++)              //i为层数
	{
		for (j = 0; j < iImageHeight; j++)         //j为行数
		{
			for (k = 0; k < iImageWidth; k++)		 //k为列数
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
				output[curpos] = 0;

				if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
				{
					output[curpos] = 0;
				}
				else
				{
					pengzhang[0] = input[curpos - iImageWidth];
					pengzhang[1] = input[curpos - 1];
					pengzhang[2] = input[curpos + 1];
					pengzhang[3] = input[curpos + iImageWidth];

					for (jj = 0; jj < 4; jj++)
					{
						if (pengzhang[jj] == 1)
						{
							output[curpos] = 1;
						}
						if (input[jj] == 1)
						{
							output[curpos] = 1;
						}
					}

				}
			}
		}
	}
}

template <class T>
void fushi2(T * input, T * output, V3DLONG iImageLayer, V3DLONG iImageWidth, V3DLONG iImageHeight)
{
	V3DLONG mCount = iImageHeight*iImageWidth;
	V3DLONG i, j, k, jj;
	double pengzhang[4];

	for (i = 0; i < iImageLayer; i++)              //i为层数
	{
		for (j = 0; j < iImageHeight; j++)         //j为行数
		{
			for (k = 0; k < iImageWidth; k++)		 //k为列数
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
				output[curpos] = 1;

				if (i<1 || j<1 || k<1 || i>iImageLayer - 2 || j>iImageHeight - 2 || k>iImageWidth - 2)
				{
					output[curpos] = 0;
				}
				else
				{
					pengzhang[0] = input[curpos - iImageWidth];
					pengzhang[1] = input[curpos - 1];
					pengzhang[2] = input[curpos + 1];
					pengzhang[3] = input[curpos - iImageWidth];

					for (jj = 0; jj < 4; jj++)
					{
						if (pengzhang[jj] == 0)
						{
							output[curpos] = 0;
						}
					}

				}
			}
		}
	}
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

template <class T>
void ModifyResult(T * input, T * output, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer,
	V3DLONG h, V3DLONG d, V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	LandmarkList landmark_list = callback.getLandmark(curwin);
	//&V3DPluginCallback::getLandmark;
	
	V3DLONG i, j, k, m, length, index, ii, jj, kk;
	length = landmark_list.size();
	v3d_msg("landmark");
	double * xx = new double[length];
	double * yy = new double[length];
	double * zz = new double[length];
	double x1, y1, z1;

	for (index = 0; index < length; index++)
	{
		xx[index] = landmark_list[index].x;
		yy[index] = landmark_list[index].y;
		zz[index] = landmark_list[index].z;
	}
	cout << "x坐标" << xx[0] << endl;
	cout << "landmarklist长度" << length << endl;

	V3DLONG mCount = iImageHeight * iImageWidth;  //mCount为一层的像素点数
	V3DLONG mNumber = iImageHeight * iImageWidth * iImageLayer;


	for (i = 0; i < iImageLayer; i++)              //i为层数
	{
		for (j = 0; j < iImageHeight; j++)         //j为行数
		{
			for (k = 0; k < iImageWidth; k++)		 //k为列数
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
				output[curpos] = input[curpos];
			}
		}
	}


	for (m = 0; m < length; m++)
	{

					//V3DLONG curpos = i * mCount + j*iImageWidth + k;   //curpos代表像素序号
					//V3DLONG curpos1 = i* mCount + j*iImageWidth;       //curpos1代表该层该行第一个像素序号
					//V3DLONG curpos2 = j* iImageWidth + k;              //curpos2代表像素在该层内的序号
					x1 = ceil(xx[m]);
					y1 = ceil(yy[m]);
					z1 = ceil(zz[m]);
					V3DLONG curpos = z1 * mCount + y1*iImageWidth + x1;   //curpos代表像素序号

					if (x1 > 3 && x1 < iImageWidth - 4 && y1 > 3 && y1 < iImageHeight - 4 && z1 > 3 && z1 < iImageLayer - 4)
					{
						for (ii = z1 - 4; ii <= z1 + 4; ii++)
						{
							for (jj = y1 - 4; jj <= y1 + 4; jj++)
							{
								for (kk = x1 - 4; kk <= x1 + 4; kk++)
								{
									curpos = ii * mCount + jj*iImageWidth + kk;
									output[curpos] = 0;
								}
							}
						}
					}
				


				
	}


}

void thimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code);
void modify(V3DPluginCallback2 &callback, QWidget *parent, int method_code);
bool thimg(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

//plugin funcs
const QString title = "adaptive threshold transform";
QStringList ThPlugin::menulist() const
{
     return QStringList()
          << tr("Vessel Enhancement")
		  << tr("Modify Result")
          << tr("3D (set parameters)")
          << tr("Help");
}

void ThPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Vessel Enhancement"))
	{
          thimg(callback, parent,1 );
     }

	else if (menu_name == tr("Modify Result"))
	{
		modify(callback, parent, 1);
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
						BinaryProcess(pSubtmp_uint8 + ich*channelsz, (unsigned char *)pData + ich*channelsz, sz0, sz1, sz2, h, d);
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
						BinaryProcess(pSubtmp_uint16 + ich*channelsz, (short int *)pData + ich*channelsz, sz0, sz1, sz2, h, d);
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
						BinaryProcess(pSubtmp_float32 + ich*channelsz, (float *)pData + ich*channelsz, sz0, sz1, sz2, h, d);
				}

				break;

			default:
				break;
		}

	//----------------------------------------------------------------------------------------------------------------------------------

	clock_t end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
	
     // =====================================================================<<<<<<<<<
    Image4DSimple outimg;
    outimg.setData((unsigned char *)pData, sz0, sz1, sz2, sz3, subject->getDatatype());

    callback.saveImage(&outimg, outimg_file);
    v3d_msg("Finish saving output file.",0);

     if(subject) {delete []subject; subject=0;}

     return true;
}

//Vessel Enhanement所用的函数
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
						BinaryProcess(pSubtmp_uint8+ich*channelsz, (unsigned char *)pData+ich*channelsz, sz0, sz1, sz2, h, d);
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
						BinaryProcess(pSubtmp_uint16+ich*channelsz, (short int *)pData+ich*channelsz, sz0, sz1, sz2, h, d);
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
						BinaryProcess(pSubtmp_float32+ich*channelsz, (float *)pData+ich*channelsz, sz0, sz1, sz2, h, d);
				}

				break;

			default:
				break;
		}

	//----------------------------------------------------------------------------------------------------------------------------------

	clock_t end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
	cout << "hahahahahahahahahahahahaahahhahahahahahahaahhahaahh" << endl << "hahahahahahahahahahahahahahahahaha" << endl;

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

//Modify Result所用的函数
void modify(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
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
		if (method_code == 2)
		{
			AdaTDialog dialog(callback, parent);
			if (dialog.exec() != QDialog::Accepted)
				return;
			else
			{
				h = dialog.Ddistance->text().toLong() - 1;
				d = dialog.Dnumber->text().toLong() - 1;
				printf("d% h,d% d \n ", h, d);
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
	void *pData = NULL;


	V3DLONG sz_data[4]; sz_data[0] = sz0; sz_data[1] = sz1; sz_data[2] = sz2; sz_data[3] = 1;
	switch (subject->getDatatype())
	{
	case V3D_UINT8:
		try
		{
			pData = (void *)(new unsigned char[sz3*channelsz]);
		}
		catch (...)
		{
			v3d_msg("Fail to allocate memory in Distance Transform.");
			if (pData) { delete[]pData; pData = 0; }
			return;
		}

				{
					unsigned char * pSubtmp_uint8 = pSub.begin();

					for (V3DLONG ich = 0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_uint8 + ich*channelsz, (unsigned char *)pData + ich*channelsz, sz0, sz1, sz2, h, d);
				}
				break;


	case V3D_UINT16:
		try
		{
			pData = (void *)(new short int[sz3*channelsz]);
		}
		catch (...)
		{
			v3d_msg("Fail to allocate memory in Distance Transform.");
			if (pData) { delete[]pData; pData = 0; }
			return;
		}

				{
					short int * pSubtmp_uint16 = (short int *)pSub.begin();

					for (V3DLONG ich = 0; ich < sz3; ich++)
						//v3d_msg("saasiongokgnsdoignsdognsdo");
						ModifyResult(pSubtmp_uint16 + ich*channelsz, (short int *)pData + ich*channelsz,
						sz0, sz1, sz2, h, d, callback, parent);
				}

				break;

	case V3D_FLOAT32:
		try
		{
			pData = (void *)(new float[sz3*channelsz]);
		}
		catch (...)
		{
			v3d_msg("Fail to allocate memory in Distance Transform.");
			if (pData) { delete[]pData; pData = 0; }
			return;
		}

				{
					float * pSubtmp_float32 = (float *)pSub.begin();

					for (V3DLONG ich = 0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_float32 + ich*channelsz, (float *)pData + ich*channelsz, sz0, sz1, sz2, h, d);
				}

				break;

	default:
		break;
	}

	//----------------------------------------------------------------------------------------------------------------------------------

	clock_t end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t - start_t) / 1000000);
	cout << "hahahahahahahahahahahahaahahhahahahahahahaahhahaahh" << endl << "hahahahahahahahahahahahahahahahaha" << endl;

	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)pData, sz0, sz1, sz2, sz3, subject->getDatatype());

	v3dhandle newwin;
	if (QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
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
