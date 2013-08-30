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
#include <vector>
#include "stackutil.h"
#include <boost/lexical_cast.hpp>
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"

#define WANT_STREAM       // include iostream and iomanipulators
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

using namespace std;
Q_EXPORT_PLUGIN2(MultisclaeEnhancement, selectiveEnhancement);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);

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

	if (func_name == tr("selectiveEnhancement"))
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


    }
    // display

    double min,max;
    unsigned char* EnahancedImage_final_nomal = 0;
    EnahancedImage_final_nomal = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((unsigned char *)EnahancedImage_final,pagesz,min,max,EnahancedImage_final_nomal);

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final_nomal,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Local_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
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

    outimg = pImage;
    return;
}
