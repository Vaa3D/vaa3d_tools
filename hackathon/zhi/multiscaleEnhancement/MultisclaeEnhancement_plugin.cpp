/* MultisclaeEnhancement_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-29 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "MultisclaeEnhancement_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <math.h>
#include "stackutil.h"
#include <boost/lexical_cast.hpp>
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"

#define WANT_STREAM       // include iostream and iomanipulators
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

using namespace std;
Q_EXPORT_PLUGIN2(MultisclaeEnhancement, selectiveEnhancement);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);
void processImage2(V3DPluginCallback2 &callback, QWidget *parent);
void processImage3(V3DPluginCallback2 &callback, QWidget *parent);
void processImage4(V3DPluginCallback2 &callback, QWidget *parent);

bool processImage3(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);

template <class T> void selectiveEnhancement(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                 double sigma,
                                                  T* &outimg);

template <class T> void callGussianoPlugin(V3DPluginCallback2 &callback,
                                                  V3DLONG pagesz,
                                                  double sigma,
                                                  unsigned int c,
                                                  T* &outimg);

template <class T> void AdpThresholding_adpwindow(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  double sigma,
                                                  T* &outimg, const T* gsdtdata1d,unsigned int dim,double ratio);

template <class T> void callgsdtPlugin(V3DPluginCallback2 &callback,const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  T* &outimg);

template <class T> double getdtmax(V3DPluginCallback2 &callback,const T* data1d,V3DLONG *in_sz);

template <class T> T zhi_abs(T num)
{
    return (num<0) ? -num : num;
}


template <class T> bool swapthree(T& dummya, T& dummyb, T& dummyc)
{

    if ( (zhi_abs(dummya) >= zhi_abs(dummyb)) && (zhi_abs(dummyb) >= zhi_abs(dummyc)) )
    {
    }
    else if ( (zhi_abs(dummya) >= zhi_abs(dummyc)) && (zhi_abs(dummyc) >= zhi_abs(dummyb)) )
    {
        T temp = dummyb;
        dummyb = dummyc;
        dummyc = temp;
    }
    else if ( (zhi_abs(dummyb) >= zhi_abs(dummya)) && (zhi_abs(dummya) >= zhi_abs(dummyc)) )
    {
        T temp = dummya;
        dummya = dummyb;
        dummyb = temp;
    }
    else if ( (zhi_abs(dummyb) >= zhi_abs(dummyc)) && (zhi_abs(dummyc) >= zhi_abs(dummya)) )
    {
        T temp = dummya;
        dummya = dummyb;
        dummyb = dummyc;
        dummyc = temp;
    }
    else if ( (zhi_abs(dummyc) >= zhi_abs(dummya)) && (zhi_abs(dummya) >= zhi_abs(dummyb)) )
    {
        T temp = dummya;
        dummya = dummyc;
        dummyc = dummyb;
        dummyb = temp;
    }
    else if ( (zhi_abs(dummyc) >= zhi_abs(dummyb)) && (zhi_abs(dummyb) >= zhi_abs(dummya)) )
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

 
QStringList selectiveEnhancement::menulist() const
{
	return QStringList() 
        <<tr("adaptive_auto")
        <<tr("adaptive_auto_2D")
        <<tr("adaptive")
		<<tr("selective")
        <<tr("about");
}

QStringList selectiveEnhancement::funclist() const
{
	return QStringList()
		<<tr("selectiveEnhancement")
		<<tr("help");
}

void selectiveEnhancement::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("selective"))
	{
         processImage(callback,parent);
	}
    else if(menu_name == tr("adaptive"))
    {
         processImage2(callback,parent);
    }
    else if(menu_name == tr("adaptive_auto"))
    {
         processImage3(callback,parent);
    }
    else if(menu_name == tr("adaptive_auto_2D"))
    {
         processImage4(callback,parent);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-08-29"));
	}
}

bool selectiveEnhancement::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("adaptive_auto"))
	{
        return processImage3(input, output,callback);
	}
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f adaptive_auto -i <inimg_file> -o <outimg_file> -p <scale> <ch> <ratio>"<<endl;
        cout<<endl;
        cout<<"scale       the iteration time, default 5"<<endl;
        cout<<"ch          the input channel value, start from 1, default 1"<<endl;
        cout<<"ratio       the window size ratio, default 0.1"<<endl;
        cout<<endl;
        cout<<endl;
    }
	else return false;

	return true;
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
     ImagePixelType pixeltype = p4DImage->getDatatype();
     V3DLONG N = p4DImage->getXDim();
     V3DLONG M = p4DImage->getYDim();
     V3DLONG P = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();

     //add input dialog

    MultiscaleEnhancementDialog dialog(callback, parent);
    if (!dialog.image)
        return;

    if (dialog.exec()!=QDialog::Accepted)
        return;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return;
    ROIList pRoiList = dialog.pRoiList;

    double d0 = dialog.d0;
    double d1 = dialog.d1;
    double range = dialog.range;
    int c = dialog.ch;
    double r = pow((d1/d0),1/(range-1));


    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;
    in_sz[3] = c;
    saveImage("temp.v3draw", (unsigned char *)data1d, in_sz, pixeltype);
    int count = 0;
    unsigned char *EnahancedImage_final=0;
    for(int d = 0; d < range; d++)
    {
        double sigma = pow(r,d)*d0/4;
        unsigned char * data1d_gf = 0;
        switch (pixeltype)
        {
        case V3D_UINT8: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf); break;
          case V3D_UINT16: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned short int* &)data1d_gf); break;
          case V3D_FLOAT32: callGussianoPlugin(callback,pagesz,sigma,c, (float* &)data1d_gf);break;
          default: v3d_msg("Invalid data type. Do nothing."); return;
        }


        unsigned char  * EnahancedImage = 0;
        switch (pixeltype)
        {
          case V3D_UINT8: selectiveEnhancement((unsigned char *)data1d_gf, in_sz, c,sigma,(unsigned char* &)EnahancedImage); break;
          case V3D_UINT16: selectiveEnhancement((unsigned short int *)data1d_gf, in_sz, c,sigma, (unsigned short int* &)EnahancedImage); break;
          case V3D_FLOAT32: selectiveEnhancement((float *)data1d_gf, in_sz, c, sigma,(float* &)EnahancedImage);break;
          default: v3d_msg("Invalid data type. Do nothing."); return;
        }


        if (count==0)
        {

            EnahancedImage_final = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
                EnahancedImage_final[i] = EnahancedImage[i];

        }
        else
        {
            unsigned char *EnahancedImage_max=0;
            EnahancedImage_max = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
            {
                int a1 = EnahancedImage[i];
                int a2 = EnahancedImage_final[i];
                if(a1>a2)
                    EnahancedImage_max[i] =  EnahancedImage[i];
                else
                    EnahancedImage_max[i] =  EnahancedImage_final[i];

            }

            EnahancedImage_final = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
                EnahancedImage_final[i] = EnahancedImage_max[i];

        }

        count++;
    }
    // display
    remove("temp.v3draw");
    double min,max;
    unsigned char* EnahancedImage_final_nomal = 0;
    EnahancedImage_final_nomal = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((unsigned char *)EnahancedImage_final,pagesz,min,max,EnahancedImage_final_nomal);

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final_nomal,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_selective_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
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
     ImagePixelType pixeltype = p4DImage->getDatatype();
     V3DLONG N = p4DImage->getXDim();
     V3DLONG M = p4DImage->getYDim();
     V3DLONG P = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();

     //add input dialog

    MultiscaleEnhancementDialog dialog(callback, parent);
    if (!dialog.image)
        return;

    if (dialog.exec()!=QDialog::Accepted)
        return;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return;
    ROIList pRoiList = dialog.pRoiList;

    double d0 = dialog.d0;
    double d1 = dialog.d1;
    double range = dialog.range;
    int c = dialog.ch;
    double r = pow((d1/d0),1/(range-1));

    double ratio = 1;
    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;
    in_sz[3] = c;
    saveImage("temp.v3draw", (unsigned char *)data1d, in_sz, pixeltype);
    int count = 0;
    unsigned char *EnahancedImage_final=0;
    double min,max;
    for(int d = 0; d < range; d++)
    {
        double sigma = pow(r,d)*d0/4;
        unsigned char * data1d_gf = 0;
        switch (pixeltype)
        {
          case V3D_UINT8: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf); break;
          case V3D_UINT16: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned short int* &)data1d_gf); break;
          case V3D_FLOAT32: callGussianoPlugin(callback,pagesz,sigma,c, (float* &)data1d_gf);break;
          default: v3d_msg("Invalid data type. Do nothing."); return;
        }

        unsigned char * gsdtld = 0;
        switch (pixeltype)
        {
            case V3D_UINT8: callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,(unsigned char* &)gsdtld); break;
            case V3D_UINT16: callgsdtPlugin(callback,(unsigned short int *)data1d_gf, in_sz, 1,(unsigned short int* &)gsdtld); break;
            case V3D_FLOAT32: callgsdtPlugin(callback,(float *)data1d_gf, in_sz, 1,(float* &)gsdtld); break;
            default: v3d_msg("Invalid data type. Do nothing."); return;
        }

        unsigned char* EnahancedImage = NULL;

        switch (pixeltype)
        {
        case V3D_UINT8: AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio); break;
        case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)data1d_gf, in_sz, 1, sigma,(unsigned short int* &)EnahancedImage,(unsigned short int *)gsdtld,3,ratio); break;
        case V3D_FLOAT32: AdpThresholding_adpwindow((float *)data1d_gf, in_sz, 1, sigma,(float* &)EnahancedImage,(float *)gsdtld,3,ratio);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
        }


        if (count==0)
        {

            EnahancedImage_final = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
                EnahancedImage_final[i] = EnahancedImage[i];

        }
        else
        {
            unsigned char *EnahancedImage_max=0;
            EnahancedImage_max = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
            {
                int a1 = EnahancedImage[i];
                int a2 = EnahancedImage_final[i];
                if(a1>a2)
                    EnahancedImage_max[i] =  EnahancedImage[i];
                else
                    EnahancedImage_max[i] =  EnahancedImage_final[i];

            }

            EnahancedImage_final = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
                EnahancedImage_final[i] = EnahancedImage_max[i];

        }
        count++;
    }
    // display
    remove("temp.v3draw");
    //double min,max;
    unsigned char* EnahancedImage_final_nomal = 0;
    EnahancedImage_final_nomal = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((unsigned char *)EnahancedImage_final,pagesz,min,max,EnahancedImage_final_nomal);

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final_nomal,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
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
     ImagePixelType pixeltype = p4DImage->getDatatype();
     V3DLONG N = p4DImage->getXDim();
     V3DLONG M = p4DImage->getYDim();
     V3DLONG P = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();

     //add input dialog
     bool ok1,ok2,ok3;
     unsigned int scale = 6, c=1;
     double ratio = 0.1;

     scale = QInputDialog::getInteger(parent, "Iteration Time",
                                       "Enter the maximum iternation time:",
                                       3, 1, 20, 1, &ok1);

     if(ok1)
     {
         if(sc==1)
         {
             c=1;
             ok2=true;
         }
         else
         {
             c = QInputDialog::getInteger(parent, "Channel",
                                          "Enter channel NO:",
                                          1, 1, sc, 1, &ok2);
         }
     }
     else
         return;

     if(ok2)
     {
         ratio = QInputDialog::getDouble(parent, "Ratio",
                                      "Enter window size ratio:",
                                      0.1, 0.1, 1, 1, &ok3);
     }
     else
         return;

     if(ok3 ==false)
         return;


    double maxDT1 = 0.5;
    double maxDT2 = 0.5;
    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;
    in_sz[3] = c;
    V3DLONG offsetc = (c-1)*pagesz;
    saveImage("temp.v3draw", (unsigned char *)data1d, in_sz, pixeltype);

    unsigned char *EnahancedImage_final=0;
    double min,max;
    for(int count = 0; count < scale; count++)
    {

        double sigma = 0.6*maxDT1;
        printf("max in dt is %.2f, sigma is %.2f\n\n\n",maxDT1,sigma);

        unsigned char * data1d_gf = 0;
        if(count==0)
        {
            data1d_gf = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
                data1d_gf[i] = data1d[offsetc+i];
        }
        else
        {
            switch (pixeltype)
            {
              case V3D_UINT8: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf); break;
              case V3D_UINT16: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned short int* &)data1d_gf); break;
              case V3D_FLOAT32: callGussianoPlugin(callback,pagesz,sigma,c, (float* &)data1d_gf);break;
              default: v3d_msg("Invalid data type. Do nothing."); return;
            }
        }

        unsigned char * gsdtld = 0;
        switch (pixeltype)
        {
            case V3D_UINT8: callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,(unsigned char* &)gsdtld); break;
            case V3D_UINT16: callgsdtPlugin(callback,(unsigned short int *)data1d_gf, in_sz, 1,(unsigned short int* &)gsdtld); break;
            case V3D_FLOAT32: callgsdtPlugin(callback,(float *)data1d_gf, in_sz, 1,(float* &)gsdtld); break;
            default: v3d_msg("Invalid data type. Do nothing."); return;
        }

        unsigned char* EnahancedImage = NULL;

        switch (pixeltype)
        {
        case V3D_UINT8: AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio); break;
        case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)data1d_gf, in_sz, 1, sigma,(unsigned short int* &)EnahancedImage,(unsigned short int *)gsdtld,3,ratio); break;
        case V3D_FLOAT32: AdpThresholding_adpwindow((float *)data1d_gf, in_sz, 1, sigma,(float* &)EnahancedImage,(float *)gsdtld,3,ratio);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
        }

        maxDT1 = getdtmax(callback,EnahancedImage,in_sz);

        printf("maxDT1 is %0.2f, maxDT2 is %0.2f, count is %d\n\n\n",maxDT1,maxDT2,count);
       if(count==0)
        {
            maxDT1 = 2;
        }

        if(maxDT1 > maxDT2 && maxDT1>=0)
        {
            if (count==0)
            {

                EnahancedImage_final = new unsigned char [pagesz];
                for(int i = 0; i<pagesz;i++)
                    EnahancedImage_final[i] = EnahancedImage[i];

            }
            else
            {
                unsigned char *EnahancedImage_max=0;
                EnahancedImage_max = new unsigned char [pagesz];
                for(int i = 0; i<pagesz;i++)
                {
                    int a1 = EnahancedImage[i];
                    int a2 = EnahancedImage_final[i];
                    if(a1>a2)
                        EnahancedImage_max[i] =  EnahancedImage[i];
                    else
                        EnahancedImage_max[i] =  EnahancedImage_final[i];

                }

                EnahancedImage_final = new unsigned char [pagesz];
                for(int i = 0; i<pagesz;i++)
                    EnahancedImage_final[i] = EnahancedImage_max[i];

            }


            maxDT2 = maxDT1;
        }
        else
            break;
    }
    // display
    remove("temp.v3draw");
    //double min,max;
    unsigned char* EnahancedImage_final_nomal = 0;
    EnahancedImage_final_nomal = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((unsigned char *)EnahancedImage_final,pagesz,min,max,EnahancedImage_final_nomal);

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final_nomal,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_adaptive_auto_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
}

void processImage4(V3DPluginCallback2 &callback, QWidget *parent)
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
     V3DLONG pagesz_3d = p4DImage->getTotalUnitNumberPerChannel();
     ImagePixelType pixeltype = p4DImage->getDatatype();
     V3DLONG N = p4DImage->getXDim();
     V3DLONG M = p4DImage->getYDim();
     V3DLONG P = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();
     //add input dialog
     bool ok1,ok2,ok3;
     unsigned int scale = 5, c=1;
     double ratio = 0.1;
     scale = QInputDialog::getInteger(parent, "Iteration Time",
                                       "Enter the maximum iternation time:",
                                       5, 1, 20, 1, &ok1);

     if(ok1)
     {
         if(sc==1)
         {
             c=1;
             ok2=true;
         }
         else
         {
             c = QInputDialog::getInteger(parent, "Channel",
                                          "Enter channel NO:",
                                          1, 1, sc, 1, &ok2);
         }
     }
     else
         return;

     if(ok2)
     {
         ratio = QInputDialog::getDouble(parent, "Ratio",
                                      "Enter window size ratio:",
                                      0.1, 1, 1, 1, &ok3);
     }
     else
         return;
     if(ok3 ==false)
         return;
     V3DLONG offsetc = (c-1)*pagesz_3d;
     unsigned char *EnahancedImage_final_3D = 0;
     EnahancedImage_final_3D = new unsigned char [pagesz_3d];
     V3DLONG pagesz  = N*M;
     for(V3DLONG iz = 0; iz < P; iz++)
     {

          unsigned char *data1d2D=0;
          data1d2D =  new unsigned char [M*N];
          V3DLONG offsetk = iz*M*N;
          for(int i = 0; i < N*M; i++)
              data1d2D[i] = data1d[offsetc+offsetk+i];

            double maxDT1 = 2;
            double maxDT2 = 1;
            V3DLONG in_sz[4];
            in_sz[0] = N; in_sz[1] = M; in_sz[2] = 1;in_sz[3] = c;
            saveImage("temp.v3draw", (unsigned char *)data1d2D, in_sz, pixeltype);


            unsigned char *EnahancedImage_final=0;
            for(int count = 0; count < scale; count++)
            {
                double sigma = maxDT1/2;
                unsigned char * data1d_gf = 0;
                switch (pixeltype)
                {
                  case V3D_UINT8: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf); break;
                  case V3D_UINT16: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned short int* &)data1d_gf); break;
                  case V3D_FLOAT32: callGussianoPlugin(callback,pagesz,sigma,c, (float* &)data1d_gf);break;
                  default: v3d_msg("Invalid data type. Do nothing."); return;
                }

                unsigned char * gsdtld = 0;
                switch (pixeltype)
                {
                    case V3D_UINT8: callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,(unsigned char* &)gsdtld); break;
                    case V3D_UINT16: callgsdtPlugin(callback,(unsigned short int *)data1d_gf, in_sz, 1,(unsigned short int* &)gsdtld); break;
                    case V3D_FLOAT32: callgsdtPlugin(callback,(float *)data1d_gf, in_sz, 1,(float* &)gsdtld); break;
                    default: v3d_msg("Invalid data type. Do nothing."); return;
                }

                unsigned char* EnahancedImage = 0;

                switch (pixeltype)
                {
                case V3D_UINT8: AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,2,ratio); break;
                case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)data1d_gf, in_sz, 1, sigma,(unsigned short int* &)EnahancedImage,(unsigned short int *)gsdtld,2,ratio); break;
                case V3D_FLOAT32: AdpThresholding_adpwindow((float *)data1d_gf, in_sz, 1, sigma,(float* &)EnahancedImage,(float *)gsdtld,2,ratio);break;
                default: v3d_msg("Invalid data type. Do nothing."); return;
                }

                maxDT1 = getdtmax(callback,EnahancedImage,in_sz);
                if(maxDT1 > maxDT2 || maxDT1>=0)
                {
                    if (count==0)
                    {

                        EnahancedImage_final = new unsigned char [pagesz];
                        for(int i = 0; i<pagesz;i++)
                            EnahancedImage_final[i] = EnahancedImage[i];

                    }
                    else
                    {
                        unsigned char *EnahancedImage_max=0;
                        EnahancedImage_max = new unsigned char [pagesz];
                        for(int i = 0; i<pagesz;i++)
                        {
                            int a1 = EnahancedImage[i];
                            int a2 = EnahancedImage_final[i];
                            if(a1>a2)
                                EnahancedImage_max[i] =  EnahancedImage[i];
                            else
                                EnahancedImage_max[i] =  EnahancedImage_final[i];

                        }

                        EnahancedImage_final = new unsigned char [pagesz];
                        for(int i = 0; i<pagesz;i++)
                            EnahancedImage_final[i] = EnahancedImage_max[i];

                    }
                    maxDT2 = maxDT1;
                     if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
                      if(gsdtld) {delete []gsdtld; gsdtld =0;}

                    count++;
                }
                else
                    break;
            }
            // display
            remove("temp.v3draw");

           for(int i = 0; i < N*M; i++)
                EnahancedImage_final_3D[offsetc+offsetk+i] = EnahancedImage_final[i];

             if(EnahancedImage_final) {delete []EnahancedImage_final; EnahancedImage_final =0;}
            if(data1d2D) {delete []data1d2D; data1d2D =0;}
     }


    double min,max;
    unsigned char* EnahancedImage_final_nomal = 0;
    EnahancedImage_final_nomal = new unsigned char [pagesz_3d];

    rescale_to_0_255_and_copy((unsigned char *)EnahancedImage_final_3D,pagesz_3d,min,max,EnahancedImage_final_nomal);

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final_nomal,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_adaptive_auto_enhancement_result_2D");
    callback.updateImageWindow(newwin);
    return;
}

bool processImage3(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter"<<endl;
    if (output.size() != 1) return false;
    unsigned int scale = 5, ch=1;
    double ratio = 0.1;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) scale = atoi(paras.at(0));
        if(paras.size() >= 2) ch = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atoi(paras.at(1));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"scale = "<<scale<<endl;
    cout<<"ch = "<<ch<<endl;
    cout<<"ratio = "<<ch<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

    unsigned char * data1d = 0;
    V3DLONG * in_sz = 0;
    unsigned int c = ch;//-1;

    int datatype;
     if(!loadImage(inimg_file, data1d, in_sz, datatype))
     {
         cerr<<"load image "<<inimg_file<<" error!"<<endl;
         return false;
     }

    double maxDT1 = 0.5;
    double maxDT2 = 0.5;
    in_sz[3] = c;
    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    saveImage("temp.v3draw", (unsigned char *)data1d, in_sz, datatype);
    V3DLONG offsetc = (c-1)*pagesz;

    unsigned char *EnahancedImage_final=0;
    double min,max;
    for(int count = 0; count < scale; count++)
    {

        double sigma = 0.6*maxDT1;
        printf("max in dt is %.2f, sigma is %.2f\n\n\n",maxDT1,sigma);

        unsigned char * data1d_gf = 0;
        if(count==0)
        {
            data1d_gf = new unsigned char [pagesz];
            for(int i = 0; i<pagesz;i++)
                data1d_gf[i] = data1d[offsetc+i];
        }
        else
        {
            switch (datatype)
            {
              case V3D_UINT8: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf); break;
              case V3D_UINT16: callGussianoPlugin(callback,pagesz,sigma,c,(unsigned short int* &)data1d_gf); break;
              case V3D_FLOAT32: callGussianoPlugin(callback,pagesz,sigma,c, (float* &)data1d_gf);break;
              default: v3d_msg("Invalid data type. Do nothing."); return false;
            }
        }

        unsigned char * gsdtld = 0;
        switch (datatype)
        {
            case V3D_UINT8: callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,(unsigned char* &)gsdtld); break;
            case V3D_UINT16: callgsdtPlugin(callback,(unsigned short int *)data1d_gf, in_sz, 1,(unsigned short int* &)gsdtld); break;
            case V3D_FLOAT32: callgsdtPlugin(callback,(float *)data1d_gf, in_sz, 1,(float* &)gsdtld); break;
            default: v3d_msg("Invalid data type. Do nothing."); return false;
        }

        unsigned char* EnahancedImage = NULL;
        switch (datatype)
        {
        case V3D_UINT8: AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio); break;
        case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)data1d_gf, in_sz, 1, sigma,(unsigned short int* &)EnahancedImage,(unsigned short int *)gsdtld,3,ratio); break;
        case V3D_FLOAT32: AdpThresholding_adpwindow((float *)data1d_gf, in_sz, 1, sigma,(float* &)EnahancedImage,(float *)gsdtld,3,ratio);break;
        default: v3d_msg("Invalid data type. Do nothing."); return false;
        }

        maxDT1 = getdtmax(callback,EnahancedImage,in_sz);

        if(count==0)
         {
             maxDT1 = 2;
         }

        if(maxDT1 > maxDT2 || maxDT1>=0)
        {
            if (count==0)
            {

                EnahancedImage_final = new unsigned char [pagesz];
                for(int i = 0; i<pagesz;i++)
                    EnahancedImage_final[i] = EnahancedImage[i];

            }
            else
            {
                unsigned char *EnahancedImage_max=0;
                EnahancedImage_max = new unsigned char [pagesz];
                for(int i = 0; i<pagesz;i++)
                {
                    int a1 = EnahancedImage[i];
                    int a2 = EnahancedImage_final[i];
                    if(a1>a2)
                        EnahancedImage_max[i] =  EnahancedImage[i];
                    else
                        EnahancedImage_max[i] =  EnahancedImage_final[i];

                }

                EnahancedImage_final = new unsigned char [pagesz];
                for(int i = 0; i<pagesz;i++)
                    EnahancedImage_final[i] = EnahancedImage_max[i];

            }
            maxDT2 = maxDT1;
        }
        else
            break;

        if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
        if(gsdtld) {delete []gsdtld; gsdtld =0;}
    }

    remove("temp.v3draw");

    unsigned char* EnahancedImage_final_nomal = 0;
    EnahancedImage_final_nomal = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((unsigned char *)EnahancedImage_final,pagesz,min,max,EnahancedImage_final_nomal);
    saveImage(outimg_file, (unsigned char *)EnahancedImage_final_nomal, in_sz, 1);

    if(EnahancedImage_final) {delete []EnahancedImage_final; EnahancedImage_final =0;}
    if(in_sz) {delete []in_sz; in_sz =0;}


    return true;
}


template <class T> void selectiveEnhancement(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  double sigma,
                                                  T* &outimg)
{
    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    int Wx = 1,Wy=1,Wz=1;
    V3DLONG offsetc = (c-1)*pagesz;

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


    for(V3DLONG iz = 0; iz < P; iz++)
    {
        printf("\r Enhancement : %d %% completed ", int(float(iz)/P*100)); fflush(stdout);
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                    V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                    V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                    V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                    V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                    V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
                    V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;
                          //Seletive approach
                        double fxx = data1d[offsetc+offsetk+offsetj+xe]+ data1d[offsetk+offsetj+xb]- 2*data1d[offsetc+offsetk+offsetj+ix];
                        double fyy = data1d[offsetc+offsetk+(ye)*N+ix]+data1d[offsetc+offsetk+(yb)*N+ix]-2*data1d[offsetc+offsetk+offsetj+ix];
                        double fzz = data1d[offsetc+(ze)*M*N+offsetj+ix]+data1d[offsetc+(zb)*M*N+offsetj+ix]- 2*data1d[offsetc+offsetk+offsetj+ix];

                        double fxy = 0.25*(data1d[offsetc+offsetk+(ye)*N+xe]+data1d[offsetk+(yb)*N+xb]-data1d[offsetc+offsetk+(ye)*N+xb]-data1d[offsetc+offsetk+(yb)*N+xe]);
                        double fxz = 0.25*(data1d[offsetc+(ze)*M*N+offsetj+xe]+data1d[(zb)*M*N+offsetj+xb]-data1d[offsetc+(ze)*M*N+offsetj+xb]-data1d[offsetc+(zb)*M*N+offsetj+xe]);
                        double fyz = 0.25*(data1d[offsetc+(ze)*M*N+(ye)*N+ix]+data1d[(zb)*M*N+(yb)*N+ix]-data1d[offsetc+(ze)*M*N+(yb)*N+ix]-data1d[offsetc+(zb)*M*N+(ye)*N+ix]);

                        SymmetricMatrix Cov_Matrix(3);
                        Cov_Matrix.Row(1) << fxx;
                        Cov_Matrix.Row(2) << fxy << fyy;
                        Cov_Matrix.Row(3) << fxz << fyz <<fzz;

                        DiagonalMatrix DD;
                        EigenValues(Cov_Matrix,DD);
                        double a1 = DD(1), a2 = DD(2), a3 = DD(3);
                        swapthree(a1, a2, a3);
                        if(a1<0 && a2 < 0)
                        {
                            T dataval = (T)(sigma*sigma*zhi_abs(a2)*(zhi_abs(a2)-zhi_abs(a3))/zhi_abs(a1));
                            pImage[offsetk+offsetj+ix] = dataval;
                        }

            }
        }

    }


    outimg = pImage;
    return;

}


template <class T> void callGussianoPlugin(V3DPluginCallback2 &callback,
                                                  V3DLONG pagesz,
                                                  double sigma,
                                                  unsigned int c,
                                                  T* &outimg)
{
    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }
    //Set guaasin parameters
    int ws = (int)ceil(3*sigma);

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;


    arg.type = "random";std::vector<char*> args1;
    args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    char channel = '0' + c;
    string winx2 = boost::lexical_cast<string>(ws); char* winx =  new char[winx2.length() + 1]; strcpy(winx, winx2.c_str());
    string sig2 = boost::lexical_cast<string>(sigma); char* sig =  new char[sig2.length() + 1]; strcpy(sig, sig2.c_str());

    args.push_back(winx);args.push_back(winx);args.push_back(winx);args.push_back(&channel); args.push_back(sig); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_Gf = 0;
    int datatype;
    V3DLONG * in_zz = 0;

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    loadImage(outimg_file, data1d_Gf, in_zz, datatype);
    remove("gfImage.v3draw");
    unsigned char* data1d = 0;
    data1d = new unsigned char [pagesz];

    double min,max;
    rescale_to_0_255_and_copy((float *)data1d_Gf,pagesz,min,max,data1d);

    T *pImage = new T [pagesz];
    if (!pImage)
    {
        printf("Fail to allocate memory.\n");
        return;
    }
    else
    {
        for(V3DLONG i=0; i<pagesz; i++)
            pImage[i] = data1d[i];
    }
    if(data1d_Gf) {delete []data1d_Gf; data1d_Gf =0;}
    if(data1d) {delete []data1d; data1d =0;}

    outimg = pImage;
    return;
}

template <class T> void AdpThresholding_adpwindow(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  double sigma,
                                                  T* &outimg, const T* gsdtdatald,unsigned int dim,double ratio)
{
    if (!in_sz)
    {
        v3d_msg("Invalid input size information");
        return;
    }

    if (c>in_sz[3] || c<1)
    {
        v3d_msg("Invalid channel information");
        return;
    }

    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }


    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M*P;
    int Wx,Wy,Wz;
    V3DLONG offsetc = (c-1)*pagesz;

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

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        printf("\r Enhancement : %d %% completed ", int(float(iz)/P*100)); fflush(stdout);
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

                T GsdtValue = gsdtdatald[offsetk + offsetj + ix];
                T PixelValue = data1d[offsetc+offsetk + offsetj + ix];
                Wx = (int)round((ratio*log(GsdtValue)/log(2)));
                if(ratio<0.5)
                {
                    if(Wx == 0) Wx = 1;
                    Wy = Wx;
                    Wz = Wx;
                }
                else
                {
                    Wy = Wx;
                    Wz = 2*Wx;
                }


                if (Wx > 0 && PixelValue > 0)
                {

                   /* V3DLONG xb = ix-Wx;
                    V3DLONG xe = ix+Wx;
                    V3DLONG yb = iy-Wy;
                    V3DLONG ye = iy+Wy;
                    V3DLONG zb = iz-Wz;
                    V3DLONG ze = iz+Wz;

                    double xleft,xright,yleft,yright,zleft,zright;
                    double xyleft,xyright,xleftyright,xrightyleft,xzleft,xzright,xleftzright,xrightzleft,yzleft,yzright,yleftzright,yrightzleft;
                    if(xb<0)
                         xleft = 0;
                    else
                          xleft = data1d[offsetk+offsetj+xb];
                    if(xe>=N-1)
                         xright = 0;
                    else
                         xright = data1d[offsetk+offsetj+xe];
                    if(yb<0)
                         yleft = 0;
                    else
                         yleft = data1d[offsetk+(yb)*N+ix];
                    if(ye >= M-1)
                         yright = 0;
                    else
                         yright = data1d[offsetk+(ye)*N+ix];
                    if(zb < 0)
                         zleft = 0;
                    else
                         zleft = data1d[(zb)*M*N+offsetj+ix];
                    if(ze >= P-1)
                         zright = 0;
                    else
                         zright = data1d[(ze)*M*N+offsetj+ix];


                    // printf("window size is %d\n",Wx);
                    //Seletive approach

                    double fxx = xleft + xright - 2*data1d[offsetk+offsetj+ix];
                    double fyy = yleft + yright - 2*data1d[offsetk+offsetj+ix];
                    double fzz = zleft + zright - 2*data1d[offsetk+offsetj+ix];

                    if(xb < 0 || yb < 0)
                         xyleft = 0;
                    else
                         xyleft = data1d[offsetk+(yb)*N+xb];
                    if(xe > N-1 || ye > M-1)
                         xyright = 0;
                    else
                         xyright = data1d[offsetk+(ye)*N+xe];
                    if(xb < 0 || ye > M-1)
                         xleftyright = 0;
                    else
                         xleftyright = data1d[offsetk+(ye)*N+xb];
                    if(xe > N-1 || yb < 0)
                         xrightyleft = 0;
                    else
                         xrightyleft =  data1d[offsetk+(yb)*N+xe];

                    double fxy = 0.25*(xyright + xyleft - xleftyright -xrightyleft);

                    if(xb < 0 || zb < 0)
                         xzleft = 0;
                    else
                         xzleft = data1d[(zb)*M*N+offsetj+xb];
                    if(xe > N-1 || ze > P-1)
                         xzright = 0;
                    else
                         xzright = data1d[(ze)*M*N+offsetj+xe];
                    if(xb <0 || ze > P-1)
                         xleftzright = 0;
                    else
                         xleftzright = data1d[(ze)*M*N+offsetj+xb];
                    if(xe > N-1 || zb < 0)
                         xrightzleft = 0;
                    else
                         xrightzleft = data1d[(zb)*M*N+offsetj+xe];

                    double fxz = 0.25*(xzright + xzleft - xleftzright -xrightzleft);

                    if(yb <0 || zb <0)
                         yzleft = 0;
                    else
                         yzleft = data1d[(zb)*M*N+(yb)*N+ix];
                    if(ye > M-1 || ze > P-1)
                         yzright = 0;
                    else
                         yzright = data1d[(ze)*M*N+(ye)*N+ix];
                    if(yb <0 || ze >= P-1)
                         yleftzright = 0;
                    else
                         yleftzright = data1d[(ze)*M*N+(yb)*N+ix];
                    if(ye > M-1 || zb < 0)
                         yrightzleft = 0;
                    else
                         yrightzleft = data1d[(zb)*M*N+(ye)*N+ix];
                    double fyz = 0.25*(yzright + yzleft - yleftzright - yrightzleft);*/

                    V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                    V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                    V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                    V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                    V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
                    V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

                   // printf("window size is %d\n",Wx);
                    //Seletive approach
                    double  fxx = data1d[offsetk+offsetj+xe]+ data1d[offsetk+offsetj+xb]- 2*data1d[offsetk+offsetj+ix];
                    double fyy = data1d[offsetk+(ye)*N+ix]+data1d[offsetk+(yb)*N+ix]-2*data1d[offsetk+offsetj+ix];
                    double fzz = data1d[(ze)*M*N+offsetj+ix]+data1d[(zb)*M*N+offsetj+ix]- 2*data1d[offsetk+offsetj+ix];

                    double fxy = 0.25*(data1d[offsetk+(ye)*N+xe]+data1d[offsetk+(yb)*N+xb]-data1d[offsetk+(ye)*N+xb]-data1d[offsetk+(yb)*N+xe]);
                    double fxz = 0.25*(data1d[(ze)*M*N+offsetj+xe]+data1d[(zb)*M*N+offsetj+xb]-data1d[(ze)*M*N+offsetj+xb]-data1d[(zb)*M*N+offsetj+xe]);
                    double fyz = 0.25*(data1d[(ze)*M*N+(ye)*N+ix]+data1d[(zb)*M*N+(yb)*N+ix]-data1d[(ze)*M*N+(yb)*N+ix]-data1d[(zb)*M*N+(ye)*N+ix]);

                    if(dim==3)
                    {
                        SymmetricMatrix Cov_Matrix(3);
                        Cov_Matrix.Row(1) << fxx;
                        Cov_Matrix.Row(2) << fxy << fyy;
                        Cov_Matrix.Row(3) << fxz << fyz <<fzz;

                        DiagonalMatrix DD;

                        EigenValues(Cov_Matrix,DD);
                        double a1 = DD(1), a2 = DD(2), a3 = DD(3);
                        swapthree(a1, a2, a3);
                        if(a1<0 && a2 < 0)
                        {
                            T dataval = (T)(sigma*sigma*pow((zhi_abs(a2)-zhi_abs(a3)),3)/zhi_abs(a1)); //seems the best at this moment. commented by HP, 2013-08-28. Do NOT Use the following formula instead.
                            //T dataval = (T)(double(PixelValue) * (double(zhi_abs(a2))-double(zhi_abs(a3)))/double(zhi_abs(a1)));

                            pImage[offsetk+offsetj+ix] = dataval;
                            if (maxfl<dataval) maxfl = dataval;
                        }
                    }
                    else
                    {
                        SymmetricMatrix Cov_Matrix(2);
                        Cov_Matrix.Row(1) << fxx;
                        Cov_Matrix.Row(2) << fxy << fyy;

                        DiagonalMatrix DD;
                        EigenValues(Cov_Matrix,DD);
                        double a1 = DD(1), a2 = DD(2);
                        if(zhi_abs(a1)<zhi_abs(a2))
                        {
                                double temp = a2;
                                a2 = a1;
                                a1 = temp;
                        }

                        if(a1<0)
                        {

                            T dataval = (T)(sigma*sigma*pow((zhi_abs(a1)-zhi_abs(a2)),3));
                            pImage[offsetk+offsetj+ix] = dataval;
                            if (maxfl<dataval) maxfl = dataval;
                        }


                    }

                }
                else
                    pImage[offsetk+offsetj+ix] = 0;
            }
        }
    }


 /*   T *pImage2 = new T [pagesz];
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

    if (maxfl>0)
    {
        for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {
                    T dataval2 = 255*pImage[offsetk+offsetj+ix]/maxfl;
                    pImage2[offsetk+offsetj+ix] = dataval2;

                }
            }
        }
    }*/


    outimg = pImage;
    return;

}


template <class T> void callgsdtPlugin(V3DPluginCallback2 &callback,const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  T* &outimg)
{
    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    //Set gsdt parameters
    double th = 0;
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy <  M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

                double PixelVaule = data1d[offsetc+offsetk + offsetj + ix];
                PixelSum = PixelSum + PixelVaule;
            }
        }
        th += + PixelSum/(M*N*P);
    }

    saveImage("temp_gf.v3draw", (unsigned char *)data1d, in_sz, 1);
    th = 1.5*th;
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("temp_gf.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    string threshold = boost::lexical_cast<string>(th); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_gsdt = 0;
    int datatype;
    V3DLONG * in_zz = 0;

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    loadImage(outimg_file, data1d_gsdt, in_zz, datatype);
    remove("temp_gf.v3draw");
    remove("gsdtImage.v3draw");

    T *pImage = new T [pagesz];
    if (!pImage)
    {
        printf("Fail to allocate memory.\n");
        return;
    }
    else
    {
        for(V3DLONG i=0; i<pagesz; i++)
            pImage[i] = data1d_gsdt[i];
    }

    outimg = pImage;
    return;

}

template <class T> double getdtmax(V3DPluginCallback2 &callback,const T* data1d,V3DLONG *in_sz)
{
    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    saveImage("temp_enhanced.v3draw", (unsigned char *)data1d, in_sz, 1);

    double th = 0;
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

            V3DLONG PixelVaule = data1d[offsetk + offsetj + ix];
            PixelSum = PixelSum + PixelVaule;

            }
        }
            th = th + PixelSum/(M*N*P);
    }

    th = th*1.6;
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("temp_enhanced.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    string threshold = boost::lexical_cast<string>(th); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back("1");args.push_back("1");args.push_back("0");args.push_back("0");args.push_back("0");args.push_back(threshold2); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "Fast_Distance";
    QString func_name = "dt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_gt = 0;
    int datatype;
    V3DLONG * in_zz = 0;

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    loadImage(outimg_file, data1d_gt, in_zz, datatype);
    remove("temp_enhanced.v3draw");
    //remove("gtImage.v3draw");
    double maxfl = 0;
    for(V3DLONG i=0; i<pagesz; i++)
    {
        double dataValue = double(data1d_gt[i]);
        if(dataValue > maxfl)   maxfl = dataValue;

    }
    printf("zhi zhou is %.2f",maxfl);

    return maxfl;


}
