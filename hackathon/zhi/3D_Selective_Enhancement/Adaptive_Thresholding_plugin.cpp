/* Adaptive_Thresholding_plugin.cpp
 * Detect fiber
 * 2013-06-17 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "Adaptive_Thresholding_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include "stackutil.h"
#include <Eigen/Dense>
#include <boost/lexical_cast.hpp>


/*#define WANT_STREAM       // include iostream and iomanipulators
#include "/home/zhiz/work/v3d_external/v3d_main/jba/newmat11/newmatap.h"
#include "/home/zhiz/work/v3d_external/v3d_main/jba/newmat11/newmatio.h"
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif
*/
using namespace Eigen;
using namespace std;
Q_EXPORT_PLUGIN2(Adaptive_Thresholding, adpThresholding);

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
void processImage2(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);
void processImage3(V3DPluginCallback2 &callback, QWidget *parent);


template <class T> void AdpThresholding(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
             unsigned int c,
             unsigned int p,
             unsigned int d,
                     T* &outimg,
		     T* gsdtdata1d);

template <class T> void AdpThresholding_adpwindow(T* data1d,
                    V3DLONG *in_sz,
                    unsigned int c,
                    T* &outimg,T* gsdtdata1d,T* gsdtsoma,
                                                  int x,
                                                  int y);

int swapthree(float& dummya, float& dummyb, float& dummyc);

template <class T> void AdpThresholding_adpwindow_v2(T* data1d,
                    V3DLONG *in_sz,
                    unsigned int c,
                    T* &outimg,T* gsdtdata1d,T* gsdtsoma,
                                                  int x,
                                                  int y);

int swapthree(float& dummya, float& dummyb, float& dummyc);

template <class T> void soma_detection(T* data1d,
                    V3DLONG *in_sz,
                    unsigned int c,
                    int x,
                    int y,
                    V3DLONG somasize,
                    T* &outimg);
 
QStringList adpThresholding::menulist() const
{
	return QStringList() 
        <<tr("Enhancement_fixedWindow")
        <<tr("Enhancement_adpWindow")
        <<tr("HighlightSoma")
		<<tr("about");
}

QStringList adpThresholding::funclist() const
{
	return QStringList()
		<<tr("AdpTheresholding")
		<<tr("help");
}

void adpThresholding::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Enhancement_fixedWindow"))
	{
		processImage(callback,parent);
	}
    else if(menu_name == tr("Enhancement_adpWindow"))
    {
       processImage2(callback,parent);

    }
    else if(menu_name == tr("HighlightSoma"))
    {
       processImage3(callback,parent);

    }
    else
	{
        (menu_name == tr("help"));
        v3d_msg(tr("Detect fiber. "
                   "Developed by Zhi Zhou, 2013-06-17"));
	}
}

bool adpThresholding::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("AdpTheresholding"))
	{
	}
	else if (func_name == tr("help"))
	{
	}
	else return false;
}

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
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

	//input
	bool ok1, ok2, ok3, ok4, ok5;
	unsigned int Wx=1, Wy=1, Wz=1, c=1,p=0,d=0;

	Wx = QInputDialog::getInteger(parent, "Window X ",
								  "Enter window size (# voxels) along x:",
								  1, 1, N, 1, &ok1);

	if(ok1)
	{
		Wy = QInputDialog::getInteger(parent, "Window Y",
									  "Enter window size  (# voxels) along y:",
									  1, 1, M, 1, &ok2);
	}
	else
		return;

	if(ok2)
	{
		Wz = QInputDialog::getInteger(parent, "Window Z",
									  "Enter window size  (# voxels) along z:",
									  1, 1, P, 1, &ok3);
	}
	else
		return;

	if(ok3)
	{
		c = QInputDialog::getInteger(parent, "Channel",
									 "Enter channel NO (starts from 1):",
									 1, 1, sc, 1, &ok4);
	}
	else
		return;


	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Include plane feature?"), QMessageBox::Yes, QMessageBox::No))    p = 1;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Include dot feature?"), QMessageBox::Yes, QMessageBox::No))    d = 1;

     // filter
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    saveImage("temp.tif", (unsigned char *)data1d, in_sz, pixeltype);

    //invoke gsdt function
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;    
    
    arg.type = "random";std::vector<char*> args1; args1.push_back("temp.tif"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;args.push_back("150");args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2; args2.push_back("gsdtImage.tiff"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

	  
    unsigned char * gsdtdata1d = 0;
    int datatype; 
    V3DLONG * in_zz = 0;
	
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    loadImage(outimg_file, gsdtdata1d, in_zz, datatype,0);
    remove( "gsdtImage.tiff"); 	
    remove("temp.tif");
    void* outimg = 0; 
    switch (pixeltype)
    {
        case V3D_UINT8: AdpThresholding(data1d, in_sz, Wx, Wy, Wz,c,p,d,(unsigned char* &)outimg,gsdtdata1d); break;
        case V3D_UINT16: AdpThresholding((unsigned short int *)data1d, in_sz, Wx, Wy, Wz,c, p,d,(unsigned short int* &)outimg,(unsigned short int *)gsdtdata1d); break;
        case V3D_FLOAT32: AdpThresholding((float *)data1d, in_sz, Wx, Wy, Wz,c, p,d,(float* &)outimg,(float *)gsdtdata1d);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }
    
     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, V3DLONG(N), V3DLONG(M), V3DLONG(P), 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "3D enhancement result");
     callback.updateImageWindow(newwin);

}



template <class T> void AdpThresholding(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
		     unsigned int p,
		     unsigned int d,	
                     T* &outimg,
		     T* gsdtdatald)
{
    
  
	     V3DLONG N = in_sz[0];
	     V3DLONG M = in_sz[1];
	     V3DLONG P = in_sz[2];
	     V3DLONG sc = in_sz[3];
	     V3DLONG pagesz = N*M*P;
         T *pImage = new T [pagesz];
		if (!pImage)
		{
			printf("Fail to allocate memory.\n");
			return;
		 }
		 else
		 {
            for(V3DLONG i=0; i<pagesz; i++)
				pImage[i] = 0;
		  }

          T maxfl = 0;
		 //outimg = pImage;

		for(V3DLONG iz = Wz; iz < P-Wz; iz++)
		{
			V3DLONG offsetk = iz*M*N;
			for(V3DLONG iy = Wy; iy < M-Wy; iy++)
			{
				V3DLONG offsetj = iy*N;
			 	for(V3DLONG ix = Wx; ix < N-Wx; ix++)
				{
                    if(gsdtdatald[offsetk+offsetj+ix]< 80)
					{
						//double fx = 0.5*(data1d[offsetk+offsetj+ix+Wx]-data1d[offsetk+offsetj+ix-Wx]);
						//double fy = 0.5*(data1d[offsetk+(iy+Wy)*N+ix]-data1d[offsetk+(iy-Wy)*N+ix]);
						//double fz = 0.5*(data1d[(iz+Wz)*M*N+offsetj+ix]-data1d[(iz-Wz)*M*N+offsetj+ix]);
						//Seletive approach
						float  fxx = data1d[offsetk+offsetj+ix+Wx]+ data1d[offsetk+offsetj+ix-Wx]- 2*data1d[offsetk+offsetj+ix];
						float fyy = data1d[offsetk+(iy+Wy)*N+ix]+data1d[offsetk+(iy-Wy)*N+ix]-2*data1d[offsetk+offsetj+ix];
						float fzz = data1d[(iz+Wz)*M*N+offsetj+ix]+data1d[(iz-Wz)*M*N+offsetj+ix]- 2*data1d[offsetk+offsetj+ix];

						float fxy = 0.25*(data1d[offsetk+(iy+Wy)*N+ix+Wx]+data1d[offsetk+(iy-Wy)*N+ix-Wx]-data1d[offsetk+(iy+Wy)*N+ix-Wx]-data1d[offsetk+(iy-Wy)*N+ix+Wx]);
						float fxz = 0.25*(data1d[(iz+Wz)*M*N+offsetj+ix+Wx]+data1d[(iz-Wz)*M*N+offsetj+ix-Wx]-data1d[(iz+Wz)*M*N+offsetj+ix-Wx]-data1d[(iz-Wz)*M*N+offsetj+ix+Wx]);
						float fyz = 0.25*(data1d[(iz+Wz)*M*N+(iy+Wy)*N+ix]+data1d[(iz-Wz)*M*N+(iy-Wy)*N+ix]-data1d[(iz+Wz)*M*N+(iy-Wy)*N+ix]-data1d[(iz-Wz)*M*N+(iy+Wy)*N+ix]);
						 
	 				 	Matrix3f A;
						A << fxx,fxy,fxz,fxy,fyy,fyz,fxz,fyz,fzz;	
	  					SelfAdjointEigenSolver<Matrix3f> eigensolver(A, false);
	       				//	if (eigensolver.info() != Success) abort();
						/*SymmetricMatrix Cov_Matrix(3);
						Cov_Matrix.Row(1) << fxx;
						Cov_Matrix.Row(2) << fxy << fyy; 
						Cov_Matrix.Row(3) << fxz << fyz <<fzz; 

						DiagonalMatrix DD;
						//cout << "Matrix" << endl;
						//cout << Cov_Matrix << endl <<endl;

						EigenValues(Cov_Matrix,DD);
						//cout << "EigenValues" << endl;
						//cout <<  DD << endl <<endl;*/

                        float a1 = eigensolver.eigenvalues()(0);
		   				float a2 = eigensolver.eigenvalues()(1);
						float a3 = eigensolver.eigenvalues()(2);
						swapthree(a1, a2, a3);
					//mak	printf("%f,%f,%f,%f,%f,%f,%f,%f,%f\n\n\n\n",fxx,fyy,fzz,fxy,fxz,fyz,a1,a2,a3);
						float output1 = 0;
						float output2 = 0;
						float output3 = 0;	
						if(a1<0)
						{
							
							if (p ==1) output3 = abs(a1) - abs(a2);	
							if(a2 < 0)
							{	
								//pImage[offsetk+offsetj+ix] = abs(a2)*(abs(a2)-abs(a3))/abs(a1);
                                output1 =  abs(a2)*(abs(a2)-abs(a3))/abs(a1);
								//T output = abs(a1)/abs(a2);	
						
								//printf("%f %f %f %d\n", a1,a2,a3,output1);
								//if(ix ==96 && iy == 37 && iz == 6)
								if(a3 < 0 && d ==1) output2 = (abs(a3),2)/abs(a1);
						
							}
						}

                        T dataval = sqrt(pow(output1,2)+pow(output2,2)+pow(output3,2));
                        pImage[offsetk+offsetj+ix] = dataval;
                        if(maxfl<dataval) maxfl = dataval;

                    }
;
				}
					
				
			}
					
		}

        T *pImage2 = new T [pagesz];
       if (!pImage2)
       {
           printf("Fail to allocate memory.\n");
           return;
        }
        else
        {
           for(V3DLONG i=0; i<pagesz; i++)
               pImage2[i] = 0;
         }
       for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {
                    if(gsdtdatald[offsetk+offsetj+ix]< 80)
                    {

                        T dataval2 = 255*pImage[offsetk+offsetj+ix]/maxfl;
                        pImage2[offsetk+offsetj+ix] = dataval2;
                    }
                   else
                    pImage2[offsetk+offsetj+ix] = data1d[offsetk+offsetj+ix];
                }


            }

        }
        outimg = pImage2;
		
}

void processImage2(V3DPluginCallback2 &callback, QWidget *parent)
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
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    int c = 1;
    unsigned char * gsdtdata1d = 0;
    int datatype;
    V3DLONG * in_zz = 0;
    //loadImage("/local1/data/Staci/L2-3_PyramidalNeuron_2/gsdtImage.v3draw", gsdtdata1d, in_zz, datatype,0);
    loadImage("/local1/data/Staci/L2-3_PyramidalNeuron_1/gsdtImage_mean.tif", gsdtdata1d, in_zz, datatype,0);

    /*unsigned char * gsdtdata1d2 = 0;
    int datatype2;
    printf("\nload second gsdt image\n");
    loadImage("/local1/data/Staci/gsdt_Diff_GassuainFilteredImage.tif", gsdtdata1d2, in_zz, datatype2,0);*/

     //add input dialog

   /* AdaptiveWindowDialog dialog(callback, parent);
    if (!dialog.image)
        return;

    if (dialog.exec()!=QDialog::Accepted)
        return;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return;
    ROIList pRoiList = dialog.pRoiList;

    int c = dialog.ch+1;
    int th_idx = dialog.th_idx;
    double th = dialog.thresh;*/


 /*   if(th_idx ==0)
    {
        V3DLONG offsetc = (c-1)*pagesz;
        for(V3DLONG iz = 0; iz < P; iz++)
        {
            double PixelSum = 0;
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {

                double PixelVaule = data1d[offsetc + offsetk + offsetj + ix];
               // filter
                PixelSum = PixelSum + PixelVaule;

                }
            }
            th = th + PixelSum/(M*N*P);
        }

    }

    // filter

    saveImage("temp.v3draw", (unsigned char *)data1d, in_sz, pixeltype);
    //invoke gsdt function
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;


    arg.type = "random";std::vector<char*> args1;
   // std:: string inputName(callback.getImageName(curwin).toStdString());char* inputName2 =  new char[inputName.length() + 1]; strcpy(inputName2, inputName.c_str());
    args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    char channel = '0' + (c-1);
    string threshold = boost::lexical_cast<string>(th); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");args.push_back(&channel);args.push_back("1"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("/local1/data/Staci/L2-3_PyramidalNeuron_2/gsdtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * gsdtdata1d = 0;
    int datatype;
    V3DLONG * in_zz = 0;

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    loadImage(outimg_file, gsdtdata1d, in_zz, datatype,0);
    remove("temp.v3draw");
    //remove("/local1/data/Staci/L2-3_PyramidalNeuron_2/gsdtImage.v3draw");*/

    //find local gsdt for soma area


    V3DLONG offsetc = (c-1)*pagesz;
    int soma_x,soma_y;//2901;//2173;
    int soma_z = 0;//2178;//1926;
    LandmarkList soma_center = callback.getLandmark(curwin);
    LocationSimple tmpLocation(0,0,0);
    tmpLocation = soma_center.at(0);
    tmpLocation.getCoord(soma_x,soma_y,soma_z);

    printf("Soma is at (%d %d %d)\n\n",soma_x,soma_y,soma_z);

    V3DLONG i = 0;
    double th_soma = 0;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = soma_y-200; iy <  soma_y+200; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = soma_x-200; ix < soma_x+200; ix++)
            {

             double PixelVaule = data1d[offsetc + offsetk + offsetj + ix];
             PixelSum = PixelSum + PixelVaule;
             i++;
            }
        }
        th_soma = th_soma + PixelSum/(400*400*P);
    }

  //  printf("threshold is %d %d %d\n\n",th_soma,PixelSum,i);

    void* somaarea = 0;
     switch (pixeltype)
     {
     case V3D_UINT8: soma_detection(data1d, in_sz, c,soma_x,soma_y,i,(unsigned char* &)somaarea); break;
     case V3D_UINT16: soma_detection((unsigned short int *)data1d, in_sz, c, soma_x,soma_y,i,(unsigned short int* &)somaarea); break;
     case V3D_FLOAT32: soma_detection((float *)data1d, in_sz, c, soma_x,soma_y,i,(float* &)somaarea);break;
     default: v3d_msg("Invalid data type. Do nothing."); return;
     }
     V3DLONG soma_sz[4];
     soma_sz[0] = 400; soma_sz[1] = 400; soma_sz[2] = P; soma_sz[3] = sc;
     saveImage("temp.v3draw", (unsigned char *)somaarea, soma_sz, pixeltype);
     V3DPluginArgItem arg;
     V3DPluginArgList input;
     V3DPluginArgList output;


     arg.type = "random";std::vector<char*> args1;
     args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
     arg.type = "random";std::vector<char*> args;
     char channel = '0' + (c-1);
     string threshold = boost::lexical_cast<string>(th_soma); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
     args.push_back(threshold2);args.push_back("1");args.push_back(&channel);args.push_back("1"); arg.p = (void *) & args; input << arg;
     arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

     QString full_plugin_name = "gsdt";
     QString func_name = "gsdt";

     callback.callPluginFunc(full_plugin_name,func_name, input,output);

     unsigned char * gsdtsoma = 0;
    // int datatype;
    // V3DLONG * in_zz = 0;

     char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
     loadImage(outimg_file, gsdtsoma, in_zz, datatype,0);
     remove("temp.v3draw");
     remove("gsdtImage.v3draw");



    void* outimg = 0;
    switch (pixeltype)
    {
    case V3D_UINT8: AdpThresholding_adpwindow(data1d, in_sz, c,(unsigned char* &)outimg, gsdtdata1d,gsdtsoma,soma_x,soma_y); break;
    case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)data1d, in_sz, c, (unsigned short int* &)outimg,(unsigned short int *)gsdtdata1d,(unsigned short int *)gsdtsoma,soma_x,soma_y); break;
    case V3D_FLOAT32: AdpThresholding_adpwindow((float *)data1d, in_sz, c, (float* &)outimg,(float *)gsdtdata1d,(float *)gsdtsoma,soma_x,soma_y);break;
    default: v3d_msg("Invalid data type. Do nothing."); return;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)outimg, N, M, P, 1, pixeltype);
    //new4DImage->setData((unsigned char *)gsdtdata1d, 400, 400, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "3D adaptive enhancement result");
    callback.updateImageWindow(newwin);

    return;
}

template <class T> void AdpThresholding_adpwindow(T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  T* &outimg,
                                                  T* gsdtdatald,T* gsdtsoma,
                                                  int x,
                                                  int y)
{


         V3DLONG N = in_sz[0];
         V3DLONG M = in_sz[1];
         V3DLONG P = in_sz[2];
         V3DLONG sc = in_sz[3];
         V3DLONG pagesz = N*M*P;
         int Wx,Wy,Wz;
         V3DLONG offsetc = (c-1)*pagesz;
         int Th_gsdt = 150;
         T *pImage = new T [pagesz];
        if (!pImage)
        {
            printf("Fail to allocate memory.\n");
            return;
         }
         else
         {
            for(V3DLONG i=0; i<pagesz; i++)
                pImage[i] = 0;
          }

        T *pSoma = new T [pagesz];
        if (!pSoma)
       {
           printf("Fail to allocate memory.\n");
           return;
        }
        else
        {
           for(V3DLONG i=0; i<pagesz; i++)
               pSoma[i] = 0;
         }
       int i = 0;
        for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = y-200; iy <  y+200; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = x-200; ix < x+200; ix++)
                {
                    pSoma[offsetk + offsetj + ix] = gsdtsoma[i];
                    i++;
                }
            }
        }

           T maxfl = 0;
         //outimg = pImage;

        for(V3DLONG iz = 0; iz < P; iz++)
        {
            printf("Enhancement : %d %% completed \n", iz);
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {

                    T GsdtValue = gsdtdatald[offsetk + offsetj + ix];
                    T PixelValue = data1d[offsetc+offsetk + offsetj + ix];
                    T SomaValue = pSoma[offsetc+offsetk + offsetj + ix];
                    Wx = (int)round((log(GsdtValue)/log(2)));
                    //printf("%d %d\n",PixelValue,Wx);

                    if (Wx > 0 && PixelValue > 0 && SomaValue <Th_gsdt)
                    {
                        Wy = Wx;
                        Wz = Wx*2;

                        V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                        V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                        V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                        V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                        V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
                        V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

                       // printf("window size is %d\n",Wx);
                        //Seletive approach
                        float  fxx = data1d[offsetk+offsetj+xe]+ data1d[offsetk+offsetj+xb]- 2*data1d[offsetk+offsetj+ix];
                        float fyy = data1d[offsetk+(ye)*N+ix]+data1d[offsetk+(yb)*N+ix]-2*data1d[offsetk+offsetj+ix];
                        float fzz = data1d[(ze)*M*N+offsetj+ix]+data1d[(zb)*M*N+offsetj+ix]- 2*data1d[offsetk+offsetj+ix];

                        float fxy = 0.25*(data1d[offsetk+(ye)*N+xe]+data1d[offsetk+(yb)*N+xb]-data1d[offsetk+(ye)*N+xb]-data1d[offsetk+(yb)*N+xe]);
                        float fxz = 0.25*(data1d[(ze)*M*N+offsetj+xe]+data1d[(zb)*M*N+offsetj+xb]-data1d[(ze)*M*N+offsetj+xb]-data1d[(zb)*M*N+offsetj+xe]);
                        float fyz = 0.25*(data1d[(ze)*M*N+(ye)*N+ix]+data1d[(zb)*M*N+(yb)*N+ix]-data1d[(ze)*M*N+(yb)*N+ix]-data1d[(zb)*M*N+(ye)*N+ix]);

                        Matrix3f A;
                        A << fxx,fxy,fxz,fxy,fyy,fyz,fxz,fyz,fzz;
                        SelfAdjointEigenSolver<Matrix3f> eigensolver(A, false);

                        float a1 = eigensolver.eigenvalues()(0);
                        float a2 = eigensolver.eigenvalues()(1);
                        float a3 = eigensolver.eigenvalues()(2);
                        swapthree(a1, a2, a3);
                        float output1 = 0;
                        float output2 = 0;
                        float output3 = 0;
                        if(a1<0 && a2 < 0)
                        {
                              T dataval =  abs(a2)*(abs(a2)-abs(a3))/abs(a1);
                              pImage[offsetk+offsetj+ix] = dataval;
                              if(maxfl<dataval) maxfl = dataval;
                        }


                    }else
                      pImage[offsetk+offsetj+ix] = 0;
                }
            }
        }

       T *pImage2 = new T [pagesz];
       if (!pImage2)
       {
           printf("Fail to allocate memory.\n");
           return;
        }
        else
        {
           for(V3DLONG i=0; i<pagesz; i++)
               pImage2[i] = 0;
         }
       for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {
                        T SomaValue = pSoma[offsetc+offsetk + offsetj + ix];

                        if(SomaValue < Th_gsdt)
                        {
                            T dataval2 = 255*pImage[offsetk+offsetj+ix]/maxfl;
                            pImage2[offsetk+offsetj+ix] = dataval2;
                        }else
                            pImage2[offsetk+offsetj+ix] =  data1d[offsetk+offsetj+ix];
                }


            }

        }
        outimg = pImage2;
        return;

}
int swapthree(float& dummya, float& dummyb, float& dummyc)
{
 
    if ( (abs(dummya) >= abs(dummyb)) && (abs(dummyb) >= abs(dummyc)) )
    {
        return 1;
    }
 
 
    else if ( (abs(dummya) >= abs(dummyc)) && (abs(dummyc) >= abs(dummyb)) )
 
    {
        float temp = dummyb;
        dummyb = dummyc;
        dummyc = temp;
 
        return 1;
    }
 
    else if ( (abs(dummyb) >= abs(dummya)) && (abs(dummya) >= abs(dummyc)) )
    {
        float temp = dummya;
        dummya = dummyb;
        dummyb = temp;
 
        return 1;
    }
 
 
    else if ( (abs(dummyb) >= abs(dummyc)) && (abs(dummyc) >= abs(dummya)) )
    {
        float temp = dummya;
        dummya = dummyb;
        dummyb = dummyc;
        dummyc = temp;
 
        return 1;
    }
 
 
    else if ( (abs(dummyc) >= abs(dummya)) && (abs(dummya) >= abs(dummyb)) )
    {
        float temp = dummya;
        dummya = dummyc;
        dummyc = dummyb;
        dummyb = temp;
 
        return 1;
    }
 
 
    else if ( (abs(dummyc) >= abs(dummyb)) && (abs(dummyb) >= abs(dummya)) )
    {
        float temp = dummyc;
        dummyc = dummya;
        dummya = temp;
 
        return 1;
    }
 
 
    else
       {
           return 0;
       }
 
}

template <class T> void soma_detection(T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       int x,
                                       int y,
                                       V3DLONG somasize,
                                       T* &outimg)

{


         V3DLONG N = in_sz[0];
         V3DLONG M = in_sz[1];
         V3DLONG P = in_sz[2];
         V3DLONG pagesz = N*M*P;
         V3DLONG offsetc = (c-1)*pagesz;

         T *pImage = new T [somasize];
        if (!pImage)
        {
            printf("Fail to allocate memory.\n");
            return;
         }
         else
         {
            int i = 0;
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG iy = y-200; iy < y+200; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = x-200; ix < x+200; ix++)
                    {

                        T PixelValue = data1d[offsetc+offsetk + offsetj + ix];
                        pImage[i] = PixelValue;
                        i++;
                    }
                }
            }
        }

        outimg = pImage;
       //if(pImage) {delete [] pImage; pImage = 0;}
}

void processImage3(V3DPluginCallback2 &callback, QWidget *parent)
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
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    int c = 1;
    unsigned char * gsdtdata1d = 0;
    int datatype;
    V3DLONG * in_zz = 0;
    loadImage("/local1/data/Staci/L2-3_PyramidalNeuron_2/blended.v3draw", gsdtdata1d, in_zz, datatype,0);

    //find local gsdt for soma area


    V3DLONG offsetc = (c-1)*pagesz;
    int soma_x,soma_y;//2901;//2173;
    int soma_z = 0;//2178;//1926;
    LandmarkList soma_center = callback.getLandmark(curwin);
    LocationSimple tmpLocation(0,0,0);
    tmpLocation = soma_center.at(0);
    tmpLocation.getCoord(soma_x,soma_y,soma_z);

    printf("Soma is at (%d %d %d)\n\n",soma_x,soma_y,soma_z);

    V3DLONG i = 0;
    double th_soma = 0;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = soma_y-200; iy <  soma_y+200; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = soma_x-200; ix < soma_x+200; ix++)
            {

             double PixelVaule = data1d[offsetc + offsetk + offsetj + ix];
             PixelSum = PixelSum + PixelVaule;
             i++;
            }
        }
        th_soma = th_soma + PixelSum/(400*400*P);
    }

  //  printf("threshold is %d %d %d\n\n",th_soma,PixelSum,i);

    void* somaarea = 0;
     switch (pixeltype)
     {
     case V3D_UINT8: soma_detection(data1d, in_sz, c,soma_x,soma_y,i,(unsigned char* &)somaarea); break;
     case V3D_UINT16: soma_detection((unsigned short int *)data1d, in_sz, c, soma_x,soma_y,i,(unsigned short int* &)somaarea); break;
     case V3D_FLOAT32: soma_detection((float *)data1d, in_sz, c, soma_x,soma_y,i,(float* &)somaarea);break;
     default: v3d_msg("Invalid data type. Do nothing."); return;
     }
     V3DLONG soma_sz[4];
     soma_sz[0] = 400; soma_sz[1] = 400; soma_sz[2] = P; soma_sz[3] = sc;
     saveImage("temp.v3draw", (unsigned char *)somaarea, soma_sz, pixeltype);
     V3DPluginArgItem arg;
     V3DPluginArgList input;
     V3DPluginArgList output;


     arg.type = "random";std::vector<char*> args1;
     args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
     arg.type = "random";std::vector<char*> args;
     char channel = '0' + (c-1);
     string threshold = boost::lexical_cast<string>(th_soma); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
     args.push_back(threshold2);args.push_back("1");args.push_back(&channel);args.push_back("1"); arg.p = (void *) & args; input << arg;
     arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

     QString full_plugin_name = "gsdt";
     QString func_name = "gsdt";

     callback.callPluginFunc(full_plugin_name,func_name, input,output);

     unsigned char * gsdtsoma = 0;
    // int datatype;
    // V3DLONG * in_zz = 0;

     char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
     loadImage(outimg_file, gsdtsoma, in_zz, datatype,0);
     remove("temp.v3draw");
     remove("gsdtImage.v3draw");



    void* outimg = 0;
    switch (pixeltype)
    {
    case V3D_UINT8: AdpThresholding_adpwindow_v2(data1d, in_sz, c,(unsigned char* &)outimg, gsdtdata1d,gsdtsoma,soma_x,soma_y); break;
    case V3D_UINT16: AdpThresholding_adpwindow_v2((unsigned short int *)data1d, in_sz, c, (unsigned short int* &)outimg,(unsigned short int *)gsdtdata1d,(unsigned short int *)gsdtsoma,soma_x,soma_y); break;
    case V3D_FLOAT32: AdpThresholding_adpwindow_v2((float *)data1d, in_sz, c, (float* &)outimg,(float *)gsdtdata1d,(float *)gsdtsoma,soma_x,soma_y);break;
    default: v3d_msg("Invalid data type. Do nothing."); return;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)outimg, N, M, P, 1, pixeltype);
    //new4DImage->setData((unsigned char *)gsdtdata1d, 400, 400, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "3D adaptive enhancement result");
    callback.updateImageWindow(newwin);

    return;
}

template <class T> void AdpThresholding_adpwindow_v2(T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  T* &outimg,
                                                  T* gsdtdatald,T* gsdtsoma,
                                                  int x,
                                                  int y)
{

         V3DLONG N = in_sz[0];
         V3DLONG M = in_sz[1];
         V3DLONG P = in_sz[2];
         V3DLONG sc = in_sz[3];
         V3DLONG pagesz = N*M*P;
         V3DLONG offsetc = (c-1)*pagesz;

         int Th_gsdt = 150;

        T *pSoma = new T [pagesz];
        if (!pSoma)
       {
           printf("Fail to allocate memory.\n");
           return;
        }
        else
        {
           for(V3DLONG i=0; i<pagesz; i++)
               pSoma[i] = 0;
         }
       int i = 0;
        for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = y-200; iy <  y+200; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = x-200; ix < x+200; ix++)
                {
                    pSoma[offsetk + offsetj + ix] = gsdtsoma[i];
                    i++;
                }
            }
        }


       T *pImage2 = new T [pagesz];
       if (!pImage2)
       {
           printf("Fail to allocate memory.\n");
           return;
        }
        else
        {
           for(V3DLONG i=0; i<pagesz; i++)
               pImage2[i] = 0;
         }
           for(V3DLONG iz = 0; iz < P; iz++)
            {
                printf("zhi zhou is %d\n",iz);
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG iy = 0; iy < M; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                            T SomaValue = pSoma[offsetc+offsetk + offsetj + ix];

                            if(SomaValue < Th_gsdt)
                            {
                                T dataval2 = gsdtdatald[offsetk+offsetj+ix];
                                pImage2[offsetk+offsetj+ix] = dataval2;
                            }else
                                pImage2[offsetk+offsetj+ix] =  data1d[offsetk+offsetj+ix];
                    }


                }

            }

        outimg = pImage2;
        return;

}
