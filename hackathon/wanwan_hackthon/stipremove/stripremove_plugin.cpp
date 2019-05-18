/* stripremove_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-13 : by wanwan
 */
 

#include "v3d_message.h"
#include <vector>
#include "stripremove_plugin.h"
#include <QtGui>
#include <v3d_interface.h>
#include <stdlib.h>
#include <iostream>
#include <stackutil.h>
#include <stdio.h>


#include "stdint.h"
#include<vector>
#include<opencv.hpp>
using namespace cv;

#include<iostream> 
#include "highgui.h"

using namespace cv;

using namespace std;







//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(stripremove, StripremovePlugin);


void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
//定义滤波器函数
template <class T> void stripremove(T* data1d,
	V3DLONG *in_sz,
	unsigned int angle,
	unsigned int cutoff,
	unsigned int radius,
	unsigned int c,
	double sigma,
	float* &outimg);


const QString title = QObject::tr("StripRemove option Plugin");
 
QStringList StripremovePlugin::menulist() const
{
	return QStringList() 
		<<tr("Stripremove option")
		<< tr("help")
		<<tr("about");
	
}

QStringList StripremovePlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}
//用VS编译，将主要内容写在domenu里
void StripremovePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Stripremove option"))
	{
		processImage(callback, parent);
	}
	else if (menu_name == tr("help"))
	{
		
		cout << "angle          filter window size (pixel #) in x direction, default 7 and maximum image xsize-1" << endl;
		cout << "cutoff          filter window size (pixel #) in y direction, default 7 and maximum image ysize-1" << endl;
		cout << "radius          filter window size (pixel #) in z direction, default 3 and maximum image zsize-1" << endl;
		cout << "channel     the input channel value, default 1 and start from 1" << endl;
		cout << "sigma       filter sigma, default 1.0" << endl;
		
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg(tr("This is a Stripremove plugin, you can use it as a demo.. "
			"Developed by wanwan, 2019-5-13"));
	}

}
//用QT命令行编译，将主要内容写在dofunc里
bool StripremovePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

//处理图像的程序
void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
		QMessageBox::information(0, "", "You don't have any image open in the main window.");
		return;
	}

	Image4DSimple* p4DImage = callback.getImage(curwin);

	if (!p4DImage)
	{
		QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}

	unsigned char* data1d = p4DImage->getRawData();
	ImagePixelType pixeltype = p4DImage->getDatatype();//获取数据类型
	//V3DLONG totalpxls = p4DImage->getTotalBytes();
	//V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

	V3DLONG N = p4DImage->getXDim();
	V3DLONG M = p4DImage->getYDim();
	V3DLONG P = p4DImage->getZDim();
	V3DLONG sc = p4DImage->getCDim();
	V3DLONG in_sz[4];
	in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;
	V3DLONG pagesz = N*M*P;//图像尺寸
	V3DLONG slicesz = N*M*p4DImage->getUnitBytes();//每张切片尺寸
	/*----------------------------------------------------------------------1.add input dialog---------------------------------------------------*/

	StripremoveDialog dialog(callback, parent);
	if (!dialog.image)
		return;

	if (dialog.exec() != QDialog::Accepted)
		return;

	dialog.update();

	Image4DSimple* subject = dialog.image;
	if (!subject)
		return;
	ROIList pRoiList = dialog.pRoiList;

	int Wx = dialog.angle;
	int Wy = dialog.cutoff;
	int Wz = dialog.radius;
	int c = dialog.ch;
	double sigma = dialog.sigma;
	//cmd窗口的显示内容
	cout << "Wx = " << Wx << endl;
	cout << "Wy = " << Wy << endl;
	cout << "Wz = " << Wz << endl;
	cout << "sigma = " << sigma << endl;
	cout << "ch = " << c << endl;
	
	/*--------------------------------------------------------------------2.取最中间的切片并show出来---------------------------------------------------------*/
	
    V3DLONG Z = (ceil((long double)P / 2));//中间的第Z张切片
    unsigned char *pImage = new unsigned char[pagesz];//指向原始3D图像的指针数组
    unsigned char *ZImage = new unsigned char[slicesz];//指向切片图像的指针数组
	if (!pImage)
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		for (V3DLONG i = 0; i<pagesz; i++)
			pImage[i] = data1d[i];//将3D图像像素值赋给指针，初始化
	}
	for (V3DLONG i = 0; i<slicesz; i++)
		ZImage[i] = 0;
	//初始化切片指针，为空
    unsigned char *tp = &pImage[Z*N*M - 1];//tp指向第Z张切片第1个像素
    unsigned char *tl = ZImage;
	
	for (int j = 0; j < M; j++)
		for (int i = 0; i < N; i++)
		{
			*(tl++) = *(tp++);
		}
	//切片像素值赋值
	// display Z slice
	Image4DSimple * new2DSlice = new Image4DSimple();
    new2DSlice->setData((unsigned char *)ZImage, N, M, 1, 1, V3D_UINT8);//ZImage切片图
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, new2DSlice);
	//callback.setImageName(newwin, title);
	callback.setImageName(newwin, "The middle slice");
	callback.updateImageWindow(newwin);
	cout << "1" << endl;
	//QString qs_filename = "D:/Allen/vaa3d/vaa3d_tools/hackathon/wanwan/1/1.tif";
	V3DLONG img_siz[4];
	img_siz[0] = N;
	img_siz[1] = M;
	img_siz[2] = 1;
	img_siz[3] = 1;
	cout << "2" << endl;
	//saveImage(qs_filename.toStdString().c_str(), (unsigned char *)ZImage, img_siz, 1);
	//cmd窗口显示内容

	//将切片存为tif格式

	simple_saveimage_wrapper(callback, "D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\1.tif", (unsigned char *)ZImage, img_siz, V3D_UINT8);
	cout << "save tiff success" << endl;
	//----------------------------------------------------------------------------------------------------------------- 3.将该切片变换到频域，频谱中心化显示频谱图--------------------------------------------------------
	
	
	cv::Mat src = cv::imread("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\1.tif", CV_LOAD_IMAGE_GRAYSCALE);
	if (!src.data)
	{
		std::cout << "Image Load Fail!!!" << "\n";
		//return 1;
	}
	//imshow("SRC", src);
	//扩充图像尺寸到最佳尺寸，边界用0填充	
	int r = cv::getOptimalDFTSize(src.rows);
	int g = cv::getOptimalDFTSize(src.cols);
	cv::Mat padded;
	cv::copyMakeBorder(src, padded, 0, r - src.rows, 0, g - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	cout << "3" << endl;
	//为傅里叶变换的结果（复数，包含实部和虚部，所以需要创建一个二维的数组）分配存储空间,	
	//需要用至少float型来存储	
	//最后将二维数组合并为二通道--傅里叶变换需要	
	cv::Mat dst1[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat dst2;
	cv::merge(dst1, 2, dst2);
	//傅里叶变换,结果依旧存储在dst2中	
	cv::dft(dst2, dst2);
	//将复数换算成幅值	
	cv::split(dst2, dst1);//把二通道图像分解为二维数组，保存到dst1中,dst1[0]中存放的为实部	
	cv::magnitude(dst1[0], dst1[1], dst1[0]);//结果存放在dst1[0]中	
	cv::Mat magnitudeImage = dst1[0];  	//对数尺度缩放以便于显示	
	//计算log(1 + sqrt(Re(DFT(dst2))**2 + Im(DFT(dst2))**2))	
	magnitudeImage += cv::Scalar::all(1);
	cv::log(magnitudeImage, magnitudeImage);

	//剪切和重分布幅度图象限	
	//若有奇数行或奇数列，进行频谱裁剪	
	magnitudeImage = magnitudeImage(cv::Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));//任何一个数&-2的结果一定是偶数 	
	cout << "4" << endl;
	//重新排列傅里叶图像的象限，使原点位于图像中心	
	int cx = magnitudeImage.cols / 2;
	int cy = magnitudeImage.rows / 2;
	cv::Mat q0(magnitudeImage(cv::Rect(0, 0, cx, cy)));
	cv::Mat q1(magnitudeImage(cv::Rect(cx, 0, cx, cy)));
	cv::Mat q2(magnitudeImage(cv::Rect(0, cy, cx, cy)));
	cv::Mat q3(magnitudeImage(cv::Rect(cy, cy, cx, cy)));

	cv::Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	//将幅度值归一化到0~1之间，这是因为magnitudeImage中的数据类型是浮点型，这时用imshow()来显示函数，会将像素值乘于255，因此需要归一化到0~1之间	
	cv::normalize(magnitudeImage, magnitudeImage, 0, 1, cv::NORM_MINMAX);
	cout << "5" << endl;
	//显示最后的频谱	
	
	//imshow("spectrum magnitude", magnitudeImage);
	/*cvSaveImage("fft.jpg", magnitudeImage)*/
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION); // paramId_1, png compression
	compression_params.push_back(9);

	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\fft.tif", magnitudeImage, compression_params);
	//cvSaveImage("FFT.jpg,", magnitudeImage);
	cv::waitKey();
	

	unsigned char *p_img = 0;
	long long *sz_img= 0;
	int datatype= 0;
	simple_loadimage_wrapper(callback,"D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\fft.tif",p_img,sz_img,datatype);
	cout << "6" << endl;


	Image4DSimple * FFT2DSlice = new Image4DSimple();
	FFT2DSlice->setData((unsigned char *)p_img, N, M, 1, 1, V3D_UINT8);//ZImage切片图
	v3dhandle FFTwin = callback.newImageWindow();
	callback.setImage(FFTwin, FFT2DSlice);
	//callback.setImageName(newwin, title);
	callback.setImageName(FFTwin, "The middle slice");
	callback.updateImageWindow(FFTwin);
	
	//----------------------------------------------------------------------------------------------------------------- 3.将该切片变换到频域，频谱中心化显示频谱图--------------------------------------------------------




	/*cv::Mat src = cv::imread(ZImage, CV_LOAD_IMAGE_GRAYSCALE);*/
	//float* outimg1 = 0;
	//float* outimg2 = 0;
	//switch (pixeltype)
	//{
	//case V3D_UINT8: FFT_2D(ZImage, in_sz, outimg1, outimg2); cout << "7" << endl;  break;
	//case V3D_UINT16: FFT_2D(ZImage, in_sz,outimg1,outimg2); cout << "8" << endl; break;
	//case V3D_FLOAT32: FFT_2D(ZImage, in_sz,outimg1, outimg2); cout << "9" << endl; break;
	//default: v3d_msg("Invalid data type. Do nothing."); return;
	//}
	//cout << "FFT sucess " << endl;
	//// show FFT后的频谱
	//Image4DSimple * FFTImge = new Image4DSimple(); cout << "10" << endl;
	//FFTImge->setData((unsigned char *)outimg1, N, M, 1, 1, V3D_FLOAT32); cout << "11" << endl;
	//v3dhandle FFTwin = callback.newImageWindow(); cout << "12" << endl;
	//callback.setImage(FFTwin, FFTImge);//v3d_msg("13");
	//callback.setImageName(FFTwin, "medialFFT  result of this slice");
	////callback.updateImageWindow(FFTwin);
	//// show 中心化后的频谱
	//Image4DSimple * FFTSHIFT = new Image4DSimple();
	//FFTSHIFT->setData((unsigned char *)outimg2, N, M, 1, 1, V3D_FLOAT32);
	//v3dhandle SHIFwin = callback.newImageWindow();
	//callback.setImage(SHIFwin, FFTSHIFT);
	//callback.setImageName(SHIFwin, "2D LOG FFT result of this slice");
	

	cout << "show sucess"<< endl;
	//


	return;
}


///*----------------------------------------------------------------2DFFT函数内容--------------------------------------------------------------------------------------------------------*/
//template <class T> void FFT_2D(T* data1d,
//	V3DLONG *in_sz,
//	//unsigned int c,
//	T* &outimg1, T* &outimg2)
//{
//	if (outimg1 || outimg2 )
//		{
//			v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
//			delete[]outimg1;
//			delete[]outimg2;
//			
//			outimg1 = 0;
//			outimg2=0;
//		}
//	/*V3DLONG N = 5;
//	V3DLONG M = 5;
//	V3DLONG zsize =5;*/
//	V3DLONG N = in_sz[0];
//	V3DLONG M = in_sz[1];
//	V3DLONG zsize = N*M;
//	//V3DLONG offsetc = (c - 1)*zsize;
//	////定义相应大小的Buff
//	//T *Image = new T[zsize];//FFT
//	T *pImage = new T[zsize];//FFTshift
//	T *LogImage = new T[zsize];//存储S=log(abs(Fc)+1)之后的值
//	T *Image = new T[zsize];
//	fftwf_complex* buf1 = new fftwf_complex[zsize];
//	fftwf_complex* buf2 = new fftwf_complex[zsize];//after fftshift
//	fftwf_complex* buf3 = new fftwf_complex[zsize];
//	fftwf_complex* tmp0 = new fftwf_complex[zsize];
//	cout << "1" << endl;
//
//	if (!pImage)
//	{
//		printf("Fail to allocate memory.\n");
//		return;
//	}
//	else
//	{
//		for (V3DLONG i = 0; i<zsize; i++)
//			pImage[i] = 0;
//	}
//	cout << "2" << endl;
//
//	////输入图片赋给buff1
//	
//	for (V3DLONG i = 0; i < zsize;i++)
//	{
//		buf1[i][0] = (float)data1d[i];
//		buf1[i][1] = 0;
//	}
//	cout << "3" << endl;
//	//图像中心化
//	//for (int k0 = 0; k0 <(zsize / 2); k0++)
//	//{
//	//	//定义是类型一定要一致
//	//	int k1 = zsize - 1 - k0;
//	//	//实部
//	//	tmp0[k0][0] = buf1[k0][0];
//	//	buf1[k0][0] = buf1[k1][0];
//	//	buf1[k1][0] = buf1[k0][0];
//	//	//虚部
//	//	/*tmp0[k0][1] = buf2[k0][1];
//	//	buf2[k0][1] = buf2[k1][1];
//	//	buf2[k1][1] = tmp0[k0][1];*/
//	//}
//
//	
//	////Transform to frequency space，buf2为fft输出结果
//	fftwf_plan pFwd = fftwf_plan_dft_2d(N, M, buf1, buf2, FFTW_FORWARD, FFTW_ESTIMATE);
//	fftwf_execute(pFwd);//执行plan
//	fftwf_destroy_plan(pFwd);//释放内存
//	cout << "4" << endl;
//
//	/*-----------------------------------------------------------------------test-------------------------------------------------------------*/
//	double A[5][5] = { 0, 5, 10, 14, 20, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 200, 210, 220, 230, 251 };
//	fftwf_complex* input = new fftwf_complex[25];
//	fftwf_complex* output = new fftwf_complex[25];
//	input[0][0] = 0; input[0][1] = 5; input[0][2] = 10; input[0][3] = 14; input[0][4] = 20;
//	input[1][0] = 20; input[1][1] = 30; input[1][2] = 40; input[1][3] = 50; input[1][4] = 60;
//	input[2][0] = 70; input[2][1] = 80; input[2][2] = 90; input[2][3] = 100; input[2][4] = 110;
//	input[3][0] = 120; input[3][1] = 130; input[3][2] = 140; input[3][3] = 150; input[3][4] = 160;
//	input[4][0] = 200; input[4][1] = 210; input[4][2] = 220; input[4][3] = 230; input[4][4] = 251;
//	fftwf_plan ptest = fftwf_plan_dft_2d(5,5, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
//	fftwf_execute(ptest);//执行plan
//	fftwf_destroy_plan(ptest);//释放内存
//
//	cout << "A数组" << endl;
//	int c;
//	int d;
//	for (c = 0; c < 5; c++)
//	{
//		for (d = 0; d < 5; d++)
//		{
//			cout << A[c][d] <<"  " ;
//		}
//		cout << endl;
//	}
//	cout << "B数组" << endl;
//	for (c = 0; c < 25; c++)
//		{
//			
//				cout << output[c][0]/25 << "+" << output[c][1]/25<<"i"<<"  ";
//				if ((c+1)%5==0)
//					cout << endl;
//		}
//	
//	
//	//-----------------原始FFT Image求模值
//	for (V3DLONG i = 0; i < zsize; i++)
//	{
//		double re3 = buf3[i][0];
//		double im3 = buf3[i][1];
//		double mag3 = sqrt(re3*re3 + im3*im3);
//		Image[i] = mag3;
//	}//Image原始FFT
//	
//
//	////将中心化后的频谱数据赋给指针pImage,得到模值信息
//	for (int j = 0; j < zsize; j++)
//	{
//		double re2 = buf2[j][0];
//		double im2 = buf2[j][1];
//		double mag2 = sqrt(re2*re2 + im2*im2);
//		pImage[j] = mag2;
//	}
//	
//	////----------------------------S=log(abs(Fc)+1);
//	//将log后的频谱赋给指针LogImage
//	for (V3DLONG k = 0; k< zsize; k++)
//	{
//		
//		LogImage[k] = log(pImage[k]);
//		//LogImage[k] = log(static_cast<double>(abs(static_cast<int>(pImage[k]) + 1)));
//		
//	}
//	cout << "5" << endl;
//    //outimg1=Image;//原始FFT模值
//	outimg1 = pImage;//中心化FFT模值
//	outimg2 = LogImage;//log后的FFT模值
//	cout << "6" << endl;
//	// Cleanup.
//	/*fftwf_free(buf1);
//	fftwf_free(buf2);*/
//	free(buf1);//用free+返回数据类型要匹配，不然会闪退
//	free(buf2);
//	free(tmp0);
//	return;
//}

////FFT函数功能
//template <class T> void FFT_HP(T* data1d,
//	V3DLONG *in_sz,
//	unsigned int c,
//	T* &outimg)
//{
//
//	if (outimg)
//	{
//		v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
//		delete[]outimg;
//		outimg = 0;
//	}
//	V3DLONG N = in_sz[0];
//	V3DLONG M = in_sz[1];
//	V3DLONG P = in_sz[2];
//
//	V3DLONG pagesz = N*M*P;
//	V3DLONG offsetc = (c - 1)*pagesz;
//
//	T *pImage = new T[pagesz];
//	fftwf_complex* buf1 = new fftwf_complex[pagesz];
//	fftwf_complex* buf2 = new fftwf_complex[pagesz];
//	if (!pImage)
//	{
//		printf("Fail to allocate memory.\n");
//		return;
//	}
//	else
//	{
//		for (V3DLONG i = 0; i<pagesz; i++)
//			pImage[i] = 0;
//	}
//
//	//outimg = pImage;fftw_plan_dft_3d
//	V3DLONG i = 0;
//	for (V3DLONG iz = 0; iz < P; iz++)
//	{
//		V3DLONG offsetk = iz*M*N;
//		for (V3DLONG iy = 0; iy < M; iy++)
//		{
//			V3DLONG offsetj = iy*N;
//			for (V3DLONG ix = 0; ix < N; ix++)
//			{
//				buf1[i][0] = (float)data1d[offsetc + offsetk + offsetj + ix];;
//				buf1[i][1] = 0;
//				i++;
//			}
//
//		}
//
//	}
//
//	// Transform to frequency space.
//	fftwf_plan pFwd = fftwf_plan_dft_3d(N, M, P, buf1, buf2, FFTW_FORWARD, FFTW_ESTIMATE);
//	fftwf_execute(pFwd);
//	fftwf_destroy_plan(pFwd);
//
//	// Zap the DC value
//	buf2[0][0] = 0;
//	buf2[0][1] = 0;
//
//	// Transform back to image space.
//	fftwf_plan pBack = fftwf_plan_dft_3d(N, M, P, buf2, buf1, FFTW_BACKWARD, FFTW_ESTIMATE);
//	fftwf_execute(pBack);
//	fftwf_destroy_plan(pBack);
//
//	// Have to scale the output values to get back to the original.
//	for (int i = 0; i < pagesz; i++) {
//		buf1[i][0] = buf1[i][0] / pagesz;
//		buf1[i][1] = buf1[i][1] / pagesz;
//	}
//
//	//outimg = pImage;
//	i = 0;
//	for (V3DLONG iz = 0; iz < P; iz++)
//	{
//		V3DLONG offsetk = iz*M*N;
//		for (V3DLONG iy = 0; iy < M; iy++)
//		{
//			V3DLONG offsetj = iy*N;
//			for (V3DLONG ix = 0; ix < N; ix++)
//			{
//				double re = buf1[i][0];
//				double im = buf1[i][1];
//				double mag = sqrt(re*re + im*im);
//				//    if(mag>10)
//				pImage[offsetk + offsetj + ix] = mag;
//				i++;
//			}
//
//		}
//
//	}
//	outimg = pImage;
//	// Cleanup.
//	fftwf_free(buf1);
//	fftwf_free(buf2);
//	return;
//
//}
