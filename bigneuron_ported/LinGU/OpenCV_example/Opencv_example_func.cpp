/* Opencv_example_func.cpp
 * This plugin will load image and swc in domenu and dofunc
 * 2012-02-21 : by Hang Xiao
 */

#include "Opencv_example_func.h"


using namespace std;
using namespace cv;
using namespace cv::ml;


const QString title = QObject::tr("Load Image And SWC");

#define GET_CURRENT_IMAGE_AND_SWC(inimg1d, in_sz, nt) \
{\
	v3dhandle curwin = callback.currentImageWindow(); \
	Image4DSimple *p4DImage = callback.getImage(curwin); \
	nt = callback.getSWC(curwin);\
	inimg1d = p4DImage->getRawData();\
	in_sz = new V3DLONG[4]; \
	in_sz[0] = p4DImage->getXDim(); \
	in_sz[1] = p4DImage->getYDim();\
	in_sz[2] = p4DImage->getZDim();\
	in_sz[3] = p4DImage->getCDim();\
	ImagePixelType datatype = p4DImage->getDatatype();\
}

#define SHOW_OUT_IMAGE_AND_SWC(outimg1d, out_sz, nt, title) \
{\
	Image4DSimple * p4DImage = new Image4DSimple();\
	p4DImage->setData(outimg1d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);\
	v3dhandle newwin = callback.newImageWindow();\
	callback.setImage(newwin, p4DImage);\
	callback.setImageName(newwin, QObject::tr(title));\
	callback.setSWC(newwin, nt);\
	callback.open3DWindow(newwin);\
	callback.getView3DControl(newwin)->updateWithTriView();\
}

string file_type(string para)
{
	int pos = para.find_last_of(".");
	if(pos == string::npos) return string("");
	else return para.substr(pos, para.size() - pos);
}

int Opencv_example(V3DPluginCallback2 &callback, QWidget *parent)
{
	if(callback.getImageWindowList().empty()) return QMessageBox::information(0, title, QObject::tr("No image is open."));

	unsigned char * data1d = 0;
	V3DLONG * in_sz = 0;
	NeuronTree nt;

	v3dhandle curwin = callback.currentImageWindow();

	Image4DSimple *p4DImage = callback.getImage(curwin);


	data1d = p4DImage->getRawData();

	in_sz = new V3DLONG[4];

	in_sz[0] = p4DImage->getXDim();

	in_sz[1] = p4DImage->getYDim();

	in_sz[2] = p4DImage->getZDim();

	in_sz[3] = p4DImage->getCDim();


	ImagePixelType datatype = p4DImage->getDatatype();


    vector<Mat> img_per_channel;

    int * img_sz = new int[3];

    img_sz[0] = (int)in_sz[0];

    img_sz[1] = (int)in_sz[1];

    img_sz[2] = (int)in_sz[2];

    int sum_value = 0;

    for(V3DLONG iz = 0; iz < in_sz[2]; iz++)
    {

        Mat img_current(in_sz[0],in_sz[1],CV_8UC1);

        V3DLONG offsetk = iz*in_sz[1]*in_sz[0];
        for(V3DLONG iy = 0; iy <  in_sz[1]; iy++)
        {
            V3DLONG offsetj = iy*in_sz[0];
            for(V3DLONG ix = 0; ix < in_sz[0]; ix++)
            {

                unsigned char PixelVaule = data1d[offsetk + offsetj + ix];

                img_current.at<uchar>(int(ix),int(iy)) = PixelVaule;

                sum_value = sum_value + (int)PixelVaule;

                int v3[3];

                v3[0] = int(ix);

                v3[1] = int(iy);

                v3[2] = int(iz);

            }
        }

        img_per_channel.push_back(img_current);

    }

    v3d_msg(QString("The sum of the image 2 is %1").arg(sum_value));

    Mat image;

    merge(img_per_channel,image);


   double sum_value2 = (double)std::accumulate(image.begin<uchar>(), image.end<uchar>(), 0.0);

   v3d_msg(QString("The sum of the image 2 is %1").arg(sum_value2));

    int sum_value3 = 0;


		  for(int i = 0; i < image.rows; i++ )
		  {

            uchar* rp = image.ptr<uchar>(i);


			  for(int j = 0; j < image.cols; j ++)
			  {
				  int offsetj = (j ) * image.channels();

				  for(int k = 0; k < image.channels(); k ++ )
				  {

					  int vc[3];

					  vc[0] = i;

					  vc[1] = j;

					  vc[2] = k;

					  sum_value3 = sum_value3 + (int)rp[k + offsetj];

				  }

			  }

		  }

		  v3d_msg(QString("The sum 3 of the image 3 is %1").arg(sum_value3));




	return 1;
}

#define READ_IMAGE_AND_SWC_FROM_INPUT(inimg1d, in_sz, nt) \
{\
	if(input.empty()) return false;\
	vector<char*> in_str = *((vector<char*> *)(input.at(0).p));\
	if(in_str.size() < 2){cerr<<"Error: no enough input."<<endl; return false;}\
	string inimg_file, inswc_file;\
	string ftp1 = file_type(in_str[0]);\
	string ftp2 = file_type(in_str[1]);\
	if(ftp1 == ".raw" || ftp1 == ".tif" || ftp1 == ".tiff" || ftp1 == ".lsm") inimg_file = in_str[0];\
	else {if(ftp1 == ".swc") inswc_file = in_str[0];}\
	if(ftp2 == ".raw" || ftp2 == ".tif" || ftp2 == ".tiff" || ftp2 == ".lsm") inimg_file = in_str[1];\
	else {if(ftp2 == ".swc") inswc_file = in_str[1];}\
	if(inimg_file == "" || inswc_file == "") {cerr<<"Error : either image file or swc file is missing."<<endl; return false;}\
	int datatype;\
	loadImage((char*) inimg_file.c_str(), inimg1d, in_sz, datatype);\
	nt = readSWC_file((char*)inswc_file.c_str());\
}

bool Opencv_example(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	unsigned char * inimg1d = 0;
	V3DLONG * in_sz = 0;
	NeuronTree nt;


	READ_IMAGE_AND_SWC_FROM_INPUT(inimg1d, in_sz, nt);

	cout<<"in_sz[0] = "<<in_sz[0]<<endl;
	cout<<"in_sz[1] = "<<in_sz[1]<<endl;
	cout<<"in_sz[2] = "<<in_sz[2]<<endl;
	cout<<"in_sz[3] = "<<in_sz[3]<<endl;
	cout<<"nt.listNeuron.size() = "<<nt.listNeuron.size()<<endl;
	return true;
}

