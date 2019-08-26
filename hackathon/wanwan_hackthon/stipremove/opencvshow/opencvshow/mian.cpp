#include<opencv2\opencv.hpp>
#include<vector>
#include<opencv.hpp>
using namespace cv;
using namespace std;
#include<iostream> 
#include "highgui.h"
int main()
{	//加载一副灰度图像	
	//cv::Mat src = cv::imread("F:\\postegraduate\\1.tif", IMREAD_GRAYSCALE);
	/*------------------------------------------------------------1.读入图像-----------------------------------------------------------------------------------------*/
	
	cv::Mat src = cv::imread("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\1.tif", IMREAD_GRAYSCALE);
	if (!src.data)
	{
		std::cout << "Image Load Fail!!!" << "\n";
		return 1;
	}
	//cv::namedWindow("SRC");
	/*imshow("SRC", src);
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\yuantu.tif", src);
	cv::waitKey();
	cv::destroyAllWindows();*/
	/*------------------------------------------------------------------------------------2.fft,show 频谱图-------------------------------------------------------------------------------------------------*/
	//扩充图像尺寸到最佳尺寸，边界用0c	
	//cv2.resize(src, (src.shape[1] * 0.5, src.shape[0] * 0.5));
	//int r = cv::getOptimalDFTSize(src.rows);//padding后的高，返回向量尺寸经过DFT变换后结果的最优尺寸
	//int c = cv::getOptimalDFTSize(src.cols);//padding后的宽
	//cv::Mat padded;
	////扩充图像边界；输入图像，输出图像，top/bottom/left/right四个方向扩充像素的值，边界类型，填充0
	//cv::copyMakeBorder(src, padded, 0, r - src.rows, 0, c - src.cols, cv::BORDER_CONSTANT,cv::Scalar::all(0));
	//cv::copyMakeBorder(src, padded, 0, 0, 0, 0, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	//为傅里叶变换的结果（复数，包含实部和虚部，所以需要创建一个二维的数组）分配存储空间,	
	//需要用至少float型来存储	
	//最后将二维数组合并为二通道--傅里叶变换需要	

	//cv::Mat dst1[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat dst1[] = { cv::Mat_<float>(src), cv::Mat::zeros(src.size(), CV_32F) };//padding后的图像值放入通道1，实部；通道2虚部，全0
	cv::Mat dst2;
	cv::merge(dst1, 2, dst2);//dst1的2个单通道合成多通道dst2
	//FFT变换,结果依旧存储在dst2中	
	cv::dft(dst2, dst2);
	
	//将复数换算成幅值	
	cv::split(dst2, dst1);//把二通道图像dst2分解为二维数组，保存到dst1中,dst1[0]中存放的为实部，dst1[1]中存放的虚部
	Mat mag;
	Mat angle;
	cv::magnitude(dst1[0], dst1[1], mag);//实部、虚部、结果，结果存放在dst1[0]（结果和第一个参数有相同的尺寸和类型）中	;得到原图幅值mag
	cv::phase(dst1[0], dst1[1], angle);//得到原图相位
	cv::Mat magnitudeImage = mag;  	//幅值结果存放到mat magbitudeImage中
	
	//对数尺度缩放以便于显示	
	//计算log(1 + sqrt(Re(DFT(dst2))**2 + Im(DFT(dst2))**2))	
	magnitudeImage += cv::Scalar::all(1);
	cv::log(magnitudeImage, magnitudeImage);

	//剪切和重分布幅度图象限	
	//若有奇数行或奇数列，进行频谱裁剪	
	//magnitudeImage = magnitudeImage(cv::Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));//任何一个数&-2的结果一定是偶数，位的和运算，-2=11111110，任何数与该数相与最低位都为0，一定是偶数

	//重新排列傅里叶图像的象限，使原点位于图像中心	
	//奇数行列的图像padding后变大了
	////将幅度值归一化到0~1之间，这是因为magnitudeImage中的数据类型是浮点型，这时用imshow()来显示函数，会将像素值乘于255，因此需要归一化到0~1之间	
	cv::normalize(magnitudeImage, magnitudeImage, 0, 1, cv::NORM_MINMAX);
	cv::Mat a(cv::Size(2 * magnitudeImage.cols, 2 * magnitudeImage.rows), CV_32F);

	cv::Mat q0(a(cv::Rect(0, 0, magnitudeImage.cols, magnitudeImage.rows)));

	cv::Mat q1(a(cv::Rect(magnitudeImage.cols, 0, magnitudeImage.cols, magnitudeImage.rows)));

	cv::Mat q2(a(cv::Rect(0, magnitudeImage.rows, magnitudeImage.cols, magnitudeImage.rows)));

	cv::Mat q3(a(cv::Rect(magnitudeImage.cols, magnitudeImage.rows, magnitudeImage.cols, magnitudeImage.rows)));

	magnitudeImage.copyTo(q0);
	magnitudeImage.copyTo(q1);
	magnitudeImage.copyTo(q2);
	magnitudeImage.copyTo(q3);
	//原始幅度频谱
	cv::Mat IMAGE(a(cv::Rect(magnitudeImage.cols/2, magnitudeImage.rows/2, magnitudeImage.cols, magnitudeImage.rows)));
	cv::Mat IMAGE_8;
	normalize(IMAGE, IMAGE_8, 0, 255, CV_MINMAX);
	IMAGE_8.convertTo(IMAGE_8, CV_8U);
	cv::namedWindow("spectrum magnitude", WINDOW_NORMAL);//窗口创建
	imshow("spectrum magnitude", IMAGE_8);
	cv::waitKey();//等待按键
	//保存图像
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\pinpu.tif", IMAGE_8);

	//------------------------------------------test--------------------------------------------
	//处理后的实部虚部合成一个矩阵，进行中心平移，还原（0,2π）图像
   /* cv:Mat ImgDFT;
	cv::merge(dst1, 2, ImgDFT);
	idft(ImgDFT, ImgDFT, DFT_SCALE | DFT_REAL_OUTPUT);
	Mat imp = ImgDFT;
	normalize(imp, imp, 0, 255, CV_MINMAX);
	imp.convertTo(imp, CV_8U);
	Mat Output = imp(Range(0, magnitudeImage.rows), Range(0, magnitudeImage.cols));
	imshow("Output", Output);
	waitKey();
	cv::destroyAllWindows();*/
	///*cv::Mat image(magnitudeImage(cv::Rect(0, 0, 2*cx,2* cy)));*/
	//imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\fft.tif", image);
	/*imshow("spectrum magnitude", IMAGE);
	cv::namedWindow("spectrum magnitude", 0);
	cv::waitKey();
	cv::destroyAllWindows();*/
	/*----------------------------------------------------------------------------------------3.creat mask--------------------------------------------------------------------------*/
	int filter_axis_dir = 10;//旋转角度，与x轴正向的夹角
	int filter_cutoff = 10;//值越大竖条越短
	int filter_radius = 8;//值越大竖条越宽
	//构建水平带状滤波器
	int max2 = (magnitudeImage.cols > magnitudeImage.rows ? magnitudeImage.cols : magnitudeImage.rows);
	int dim_max2 = max2*2;//取长宽中较大的值×2
	//cv::Mat H_max = Mat::ones(dim_max2, dim_max2, CV_32F);//构建大矩阵
	//cv::Mat H_max(dim_max2, dim_max2, CV_8UC1, cv::Scalar(1));//构建double矩阵
	cv::Mat H_max(dim_max2, dim_max2, CV_32F, cv::Scalar(1));//构建全1大矩阵
	cv::Mat mask_1(2 * filter_radius, dim_max2, CV_32F, cv::Scalar(0));//0黑，255白，20行
	cv::Mat mask_2(dim_max2, 2 * filter_cutoff, CV_32F, cv::Scalar(1));//int rows,int cols,type，20列
	
	int centerpos_max = round(dim_max2 / 2);
	//mask区域赋值
	cv::Mat mask_region_1(H_max(cv::Rect(0, centerpos_max - filter_radius, dim_max2, filter_radius*2)));//取出该区域,列，行
	mask_1.copyTo(mask_region_1);
	cv::Mat mask_region_2(H_max(cv::Rect(centerpos_max - filter_cutoff, 0, 2 * filter_cutoff, dim_max2)));//取出该区域,列，行
	mask_2.copyTo(mask_region_2);
	//实现矩阵旋转
	cv::Point center = cv::Point(centerpos_max, centerpos_max);//旋转中心
	cv::Mat rot_mat = cv::getRotationMatrix2D(center, filter_axis_dir, 1.0);//获得仿射变换矩阵
	cv::Size dst_sz(dim_max2, dim_max2);
	cv::warpAffine(H_max, H_max, rot_mat, dst_sz);
	//截取和slice相同大小的矩阵
	cv::Mat filter(H_max(cv::Rect(centerpos_max - round((magnitudeImage.cols - 1) / 2), centerpos_max - round((magnitudeImage.rows - 1) / 2), magnitudeImage.cols, magnitudeImage.rows)));//取出该区域
	cv::GaussianBlur(filter, filter, cv::Size(11, 11), 5, 5);//sigama越大平滑效果模糊效果越明显，核必须为正奇数
	cv::multiply(IMAGE, filter, filter);//相乘，去除条纹所在频域，置0
	//mask figure，中心平移后的
	cv::Mat filter_8;
	normalize(filter, filter_8, 0, 255, CV_MINMAX);
	filter_8.convertTo(filter_8, CV_8U);
	cv::namedWindow("mask figure");
	imshow("mask figure", filter_8);
	cv::waitKey();//等待用户按键
	cv::destroyAllWindows();//销毁指定窗口
	//save image
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\mask.tif", filter_8);
	/*--------------------------------------------------------------------------------4.mask 做中心平移，还原原始非中心化状态--------------------------------------------------------------*/
	//IMAGE
	//filter  两个矩阵对应位置数值相乘，返回的是矩阵

	cv::Mat b(cv::Size(2 * filter.cols, 2 * filter.rows), CV_32F);

	cv::Mat d0(b(cv::Rect(0, 0, filter.cols, filter.rows)));

	cv::Mat d1(b(cv::Rect(filter.cols, 0, filter.cols, filter.rows)));

	cv::Mat d2(b(cv::Rect(0, filter.rows, filter.cols, filter.rows)));

	cv::Mat d3(b(cv::Rect(filter.cols, filter.rows, filter.cols, filter.rows)));

	filter.copyTo(d0);
	filter.copyTo(d1);
	filter.copyTo(d2);
	filter.copyTo(d3);
	cv::Mat ifft_filter(b(cv::Rect(filter.cols / 2, filter.rows / 2, filter.cols, filter.rows)));//此时滤波器iff_filter只有实部
	/*----------------------------------------------------------------------------------5.对频谱滤波--------------------------------------------------------------*/
	//频谱（幅度谱）中心化只是为了确定mask的范围，频域相乘滤波时不可以只乘以幅度
	cv::multiply(dst1[0], ifft_filter, dst1[0]);//和实部相乘
	cv::multiply(dst1[1], ifft_filter, dst1[1]);//和虚部相乘
	//实部虚部乘以相同倍数时，角度不变，相位（位移）不变
     cv:Mat output;
	//cv::multiply(dst2, FITER, output);//dst2 不是单纯的幅度谱而是fft后既有实部又有虚部的完整的结果；不可以用幅度谱IMAGE，否则会丢失相位信息。 
	cv::merge(dst1, 2, output);//实部虚部相合，得到幅度+相位信息
	//show 滤波后的频谱
	cv::Mat output1[] = { cv::Mat_<float>(output), cv::Mat::zeros(output.size(), CV_32F) };
	cv::split(output, output1);
	cv::Mat REAL;
	cv::magnitude(output1[0], output1[1], REAL);
	REAL += cv::Scalar::all(1);
	cv::log(REAL, REAL);
	cv::normalize(REAL, REAL, 0, 1, cv::NORM_MINMAX);
	//滤波后频谱
	cv::Mat REAL_8;
	normalize(REAL, REAL_8, 0, 255, CV_MINMAX);
	REAL_8.convertTo(REAL_8, CV_8U);
	cv::namedWindow("chonggou");
	imshow("chonggou", REAL_8);
	cv::waitKey();
	cv::destroyAllWindows();
	//save image
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\chonggou.tif", REAL_8);
	/*----------------------------------------------------------------------------------6.将频谱映射回空间域--------------------------------------------------------------*/

   /* cv:Mat OUTPUT;
	cv::merge(output, 2, OUTPUT);*/
	idft(output, output, DFT_SCALE | DFT_REAL_OUTPUT);
	Mat imp = output;
	normalize(imp, imp, 0, 255, CV_MINMAX);
	imp.convertTo(imp, CV_8U);
	//Mat Output = imp(Range(0, magnitudeImage.rows), Range(0, magnitudeImage.cols));
	cv::namedWindow("Output");
	imshow("Output", imp);
	cv::waitKey();
	cv::destroyAllWindows();
	imwrite("D:\\Allen\\vaa3d\\vaa3d_tools\\hackathon\\wanwan\\1\\slice_stripremove.tif", imp);
	return 0;
}
