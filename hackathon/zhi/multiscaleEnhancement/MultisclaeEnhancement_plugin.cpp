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
void processImage_selective(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive_auto(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive_auto_2D(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive_auto_blocks(V3DPluginCallback2 &callback, QWidget *parent);

bool processImage_adaptive_auto(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);
bool processImage_adaptive_auto_blocks(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);

template <class T> void selectiveEnhancement(const T* data1d,
                                             V3DLONG *in_sz,
                                             unsigned int c,
                                             double sigma,
                                             T* &outimg);

template <class T> void callGaussianPlugin(V3DPluginCallback2 &callback,
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
                                       double th_global,
                                       T* &outimg);

template <class T> void enhancementWithsoma(V3DPluginCallback2 &callback,
                                            const T* data1d,
                                            T* data1d_enhanced,
                                            V3DLONG *in_sz,
                                            unsigned int c,
                                            T* &outimg);

template <class T> void somalocation(V3DPluginCallback2 &callback,
                                     const T* data1d,
                                     V3DLONG *in_sz,
                                     unsigned int c,
                                     int &soma_x,
                                     int &soma_y);

template <class T> void soma_detection(T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       int x,
                                       int y,
                                       V3DLONG somasize,
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
template <class SDATATYPE> int pwi_fusing(SDATATYPE *data1d, SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, V3DLONG *offset, V3DLONG new_sz0, V3DLONG new_sz1, V3DLONG new_sz2);


QStringList selectiveEnhancement::menulist() const
{
    return QStringList()
            <<tr("adaptive_auto")
           <<tr("adaptive_auto_2D")
          <<tr("adaptive_auto_blocks")
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
        processImage_selective(callback,parent);
    }
    else if(menu_name == tr("adaptive"))
    {
        processImage_adaptive(callback,parent);
    }
    else if(menu_name == tr("adaptive_auto"))
    {
        processImage_adaptive_auto(callback,parent);
    }
    else if(menu_name == tr("adaptive_auto_2D"))
    {
        processImage_adaptive_auto_2D(callback,parent);
    }
    else if(menu_name == tr("adaptive_auto_blocks"))
    {
        processImage_adaptive_auto_blocks(callback,parent);
    }
    else
    {
        v3d_msg(QString("Multi-scale enhancement of an image with fiber structures. v%1"
                        "Developed by Zhi Zhou and Hanchuan Peng (2013)").arg(getPluginVersion()));
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
        return processImage_adaptive_auto(input, output,callback);
    }
    else if (func_name == tr("adaptive_auto_block"))
    {
        return processImage_adaptive_auto_blocks(input, output,callback);
    }
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f adaptive_auto -i <inimg_file> -o <outimg_file> -p <scale> <ch> <ratio> <soma>"<<endl;
        cout<<endl;
        cout<<"scale       the iteration time, default 2"<<endl;
        cout<<"ch          the input channel value, start from 1, default 1"<<endl;
        cout<<"ratio       the window size calibration ratio, default 1"<<endl;
        cout<<"soma        soma detection, 1: detect, 0: not detect, default 0"<<endl;
        cout<<endl;
        cout<<endl;

        cout<<"Usage : v3d -x dllname -f adaptive_auto_block -i <inimg_file> -o <outimg_file> -p <ws> <ch> <ratio> <soma>" <<endl;
        cout<<endl;
        cout<<"ws          block window size (pixel #), default 1000"<<endl;
        cout<<"ch          the input channel value, start from 1, default 1"<<endl;
        cout<<"ratio       the window size calibration ratio, default 1"<<endl;
        cout<<"soma        soma detection, 1: detect, 0: not detect, default 0"<<endl;
        cout<<endl;
        cout<<endl;;
    }
    else return false;

    return true;
}

void processImage_selective(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
    ImagePixelType pixeltype = p4DImage->getDatatype();

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
    double d0 = dialog.d0;
    double d1 = dialog.d1;
    double range = dialog.range;
    int c = dialog.ch;
    double r = pow((d1/d0),1/(range-1));

    V3DLONG in_sz[4];
    in_sz[0] = p4DImage->getXDim();
    in_sz[1] = p4DImage->getYDim();
    in_sz[2] = p4DImage->getZDim();
    in_sz[3] = 1;

    simple_saveimage_wrapper(callback, "temp.v3draw",  (unsigned char *)p4DImage->getRawDataAtChannel(c-1), in_sz, pixeltype);
    int count = 0;
    unsigned char *EnahancedImage_final=0;
    try {EnahancedImage_final = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for EnahancedImage_final."); return;}


    for(int d = 0; d < range; d++)
    {
        double sigma = pow(r,d)*d0/2;
        unsigned char * data1d_gf = 0;
        unsigned char  * EnahancedImage = 0;
        switch (pixeltype)
        {
        case V3D_UINT8:
            callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);
            selectiveEnhancement((unsigned char *)data1d_gf, in_sz, c,sigma,(unsigned char* &)EnahancedImage);
            break;
            default: v3d_msg("Invalid data type. Do nothing."); return;
        }

        if (count==0)
        {

           memcpy(EnahancedImage_final, EnahancedImage, pagesz);
        }
        else
        {
            for(V3DLONG i = 0; i<pagesz; i++)
            {
                if (EnahancedImage_final[i] < EnahancedImage[i])
                    EnahancedImage_final[i] = EnahancedImage[i];
            }

        }

        if (EnahancedImage) {delete []EnahancedImage; EnahancedImage=0;}
        if (data1d_gf) { delete []data1d_gf; data1d_gf=0;}
        count++;
    }

    remove("temp.v3draw");
   //set up output image

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final,in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_selective_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
}

void processImage_adaptive(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }


    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
    ImagePixelType pixeltype = p4DImage->getDatatype();

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
    in_sz[0] = p4DImage->getXDim();
    in_sz[1] = p4DImage->getYDim();
    in_sz[2] = p4DImage->getZDim();
    in_sz[3] = 1;

    simple_saveimage_wrapper(callback, "temp.v3draw", (unsigned char *)p4DImage->getRawDataAtChannel(c-1), in_sz, pixeltype);
    int count = 0;
    unsigned char *EnahancedImage_final=0;
    try {EnahancedImage_final = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for EnahancedImage_final."); return;}
    for(int d = 0; d < range; d++)
    {
        double sigma = pow(r,d)*d0/4;
        unsigned char * data1d_gf = 0;
        unsigned char * gsdtld = 0;
        unsigned char* EnahancedImage = 0;
        switch (pixeltype)
        {
        case V3D_UINT8:
            callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);
            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld);
            AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio); break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
        }

        if (count==0)
        {

           memcpy(EnahancedImage_final, EnahancedImage, pagesz);
        }
        else
        {
            for(V3DLONG i = 0; i<pagesz; i++)
            {
                if (EnahancedImage_final[i] < EnahancedImage[i])
                    EnahancedImage_final[i] = EnahancedImage[i];
            }

        }

        if (EnahancedImage) {delete []EnahancedImage; EnahancedImage=0;}
        if (data1d_gf) { delete []data1d_gf; data1d_gf=0;}
        if (gsdtld) { delete []gsdtld; gsdtld=0;}
        count++;
    }

    remove("temp.v3draw");

    //set up output image
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final,in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
}

#define FREE_ENHANCED_IMAGES3 \
    { \
        if (EnahancedImage) {delete []EnahancedImage; EnahancedImage=0;}\
        if (gsdtld) {delete []gsdtld; gsdtld=0;} \
        if (data1d_gf) { delete []data1d_gf; data1d_gf=0;}\
    }

void processImage_adaptive_auto(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
    ImagePixelType pixeltype = p4DImage->getDatatype();

    //add input dialog
    bool ok1=false,ok2=false,ok3=false;
    unsigned int scale = 6, c=1, p = 0;
    double ratio = 0.1;

    scale = QInputDialog::getInteger(parent, "Iteration Time",
                                     "Enter the maximum iternation time:",
                                     2, 1, 20, 1, &ok1);

    if(ok1)
    {
        if(p4DImage->getCDim()==1)
        {
            c=1;
            ok2=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, p4DImage->getCDim(), 1, &ok2);
        }
    }
    else
        return;

    if(ok2)
    {
        ratio = QInputDialog::getDouble(parent, "Calibration Ratio",
                                        "Enter window size calibration ratio:",
                                        0.1, 0.1, 3, 1, &ok3);
    }
    else
        return;

    if(!ok3)
        return;

    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Include soma detection?"), QMessageBox::Yes, QMessageBox::No))    p = 1;


    double sigma = 0.3;
    V3DLONG in_sz[4];
    in_sz[0] = p4DImage->getXDim();
    in_sz[1] = p4DImage->getYDim();
    in_sz[2] = p4DImage->getZDim();
    in_sz[3] = 1;

    V3DLONG offsetc = (c-1)*pagesz;
    simple_saveimage_wrapper(callback, "temp.v3draw", (unsigned char *)p4DImage->getRawDataAtChannel(c-1), in_sz, pixeltype); //TODO: Zhi: correct the original bug by using getRawDataAtChannel()!

    unsigned char *EnahancedImage_final=0;
    try {EnahancedImage_final = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for EnahancedImage_final."); return;}

    for(unsigned int count = 0; count < scale; count++)
    {

        //Gaussian smoothing

        unsigned char * data1d_gf = 0;
        unsigned char * gsdtld = 0;
        unsigned char* EnahancedImage = 0;

        switch (pixeltype)
        {
        case V3D_UINT8:
            if (count==0 && ratio == 0.1) //do not filter for the scale 0
            {

                data1d_gf = new unsigned char [pagesz];
                for(int i = 0; i<pagesz;i++)
                    data1d_gf[i] = data1d[offsetc+i];
            }
            else
                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);

            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld);
            AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
            break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
        }

         if (count==0)
                memcpy(EnahancedImage_final, EnahancedImage, pagesz);
            else
            {
                for(V3DLONG i = 0; i<pagesz; i++)
                {
                    if (EnahancedImage_final[i] < EnahancedImage[i])
                        EnahancedImage_final[i] = EnahancedImage[i];
                }
            }
        sigma += 1.5;
        //free all intermediate variables
        FREE_ENHANCED_IMAGES3
    }

    // display
    remove("temp.v3draw");

    unsigned char* Enhancement_soma = 0;
    if(p==1)
    {
        enhancementWithsoma(callback,(unsigned char *)p4DImage->getRawDataAtChannel(c-1),(unsigned char*)EnahancedImage_final,in_sz,1,(unsigned char *&)Enhancement_soma);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, EnahancedImage_final, pagesz);
    }

    simple_saveimage_wrapper(callback,"result_woGf.v3draw", (unsigned char *)Enhancement_soma, in_sz, 1);
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("result_woGf.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage_v2.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_float = 0;
    int datatype;
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    if(!simple_loadimage_wrapper(callback,outimg_file, data1d_float, in_zz, datatype))
    {
        cerr<<"load image "<<outimg_file<<" error!"<<endl;
        return;
    }
    remove("result_woGf.v3draw");
    remove("gfImage_v2.v3draw");

    double min,max;
    unsigned char* data1d_uint8 = 0;
    data1d_uint8 = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);

    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}
    if (data1d_float) { delete []data1d_float; data1d_float=0;}
    if (EnahancedImage_final) { delete []EnahancedImage_final; EnahancedImage_final=0;}
    //set up output image

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_uint8,in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_adaptive_auto_enhancement_result");
    callback.updateImageWindow(newwin);

    return;
}

void processImage_adaptive_auto_2D(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
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
    bool ok1=false,ok2=false,ok3=false;
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
    if(!ok3)
        return;

    V3DLONG offsetc = (c-1)*pagesz_3d;
    unsigned char *EnahancedImage_final_3D = 0;
    try {EnahancedImage_final_3D = new unsigned char [pagesz_3d];}
    catch(...)  {v3d_msg("cannot allocate memory for EnahancedImage_final_3D."); return;}

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
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = 1;in_sz[3] = 1;
        simple_saveimage_wrapper(callback, "temp.v3draw", (unsigned char *)data1d2D, in_sz, pixeltype);


        unsigned char *EnahancedImage_final=0;
        for(unsigned int  count = 0; count < scale; count++)
        {
            double sigma = maxDT1/2;
            unsigned char * data1d_gf = 0;
            unsigned char * gsdtld = 0;
            unsigned char* EnahancedImage = 0;

            switch (pixeltype)
            {
            case V3D_UINT8:
                     callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);
                     callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld);
                     AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,2,ratio); break;
            default: v3d_msg("Invalid data type. Do nothing."); return;
            }

            maxDT1 = getdtmax(callback,EnahancedImage,in_sz);
            if(maxDT1 > maxDT2)
            {
                if (count==0)
                       memcpy(EnahancedImage_final, EnahancedImage, pagesz);
                   else
                   {
                       for(V3DLONG i = 0; i<pagesz; i++)
                       {
                           if (EnahancedImage_final[i] < EnahancedImage[i])
                               EnahancedImage_final[i] = EnahancedImage[i];
                       }
                   }

                maxDT2 = maxDT1;
                if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
                if(gsdtld) {delete []gsdtld; gsdtld =0;}
                if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
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

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)EnahancedImage_final_3D,N, M, P, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Multiscale_adaptive_auto_enhancement_result_2D");
    callback.updateImageWindow(newwin);
    return;
}

void processImage_adaptive_auto_blocks(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    ImagePixelType pixeltype = p4DImage->getDatatype();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = 1;


    //input
    bool ok1 = false,ok2 = false,ok3 = false;
    unsigned int Ws=1000, c=1,p=0;
    double ratio = 1.0;
    if(N < M)
        Ws = QInputDialog::getInteger(parent, "Block Size",
                                      "Enter block size:",
                                      1000, 1, N, 1, &ok1);
    else
        Ws = QInputDialog::getInteger(parent, "Block Size",
                                      "Enter block size:",
                                      1000, 1, M, 1, &ok1);
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
        ratio = QInputDialog::getDouble(parent, "Calibration Ratio",
                                        "Enter window size calibration ratio:",
                                        1, 0.1, 3, 1, &ok3);
    }
    else
        return;

    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Include soma detection?"), QMessageBox::Yes, QMessageBox::No))    p = 1;


    V3DLONG offsetc = (c-1)*pagesz;
    // Ws = 2000;
    int county = 0;
    unsigned char* subject1d_y = 0;
    unsigned char* target1d_y = 0;
    V3DLONG szSub_y[4];
    V3DLONG szTar_y[4];

    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
    {
        unsigned char* subject1d = 0;
        unsigned char* target1d = 0;

        V3DLONG szSub[4];
        V3DLONG szTar[4];
        V3DLONG new_sz0 = 0;
        V3DLONG new_sz1 = 0;
        V3DLONG new_sz2 = 0;
        V3DLONG yb = iy;
        V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

        int count = 0;
        for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
        {
            V3DLONG xb = ix;
            V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

            unsigned char *blockarea=0;
            V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*P;
            blockarea = new unsigned char [blockpagesz];
            double th_global = 0;
            int i = 0;
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG iy = yb; iy < ye+1; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = xb; ix < xe+1; ix++)
                    {

                        blockarea[i] = data1d[offsetc+offsetk + offsetj + ix];
                        i++;
                    }
                }
            }
            V3DLONG block_sz[4];
            block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;
            simple_saveimage_wrapper(callback, "temp.v3draw", (unsigned char *)blockarea, block_sz, 1);
            unsigned char *localEnahancedArea=0;
            try {localEnahancedArea = new unsigned char [blockpagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea."); return;}
            double sigma = 0;
            for(int scale = 0; scale < 2; scale++)
            {
                unsigned char * data1d_gf = 0;
                unsigned char * gsdtld = 0;
                unsigned char* EnahancedImage = 0;
                switch (pixeltype)
                {
                case V3D_UINT8:
                    if (scale==0) //do not filter for the scale 0
                    {

                        sigma = 0.3;
                        data1d_gf = new unsigned char [blockpagesz];
                        memcpy(data1d_gf, blockarea, blockpagesz);
                    }
                    else
                    {
                        sigma = 1.2;
                        callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);
                    }

                    callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld);
                    AdpThresholding_adpwindow((unsigned char *)data1d_gf, block_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
                    break;
                default: v3d_msg("Invalid data type. Do nothing."); return;
                }

                if (scale==0)

                    memcpy(localEnahancedArea, EnahancedImage, blockpagesz);
                   else
                   {
                       for(V3DLONG i = 0; i<blockpagesz; i++)
                       {
                           if (localEnahancedArea[i] < EnahancedImage[i])
                               localEnahancedArea[i] = EnahancedImage[i]+1;
                           else
                               localEnahancedArea[i] =  localEnahancedArea[i]+1;
                       }
                       remove("temp.v3draw");
                   }

                if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
                if(gsdtld) {delete []gsdtld; gsdtld =0;}
                if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
           }

            if (count==0)
            {
                V3DLONG targetsize = block_sz[0]*block_sz[1]*block_sz[2];
                target1d = new unsigned char [targetsize];
                memcpy(target1d, localEnahancedArea, targetsize);
                szTar[0] = xe-xb+1; szTar[1] = ye-yb+1; szTar[2] = P; szTar[3] = 1;

            }
            else
            {

                V3DLONG subjectsize = block_sz[0]*block_sz[1]*block_sz[2];
                subject1d = new unsigned char [subjectsize];
                memcpy(subject1d, localEnahancedArea, subjectsize);
                szSub[0] = xe-xb+1; szSub[1] = ye-yb+1; szSub[2] = P; szSub[3] = 1;

                V3DLONG *offset = new V3DLONG [3];
                offset[0] = xb;
                offset[1] = 0;
                offset[2] = 0;
                new_sz0 = xe+1;
                new_sz1 = szSub[1];
                new_sz2 = szSub[2];

                V3DLONG totalplxs = new_sz0*new_sz1*new_sz2;
                unsigned char* data1d_blended = NULL;
                data1d_blended = new unsigned char [totalplxs];
                memset(data1d_blended, 0, sizeof(unsigned char)*totalplxs);
                int success;

                success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended, (unsigned char *)subject1d, szSub, (unsigned char *)target1d, szTar, offset, new_sz0, new_sz1, new_sz2);

                V3DLONG targetsize = new_sz0*new_sz1*new_sz2;
                target1d = new unsigned char [targetsize];
                memcpy(target1d, data1d_blended, targetsize);
                szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = new_sz2; szTar[3] = 1;
            }
            count ++;

            if(blockarea) {delete []blockarea; blockarea =0;}
            if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}
        }


        if (county==0)
        {

            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            memcpy(target1d_y, target1d, targetsize_y);
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = P; szTar_y[3] = 1;
        }
        else
        {
            V3DLONG subjectsize_y = new_sz0*new_sz1*new_sz2;
            subject1d_y = new unsigned char [subjectsize_y];
            memcpy(subject1d_y, target1d, subjectsize_y);
            szSub_y[0] = new_sz0; szSub_y[1] = new_sz1; szSub_y[2] = P; szSub_y[3] = 1;

            V3DLONG *offset = new V3DLONG [3];
            offset[0] = 0;
            offset[1] = yb;
            offset[2] = 0;
            new_sz0 = szSub_y[0];
            new_sz1 = ye+1;
            new_sz2 = szSub_y[2];

            V3DLONG totalplxs = new_sz0*new_sz1*new_sz2;
            unsigned char* data1d_blended_y = 0;
            data1d_blended_y = new unsigned char [totalplxs];
            memset(data1d_blended_y, 0, sizeof(unsigned char)*totalplxs);
            int success;
            success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended_y, (unsigned char *)subject1d_y, szSub_y, (unsigned char *)target1d_y, szTar_y, offset, new_sz0, new_sz1, new_sz2);


            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            memcpy(target1d_y, data1d_blended_y, targetsize_y);
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = new_sz2; szTar_y[3] = 1;

            if(data1d_blended_y) {delete []data1d_blended_y; data1d_blended_y =0;}

        }
        county++;

        if(subject1d) {delete []subject1d; subject1d =0;}
        if(target1d) {delete []target1d; target1d =0;}
    }

    unsigned char* Enhancement_soma = 0;
    if(p==1)
    {
        enhancementWithsoma(callback,(unsigned char *)data1d,(unsigned char*)target1d_y,in_sz,1,(unsigned char *&)Enhancement_soma);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, target1d_y, pagesz);
    }

    simple_saveimage_wrapper(callback,"result_woGf.v3draw", (unsigned char *)Enhancement_soma, in_sz, 1);
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("result_woGf.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage_v2.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_float = 0;
    int datatype;
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    if(!simple_loadimage_wrapper(callback,outimg_file, data1d_float, in_zz, datatype))
    {
        cerr<<"load image "<<outimg_file<<" error!"<<endl;
        return;
    }
    remove("result_woGf.v3draw");
    remove("gfImage_v2.v3draw");

    double min,max;
    unsigned char* data1d_uint8 = 0;
    data1d_uint8 = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);

    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}
    if (data1d_float) { delete []data1d_float; data1d_float=0;}
    if(subject1d_y) {delete []subject1d_y; subject1d_y =0;}
    if(target1d_y) {delete []target1d_y; target1d_y =0;}

    //set up output image
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_uint8,N, M, P, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Local_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
}

bool processImage_adaptive_auto(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter"<<endl;
    if (output.size() != 1) return false;
    unsigned int scale = 2, c=1, p = 0;
    float ratio = 1;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) scale = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
        if(paras.size() >= 4) p = atoi(paras.at(3));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"scale = "<<scale<<endl;
    cout<<"ch = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"soma = "<<p<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;


    Image4DSimple *subject = callback.loadImage(inimg_file);
    if(!subject || !subject->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (subject) {delete subject; subject=0;}
         return false;
    }

    V3DLONG in_sz[4];
    in_sz[0] = subject->getXDim();
    in_sz[1] = subject->getYDim();
    in_sz[2] = subject->getZDim();
    in_sz[3] = 1;
    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    V3DLONG offsetc = (c-1)*pagesz;

    int datatype = subject->getDatatype();


    simple_saveimage_wrapper(callback, "temp.v3draw", (unsigned char *)subject->getRawDataAtChannel(c-1), in_sz, datatype);

    unsigned char *EnahancedImage_final=0;
    try {EnahancedImage_final = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for EnhancedImage_final."); return false;}


    double sigma = 0.3;
    for(unsigned int  count = 0; count < scale; count++)
    {

        unsigned char * data1d_gf = 0;
        unsigned char * gsdtld = 0;
        unsigned char* EnahancedImage = 0;

        switch (datatype)
        {
        case V3D_UINT8:
            if (count==0 && ratio == 0.1) //do not filter for the scale 0
            {

                data1d_gf = new unsigned char [pagesz];
                memcpy(data1d_gf, subject->getRawDataAtChannel(c-1), pagesz);
            }
            else
                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);

            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld);
            AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
            break;
        default: v3d_msg("Invalid data type. Do nothing."); return false;
        }

         if (count==0)
                memcpy(EnahancedImage_final, EnahancedImage, pagesz);
            else
            {
                for(V3DLONG i = 0; i<pagesz; i++)
                {
                    if (EnahancedImage_final[i] < EnahancedImage[i])
                        EnahancedImage_final[i] = EnahancedImage[i];
                }
            }
        sigma += 1.5;

        if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
        if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
        if(gsdtld) {delete []gsdtld; gsdtld =0;}
    }

    remove("temp.v3draw");

    unsigned char* Enhancement_soma = 0;
    if(p==1)
    {
        enhancementWithsoma(callback,(unsigned char *)subject->getRawDataAtChannel(c-1),(unsigned char*)EnahancedImage_final,in_sz,1,(unsigned char *&)Enhancement_soma);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, EnahancedImage_final, pagesz);
    }

    simple_saveimage_wrapper(callback,"result_woGf.v3draw", (unsigned char *)Enhancement_soma, in_sz, 1);
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("result_woGf.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage_v2.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_float = 0;
    V3DLONG in_zz[4];
    datatype = 0;
    char * outimg_file2 = ((vector<char*> *)(output.at(0).p))->at(0);
    if(!simple_loadimage_wrapper(callback,outimg_file2, data1d_float, in_zz, datatype))
    {
        cerr<<"load image "<<outimg_file2<<" error!"<<endl;
        return false;
    }

    remove("result_woGf.v3draw");
    remove("gfImage_v2.v3draw");

    double min,max;
    unsigned char* data1d_uint8 = 0;
    data1d_uint8 = new unsigned char [pagesz];
    rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);

    simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d_uint8, in_sz, 1);


    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}
    if (data1d_float) { delete []data1d_float; data1d_float=0;}
    if (EnahancedImage_final) { delete []EnahancedImage_final; EnahancedImage_final=0;}
    if (data1d_uint8) {delete []data1d_uint8; data1d_uint8=0;}
    if (subject) {delete subject; subject=0;}

   return true;
}

bool processImage_adaptive_auto_blocks(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter with blocks"<<endl;
    if (output.size() != 1) return false;
    unsigned int Ws = 1000, c=1, p = 0;
    float ratio = 1.0;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) Ws = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
        if(paras.size() >= 4) p = atoi(paras.at(3));

    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"Ws = "<<Ws<<endl;
    cout<<"c = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"soma = "<<p<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;


    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    int county = 0;
    unsigned char* subject1d_y = NULL;
    unsigned char* target1d_y = NULL;
    V3DLONG szSub_y[4];
    V3DLONG szTar_y[4];

    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
    {
        unsigned char* subject1d = 0;
        unsigned char* target1d = 0;

        V3DLONG szSub[4];
        V3DLONG szTar[4];
        V3DLONG new_sz0 = 0;
        V3DLONG new_sz1 = 0;
        V3DLONG new_sz2 = 0;
        V3DLONG yb = iy;
        V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

        int count = 0;
        for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
        {
            V3DLONG xb = ix;
            V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

            unsigned char *blockarea=0;
            V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*P;
            blockarea = new unsigned char [blockpagesz];
            double th_global = 0;
            int i = 0;
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG iy = yb; iy < ye+1; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = xb; ix < xe+1; ix++)
                    {

                        blockarea[i] = data1d[offsetc+offsetk + offsetj + ix];
                        i++;
                    }
                }
            }
            V3DLONG block_sz[4];
            block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;
            simple_saveimage_wrapper(callback, "temp.v3draw", (unsigned char *)blockarea, block_sz, 1);
            unsigned char *localEnahancedArea=0;
            try {localEnahancedArea = new unsigned char [blockpagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea."); return false;}
            double sigma = 0;
            for(int scale = 0; scale < 2; scale++)
            {
                unsigned char * data1d_gf = 0;
                unsigned char * gsdtld = 0;
                unsigned char* EnahancedImage = 0;
                switch (datatype)
                {
                case V3D_UINT8:
                    if (scale==0) //do not filter for the scale 0
                    {

                        sigma = 0.3;
                        data1d_gf = new unsigned char [blockpagesz];
                        memcpy(data1d_gf, blockarea, blockpagesz);
                    }
                    else
                    {
                        sigma = 1.2;
                        callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);
                    }

                    callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld);
                    AdpThresholding_adpwindow((unsigned char *)data1d_gf, block_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
                    break;
                    default: v3d_msg("Invalid data type. Do nothing."); return false;
                }

                if (scale==0)

                    memcpy(localEnahancedArea, EnahancedImage, blockpagesz);
                   else
                   {
                       for(V3DLONG i = 0; i<blockpagesz; i++)
                       {
                           if (localEnahancedArea[i] < EnahancedImage[i])
                               localEnahancedArea[i] = EnahancedImage[i]+1;
                           else
                               localEnahancedArea[i] =  localEnahancedArea[i]+1;
                       }
                       remove("temp.v3draw");
                   }

                if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
                if(gsdtld) {delete []gsdtld; gsdtld =0;}
                if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
           }

            if (count==0)
            {
                V3DLONG targetsize = block_sz[0]*block_sz[1]*block_sz[2];
                target1d = new unsigned char [targetsize];
                memcpy(target1d, localEnahancedArea, targetsize);
                szTar[0] = xe-xb+1; szTar[1] = ye-yb+1; szTar[2] = P; szTar[3] = 1;

            }
            else
            {

                V3DLONG subjectsize = block_sz[0]*block_sz[1]*block_sz[2];
                subject1d = new unsigned char [subjectsize];
                memcpy(subject1d, localEnahancedArea, subjectsize);
                szSub[0] = xe-xb+1; szSub[1] = ye-yb+1; szSub[2] = P; szSub[3] = 1;

                V3DLONG *offset = new V3DLONG [3];
                offset[0] = xb;
                offset[1] = 0;
                offset[2] = 0;
                new_sz0 = xe+1;
                new_sz1 = szSub[1];
                new_sz2 = szSub[2];

                V3DLONG totalplxs = new_sz0*new_sz1*new_sz2;
                unsigned char* data1d_blended = NULL;
                data1d_blended = new unsigned char [totalplxs];
                memset(data1d_blended, 0, sizeof(unsigned char)*totalplxs);
                int success;

                success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended, (unsigned char *)subject1d, szSub, (unsigned char *)target1d, szTar, offset, new_sz0, new_sz1, new_sz2);

                V3DLONG targetsize = new_sz0*new_sz1*new_sz2;
                target1d = new unsigned char [targetsize];
                memcpy(target1d, data1d_blended, targetsize);
                szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = new_sz2; szTar[3] = 1;
            }
            count ++;

            if(blockarea) {delete []blockarea; blockarea =0;}
            if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}
        }


        if (county==0)
        {

            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            memcpy(target1d_y, target1d, targetsize_y);
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = P; szTar_y[3] = 1;
        }
        else
        {
            V3DLONG subjectsize_y = new_sz0*new_sz1*new_sz2;
            subject1d_y = new unsigned char [subjectsize_y];
            memcpy(subject1d_y, target1d, subjectsize_y);
            szSub_y[0] = new_sz0; szSub_y[1] = new_sz1; szSub_y[2] = P; szSub_y[3] = 1;

            V3DLONG *offset = new V3DLONG [3];
            offset[0] = 0;
            offset[1] = yb;
            offset[2] = 0;
            new_sz0 = szSub_y[0];
            new_sz1 = ye+1;
            new_sz2 = szSub_y[2];

            V3DLONG totalplxs = new_sz0*new_sz1*new_sz2;
            unsigned char* data1d_blended_y = 0;
            data1d_blended_y = new unsigned char [totalplxs];
            memset(data1d_blended_y, 0, sizeof(unsigned char)*totalplxs);
            int success;
            success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended_y, (unsigned char *)subject1d_y, szSub_y, (unsigned char *)target1d_y, szTar_y, offset, new_sz0, new_sz1, new_sz2);


            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            memcpy(target1d_y, data1d_blended_y, targetsize_y);
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = new_sz2; szTar_y[3] = 1;

            if(data1d_blended_y) {delete []data1d_blended_y; data1d_blended_y =0;}

        }
        county++;

        if(subject1d) {delete []subject1d; subject1d =0;}
        if(target1d) {delete []target1d; target1d =0;}
    }

    unsigned char* Enhancement_soma = 0;
    if(p==1)
    {
        enhancementWithsoma(callback,(unsigned char *)data1d,(unsigned char*)target1d_y,in_sz,1,(unsigned char *&)Enhancement_soma);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, target1d_y, pagesz);
    }
    in_sz[3] = 1;
    simple_saveimage_wrapper(callback,"result_woGf.v3draw", (unsigned char *)Enhancement_soma, in_sz, 1);
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("result_woGf.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage_v2.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_float = 0;
    V3DLONG in_zz[4];
    datatype = 0;
    char * outimg_file2 = ((vector<char*> *)(output.at(0).p))->at(0);
    if(!simple_loadimage_wrapper(callback,outimg_file2, data1d_float, in_zz, datatype))
    {
        cerr<<"load image "<<outimg_file2<<" error!"<<endl;
        return false;
    }

    remove("result_woGf.v3draw");
    remove("gfImage_v2.v3draw");

    double min,max;
    unsigned char* data1d_uint8 = 0;
    data1d_uint8 = new unsigned char [pagesz];
    rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);
    simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d_uint8, in_sz, 1);


    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}
    if (data1d_float) { delete []data1d_float; data1d_float=0;}
    if (data1d_uint8) {delete []data1d_uint8; data1d_uint8=0;}
    if(target1d_y) {delete []target1d_y; target1d_y =0;}
    if(subject1d_y) {delete []subject1d_y; subject1d_y =0;}
    if (data1d) {delete []data1d; data1d=0;}
    return true;
}

template <class T> void selectiveEnhancement(const T* data1d,
                                             V3DLONG *in_sz,
                                             unsigned int c,
                                             double sigma,
                                             T* &outimg)
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


template <class T> void callGaussianPlugin(V3DPluginCallback2 &callback,
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
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    simple_loadimage_wrapper(callback, outimg_file, data1d_Gf, in_zz, datatype);
    remove("gfImage.v3draw");
    unsigned char* data1d = 0;
    data1d = new unsigned char [pagesz];

    double min,max;
    rescale_to_0_255_and_copy((float *)data1d_Gf,pagesz,min,max,data1d);
    if(data1d_Gf) {delete []data1d_Gf; data1d_Gf =0;}

    try
    {
        outimg = new T [pagesz];
    }
    catch (...)
    {
        printf("Fail to allocate memory.\n");
        if(data1d) {delete []data1d; data1d =0;}
        return;
    }

    for(V3DLONG i=0; i<pagesz; i++)
        outimg[i] = data1d[i];

    if(data1d) {delete []data1d; data1d =0;}

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

    double LOG2 = log(2.0);

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
                Wx = (int)round((ratio*log(double(GsdtValue)+1.0)/LOG2));
                if(ratio ==0.1)
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

                    V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                    V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                    V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                    V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                    V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
                    V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

                    double fxx = data1d[offsetk+offsetj+xe]+ data1d[offsetk+offsetj+xb]- 2*data1d[offsetk+offsetj+ix];
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
                        if(a1<0 && a2<0)
                        {
                            T dataval = (T)(sigma*sigma*pow((zhi_abs(a2)-zhi_abs(a3)),3)/zhi_abs(a1));
                            pImage[offsetk+offsetj+ix] = dataval;
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

                        }
                    }
                }
                else
                    pImage[offsetk+offsetj+ix] = 0;
            }
        }
    }

    outimg = pImage;
    return;
}


template <class T> void callgsdtPlugin(V3DPluginCallback2 &callback,const T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       double th_global,
                                       T* &outimg)
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
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;
    double th_final = 0;

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
        th += PixelSum/(M*N*P);
    }

    double std = 0;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double stdSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy <  M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

                double PixelVaule = data1d[offsetc+offsetk + offsetj + ix];
                stdSum = stdSum + pow(PixelVaule-th,2);
            }
        }
        std += stdSum/(M*N*P);
    }

    if(th > th_global)
        th_final = th;
    else
        th_final = th_global;
    printf("mean is %.2f, std is %.2f\n\n\n",th,sqrt(std));
    simple_saveimage_wrapper(callback, "temp_gf.v3draw", (unsigned char *)data1d, in_sz, 1);

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("temp_gf.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    string threshold = boost::lexical_cast<string>(th_final); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_gsdt = 0;
    int datatype;
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    simple_loadimage_wrapper(callback, outimg_file, data1d_gsdt, in_zz, datatype);
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
    if(data1d_gsdt) {delete []data1d_gsdt; data1d_gsdt =0;}
    return;

}

template <class T> double getdtmax(V3DPluginCallback2 &callback,const T* data1d,V3DLONG *in_sz)
{
    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    simple_saveimage_wrapper(callback, "temp_enhanced.v3draw", (unsigned char *)data1d, in_sz, 1);

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
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    simple_loadimage_wrapper(callback, outimg_file, data1d_gt, in_zz, datatype);
    remove("temp_enhanced.v3draw");
    remove("gtImage.v3draw");
    double maxfl = 0;
    for(V3DLONG i=0; i<pagesz; i++)
    {
        double dataValue = double(data1d_gt[i]);
        if(dataValue > maxfl)   maxfl = dataValue;

    }


    if (data1d_gt) {delete []data1d_gt; data1d_gt=0;}
    return maxfl;


}

// pairwise image blending function
template <class SDATATYPE>
int pwi_fusing(SDATATYPE *data1d, SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, V3DLONG *offset,V3DLONG new_sz0, V3DLONG new_sz1, V3DLONG new_sz2)
{
    //

    V3DLONG sx = sz_subject[0], sy = sz_subject[1], sz = sz_subject[2], sc = sz_subject[3];
    V3DLONG tx = sz_target[0], ty = sz_target[1], tz = sz_target[2], tc = sz_target[3];

    V3DLONG offset_tx, offset_ty, offset_tz, offset_sx, offset_sy, offset_sz;
    if(offset[0]<0)
    {
        offset_sx = 0; offset_tx = -offset[0];
    }
    else
    {
        offset_sx = offset[0]; offset_tx = 0;
    }
    if(offset[1]<0)
    {
        offset_sy = 0; offset_ty = -offset[1];
    }
    else
    {
        offset_sy = offset[1]; offset_ty = 0;
    }
    if(offset[2]<0)
    {
        offset_sz = 0; offset_tz = -offset[2];
    }
    else
    {
        offset_sz = offset[2]; offset_tz = 0;
    }

    qDebug("new_sz0 %ld new_sz1 %ld offset_tx %ld offset_ty %ld offset_sx %ld offset_sy %ld", new_sz0, new_sz1, offset_tx, offset_ty, offset_sx, offset_sy);

    //outputs
    V3DLONG offset_data = new_sz0*new_sz1*new_sz2;
    V3DLONG offset_target = tx*ty*tz;
    V3DLONG offset_subject = sx*sy*sz;

    V3DLONG i_start, j_start, k_start;
    V3DLONG sz_start = offset_sz, sz_end = sz_start + sz; if(sz_start<0) k_start=0; else k_start=sz_start; if(sz_end>new_sz2) sz_end=new_sz2;
    V3DLONG sy_start = offset_sy, sy_end = sy_start + sy; if(sy_start<0) j_start=0; else j_start=sy_start; if(sy_end>new_sz1) sy_end=new_sz1;
    V3DLONG sx_start = offset_sx, sx_end = sx_start + sx; if(sx_start<0) i_start=0; else i_start=sx_start; if(sx_end>new_sz0) sx_end=new_sz0;

    //cout<< k_start << " " << sz_end  << " " << j_start  << " " << sy_end  << " " << i_start  << " " << sx_end << endl;

    for(V3DLONG c=0; c<sc; c++)
    {
        V3DLONG offset_c = c*offset_data;
        V3DLONG offsets_c = c*offset_subject;
        for(V3DLONG k=k_start; k<sz_end; k++)
        {
            V3DLONG offset_k = offset_c + k*new_sz0*new_sz1;
            V3DLONG offsets_k = offsets_c + (k-k_start)*sx*sy; if (k-k_start >= sz) continue;
            for(V3DLONG j=j_start; j<sy_end; j++)
            {
                V3DLONG offset_j = offset_k + j*new_sz0;
                V3DLONG offsets_j = offsets_k + (j-j_start)*sx; if (j-j_start >= sy) continue;
                for(V3DLONG i=i_start; i<sx_end; i++)
                {
                    V3DLONG idx = offset_j + i;

                    data1d[idx] = subject1d[offsets_j + i - i_start];
                }
            }
        }
    }

    V3DLONG tz_start = offset_tz, tz_end = offset_tz + tz; if(tz_start<0) tz_start=0; if(tz_end>new_sz2) tz_end=new_sz2;
    V3DLONG ty_start = offset_ty, ty_end = offset_ty + ty; if(ty_start<0) ty_start=0; if(ty_end>new_sz1) ty_end=new_sz1;
    V3DLONG tx_start = offset_tx, tx_end = offset_tx + tx;	if(tx_start<0) tx_start=0; if(tx_end>new_sz0) tx_end=new_sz0;

    //cout<< tz_start << " " << tz_end  << " " << ty_start  << " " << ty_end  << " " << tx_start  << " " << tx_end << endl;

    for(V3DLONG c=0; c<sc; c++)
    {
        V3DLONG offset_c = c*offset_data;
        V3DLONG offsets_c = c*offset_target;
        for(V3DLONG k=tz_start; k<tz_end; k++)
        {
            V3DLONG offset_k = offset_c + k*new_sz0*new_sz1;
            V3DLONG offsets_k = offsets_c + (k-tz_start)*tx*ty;
            for(V3DLONG j=ty_start; j<ty_end; j++)
            {
                V3DLONG offset_j = offset_k + j*new_sz0;
                V3DLONG offsets_j = offsets_k + (j-ty_start)*tx;
                for(V3DLONG i=tx_start; i<tx_end; i++)
                {
                    V3DLONG idx = offset_j + i;
                    if(data1d[idx])
                    {
                        if(offset[0] > 0)
                        {
                            V3DLONG overlap_range = tx - offset[0];
                            V3DLONG tar_range= i - offset[0]+1;
                            double tar_ratio = (double)tar_range/overlap_range;
                            //qDebug("overlap_range %ld tar_range %ld tar_ratio %.4f sub_ratio %.4f", overlap_range, tar_range, tar_ratio, 1-tar_ratio);
                            data1d[idx] = (SDATATYPE) ( tar_ratio*data1d[idx] + (1-tar_ratio)*target1d[offsets_j + i - tx_start] );

                            //data1d[idx] = (SDATATYPE) ( (data1d[idx] + target1d[offsets_j + i - tx_start])/2.0 );
                        }else
                        {
                            V3DLONG overlap_range = ty - offset[1];
                            V3DLONG tar_range= j - offset[1]+1;
                            double tar_ratio = (double)tar_range/overlap_range;
                            data1d[idx] = (SDATATYPE) ( tar_ratio*data1d[idx] + (1-tar_ratio)*target1d[offsets_j + i - tx_start] );


                        }
                    }
                    else
                    {
                        data1d[idx] = (SDATATYPE) target1d[offsets_j + i - tx_start];
                    }

                }
            }
        }
    }



    return true;
}

template <class T> void enhancementWithsoma(V3DPluginCallback2 &callback,
                                            const T* data1d,
                                            T* data1d_enhanced,
                                            V3DLONG *in_sz,
                                            unsigned int c,
                                            T* &outimg)

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

    int soma_x = 0;
    int soma_y = 0;
    somalocation(callback,data1d,in_sz,1,soma_x,soma_y);
    printf("\n soma location is (%d, %d)\n\n", soma_x,soma_y);

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    double th_soma = 0;
    V3DLONG xb = soma_x-200; if(xb<0) xb = 0;
    V3DLONG xe = soma_x+200; if(xe>=N-1) xe = N-1;
    V3DLONG yb = soma_y-200; if(yb<0) yb = 0;
    V3DLONG ye = soma_y+200; if(ye>=M-1) ye = M-1;
    V3DLONG somasz = (xe-xb)*(ye-yb)*P;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;

        for(V3DLONG iy = yb; iy < ye; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = xb; ix < xe; ix++)
            {

                double PixelVaule = data1d[offsetc + offsetk + offsetj + ix];
                PixelSum = PixelSum + PixelVaule;
            }
        }
        th_soma = th_soma + PixelSum/((xe-xb)*(ye-yb)*P);
    }

    void* somaarea = 0;
    soma_detection((unsigned char*)data1d, in_sz, c,soma_x,soma_y,somasz,(unsigned char* &)somaarea);

    V3DLONG soma_sz[4];
    soma_sz[0] = xe-xb; soma_sz[1] = ye-yb; soma_sz[2] = P; soma_sz[3] = 1;
    simple_saveimage_wrapper(callback,"temp_soma.v3draw", (unsigned char *)somaarea, soma_sz, 1);
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;


    arg.type = "random";std::vector<char*> args1;
    args1.push_back("temp_soma.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    char channel = '0' + (c-1);
    string threshold = boost::lexical_cast<string>(th_soma); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");args.push_back(&channel);args.push_back("1"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage_soma.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * gsdtsoma = 0;
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    int datatype;
    V3DLONG in_zz[4];

    if(!simple_loadimage_wrapper(callback,outimg_file, gsdtsoma, in_zz, datatype))
    {
        cerr<<"load image "<<outimg_file<<" error!"<<endl;
        return;
    }

    remove("temp_soma.v3draw");
    remove("gsdtImage_soma.v3draw");

    int Th_gsdt = 100;

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

    V3DLONG i = 0;
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;

        for(V3DLONG iy = yb; iy <  ye; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = xb; ix < xe; ix++)
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
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                T SomaValue = pSoma[offsetc+offsetk + offsetj + ix];

                if(SomaValue < Th_gsdt)
                {
                    T dataval2 = data1d_enhanced[offsetk+offsetj+ix];
                    pImage2[offsetk+offsetj+ix] = dataval2;
                }
                else
                    pImage2[offsetk+offsetj+ix] =  data1d[offsetk+offsetj+ix];
            }


        }

    }

    outimg = pImage2;

    if(gsdtsoma) {delete []gsdtsoma; gsdtsoma =0;}
    if(somaarea) {delete []somaarea; somaarea =0;}
    if(pSoma) {delete []pSoma; pSoma =0;}
    return;


}

template <class T> void somalocation(V3DPluginCallback2 &callback,
                                     const T* data1d,
                                     V3DLONG *in_sz,
                                     unsigned int c,
                                     int &soma_x,
                                     int &soma_y)
{


    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    V3DLONG DSN = floor(in_sz[0]/4);
    V3DLONG DSM = floor(in_sz[1]/4);
    V3DLONG DSP = floor(in_sz[2]/4);


    V3DLONG pageszDS = DSN*DSM*DSP;
    unsigned char* datald_downsample = 0;
    datald_downsample = new unsigned char [pageszDS];
    for(V3DLONG i=0; i<pageszDS; i++)
        datald_downsample[i] = 0;

    double th = 0;
    V3DLONG dsiz=0;
    for(V3DLONG iz = 0; iz < P; iz = iz+4)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        V3DLONG dsoffsetk = dsiz*DSM*DSN;
        V3DLONG dsiy=0;
        for(V3DLONG iy = 0; iy <  M; iy = iy+4)
        {
            V3DLONG offsetj = iy*N;
            V3DLONG dsoffsetj = dsiy*DSN;
            V3DLONG dsix=0;
            for(V3DLONG ix = 0; ix < N; ix = ix+4)
            {

                V3DLONG index_pim = offsetc+offsetk + offsetj + ix;
                V3DLONG DSindex_pim = dsoffsetk+dsoffsetj+dsix;
                if(DSindex_pim < pageszDS)
                {
                    double PixelVaule = data1d[index_pim];
                    PixelSum = PixelSum + PixelVaule;
                    datald_downsample[DSindex_pim] = data1d[index_pim];
                    dsix++;
                }
            }
            dsiy++;
        }
        th += PixelSum/pageszDS;
        dsiz++;
    }

    double std = 0;
    for(V3DLONG dsiz = 0; dsiz < DSP; dsiz++)
    {
        V3DLONG dsoffsetk = dsiz*DSM*DSN;
        double stdSum=0;
        for(V3DLONG dsiy = 0; dsiy < DSM; dsiy++)
        {
            V3DLONG dsoffsetj = dsiy*DSN;
            for(V3DLONG dsix = 0; dsix < DSN; dsix++)
            {
                double PixelVaule = datald_downsample[dsoffsetk+dsoffsetj+dsix];
                stdSum = stdSum + pow(PixelVaule-th,2);

            }
        }
        std += stdSum/pageszDS;
    }


    double th_gsdt = th + 4*sqrt(std);
    printf("\n\ndownsample mean is %.2f, std is %.2f\n\n\n",th,sqrt(std));
    V3DLONG in_ds[4];
    in_ds[0] = DSN; in_ds[1] = DSM; in_ds[2] = DSP; in_ds[3] = 1;
    simple_saveimage_wrapper(callback,"temp_ds.v3draw", (unsigned char *)datald_downsample, in_ds, 1);

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    args1.push_back("temp_ds.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    string threshold = boost::lexical_cast<string>(th_gsdt); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage_ds.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_gsdt_ds = 0;
    int datatype;
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    simple_loadimage_wrapper(callback,outimg_file, data1d_gsdt_ds, in_zz, datatype);
    remove("temp_ds.v3draw");
    remove("gsdtImage_ds.v3draw");

    double maxdl = 0;
    for(V3DLONG dsiz = 0; dsiz < DSP; dsiz++)
    {
        V3DLONG dsoffsetk = dsiz*DSM*DSN;
        for(V3DLONG dsiy = 0; dsiy < DSM; dsiy++)
        {
            V3DLONG dsoffsetj = dsiy*DSN;
            for(V3DLONG dsix = 0; dsix < DSN; dsix++)
            {
                double dsValue = data1d_gsdt_ds[dsoffsetk+dsoffsetj+dsix];
                if(dsValue>maxdl)
                {
                    maxdl = dsValue;
                    soma_x = dsix*4;
                    soma_y = dsiy*4;

                }
            }
        }
    }

    if(datald_downsample) {delete []datald_downsample; datald_downsample =0;}
    if(data1d_gsdt_ds) {delete []data1d_gsdt_ds; data1d_gsdt_ds =0;}
    return;
}

template <class T> void soma_detection(T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       int x,
                                       int y,
                                       V3DLONG somasize,
                                       T* &outimg)

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
        V3DLONG xb = x-200; if(xb<0) xb = 0;
        V3DLONG xe = x+200; if(xe>=N-1) xe = N-1;
        V3DLONG yb = y-200; if(yb<0) yb = 0;
        V3DLONG ye = y+200; if(ye>=M-1) ye = M-1;
        for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = yb; iy < ye; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = xb; ix < xe; ix++)
                {

                    T PixelValue = data1d[offsetc+offsetk + offsetj + ix];
                    pImage[i] = PixelValue;
                    i++;
                }
            }
        }
    }

    outimg = pImage;
    return;
}
