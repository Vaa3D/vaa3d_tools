/* MultisclaeEnhancement_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-29 : by Zhi Zhou
 */

#include <QByteArray>

#include "v3d_message.h"
#include <vector>
#include "MultisclaeEnhancement_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <string>
#include <math.h>
#include "stackutil.h"
#include <boost/lexical_cast.hpp>
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"
#include "../istitch/y_imglib.h"
#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../neurontracing_vn2/vn_imgpreprocess.h"


#if  defined(Q_OS_LINUX)
    #include <omp.h>
#endif


#include "../neurontracing_vn2/app2/my_surf_objs.h"


#define WANT_STREAM       // include iostream and iomanipulators
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

//QString temp_raw,temp_gf,temp_gsdt,temp_gsdt_v2,temp_wogf,temp_soma,temp_gsdtsoma,temp_ds,temp_gsdtds; // FL comment out, for multithreading purpose

using namespace std;
Q_EXPORT_PLUGIN2(MultisclaeEnhancement, selectiveEnhancement);
void processImage_selective(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive_auto(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive_auto_2D(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_adaptive_auto_blocks(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_detect_soma(V3DPluginCallback2 &callback, QWidget *parent);

bool processImage_adaptive_auto(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);
bool processImage_adaptive_auto_2D(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);
bool processImage_adaptive_auto_blocks(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);
bool processImage_adaptive_auto_blocks_indv(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);
bool processImage_adaptive_auto_blocks_indv_v2(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);
bool processImage_adaptive_auto_blocks_indv_multithread(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);
bool processImage_adaptive_auto_blocks_indv_multithread_v2(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);

bool processImage_detect_soma(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);


template <class T> void selective_Enhancement(const T* data1d,
                                             V3DLONG *in_sz,
                                             unsigned int c,
                                             double sigma,
                                             T* &outimg);

template <class T> void callGaussianPlugin(V3DPluginCallback2 &callback,
                                           V3DLONG pagesz,
                                           double sigma,
                                           unsigned int c,
                                           T* &outimg,
                                           QString temp_raw,
                                           QString temp_gf);

template <class T> void AdpThresholding_adpwindow(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  double sigma,
                                                  T* &outimg, const T* gsdtdata1d,unsigned int dim,double ratio);

template <class T> void AdpThresholding_adpwindow_v2(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  double sigma,
                                                  T* &outimg, const T* gsdtdata1d,unsigned int dim,double ratio);

template <class T> void callgsdtPlugin(V3DPluginCallback2 &callback,const T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       double th_global,
                                       T* &outimg,
									   QString temp_gf,
  									   QString temp_gsdt);

template <class T> void enhancementWithsoma(V3DPluginCallback2 &callback,
                                            const T* data1d,
                                            T* data1d_enhanced,
                                            V3DLONG *in_sz,
                                            unsigned int c,
                                            T* &outimg,
                                            QString temp_soma,
                                            QString temp_gsdtsoma,
                                            QString temp_ds,
                                            QString temp_gsdtds,
                                            LandmarkList listLandmarks);

template <class T> void somalocation(V3DPluginCallback2 &callback,
                                     const T* data1d,
                                     V3DLONG *in_sz,
                                     unsigned int c,
                                     int &soma_x,
                                     int &soma_y,
                                     int &soma_z,
                                     QString temp_ds,
                                     QString temp_gsdtds);

template <class T> void soma_detection(T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       int x,
                                       int y,
                                       V3DLONG somasize,
                                       T* &outimg);


template <class T> void median_filter(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Wx,
                                      unsigned int Wy,
                                      unsigned int Wz,
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
template <class SDATATYPE> int pwi_fusing(SDATATYPE *data1d, SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, V3DLONG *offset, V3DLONG new_sz0, V3DLONG new_sz1, V3DLONG new_sz2);

//FL added
template <class T> void fusing(T *data1d, T *block1d, V3DLONG xb, V3DLONG xe, V3DLONG yb, V3DLONG ye, V3DLONG *sz_data1d, unsigned int ws, bool tag_leftmost_block);

//FL added
template <class T> void fusing2D(unsigned int *data, unsigned char *counterImg, T *blockSection2D, V3DLONG xb, V3DLONG xe, V3DLONG yb, V3DLONG ye, V3DLONG *sz_enhanced2d);


//FL added
QStringList getSortedFileList(const QString & curFilePath, const char * suffix)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}

QStringList selectiveEnhancement::menulist() const
{
    return QStringList()
            <<tr("adaptive_auto")
           <<tr("adaptive_auto_2D")
          <<tr("adaptive_auto_blocks")
         <<tr("adaptive")
        <<tr("selective")
       <<tr("soma detection")
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
    else if(menu_name == tr("soma detection"))
    {
        processImage_detect_soma(callback,parent);
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
    else if (func_name == tr("adaptive_auto_2D"))
    {
        return processImage_adaptive_auto_2D(input, output,callback);
    }
    else if (func_name == tr("adaptive_auto_block"))
    {
        return processImage_adaptive_auto_blocks(input, output,callback);
    }
    else if (func_name == tr("adaptive_auto_block_indv"))
    {
        return processImage_adaptive_auto_blocks_indv(input, output,callback);
    }
    else if (func_name == tr("adaptive_auto_block_indv_v2"))
    {
        return processImage_adaptive_auto_blocks_indv_v2(input, output,callback);
    }
    else if (func_name == tr("adaptive_auto_block_indv_multithread"))
    {
        #if  defined(Q_OS_LINUX)
    	return processImage_adaptive_auto_blocks_indv_multithread(input, output,callback);
        #endif
    }
    else if (func_name == tr("adaptive_auto_block_indv_multithread_v2"))
    {
        #if  defined(Q_OS_LINUX)
        return processImage_adaptive_auto_blocks_indv_multithread_v2(input, output,callback);
        #endif
    }
    else if (func_name == tr("soma_detection"))
    {
        return processImage_detect_soma(input, output,callback);
    }
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f adaptive_auto -i <inimg_file> -o <outimg_file> -p <scale> <ch> <ratio> <soma> <bulr>"<<endl;
        cout<<endl;
        cout<<"scale       the iteration time, default 2"<<endl;
        cout<<"ch          the input channel value, start from 1, default 1"<<endl;
        cout<<"ratio       the window size calibration ratio, default 1"<<endl;
        cout<<"soma        soma detection, 1: detect, 0: not detect, default 0"<<endl;
        cout<<"blur        Gaussian blur after enhancement, 1: blur, 0: not blur, default 0"<<endl;
        cout<<endl;
        cout<<endl;

        cout<<"Usage : v3d -x dllname -f adaptive_auto_block -i <inimg_file> -o <outimg_file> -p <ws> <ch> <ratio> <soma>" <<endl;
        cout<<endl;
        cout<<"ws          block window size (pixel #), default 1000"<<endl;
        cout<<"ch          the input channel value, start from 1, default 1"<<endl;
        cout<<"ratio       the window size calibration ratio, default 1"<<endl;
        cout<<"soma        soma detection, 1: detect, 0: not detect, default 0"<<endl;
        cout<<endl;
        cout<<endl;

        cout<<"Usage : v3d -x dllname -f adaptive_auto_block_indv -i <inimg_file> -o <outimg_folder> -p <ws> <ch> <ratio>" <<endl;
        cout<<endl;
        cout<<"ws          block window size (pixel #), default 1000"<<endl;
        cout<<"ch          the input channel value, start from 1, default 1"<<endl;
        cout<<"ratio       the window size calibration ratio, default 1"<<endl;
        cout<<endl;
        cout<<endl;

        cout<<"Usage : v3d -x dllname -f adaptive_auto_block_indv_multithread -i <inimg_file> -o <outimg_folder> -p <ws> <ch> <ratio> <threads> <2Dsections>" <<endl;
        cout<<endl;
        cout<<"ws          block window size (pixel #), default 1000"<<endl;
        cout<<"ch          the input channel value, start from 1, default 1"<<endl;
        cout<<"ratio       the window size calibration ratio, default 1"<<endl;
        cout<<"threads     number of threads, default 8"<<endl;
        cout<<"2Dsections  save 2D enhanced section, 1: save, 0: not save, default 1"<<endl;
        cout<<endl;
        cout<<endl;

        cout<<"Usage : v3d -x dllname -f soma_detection -i <inimg_file> -p <marker_file> <tc_file>" <<endl;
        cout<<endl;
        cout<<"inimg_file  the original 3D image"<<endl;
        cout<<"marker_file the marker file indicates the soma location in the original 3D image"<<endl;
        cout<<"tc_file     the tc file includes all enhanced 3D tiles"<<endl;
        cout<<"output      it will automatically generate the enhance 3D tile with soma detection, and the marker file which indicates the soma location in this tile."<<endl;
        cout<<endl;
        cout<<endl;

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

    QString temp_raw = QString(p4DImage->getFileName()) + "_temp.v3draw";
    QString temp_gf = QString(p4DImage->getFileName()) + "_gf.v3draw";
//    QString temp_gsdt = QString(p4DImage->getFileName()) + "_gsdt.v3draw";
//    QString temp_gsdt_v2 = QString(p4DImage->getFileName()) + "_gsdt_v2.v3draw";
//    QString temp_wogf = QString(p4DImage->getFileName()) + "_woGf.v3draw";
//
//
//    QString temp_soma = QString(p4DImage->getFileName()) + "_soma.v3draw";
//    QString temp_gsdtsoma  = QString(p4DImage->getFileName()) + "_gsdtsoma.v3draw";
//    QString temp_ds = QString(p4DImage->getFileName()) + "_ds.v3draw";
//    QString temp_gsdtds = QString(p4DImage->getFileName()) + "_gsdtds.v3draw";

    simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(),  (unsigned char *)p4DImage->getRawDataAtChannel(c-1), in_sz, pixeltype);
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
            callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);
            selective_Enhancement((unsigned char *)data1d_gf, in_sz, c,sigma,(unsigned char* &)EnahancedImage);
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

    remove(temp_raw.toStdString().c_str());
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

    QString temp_raw = QString(p4DImage->getFileName()) + "_temp.v3draw";
    QString temp_gf = QString(p4DImage->getFileName()) + "_gf.v3draw";
    QString temp_gsdt = QString(p4DImage->getFileName()) + "_gsdt.v3draw";
//    QString temp_gsdt_v2 = QString(p4DImage->getFileName()) + "_gsdt_v2.v3draw";
//    QString temp_wogf = QString(p4DImage->getFileName()) + "_woGf.v3draw";
//
//
//    QString temp_soma = QString(p4DImage->getFileName()) + "_soma.v3draw";
//    QString temp_gsdtsoma  = QString(p4DImage->getFileName()) + "_gsdtsoma.v3draw";
//    QString temp_ds = QString(p4DImage->getFileName()) + "_ds.v3draw";
//    QString temp_gsdtds = QString(p4DImage->getFileName()) + "_gsdtds.v3draw";

    simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)p4DImage->getRawDataAtChannel(c-1), in_sz, pixeltype);
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
            callGaussianPlugin(callback,pagesz,2.3,c,(unsigned char* &)gsdtld, temp_raw, temp_gf);
            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
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

    remove(temp_raw.toStdString().c_str());

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
    unsigned int scale = 6, c=1, p = 0, q = 0;
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
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Gaussian blur after enhancement?"), QMessageBox::Yes, QMessageBox::No))    q = 1;


    QString temp_raw = QString(p4DImage->getFileName()) + "_temp.v3draw";
    QString temp_gf = QString(p4DImage->getFileName()) + "_gf.v3draw";
    QString temp_gsdt = QString(p4DImage->getFileName()) + "_gsdt.v3draw";
    QString temp_gsdt_v2 = QString(p4DImage->getFileName()) + "_gsdt_v2.v3draw";
    QString temp_wogf = QString(p4DImage->getFileName()) + "_woGf.v3draw";

    QString temp_soma = QString(p4DImage->getFileName()) + "_soma.v3draw";
    QString temp_gsdtsoma  = QString(p4DImage->getFileName()) + "_gsdtsoma.v3draw";
    QString temp_ds = QString(p4DImage->getFileName()) + "_ds.v3draw";
    QString temp_gsdtds = QString(p4DImage->getFileName()) + "_gsdtds.v3draw";

    double sigma = 0.3;
    V3DLONG in_sz[4];
    in_sz[0] = p4DImage->getXDim();
    in_sz[1] = p4DImage->getYDim();
    in_sz[2] = p4DImage->getZDim();
    in_sz[3] = 1;

    V3DLONG offsetc = (c-1)*pagesz;
    simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)p4DImage->getRawDataAtChannel(c-1), in_sz, pixeltype);

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
                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);

            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
            //callGaussianPlugin(callback,pagesz,2.3,c,(unsigned char* &)gsdtld);
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
    remove(temp_raw.toStdString().c_str());

    unsigned char* Enhancement_soma = 0;
    if(p==1)
    {
        LandmarkList listLandmarks = callback.getLandmark(curwin);
        enhancementWithsoma(callback,(unsigned char *)p4DImage->getRawDataAtChannel(c-1),(unsigned char*)EnahancedImage_final,in_sz,1,(unsigned char *&)Enhancement_soma, temp_soma, temp_gsdtsoma, temp_ds, temp_gsdtds,listLandmarks);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, EnahancedImage_final, pagesz);
    }

    unsigned char* data1d_uint8 = 0;
    if(q ==1)
    {
        simple_saveimage_wrapper(callback,temp_wogf.toStdString().c_str(), (unsigned char *)Enhancement_soma, in_sz, 1);
        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;

        arg.type = "random";std::vector<char*> args1;
        char* char_temp_wogf =  new char[temp_wogf.length() + 1];strcpy(char_temp_wogf, temp_wogf.toStdString().c_str());
        args1.push_back(char_temp_wogf); arg.p = (void *) & args1; input<< arg;
        arg.type = "random";std::vector<char*> args;
        args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input << arg;
        char* char_temp_gsdt_v2 =  new char[temp_gsdt_v2.length() + 1];strcpy(char_temp_gsdt_v2, temp_gsdt_v2.toStdString().c_str());
        arg.type = "random";std::vector<char*> args2;args2.push_back(char_temp_gsdt_v2); arg.p = (void *) & args2; output<< arg;

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
        remove(temp_wogf.toStdString().c_str());
        remove(temp_gsdt_v2.toStdString().c_str());

        double min,max;

        data1d_uint8 = new unsigned char [pagesz];

        rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);
        if (data1d_float) { delete []data1d_float; data1d_float=0;}
    }
    else
    {
        data1d_uint8 = new unsigned char [pagesz];
        memcpy(data1d_uint8, Enhancement_soma, pagesz);
    }


    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}
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

    QString temp_raw = QString(p4DImage->getFileName()) + "_temp.v3draw";
    QString temp_gf = QString(p4DImage->getFileName()) + "_gf.v3draw";
    QString temp_gsdt = QString(p4DImage->getFileName()) + "_gsdt.v3draw";
//    QString temp_gsdt_v2 = QString(p4DImage->getFileName()) + "_gsdt_v2.v3draw";
//    QString temp_wogf = QString(p4DImage->getFileName()) + "_woGf.v3draw";
//
//
//    QString temp_soma = QString(p4DImage->getFileName()) + "_soma.v3draw";
//    QString temp_gsdtsoma  = QString(p4DImage->getFileName()) + "_gsdtsoma.v3draw";
//    QString temp_ds = QString(p4DImage->getFileName()) + "_ds.v3draw";
//    QString temp_gsdtds = QString(p4DImage->getFileName()) + "_gsdtds.v3draw";

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


        simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)data1d2D, in_sz, pixeltype);


        unsigned char *EnahancedImage_final=0;
        EnahancedImage_final = new unsigned char [M*N];

        for(unsigned int  count = 0; count < scale; count++)
        {
            double sigma = maxDT1/2;
            unsigned char * data1d_gf = 0;
            unsigned char * gsdtld = 0;
            unsigned char* EnahancedImage = 0;

            switch (pixeltype)
            {
            case V3D_UINT8:
            	
//					callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);
//					callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld);
            	
            		// FL for multithreading purpose
                     callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);
                     callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
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
        remove(temp_raw.toStdString().c_str());

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


    QString temp_raw = QString(p4DImage->getFileName()) + "_temp.v3draw";
    QString temp_gf = QString(p4DImage->getFileName()) + "_gf.v3draw";
    QString temp_gsdt = QString(p4DImage->getFileName()) + "_gsdt.v3draw";
    QString temp_gsdt_v2 = QString(p4DImage->getFileName()) + "_gsdt_v2.v3draw";
    QString temp_wogf = QString(p4DImage->getFileName()) + "_woGf.v3draw";


    QString temp_soma = QString(p4DImage->getFileName()) + "_soma.v3draw";
    QString temp_gsdtsoma  = QString(p4DImage->getFileName()) + "_gsdtsoma.v3draw";
    QString temp_ds = QString(p4DImage->getFileName()) + "_ds.v3draw";
    QString temp_gsdtds = QString(p4DImage->getFileName()) + "_gsdtds.v3draw";


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

//            temp_raw = QString(p4DImage->getFileName()) + "_temp.v3draw";
//            temp_gf = QString(p4DImage->getFileName()) + "_gf.v3draw";
//            temp_gsdt = QString(p4DImage->getFileName()) + "_gsdt.v3draw";

            simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);
            unsigned char *localEnahancedArea=0;
            try {localEnahancedArea = new unsigned char [blockpagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea."); return;}
            double sigma = 0;
            for(int scale = 0; scale < 1; scale++)
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
                        if(blockarea) {delete []blockarea; blockarea =0;}
                    }
                    else
                    {
                        sigma = 1.2;
//                        callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);
                        
                        callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);
                    }

//                    callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld);
                    
                    callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
                    
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
                       remove(temp_raw.toStdString().c_str());
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
                if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}

            }
            else
            {

                V3DLONG subjectsize = block_sz[0]*block_sz[1]*block_sz[2];
                subject1d = new unsigned char [subjectsize];
                memcpy(subject1d, localEnahancedArea, subjectsize);
                szSub[0] = xe-xb+1; szSub[1] = ye-yb+1; szSub[2] = P; szSub[3] = 1;
                if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}

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
                if(target1d) {delete []target1d; target1d =0;}
                if(subject1d) {delete []subject1d; subject1d =0;}

                V3DLONG targetsize = new_sz0*new_sz1*new_sz2;
                target1d = new unsigned char [targetsize];
                memcpy(target1d, data1d_blended, targetsize);
                szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = new_sz2; szTar[3] = 1;

                if(data1d_blended) {delete []data1d_blended; data1d_blended =0;}

            }
            count ++;


        }


        if (county==0)
        {

            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            memcpy(target1d_y, target1d, targetsize_y);
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = P; szTar_y[3] = 1;
            if(target1d) {delete []target1d; target1d =0;}

        }
        else
        {
            V3DLONG subjectsize_y = new_sz0*new_sz1*new_sz2;
            subject1d_y = new unsigned char [subjectsize_y];
            memcpy(subject1d_y, target1d, subjectsize_y);
            szSub_y[0] = new_sz0; szSub_y[1] = new_sz1; szSub_y[2] = P; szSub_y[3] = 1;
            if(target1d) {delete []target1d; target1d =0;}

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
            if(subject1d_y) {delete []subject1d_y; subject1d_y =0;}
            if(target1d_y) {delete []target1d_y; target1d_y =0;}

            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            memcpy(target1d_y, data1d_blended_y, targetsize_y);
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = new_sz2; szTar_y[3] = 1;

            if(data1d_blended_y) {delete []data1d_blended_y; data1d_blended_y =0;}

        }
        county++;
    }

    unsigned char* Enhancement_soma = 0;
    if(p==1)
    {
        LandmarkList listLandmarks = callback.getLandmark(curwin);

        enhancementWithsoma(callback,(unsigned char *)data1d,(unsigned char*)target1d_y,in_sz,1,(unsigned char *&)Enhancement_soma, temp_soma, temp_gsdtsoma, temp_ds, temp_gsdtds,listLandmarks);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, target1d_y, pagesz);
    }


    simple_saveimage_wrapper(callback,temp_wogf.toStdString().c_str(), (unsigned char *)Enhancement_soma, in_sz, 1);

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    char* char_temp_wogf =  new char[temp_wogf.length() + 1];strcpy(char_temp_wogf, temp_wogf.toStdString().c_str());
    args1.push_back(char_temp_wogf);arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input << arg;
    char* char_temp_gsdt_v2 =  new char[temp_gsdt_v2.length() + 1];strcpy(char_temp_gsdt_v2, temp_gsdt_v2.toStdString().c_str());
    arg.type = "random";std::vector<char*> args2;args2.push_back(char_temp_gsdt_v2); arg.p = (void *) & args2; output<< arg;

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

    remove(temp_wogf.toStdString().c_str());
    remove(temp_gsdt_v2.toStdString().c_str());

    double min,max;
    unsigned char* data1d_uint8 = 0;
    data1d_uint8 = new unsigned char [pagesz];

    rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);

    if (data1d_float) { delete []data1d_float; data1d_float=0;}
    if(target1d_y) {delete []target1d_y; target1d_y =0;}
    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}


    //set up output image
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_uint8,N, M, P, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Local_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
}

void processImage_detect_soma(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);
    QString p4DImage_name = callback.getImageName(curwin);
    p4DImage_name.append("_soma.v3draw");

    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d_enhanced = p4DImage->getRawData();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    ImagePixelType pixeltype = p4DImage->getDatatype();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose original image dir:"));

    unsigned char * data1d_original = 0;
    int datatype;
    V3DLONG in_zz[4];

    if(!simple_loadimage_wrapper(callback,fileOpenName.toStdString().c_str(), data1d_original, in_zz, datatype))
    {
        cerr<<"load image "<<fileOpenName.toStdString().c_str()<<" error!"<<endl;
        return;
    }

    if(N != in_zz[0] || M != in_zz[1] || P != in_zz[2])
    {
        v3d_msg("The size of these two image is not the same. Please try it again!");
        return;

    }
//    QString temp_raw = QString(p4DImage->getFileName()) + "_temp.v3draw";
//    QString temp_gf = QString(p4DImage->getFileName()) + "_gf.v3draw";
//    QString temp_gsdt = QString(p4DImage->getFileName()) + "_gsdt.v3draw";
//    QString temp_gsdt_v2 = QString(p4DImage->getFileName()) + "_gsdt_v2.v3draw";
//    QString temp_wogf = QString(p4DImage->getFileName()) + "_woGf.v3draw";


    QString temp_soma = QString(p4DImage->getFileName()) + "_soma.v3draw";
    QString temp_gsdtsoma  = QString(p4DImage->getFileName()) + "_gsdtsoma.v3draw";
    QString temp_ds = QString(p4DImage->getFileName()) + "_ds.v3draw";
    QString temp_gsdtds = QString(p4DImage->getFileName()) + "_gsdtds.v3draw";

    LandmarkList listLandmarks = callback.getLandmark(curwin);
    unsigned char* Enhancement_soma = 0;
    enhancementWithsoma(callback,(unsigned char *)data1d_original,(unsigned char*)data1d_enhanced,in_sz,1,(unsigned char *&)Enhancement_soma, temp_soma, temp_gsdtsoma, temp_ds, temp_gsdtds,listLandmarks);
    simple_saveimage_wrapper(callback,p4DImage_name.toStdString().c_str(), (unsigned char *)Enhancement_soma, in_sz, 1);
    if(Enhancement_soma) {delete Enhancement_soma; Enhancement_soma = 0;}
    if(data1d_original) {delete data1d_original; data1d_original = 0;}
    return;

}

bool processImage_adaptive_auto(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter"<<endl;
    if (output.size() != 1) return false;
    unsigned int count = 2, c=1, p = 0,q = 0;
    double ratio = 1.0;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) count = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
        if(paras.size() >= 4) p = atoi(paras.at(3));
        if(paras.size() >= 5) q = atoi(paras.at(4));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"scale = "<<count<<endl;
    cout<<"ch = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"soma = "<<p<<endl;
    cout<<"Gaussian blur = "<<q<<endl;
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

    QString temp_raw = QString(inimg_file) + "_temp.v3draw";
    QString temp_gf = QString(inimg_file) + "_gf.v3draw";
    QString temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";
    QString temp_gsdt_v2 = QString(inimg_file) + "_gsdt_v2.v3draw";
    QString temp_wogf = QString(inimg_file) + "_woGf.v3draw";


    QString temp_soma = QString(inimg_file) + "_soma.v3draw";
    QString temp_gsdtsoma  = QString(inimg_file) + "_gsdtsoma.v3draw";
    QString temp_ds = QString(inimg_file) + "_ds.v3draw";
    QString temp_gsdtds = QString(inimg_file) + "_gsdtds.v3draw";

    simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)subject->getRawDataAtChannel(c-1), in_sz, datatype);

    unsigned char *EnahancedImage_final=0;
    try {EnahancedImage_final = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for EnhancedImage_final."); return false;}


    double sigma = 0.3;
    for(unsigned int  scale = 0; scale < count; scale++)
    {

        unsigned char * data1d_gf = 0;
        unsigned char * gsdtld = 0;
        unsigned char* EnahancedImage = 0;

        switch (datatype)
        {
        case V3D_UINT8:
            if (scale==0) //do not filter for the scale 0
            {
                if(count ==2)
                    sigma = 0.3;

                data1d_gf = new unsigned char [pagesz];
                memcpy(data1d_gf, subject->getRawDataAtChannel(c-1), pagesz);
            }
            else
            {
                sigma = 1.2;
//                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);
                
                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);
                
            }
//            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld);
            
            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
            AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
            break;
        default: v3d_msg("Invalid data type. Do nothing."); return false;
        }

         if (scale==0)
                memcpy(EnahancedImage_final, EnahancedImage, pagesz);
         else
         {
             for(V3DLONG i = 0; i<pagesz; i++)
             {
                 if (EnahancedImage_final[i] < EnahancedImage[i])
                     EnahancedImage_final[i] = EnahancedImage[i];
             }
         }

        if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
        if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
        if(gsdtld) {delete []gsdtld; gsdtld =0;}
    }

    remove(temp_raw.toStdString().c_str());

    unsigned char* Enhancement_soma = 0;
    if(p==1)
    {
        LandmarkList listLandmarks;

        enhancementWithsoma(callback,(unsigned char *)subject->getRawDataAtChannel(c-1),(unsigned char*)EnahancedImage_final,in_sz,1,(unsigned char *&)Enhancement_soma, temp_soma, temp_gsdtsoma, temp_ds, temp_gsdtds,listLandmarks);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, EnahancedImage_final, pagesz);
    }

    unsigned char* data1d_uint8 = 0;
    if(q==1)
    {
        simple_saveimage_wrapper(callback,temp_wogf.toStdString().c_str(), (unsigned char *)Enhancement_soma, in_sz, 1);
        V3DPluginArgItem arg;
        V3DPluginArgList input_gf;
        V3DPluginArgList output_gf;

        arg.type = "random";std::vector<char*> args1;
        char* char_temp_wogf =  new char[temp_wogf.length() + 1];strcpy(char_temp_wogf, temp_wogf.toStdString().c_str());
        args1.push_back(char_temp_wogf);arg.p = (void *) & args1; input_gf<< arg;
        arg.type = "random";std::vector<char*> args;
        args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input_gf << arg;
        char* char_temp_gsdt_v2 =  new char[temp_gsdt_v2.length() + 1];strcpy(char_temp_gsdt_v2, temp_gsdt_v2.toStdString().c_str());
        arg.type = "random";std::vector<char*> args2;args2.push_back(char_temp_gsdt_v2); arg.p = (void *) & args2; output_gf<< arg;

        QString full_plugin_name = "gaussian";
        QString func_name = "gf";

        callback.callPluginFunc(full_plugin_name,func_name, input_gf,output_gf);

        unsigned char * data1d_float = 0;
        V3DLONG in_zz[4];
        datatype = 0;
        char * outimg_file2 = ((vector<char*> *)(output_gf.at(0).p))->at(0);
        if(!simple_loadimage_wrapper(callback,outimg_file2, data1d_float, in_zz, datatype))
        {
            cerr<<"load image "<<outimg_file2<<" error!"<<endl;
            return false;
        }

        remove(temp_wogf.toStdString().c_str());
        remove(temp_gsdt_v2.toStdString().c_str());

        double min,max;

        data1d_uint8 = new unsigned char [pagesz];
        rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);
        if (data1d_float) { delete []data1d_float; data1d_float=0;}

    }
    else
    {
        data1d_uint8 = new unsigned char [pagesz];
        memcpy(data1d_uint8, Enhancement_soma, pagesz);
    }
    simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d_uint8, in_sz, 1);


    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}
    if (EnahancedImage_final) { delete []EnahancedImage_final; EnahancedImage_final=0;}
    if (data1d_uint8) {delete []data1d_uint8; data1d_uint8=0;}
    if (subject) {delete subject; subject=0;}

   return true;
}

bool processImage_adaptive_auto_2D(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive 2D enhancement filter"<<endl;
    if (output.size() != 1) return false;
    unsigned int scale = 2, c=1;
    double ratio = 1.0;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) scale = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"scale = "<<scale<<endl;
    cout<<"ch = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;


    Image4DSimple *subject = callback.loadImage(inimg_file);
    if(!subject || !subject->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (subject) {delete subject; subject=0;}
         return false;
    }

    V3DLONG N = subject->getXDim();
    V3DLONG M = subject->getYDim();
    V3DLONG P = subject->getZDim();
    V3DLONG pagesz_3d = subject->getTotalUnitNumberPerChannel();
    V3DLONG offsetc = (c-1)*pagesz_3d;

    unsigned char *EnahancedImage_final_3D = 0;
    try {EnahancedImage_final_3D = new unsigned char [pagesz_3d];}
    catch(...)  {v3d_msg("cannot allocate memory for EnahancedImage_final_3D."); return false;}


    int datatype = subject->getDatatype();

    QString temp_raw = QString(inimg_file) + "_temp.v3draw";
    QString temp_gf = QString(inimg_file) + "_gf.v3draw";
    QString temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";

    unsigned char *data1d = subject->getRawData();
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


        simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)data1d2D, in_sz, datatype);


        unsigned char *EnahancedImage_final=0;
        EnahancedImage_final = new unsigned char [M*N];

        for(unsigned int  count = 0; count < scale; count++)
        {
            double sigma = maxDT1/2;
            unsigned char * data1d_gf = 0;
            unsigned char * gsdtld = 0;
            unsigned char* EnahancedImage = 0;

            switch (datatype)
            {
            case V3D_UINT8:
                // FL for multithreading purpose
                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);
                callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
                AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,2,ratio); break;
            default: v3d_msg("Invalid data type. Do nothing."); return false;
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
        remove(temp_raw.toStdString().c_str());

        for(int i = 0; i < N*M; i++)
            EnahancedImage_final_3D[offsetc+offsetk+i] = EnahancedImage_final[i];

        if(EnahancedImage_final) {delete []EnahancedImage_final; EnahancedImage_final =0;}
        if(data1d2D) {delete []data1d2D; data1d2D =0;}
    }
    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P;in_sz[3] = 1;

    simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)EnahancedImage_final_3D, in_sz, 1);


    if (EnahancedImage_final_3D) { delete []EnahancedImage_final_3D; EnahancedImage_final_3D=0;}
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


    QString temp_raw = QString(inimg_file) + "_temp.v3draw";
    QString temp_gf = QString(inimg_file) + "_gf.v3draw";
    QString temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";
    QString temp_gsdt_v2 = QString(inimg_file) + "_gsdt_v2.v3draw";
    QString temp_wogf = QString(inimg_file) + "_woGf.v3draw";


    QString temp_soma = QString(inimg_file) + "_soma.v3draw";
    QString temp_gsdtsoma  = QString(inimg_file) + "_gsdtsoma.v3draw";
    QString temp_ds = QString(inimg_file) + "_ds.v3draw";
    QString temp_gsdtds = QString(inimg_file) + "_gsdtds.v3draw";

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
            simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);
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
//                        callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);
                        
                        callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);                        
                    }

//                    callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld);
                    callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
                    
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
                       remove(temp_raw.toStdString().c_str());
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
        LandmarkList listLandmarks;
        enhancementWithsoma(callback,(unsigned char *)data1d,(unsigned char*)target1d_y,in_sz,1,(unsigned char *&)Enhancement_soma, temp_soma, temp_gsdtsoma, temp_ds, temp_gsdtds,listLandmarks);
    }
    else
    {
        Enhancement_soma = new unsigned char [pagesz];
        memcpy(Enhancement_soma, target1d_y, pagesz);
    }
    in_sz[3] = 1;
    simple_saveimage_wrapper(callback,temp_wogf.toStdString().c_str(), (unsigned char *)Enhancement_soma, in_sz, 1);
    V3DPluginArgItem arg;
    V3DPluginArgList input_gf;
    V3DPluginArgList output_gf;

    arg.type = "random";std::vector<char*> args1;
    char* char_temp_wogf =  new char[temp_wogf.length() + 1];strcpy(char_temp_wogf, temp_wogf.toStdString().c_str());
    args1.push_back(char_temp_wogf); arg.p = (void *) & args1; input_gf<< arg;
    arg.type = "random";std::vector<char*> args;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input_gf << arg;
    char* char_temp_gsdt_v2 =  new char[temp_gsdt_v2.length() + 1];strcpy(char_temp_gsdt_v2, temp_gsdt_v2.toStdString().c_str());
    arg.type = "random";std::vector<char*> args2;args2.push_back(char_temp_gsdt_v2); arg.p = (void *) & args2; output_gf<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input_gf,output_gf);

    unsigned char * data1d_float = 0;
    V3DLONG in_zz[4];
    datatype = 0;
    char * outimg_file2 = ((vector<char*> *)(output_gf.at(0).p))->at(0);
    if(!simple_loadimage_wrapper(callback,outimg_file2, data1d_float, in_zz, datatype))
    {
        cerr<<"load image "<<outimg_file2<<" error!"<<endl;
        return false;
    }

    remove(temp_wogf.toStdString().c_str());
    remove(temp_gsdt_v2.toStdString().c_str());

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

// single thread enhancement
bool processImage_adaptive_auto_blocks_indv(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter with individual blocks"<<endl;
    if (output.size() != 1) return false;
    unsigned int Ws = 1000, c=1;
    float ratio = 1.0;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) Ws = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"Ws = "<<Ws<<endl;
    cout<<"c = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_folder = "<<outimg_file<<endl;


  /*  unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    
    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }*/

    unsigned char * data1d = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;

    if (!loadRawRegion(inimg_file, data1d, in_sz, in_zz,datatype,0,0,0,1,1,1))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    if(data1d) {delete []data1d; data1d = 0;}


    QString temp_raw = QString(inimg_file) + "_temp.v3draw";
    QString temp_gf = QString(inimg_file) + "_gf.v3draw";
    QString temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";
    
//    QString temp_gsdt_v2 = QString(inimg_file) + "_gsdt_v2.v3draw";
//    QString temp_wogf = QString(inimg_file) + "_woGf.v3draw";
//
//    QString temp_soma = QString(inimg_file) + "_soma.v3draw";
//    QString temp_gsdtsoma  = QString(inimg_file) + "_gsdtsoma.v3draw";
//    QString temp_ds = QString(inimg_file) + "_ds.v3draw";
//    QString temp_gsdtds = QString(inimg_file) + "_gsdtds.v3draw";

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
   // V3DLONG offsetc = (c-1)*pagesz;

    
    V3DLONG tilenum = (floor(N/(0.9*Ws))+1.0)*(floor(M/(0.9*Ws))+1.0);
    int count = 2;
    ifstream ifile(outimg_file);
    if (!ifile)
    {
       printf("Can not find the output folder");
       return false;
    }
    
 
    QString tc_name(outimg_file);
    tc_name.append("/stitched_image.tc");

    ofstream myfile;
    
    // FL added, if the file exist, delete it first
    myfile.open(tc_name.toStdString().c_str(), ios::in);
    
    if (myfile.is_open()==true)
    {
    	myfile.close();
       	remove(tc_name.toStdString().c_str());    	
    }
    // end of FL added
    
    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
     myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << tilenum << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << N << " " << M << " " << P << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";
    myfile.close(); 
    
	for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
	{
		V3DLONG yb = iy;
		V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

		for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
		
		{	
			
			V3DLONG xb = ix;
			V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

			// crop block
			unsigned char *blockarea=0;
            V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*P;
        //	blockarea = new unsigned char [blockpagesz];
			double th_global = 0;
            /*int i = 0;
			
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
            }*/

            if (!loadRawRegion(inimg_file, blockarea, in_sz, in_zz,datatype,xb,yb,0,xe+1,ye+1,P))
            {
              return false;
            }


            V3DLONG block_sz[4];
            block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;

		   /* unsigned char *blockarea_median=0;
			int ws = 2;
			//apply median filter
			switch (datatype)
			{
			case V3D_UINT8: median_filter(blockarea, block_sz, ws, ws, ws, c,(unsigned char* &)blockarea_median);
				 break;
				 default: v3d_msg("Invalid data type. Do nothing."); return false;
			}

			if(blockarea) {delete []blockarea; blockarea =0;}*/

			simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);

			unsigned char *localEnahancedArea=0;
			try {localEnahancedArea = new unsigned char [blockpagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea."); return false;} 

	
			double sigma = 1;
			for(int scale = 0; scale < count; scale++)
			{
				unsigned char * data1d_gf = 0;
				unsigned char * gsdtld = 0;
				unsigned char* EnahancedImage = 0;
				switch (datatype)
				{
				case V3D_UINT8:
					if (scale==0) //do not filter for the scale 0
					{

						if(count ==2)
							sigma = 0.3;

						data1d_gf = new unsigned char [blockpagesz];
						memcpy(data1d_gf, blockarea, blockpagesz);
					}
					else
					{
						sigma = 1.2;
//							callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);
						
						callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf); //FL, change for multithreading
						
					}

//						callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld);
					callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld, temp_gf, temp_gsdt); // FL, change for multithreading
					
					AdpThresholding_adpwindow((unsigned char *)data1d_gf, block_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
					
					break;
					default: v3d_msg("Invalid data type. Do nothing."); return false; //FL comment out, openMP does not allow exit from blocked structure

				}

		
				if (scale==0)
				{
					memcpy(localEnahancedArea, EnahancedImage, blockpagesz);
				}
				else
				{
				   for(V3DLONG i = 0; i<blockpagesz; i++)
				   {
					  if (localEnahancedArea[i] < EnahancedImage[i])
						  localEnahancedArea[i] = EnahancedImage[i];
					  else
						  localEnahancedArea[i] =  localEnahancedArea[i];
				   }
				   remove(temp_raw.toStdString().c_str());
				}

				if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
				if(gsdtld) {delete []gsdtld; gsdtld =0;}
				if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
				if(blockarea) {delete []blockarea; blockarea =0;}
			  //  if(blockarea_median) {delete []blockarea_median; blockarea_median =0;}
				
			} // for scale end
	   
			QString outputTile(outimg_file);
			outputTile.append(QString("/x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
			simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)localEnahancedArea, block_sz, 1); //write individual enhanced block images
				
			// write the block location file
			myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app ); 						
			QString outputilefull;
			outputilefull.append(QString("x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
			outputilefull.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(xb).arg(yb).arg(xe).arg(ye).arg(P-1));
			myfile << outputilefull.toStdString();
			myfile << "\n";
			myfile.close(); 
			
			if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}

		} //for ix end
	
	} //for iy end


    //if (data1d) {delete []data1d; data1d=0;}
	

	myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
	myfile << "\n# MST LUT\n";
	myfile.close();

	return true;
 
}

// single thread enhancement_v2
bool processImage_adaptive_auto_blocks_indv_v2(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter with individual blocks"<<endl;
    if (output.size() != 1) return false;
    unsigned int Ws = 1000, c=1;
    float ratio = 1.0;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) Ws = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"Ws = "<<Ws<<endl;
    cout<<"c = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_folder = "<<outimg_file<<endl;


  /*  unsigned char * data1d = 0;
    V3DLONG in_sz[4];

    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }*/

    unsigned char * data1d = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;

    if (!loadRawRegion(inimg_file, data1d, in_sz, in_zz,datatype,0,0,0,1,1,1))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    if(data1d) {delete []data1d; data1d = 0;}


    QString temp_raw = QString(inimg_file) + "_temp.v3draw";
    QString temp_gf = QString(inimg_file) + "_gf.v3draw";
    QString temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";

//    QString temp_gsdt_v2 = QString(inimg_file) + "_gsdt_v2.v3draw";
//    QString temp_wogf = QString(inimg_file) + "_woGf.v3draw";
//
//    QString temp_soma = QString(inimg_file) + "_soma.v3draw";
//    QString temp_gsdtsoma  = QString(inimg_file) + "_gsdtsoma.v3draw";
//    QString temp_ds = QString(inimg_file) + "_ds.v3draw";
//    QString temp_gsdtds = QString(inimg_file) + "_gsdtds.v3draw";

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
   // V3DLONG offsetc = (c-1)*pagesz;


    V3DLONG tilenum = (floor(N/(0.9*Ws))+1.0)*(floor(M/(0.9*Ws))+1.0);
    int count = 2;
    ifstream ifile(outimg_file);
    if (!ifile)
    {
       printf("Can not find the output folder");
       return false;
    }


    QString tc_name(outimg_file);
    tc_name.append("/stitched_image.tc");

    ofstream myfile;

    // FL added, if the file exist, delete it first
    myfile.open(tc_name.toStdString().c_str(), ios::in);

    if (myfile.is_open()==true)
    {
        myfile.close();
        remove(tc_name.toStdString().c_str());
    }
    // end of FL added

    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
     myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << tilenum << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << N << " " << M << " " << P << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";
    myfile.close();

    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
    {
        V3DLONG yb = iy;
        V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

        for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)

        {

            V3DLONG xb = ix;
            V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

            // crop block
            unsigned char *blockarea=0;
            V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*P;
        //	blockarea = new unsigned char [blockpagesz];
            double th_global = 0;
            /*int i = 0;

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
            }*/

            if (!loadRawRegion(inimg_file, blockarea, in_sz, in_zz,datatype,xb,yb,0,xe+1,ye+1,P))
            {
              return false;
            }


            V3DLONG block_sz[4];
            block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;

           /* unsigned char *blockarea_median=0;
            int ws = 2;
            //apply median filter
            switch (datatype)
            {
            case V3D_UINT8: median_filter(blockarea, block_sz, ws, ws, ws, c,(unsigned char* &)blockarea_median);
                 break;
                 default: v3d_msg("Invalid data type. Do nothing."); return false;
            }

            if(blockarea) {delete []blockarea; blockarea =0;}*/

            simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);

            unsigned char *localEnahancedArea=0;
            try {localEnahancedArea = new unsigned char [blockpagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea."); return false;}


            double sigma = 1;
            for(int scale = 0; scale < count; scale++)
            {
                unsigned char * data1d_gf = 0;
                unsigned char * gsdtld = 0;
                unsigned char* EnahancedImage = 0;
                unsigned char* data1d_gf_ds = 0;

                switch (datatype)
                {
                case V3D_UINT8:
                    if (scale==0) //do not filter for the scale 0
                    {

                        if(count ==2)
                            sigma = 0.3;

                        data1d_gf = new unsigned char [blockpagesz];
                        memcpy(data1d_gf, blockarea, blockpagesz);
                    }
                    else
                    {
                        sigma = 1.2;
//							callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);

                        callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf); //FL, change for multithreading

                    }

//						callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld);

                    V3DLONG block_sz_ds[4];
                    if (!downsampling_img_xyz( data1d_gf, block_sz, 4, 4, data1d_gf_ds, block_sz_ds))
                        return false;


                    callgsdtPlugin(callback,(unsigned char *)data1d_gf_ds, block_sz_ds, 1,th_global,(unsigned char* &)gsdtld, temp_gf, temp_gsdt); // FL, change for multithreading

                    AdpThresholding_adpwindow_v2((unsigned char *)data1d_gf, block_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);

                    break;
                    default: v3d_msg("Invalid data type. Do nothing."); return false; //FL comment out, openMP does not allow exit from blocked structure

                }


                if (scale==0)
                {
                    memcpy(localEnahancedArea, EnahancedImage, blockpagesz);
                }
                else
                {
                   for(V3DLONG i = 0; i<blockpagesz; i++)
                   {
                      if (localEnahancedArea[i] < EnahancedImage[i])
                          localEnahancedArea[i] = EnahancedImage[i];
                      else
                          localEnahancedArea[i] =  localEnahancedArea[i];
                   }
                   remove(temp_raw.toStdString().c_str());
                }

                if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
                if(gsdtld) {delete []gsdtld; gsdtld =0;}
                if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
                if(blockarea) {delete []blockarea; blockarea =0;}
              //  if(blockarea_median) {delete []blockarea_median; blockarea_median =0;}

            } // for scale end

            QString outputTile(outimg_file);
            outputTile.append(QString("/x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
            simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)localEnahancedArea, block_sz, 1); //write individual enhanced block images

            // write the block location file
            myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
            QString outputilefull;
            outputilefull.append(QString("x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
            outputilefull.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(xb).arg(yb).arg(xe).arg(ye).arg(P-1));
            myfile << outputilefull.toStdString();
            myfile << "\n";
            myfile.close();

            if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}

        } //for ix end

    } //for iy end


    //if (data1d) {delete []data1d; data1d=0;}


    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    myfile << "\n# MST LUT\n";
    myfile.close();

    return true;

}

#if  defined(Q_OS_LINUX)
// FL, for multithread enhancement
bool processImage_adaptive_auto_blocks_indv_multithread(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter with individual blocks"<<endl;
    if (output.size() != 1) return false;
    unsigned int Ws = 1000, c=1;
    float ratio = 1.0;

    // FL added the following 3
    unsigned int numOfThreads = 8; // default value for number of theads
    unsigned int saveEnhanced2DSections = 1; // default value for saving enhanced 2D section images, 1: save; 0: not save
    char *oriFileFolder;
    
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) Ws = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
        if(paras.size() >= 4) numOfThreads = atoi(paras.at(3));
        if(paras.size() >= 5) saveEnhanced2DSections = atoi(paras.at(4));
        if(paras.size() >= 6) oriFileFolder = paras.at(5);
       
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"Ws = "<<Ws<<endl;
    cout<<"c = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"numOfThreads = "<<numOfThreads<<endl;
    cout<<"saveEnhanced2DSections = "<<saveEnhanced2DSections<<endl;
    
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_folder = "<<outimg_file<<endl;


   /* unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }*/

    unsigned char * data1d = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;

    if (!loadRawRegion(inimg_file, data1d, in_sz, in_zz,datatype,0,0,0,1,1,1))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    if(data1d) {delete []data1d; data1d = 0;}


//    temp_raw = QString(inimg_file) + "_temp.v3draw";
//    temp_gf = QString(inimg_file) + "_gf.v3draw";
//    temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";
//    temp_gsdt_v2 = QString(inimg_file) + "_gsdt_v2.v3draw";
//    temp_wogf = QString(inimg_file) + "_woGf.v3draw";
//
//
//    temp_soma = QString(inimg_file) + "_soma.v3draw";
//    temp_gsdtsoma  = QString(inimg_file) + "_gsdtsoma.v3draw";
//    temp_ds = QString(inimg_file) + "_ds.v3draw";
//    temp_gsdtds = QString(inimg_file) + "_gsdtds.v3draw";

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    V3DLONG sz2d = N*M;
    
    V3DLONG tilenum = (floor(N/(0.9*Ws))+1.0)*(floor(M/(0.9*Ws))+1.0);
    int count = 2;
    ifstream ifile(outimg_file);
    if (!ifile)
    {
       printf("Can not find the output folder");
       return false;
    }

    QString tc_name(outimg_file);
    tc_name.append("/stitched_image.tc");

    ofstream myfile;
    
    // FL added, if the file exist, delete it first
    myfile.open(tc_name.toStdString().c_str(), ios::in);
    
    if (myfile.is_open()==true)
    {
    	myfile.close();
       	remove(tc_name.toStdString().c_str());    	
    }
    // end of FL added
    
    
    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << tilenum << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << N << " " << M << " " << P << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";
    myfile.close(); // FL revise for multithreading

    
	std::vector<QString> EnhancedBlockFilelist;
    
//    unsigned char* EnhancedStack = 0;
    
//    if (saveEnhanced2DSections==1)
//    	EnhancedStack = new unsigned char [N*M*P]; // for saving enhanced 2D section purpose
	
    
    bool falsetag = true; //FL add, openMP does not allow exit from blocked structure, it can be set by any thread competitively
    
    omp_set_num_threads(numOfThreads);
    
    #pragma omp parallel for  // FL add parallelizing
    
	for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
//	for(V3DLONG iy = 0; iy < 10; iy = iy+Ws-Ws/10)

	{
		V3DLONG yb = iy;
		V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

		printf("number of threads for iy = %d\n", omp_get_num_threads());
		
		#pragma omp parllel for //FL add for parallelizing

		for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
//		for(V3DLONG ix = 0; ix < 10; ix = ix+Ws-Ws/10)
		
		{	
			
//			printf("number of threads for ix and iy = %d\n", omp_get_num_threads());
//			
//			printf("ix=%d, iy=%d\n", ix, iy);
			
			V3DLONG xb = ix;
			V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

			//set file names for calling plugin 
			QString temp_raw = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_temp.v3draw";
			QString temp_gf = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gf.v3draw";
			QString temp_gsdt = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdt.v3draw";
//			QString temp_gsdt_v2 = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdt_v2.v3draw";
//			QString temp_wogf = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_woGf.v3draw";
//		
//			QString temp_soma = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_soma.v3draw";
//			QString temp_gsdtsoma  = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdtsoma.v3draw";
//			QString temp_ds = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_ds.v3draw";
//			QString temp_gsdtds = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdtds.v3draw";
			
			// crop block
			unsigned char *blockarea=0;
			V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*P;
            double th_global = 0;

            /*blockarea = new unsigned char [blockpagesz];
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
            }*/

            if(!loadRawRegion(inimg_file, blockarea, in_sz, in_zz,datatype,xb,yb,0,xe+1,ye+1,P))
            {
                if(blockarea) {delete []blockarea; blockarea = 0;}
                cerr<<"load image "<<inimg_file<<" error!"<<endl;
                falsetag == false;
            }

            unsigned char *localEnahancedArea=0;
            try {localEnahancedArea = new unsigned char [blockpagesz];}
//            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea."); return false;} // FL comment out, openMP does not allow exit from blocked structure

            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea.",0); falsetag = false;} //FL

            if (falsetag == true) //FL
            {

                V3DLONG block_sz[4];
                block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;
               /* unsigned char *blockarea_median=0;
                int ws = 2;
                //apply median filter
                switch (datatype)
                {
                case V3D_UINT8: median_filter(blockarea, block_sz, ws, ws, ws, c,(unsigned char* &)blockarea_median);
                     break;
                     default: v3d_msg("Invalid data type. Do nothing."); return false;
                }

                if(blockarea) {delete []blockarea; blockarea =0;}*/

                simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);
	
				double sigma = 1;
				for(int scale = 0; scale < count; scale++)
				{
					unsigned char * data1d_gf = 0;
					unsigned char * gsdtld = 0;
					unsigned char* EnahancedImage = 0;
					switch (datatype)
					{
					case V3D_UINT8:
						if (scale==0) //do not filter for the scale 0
						{
	
							if(count ==2)
								sigma = 0.3;
	
							data1d_gf = new unsigned char [blockpagesz];
							memcpy(data1d_gf, blockarea, blockpagesz);
						}
						else
						{
							sigma = 1.2;
//							callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);
							
							callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf); //FL, change for multithreading
							
						}
	
//						callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld);
						callgsdtPlugin(callback,(unsigned char *)data1d_gf, block_sz, 1,th_global,(unsigned char* &)gsdtld, temp_gf, temp_gsdt); // FL, change for multithreading
						
						AdpThresholding_adpwindow((unsigned char *)data1d_gf, block_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
						
						break;
	 //                   default: v3d_msg("Invalid data type. Do nothing."); return false; //FL comment out, openMP does not allow exit from blocked structure
                        default: v3d_msg("Invalid data type. Do nothing.",0); falsetag = false; //FL
	
					}

					if (falsetag == true)
					{
			
							if (scale==0)
							{
								memcpy(localEnahancedArea, EnahancedImage, blockpagesz);
							}
							else
							{
							   for(V3DLONG i = 0; i<blockpagesz; i++)
							   {
								  if (localEnahancedArea[i] < EnahancedImage[i])
									  localEnahancedArea[i] = EnahancedImage[i];
								  else
									  localEnahancedArea[i] =  localEnahancedArea[i];
							   }
							   remove(temp_raw.toStdString().c_str());
							}
			
							if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
							if(gsdtld) {delete []gsdtld; gsdtld =0;}
							if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
							if(blockarea) {delete []blockarea; blockarea =0;}
						  //  if(blockarea_median) {delete []blockarea_median; blockarea_median =0;}
					}
					else scale = count; // FL, for exiting the for scale loop
				} // for scale end
		   
				
				if (falsetag == true)
				{
					QString outputTile(outimg_file);
					outputTile.append(QString("/x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
					simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)localEnahancedArea, block_sz, 1); //write individual enhanced block images
		
					#pragma omp critical  // FL, does not allow multiple thread write at the same time
					{
						
						// write the block location file
						myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app ); //FL revise for multithreading						
						QString outputilefull;
						
						QString fn = QString("x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye);
						outputilefull.append(fn);
						outputilefull.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(xb).arg(yb).arg(xe).arg(ye).arg(P-1));
						myfile << outputilefull.toStdString();
						myfile << "\n";
						myfile.close(); //FL revise for multithreading
						
						
//						// stcitch blocks back to the 3D enhanced stack
						if (saveEnhanced2DSections==1)
						{
						
							QString blockfn(outimg_file);
							blockfn.append(fn);
							
							cout << "blockfn=" << blockfn.toStdString().c_str() << endl;
							
							EnhancedBlockFilelist.push_back(blockfn);
						
//							bool tag_leftmost_block;
//							if (ix==0)
//								tag_leftmost_block = true;
//							else 
//								tag_leftmost_block = false;
//							
//							fusing(EnhancedStack, localEnahancedArea, xb, xe, yb, ye, in_sz, Ws, tag_leftmost_block);
						}
												
												
						
					}
					
					if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}
					
			
				}
				else ix = N; // FL, for exiting the for ix loop
				
			} //if (falsetag == true) end
			else ix = N; // FL, for exiting the for ix loop
		} //for ix end
	
		if (falsetag == false) iy = M; // FL, for exiting the iy loop

	} //for iy end

  //  myfile.close(); //FL revise for multithreading
	
//	if (data1d) {delete []data1d; data1d=0;}
	printf("finished enhancement\n");
	
    if (falsetag == false)
    {
//    	if (EnhancedStack) {delete []EnhancedStack; EnhancedStack=0;}
    	return false;
    }
    else
    {
		myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
		myfile << "\n# MST LUT\n";
		myfile.close();
		
		// write 2D enhanced sections

		if (saveEnhanced2DSections==1)
		{

			V3DLONG sz_enhanced2d[4];
			sz_enhanced2d[0]=in_sz[0]; 
			sz_enhanced2d[1]=in_sz[1]; 
			sz_enhanced2d[2]=1;
			sz_enhanced2d[3]=1;

			long enhancedBlockNum = EnhancedBlockFilelist.size();
			QString qstr_oriFileFolder(oriFileFolder);
			QStringList oriFileList = getSortedFileList(qstr_oriFileFolder, ".tif");
			
			#pragma omp parallel for  // FL add parallelizing
					
			for (long k=0; k<in_sz[2]; k++)
			{
				long cnt = 0;
				
				long m;

				
				unsigned char *EnhancedSectionImage = new unsigned char [sz2d];
				unsigned char *counterImg = new unsigned char [sz2d]; // compute how many times each pixel of EnhancedSectionImage is written by blocks

				unsigned int *tmpImg = new unsigned int [sz2d]; // tmpImg is unsigned int type of EnhancedSectionImage to prevent overflowing when adding

				// initialize tmpImg and counterImg
				long cc=0;
				for (long int i=0; i<in_sz[1]; i++)
				for (long int j=0; j<in_sz[0]; j++)
				{
					tmpImg[cc] = 0;
					counterImg[cc] = 0;
					cc++;
				}
				
				V3DLONG *sz_block3D = 0;
				V3DLONG *sz_block_section2D = 0;
				unsigned char *blockSection2D=0;
				
				
				for (int n=0; n<enhancedBlockNum; n++)
				{
					QString blockFileName = EnhancedBlockFilelist.at(n);
 		            
					// load section k of the current 3D block

//					char *bn = (char *)(blockFileName.toStdString().c_str();	

					QByteArray byteArray = blockFileName.toUtf8();
					char* bn = byteArray.data();
					
					cout << "blockFileName=" << bn << endl;

					
					V3DLONG xb, xe, yb, ye;
					
					// get xb, xe, yb, ye values, this will be changed depending how block filenames are given
					int ind0 = blockFileName.lastIndexOf("/");
					int ind1 = blockFileName.indexOf("_", ind0+1);
					int ind2 = blockFileName.indexOf("_", ind1+1);
					int ind3 = blockFileName.indexOf("_", ind2+1);
					int ind4 = blockFileName.indexOf("_", ind3+1);
					int ind5 = blockFileName.indexOf("_", ind4+1);
					int ind6 = blockFileName.lastIndexOf(".");
					
//					printf("%d, %d, %d, %d, %d, %d\n", ind1, ind2, ind3, ind4, ind5, ind6);
					
					xb = blockFileName.mid(ind1+1, ind2-(ind1+1)).toInt();
					xe = blockFileName.mid(ind2+1, ind3-(ind2+1)).toInt();
					yb = blockFileName.mid(ind4+1, ind5-(ind4+1)).toInt();
					ye = blockFileName.mid(ind5+1, ind6-(ind5+1)).toInt();
					
//					printf("xb=%d, xe=%d, yb=%d, ye=%d\n", xb, xe, yb, ye);
					

					loadRawRegion(bn, blockSection2D, sz_block3D, sz_block_section2D, datatype,0,0,k,xe-xb+1,ye-yb+1,k+1);
					
//					printf("block file has been loaded!\n");
					
					
					// fusing
					fusing2D(tmpImg, counterImg, blockSection2D, xb, xe, yb, ye, sz_enhanced2d);
					
					if (blockSection2D) {delete []blockSection2D; blockSection2D=0;}
					if (sz_block3D) {delete []sz_block3D; sz_block3D=0;}
					if (sz_block_section2D) {delete []sz_block_section2D; sz_block_section2D=0;}
					
				} //for (int n=0; n<enhancedBlockNum; n++)

				printf("Finish fusing section %d\n", k);
				
				// compute the final EnhancedSectionImage by averaging
				
				for (long i=0; i<sz_enhanced2d[1]; i++)
				{
					long tmp = i*sz_enhanced2d[0];
					long tmp2 = tmp;
					for (long j=0; j<sz_enhanced2d[0]; j++)
					{
						if (counterImg[tmp2]==0)
							EnhancedSectionImage[tmp2] = 0;
						else
							EnhancedSectionImage[tmp2] = (unsigned char)(tmpImg[tmp2]/counterImg[tmp2]);
						tmp2++;
					}
				}
				
				printf("Finish computing EnhancedSectionImage\n");
				
				
//				for (m=k*sz2d; m<(k+1)*sz2d; m++)
//				{
//					EnhancedSectionImage[cnt] = EnhancedStack[m];
//					cnt++;
//				}
				
				QString filename;
				int idx0, idx1;
				
				QString qstr_outimg_file = QString(outimg_file);
				
				
				cout << oriFileList.at(k).toStdString().c_str() << endl;
				
				
				idx0 = oriFileList.at(k).lastIndexOf('/');
				idx1 = oriFileList.at(k).lastIndexOf('.');
			
//				printf("idx0=%d, idx1=%d", idx0, idx1);
				filename = qstr_outimg_file + oriFileList.at(k).mid(idx0+1, idx1-idx0-1) + "_tubularity.tif";
				
//				cout<< filename.toStdString().c_str() << endl;
				
				saveImage(filename.toStdString().c_str(), EnhancedSectionImage, sz_enhanced2d, 1);
				
				if (EnhancedSectionImage) {delete []EnhancedSectionImage; EnhancedSectionImage = 0;} 
				if (tmpImg) {delete []tmpImg; tmpImg=0;}
				if (counterImg) {delete []counterImg; counterImg=0;}
				
				
			} //for (long k=0; k<in_sz[2]; k++)
			
//			if (EnhancedSectionImage) {delete []EnhancedSectionImage; EnhancedSectionImage = 0;} 
		} //if (saveEnhanced2DSections==1)
		
//    	if (EnhancedStack) {delete []EnhancedStack; EnhancedStack=0;}

		return true;
    }

    if (in_sz) {delete []in_sz; in_sz=0;}
    if (in_zz) {delete []in_zz; in_zz=0;}

}

bool processImage_adaptive_auto_blocks_indv_multithread_v2(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to adaptive enhancement filter with individual blocks v2"<<endl;
    if (output.size() != 1) return false;
    unsigned int Ws = 1000, c=1;
    float ratio = 1.0;

    // FL added the following 3
    unsigned int numOfThreads = 8; // default value for number of theads
    unsigned int saveEnhanced2DSections = 1; // default value for saving enhanced 2D section images, 1: save; 0: not save
    char *oriFileFolder;

    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) Ws = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
        if(paras.size() >= 3) ratio = atof(paras.at(2));
        if(paras.size() >= 4) numOfThreads = atoi(paras.at(3));
        if(paras.size() >= 5) saveEnhanced2DSections = atoi(paras.at(4));
        if(paras.size() >= 6) oriFileFolder = paras.at(5);

    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"Ws = "<<Ws<<endl;
    cout<<"c = "<<c<<endl;
    cout<<"ratio = "<<ratio<<endl;
    cout<<"numOfThreads = "<<numOfThreads<<endl;
    cout<<"saveEnhanced2DSections = "<<saveEnhanced2DSections<<endl;

    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_folder = "<<outimg_file<<endl;


   /* unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }*/

    unsigned char * data1d = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;

    if (!loadRawRegion(inimg_file, data1d, in_sz, in_zz,datatype,0,0,0,1,1,1))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }
    if(data1d) {delete []data1d; data1d = 0;}


//    temp_raw = QString(inimg_file) + "_temp.v3draw";
//    temp_gf = QString(inimg_file) + "_gf.v3draw";
//    temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";
//    temp_gsdt_v2 = QString(inimg_file) + "_gsdt_v2.v3draw";
//    temp_wogf = QString(inimg_file) + "_woGf.v3draw";
//
//
//    temp_soma = QString(inimg_file) + "_soma.v3draw";
//    temp_gsdtsoma  = QString(inimg_file) + "_gsdtsoma.v3draw";
//    temp_ds = QString(inimg_file) + "_ds.v3draw";
//    temp_gsdtds = QString(inimg_file) + "_gsdtds.v3draw";

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    V3DLONG sz2d = N*M;

    V3DLONG tilenum = (floor(N/(0.9*Ws))+1.0)*(floor(M/(0.9*Ws))+1.0);
    int count = 2;
    ifstream ifile(outimg_file);
    if (!ifile)
    {
       printf("Can not find the output folder");
       return false;
    }

    QString tc_name(outimg_file);
    tc_name.append("/stitched_image.tc");

    ofstream myfile;

    // FL added, if the file exist, delete it first
    myfile.open(tc_name.toStdString().c_str(), ios::in);

    if (myfile.is_open()==true)
    {
        myfile.close();
        remove(tc_name.toStdString().c_str());
    }
    // end of FL added


    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << tilenum << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << N << " " << M << " " << P << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";
    myfile.close(); // FL revise for multithreading


    std::vector<QString> EnhancedBlockFilelist;

//    unsigned char* EnhancedStack = 0;

//    if (saveEnhanced2DSections==1)
//    	EnhancedStack = new unsigned char [N*M*P]; // for saving enhanced 2D section purpose


    bool falsetag = true; //FL add, openMP does not allow exit from blocked structure, it can be set by any thread competitively

    omp_set_num_threads(numOfThreads);

    #pragma omp parallel for  // FL add parallelizing

    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
//	for(V3DLONG iy = 0; iy < 10; iy = iy+Ws-Ws/10)

    {
        V3DLONG yb = iy;
        V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

        printf("number of threads for iy = %d\n", omp_get_num_threads());

        #pragma omp parllel for //FL add for parallelizing

        for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
//		for(V3DLONG ix = 0; ix < 10; ix = ix+Ws-Ws/10)

        {

//			printf("number of threads for ix and iy = %d\n", omp_get_num_threads());
//
//			printf("ix=%d, iy=%d\n", ix, iy);

            V3DLONG xb = ix;
            V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

            //set file names for calling plugin
            QString temp_raw = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_temp.v3draw";
            QString temp_gf = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gf.v3draw";
            QString temp_gsdt = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdt.v3draw";
//			QString temp_gsdt_v2 = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdt_v2.v3draw";
//			QString temp_wogf = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_woGf.v3draw";
//
//			QString temp_soma = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_soma.v3draw";
//			QString temp_gsdtsoma  = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdtsoma.v3draw";
//			QString temp_ds = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_ds.v3draw";
//			QString temp_gsdtds = QString(inimg_file).append(QString("_x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye)) + "_gsdtds.v3draw";

            // crop block
            unsigned char *blockarea=0;
            V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*P;
            double th_global = 0;

            /*blockarea = new unsigned char [blockpagesz];
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
            }*/

            if(!loadRawRegion(inimg_file, blockarea, in_sz, in_zz,datatype,xb,yb,0,xe+1,ye+1,P))
            {
                if(blockarea) {delete []blockarea; blockarea = 0;}
                cerr<<"load image "<<inimg_file<<" error!"<<endl;
                falsetag == false;
            }

            unsigned char *localEnahancedArea=0;
            try {localEnahancedArea = new unsigned char [blockpagesz];}
//            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea."); return false;} // FL comment out, openMP does not allow exit from blocked structure

            catch(...)  {v3d_msg("cannot allocate memory for localEnahancedArea.",0); falsetag = false;} //FL

            if (falsetag == true) //FL
            {

                V3DLONG block_sz[4];
                block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;
               /* unsigned char *blockarea_median=0;
                int ws = 2;
                //apply median filter
                switch (datatype)
                {
                case V3D_UINT8: median_filter(blockarea, block_sz, ws, ws, ws, c,(unsigned char* &)blockarea_median);
                     break;
                     default: v3d_msg("Invalid data type. Do nothing."); return false;
                }

                if(blockarea) {delete []blockarea; blockarea =0;}*/

                simple_saveimage_wrapper(callback, temp_raw.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);

                double sigma = 1;
                for(int scale = 0; scale < count; scale++)
                {
                    unsigned char * data1d_gf = 0;
                    unsigned char * gsdtld = 0;
                    unsigned char* EnahancedImage = 0;
                    unsigned char* data1d_gf_ds = 0;

                    switch (datatype)
                    {
                    case V3D_UINT8:
                        if (scale==0) //do not filter for the scale 0
                        {

                            if(count ==2)
                                sigma = 0.3;

                            data1d_gf = new unsigned char [blockpagesz];
                            memcpy(data1d_gf, blockarea, blockpagesz);
                        }
                        else
                        {
                            sigma = 1.2;
//							callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf);

                            callGaussianPlugin(callback,blockpagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf); //FL, change for multithreading

                        }

                        break;
     //                   default: v3d_msg("Invalid data type. Do nothing."); return false; //FL comment out, openMP does not allow exit from blocked structure
                        default: v3d_msg("Invalid data type. Do nothing.",0); falsetag = false; //FL

                    }

                    V3DLONG block_sz_ds[4];
                    downsampling_img_xyz( data1d_gf, block_sz, 4, 4, data1d_gf_ds, block_sz_ds );


                    callgsdtPlugin(callback,(unsigned char *)data1d_gf_ds, block_sz_ds, 1,th_global,(unsigned char* &)gsdtld, temp_gf, temp_gsdt); // FL, change for multithreading

                    AdpThresholding_adpwindow_v2((unsigned char *)data1d_gf, block_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);


                    if (falsetag == true)
                    {

                            if (scale==0)
                            {
                                memcpy(localEnahancedArea, EnahancedImage, blockpagesz);
                            }
                            else
                            {
                               for(V3DLONG i = 0; i<blockpagesz; i++)
                               {
                                  if (localEnahancedArea[i] < EnahancedImage[i])
                                      localEnahancedArea[i] = EnahancedImage[i];
                                  else
                                      localEnahancedArea[i] =  localEnahancedArea[i];
                               }
                               remove(temp_raw.toStdString().c_str());
                            }

                            if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
                            if(gsdtld) {delete []gsdtld; gsdtld =0;}
                            if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
                            if(blockarea) {delete []blockarea; blockarea =0;}
                          //  if(blockarea_median) {delete []blockarea_median; blockarea_median =0;}
                    }
                    else scale = count; // FL, for exiting the for scale loop
                } // for scale end


                if (falsetag == true)
                {
                    QString outputTile(outimg_file);
                    outputTile.append(QString("/x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
                    simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)localEnahancedArea, block_sz, 1); //write individual enhanced block images

                    #pragma omp critical  // FL, does not allow multiple thread write at the same time
                    {

                        // write the block location file
                        myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app ); //FL revise for multithreading
                        QString outputilefull;

                        QString fn = QString("x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye);
                        outputilefull.append(fn);
                        outputilefull.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(xb).arg(yb).arg(xe).arg(ye).arg(P-1));
                        myfile << outputilefull.toStdString();
                        myfile << "\n";
                        myfile.close(); //FL revise for multithreading


//						// stcitch blocks back to the 3D enhanced stack
                        if (saveEnhanced2DSections==1)
                        {

                            QString blockfn(outimg_file);
                            blockfn.append(fn);

                            cout << "blockfn=" << blockfn.toStdString().c_str() << endl;

                            EnhancedBlockFilelist.push_back(blockfn);

//							bool tag_leftmost_block;
//							if (ix==0)
//								tag_leftmost_block = true;
//							else
//								tag_leftmost_block = false;
//
//							fusing(EnhancedStack, localEnahancedArea, xb, xe, yb, ye, in_sz, Ws, tag_leftmost_block);
                        }



                    }

                    if(localEnahancedArea) {delete []localEnahancedArea; localEnahancedArea =0;}


                }
                else ix = N; // FL, for exiting the for ix loop

            } //if (falsetag == true) end
            else ix = N; // FL, for exiting the for ix loop
        } //for ix end

        if (falsetag == false) iy = M; // FL, for exiting the iy loop

    } //for iy end

  //  myfile.close(); //FL revise for multithreading

//	if (data1d) {delete []data1d; data1d=0;}
    printf("finished enhancement\n");

    if (falsetag == false)
    {
//    	if (EnhancedStack) {delete []EnhancedStack; EnhancedStack=0;}
        return false;
    }
    else
    {
        myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
        myfile << "\n# MST LUT\n";
        myfile.close();

        // write 2D enhanced sections

        if (saveEnhanced2DSections==1)
        {

            V3DLONG sz_enhanced2d[4];
            sz_enhanced2d[0]=in_sz[0];
            sz_enhanced2d[1]=in_sz[1];
            sz_enhanced2d[2]=1;
            sz_enhanced2d[3]=1;

            long enhancedBlockNum = EnhancedBlockFilelist.size();
            QString qstr_oriFileFolder(oriFileFolder);
            QStringList oriFileList = getSortedFileList(qstr_oriFileFolder, ".tif");

            #pragma omp parallel for  // FL add parallelizing

            for (long k=0; k<in_sz[2]; k++)
            {
                long cnt = 0;

                long m;


                unsigned char *EnhancedSectionImage = new unsigned char [sz2d];
                unsigned char *counterImg = new unsigned char [sz2d]; // compute how many times each pixel of EnhancedSectionImage is written by blocks

                unsigned int *tmpImg = new unsigned int [sz2d]; // tmpImg is unsigned int type of EnhancedSectionImage to prevent overflowing when adding

                // initialize tmpImg and counterImg
                long cc=0;
                for (long int i=0; i<in_sz[1]; i++)
                for (long int j=0; j<in_sz[0]; j++)
                {
                    tmpImg[cc] = 0;
                    counterImg[cc] = 0;
                    cc++;
                }

                V3DLONG *sz_block3D = 0;
                V3DLONG *sz_block_section2D = 0;
                unsigned char *blockSection2D=0;


                for (int n=0; n<enhancedBlockNum; n++)
                {
                    QString blockFileName = EnhancedBlockFilelist.at(n);

                    // load section k of the current 3D block

//					char *bn = (char *)(blockFileName.toStdString().c_str();

                    QByteArray byteArray = blockFileName.toUtf8();
                    char* bn = byteArray.data();

                    cout << "blockFileName=" << bn << endl;


                    V3DLONG xb, xe, yb, ye;

                    // get xb, xe, yb, ye values, this will be changed depending how block filenames are given
                    int ind0 = blockFileName.lastIndexOf("/");
                    int ind1 = blockFileName.indexOf("_", ind0+1);
                    int ind2 = blockFileName.indexOf("_", ind1+1);
                    int ind3 = blockFileName.indexOf("_", ind2+1);
                    int ind4 = blockFileName.indexOf("_", ind3+1);
                    int ind5 = blockFileName.indexOf("_", ind4+1);
                    int ind6 = blockFileName.lastIndexOf(".");

//					printf("%d, %d, %d, %d, %d, %d\n", ind1, ind2, ind3, ind4, ind5, ind6);

                    xb = blockFileName.mid(ind1+1, ind2-(ind1+1)).toInt();
                    xe = blockFileName.mid(ind2+1, ind3-(ind2+1)).toInt();
                    yb = blockFileName.mid(ind4+1, ind5-(ind4+1)).toInt();
                    ye = blockFileName.mid(ind5+1, ind6-(ind5+1)).toInt();

//					printf("xb=%d, xe=%d, yb=%d, ye=%d\n", xb, xe, yb, ye);


                    loadRawRegion(bn, blockSection2D, sz_block3D, sz_block_section2D, datatype,0,0,k,xe-xb+1,ye-yb+1,k+1);

//					printf("block file has been loaded!\n");


                    // fusing
                    fusing2D(tmpImg, counterImg, blockSection2D, xb, xe, yb, ye, sz_enhanced2d);

                    if (blockSection2D) {delete []blockSection2D; blockSection2D=0;}
                    if (sz_block3D) {delete []sz_block3D; sz_block3D=0;}
                    if (sz_block_section2D) {delete []sz_block_section2D; sz_block_section2D=0;}

                } //for (int n=0; n<enhancedBlockNum; n++)

                printf("Finish fusing section %d\n", k);

                // compute the final EnhancedSectionImage by averaging

                for (long i=0; i<sz_enhanced2d[1]; i++)
                {
                    long tmp = i*sz_enhanced2d[0];
                    long tmp2 = tmp;
                    for (long j=0; j<sz_enhanced2d[0]; j++)
                    {
                        if (counterImg[tmp2]==0)
                            EnhancedSectionImage[tmp2] = 0;
                        else
                            EnhancedSectionImage[tmp2] = (unsigned char)(tmpImg[tmp2]/counterImg[tmp2]);
                        tmp2++;
                    }
                }

                printf("Finish computing EnhancedSectionImage\n");


//				for (m=k*sz2d; m<(k+1)*sz2d; m++)
//				{
//					EnhancedSectionImage[cnt] = EnhancedStack[m];
//					cnt++;
//				}

                QString filename;
                int idx0, idx1;

                QString qstr_outimg_file = QString(outimg_file);


                cout << oriFileList.at(k).toStdString().c_str() << endl;


                idx0 = oriFileList.at(k).lastIndexOf('/');
                idx1 = oriFileList.at(k).lastIndexOf('.');

//				printf("idx0=%d, idx1=%d", idx0, idx1);
                filename = qstr_outimg_file + oriFileList.at(k).mid(idx0+1, idx1-idx0-1) + "_tubularity.tif";

//				cout<< filename.toStdString().c_str() << endl;

                saveImage(filename.toStdString().c_str(), EnhancedSectionImage, sz_enhanced2d, 1);

                if (EnhancedSectionImage) {delete []EnhancedSectionImage; EnhancedSectionImage = 0;}
                if (tmpImg) {delete []tmpImg; tmpImg=0;}
                if (counterImg) {delete []counterImg; counterImg=0;}


            } //for (long k=0; k<in_sz[2]; k++)

//			if (EnhancedSectionImage) {delete []EnhancedSectionImage; EnhancedSectionImage = 0;}
        } //if (saveEnhanced2DSections==1)

//    	if (EnhancedStack) {delete []EnhancedStack; EnhancedStack=0;}

        return true;
    }

    if (in_sz) {delete []in_sz; in_sz=0;}
    if (in_zz) {delete []in_zz; in_zz=0;}

}
#endif


bool processImage_detect_soma(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{

    cout<<"Welcome to soma detection"<<endl;
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

    if(infiles.empty())
    {
        cerr<<"Need input image"<<endl;
        return false;
    }

    QString  inimg_file =  infiles[0];
    int k=0;

    QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
    if(inmarker_file.isEmpty())
    {
        cerr<<"Need a marker file"<<endl;
        return false;
    }

    QString tcfilename  =  paras.empty() ? "" : paras[k]; if(tcfilename == "NULL") tcfilename = "";
    if(tcfilename.isEmpty())
    {
        cerr<<"Need a tc file"<<endl;
        return false;
    }


    cout<<"inimg_file = "<<inimg_file.toStdString().c_str()<<endl;
    cout<<"inmarker_file = "<<inmarker_file.toStdString().c_str()<<endl;
    cout<<"tcfilename = "<<tcfilename.toStdString().c_str()<<endl;

    vector<MyMarker> file_inmarkers;
    file_inmarkers = readMarker_file(string(qPrintable(inmarker_file)));
    int soma_x = file_inmarkers[0].x;
    int soma_y = file_inmarkers[0].y;

    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    if( !vim.y_load(tcfilename.toStdString()) )
    {
        printf("Wrong stitching configuration file to be load!\n");
        return false;
    }

    unsigned char * data1d_enhanced = 0;
    V3DLONG *sz_enhanced = 0;
    int datatype_enhanced = 0;

    QString curFilePath = QFileInfo(tcfilename).path();
    curFilePath.append("/");
    string fn;

    V3DLONG index_tile = -1;
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        if(soma_x > vim.lut[ii].start_pos[0] && soma_y > vim.lut[ii].start_pos[1] && soma_x < vim.lut[ii].end_pos[0] && soma_y < vim.lut[ii].end_pos[1])
        {
            cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;
            fn = curFilePath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();

            if (loadImage(const_cast<char *>(fn.c_str()), data1d_enhanced, sz_enhanced, datatype_enhanced)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return false;
            }

            index_tile = ii;
            break;

         }
    }

//    unsigned char * data1d_original = 0;
//    int datatype;
//    V3DLONG in_zz[4];

//    if(!simple_loadimage_wrapper(callback,inimg_file.toStdString().c_str(), data1d_original, in_zz, datatype))
//    {
//        cerr<<"load image "<<inimg_file.toStdString().c_str()<<" error!"<<endl;
//        return false;
//    }

//    V3DLONG N = in_zz[0];
//    V3DLONG M = in_zz[1];
//    V3DLONG P = in_zz[2];

    unsigned char * data1d_region = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;

    int datatype;

//    V3DLONG blockpagesz = (vim.lut[index_tile].end_pos[1] - vim.lut[index_tile].start_pos[1] + 1) * (vim.lut[index_tile].end_pos[0] - vim.lut[index_tile].start_pos[0] + 1) * P;
//    try {data1d_region = new unsigned char [blockpagesz];}
//    catch(...)  {v3d_msg("cannot allocate memory for data1d_region."); return false;}

    if (!loadRawRegion((char *)inimg_file.toStdString().c_str(), data1d_region, in_sz, in_zz,datatype,vim.lut[index_tile].start_pos[0],vim.lut[index_tile].start_pos[1],
                                    vim.lut[index_tile].start_pos[2],vim.lut[index_tile].end_pos[0]+1,vim.lut[index_tile].end_pos[1]+1,vim.lut[index_tile].end_pos[2]+1))
    {
        cerr<<"load image "<<inimg_file.toStdString().c_str()<<" error!"<<endl;
        return false;
    }


//    V3DLONG i = 0;
//    for(V3DLONG iz = 0; iz < P; iz++)
//    {
//        V3DLONG offsetk = iz*M*N;
//        for(V3DLONG iy = vim.lut[index_tile].start_pos[1]; iy <= vim.lut[index_tile].end_pos[1]; iy++)
//        {
//            V3DLONG offsetj = iy*N;
//            for(V3DLONG ix = vim.lut[index_tile].start_pos[0]; ix <= vim.lut[index_tile].end_pos[0]; ix++)
//            {

//                data1d_region[i] = data1d_original[offsetk + offsetj + ix];
//                i++;
//            }
//        }
//    }

//    if(data1d_original) {delete  []data1d_original; data1d_original = 0;}

//    QString temp_raw = inimg_file + "_temp.v3draw";
//    QString temp_gf = inimg_file + "_gf.v3draw";
//    QString temp_gsdt = inimg_file + "_gsdt.v3draw";
//    QString temp_gsdt_v2 = inimg_file + "_gsdt_v2.v3draw";
//    QString temp_wogf = inimg_file + "_woGf.v3draw";


    QString temp_soma = inimg_file + "_soma.v3draw";
    QString temp_gsdtsoma  = inimg_file + "_gsdtsoma.v3draw";
    QString temp_ds = inimg_file + "_ds.v3draw";
    QString temp_gsdtds = inimg_file + "_gsdtds.v3draw";


    ImageMarker S;
    QList <ImageMarker> marklist;
    S.x = file_inmarkers[0].x - vim.lut[index_tile].start_pos[0] + 1;
    S.y = file_inmarkers[0].y - vim.lut[index_tile].start_pos[1] + 1;
    S.z = file_inmarkers[0].z;

    LandmarkList listLandmarks;
    LocationSimple tmpLocation(S.x,S.y ,S.z);

    listLandmarks.append(tmpLocation);
    unsigned char* Enhancement_soma = 0;
    enhancementWithsoma(callback,(unsigned char *)data1d_region,(unsigned char*)data1d_enhanced,sz_enhanced,1,(unsigned char *&)Enhancement_soma, temp_soma, temp_gsdtsoma, temp_ds, temp_gsdtds,listLandmarks);
    string image_nosoma = fn +("_nosoma.raw");
    rename(fn.c_str(),image_nosoma.c_str());
    simple_saveimage_wrapper(callback,fn.c_str(), (unsigned char *)Enhancement_soma, sz_enhanced, 1);

    marklist.append(S);
    string markerpath = fn +(".marker");
    writeMarker_file(markerpath.c_str(),marklist);

    if(Enhancement_soma) {delete Enhancement_soma; Enhancement_soma = 0;}
    if(data1d_region) {delete data1d_region; data1d_region = 0;}

    return true;
}

template <class T> void selective_Enhancement(const T* data1d,
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
                                           T* &outimg,
                                           QString temp_raw,
                                           QString temp_gf)
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

    arg.type = "random";
    std::vector<char*> args1;
    char* char_temp_raw =  new char[temp_raw.length() + 1];
    strcpy(char_temp_raw, temp_raw.toStdString().c_str());
    args1.push_back(char_temp_raw); 
    arg.p = (void *) & args1; 
    input<< arg;

    arg.type = "random";
    std::vector<char*> args;
    char channel = '0' + c;
    string winx2 = boost::lexical_cast<string>(ws); 
    char* winx =  new char[winx2.length() + 1]; 
    strcpy(winx, winx2.c_str());
    string sig2 = boost::lexical_cast<string>(sigma); 
    char* sig =  new char[sig2.length() + 1]; 
    strcpy(sig, sig2.c_str());

    args.push_back(winx);
    args.push_back(winx);
    args.push_back(winx);
    args.push_back(&channel);
    args.push_back(sig); 
    arg.p = (void *) & args; 
    input << arg;
    
    char* char_temp_gf =  new char[temp_gf.length() + 1];
    strcpy(char_temp_gf, temp_gf.toStdString().c_str());
    arg.type = "random";
    std::vector<char*> args2;
    args2.push_back(char_temp_gf); 
    arg.p = (void *) & args2; 
    output<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_Gf = 0;
    int datatype;
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    simple_loadimage_wrapper(callback, outimg_file, data1d_Gf, in_zz, datatype);
    
    remove(temp_gf.toStdString().c_str());
    
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
                                                  T* &outimg, 
                                                  const T* gsdtdatald,
                                                  unsigned int dim,
                                                  double ratio)
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

    double LOG2 = log(2.0);
//    V3DLONG DSN = floor(in_sz[0]/4);
//    V3DLONG DSM = floor(in_sz[1]/4);
//    V3DLONG DSP = floor(in_sz[2]/4);

//    V3DLONG pageszDS = DSN*DSM*DSP;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        printf("\r Enhancement : %d %% completed ", int(float(iz)/P*100)); fflush(stdout);
        V3DLONG offsetk = iz*M*N;
      //  V3DLONG dsoffsetk = floor(iz/4)*DSM*DSN;

        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
        //    V3DLONG dsoffsetj = floor(iy/4)*DSN;

            for(V3DLONG ix = 0; ix < N; ix++)
            {

              /*  double pixelWin = 0;

                V3DLONG xb = ix-3; if(xb<0) xb = 0;
                V3DLONG xe = ix+3; if(xe>=N-1) xe = N-1;
                V3DLONG yb = iy-3; if(yb<0) yb = 0;
                V3DLONG ye = iy+3; if(ye>=M-1) ye = M-1;
                V3DLONG zb = iz-3; if(zb<0) zb = 0;
                V3DLONG ze = iz+3; if(ze>=P-1) ze = P-1;

                for(V3DLONG k=zb; k<=ze; k++)
                {
                    V3DLONG offsetkl = k*M*N;
                    for(V3DLONG j=yb; j<=ye; j++)
                    {
                        V3DLONG offsetjl = j*N;
                        for(V3DLONG i=xb; i<=xe; i++)
                        {
                            T dataval = data1d[ offsetc + offsetkl + offsetjl + i];
                            pixelWin = pixelWin + dataval;
                        }
                    }
                }

                pixelWin = pixelWin/21.0;*/
//                V3DLONG DSindex_pim = dsoffsetk+dsoffsetj+floor(ix/4);
//                T GsdtValue;
//                if(DSindex_pim < pageszDS)
//                    GsdtValue = gsdtdatald[DSindex_pim];
//                else
//                    GsdtValue = 1;
                T GsdtValue = gsdtdatald[offsetk + offsetj + ix];
                T PixelValue = data1d[offsetc+offsetk + offsetj + ix];

//#define  __PHC_DEBUG__
#ifdef __PHC_DEBUG__

                Wx = Wy = Wz = 1;
#else
                Wx = (int)round((ratio*log(double(GsdtValue)+1.0)/LOG2));
               // Wx = (int)round((ratio*log(double(pixelWin)+1.0)/LOG2));
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
#endif

                if (Wx > 0 && PixelValue > th)
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

template <class T> void AdpThresholding_adpwindow_v2(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  double sigma,
                                                  T* &outimg,
                                                  const T* gsdtdatald,
                                                  unsigned int dim,
                                                  double ratio)
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

    double LOG2 = log(2.0);
    V3DLONG DSN = floor(in_sz[0]/4);
    V3DLONG DSM = floor(in_sz[1]/4);
    V3DLONG DSP = floor(in_sz[2]/4);

    V3DLONG pageszDS = DSN*DSM*DSP;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        printf("\r Enhancement : %d %% completed ", int(float(iz)/P*100)); fflush(stdout);
        V3DLONG offsetk = iz*M*N;
        V3DLONG dsoffsetk = floor(iz/4)*DSM*DSN;

        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            V3DLONG dsoffsetj = floor(iy/4)*DSN;

            for(V3DLONG ix = 0; ix < N; ix++)
            {
                V3DLONG DSindex_pim = dsoffsetk+dsoffsetj+floor(ix/4);
                T GsdtValue;
                if(DSindex_pim < pageszDS)
                    GsdtValue = gsdtdatald[DSindex_pim];
                else
                    GsdtValue = 1;
                T PixelValue = data1d[offsetc+offsetk + offsetj + ix];

//#define  __PHC_DEBUG__
#ifdef __PHC_DEBUG__

                Wx = Wy = Wz = 1;
#else
                Wx = (int)round((ratio*log(double(GsdtValue)+1.0)/LOG2));
               // Wx = (int)round((ratio*log(double(pixelWin)+1.0)/LOG2));
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
#endif

                if (Wx > 0 && PixelValue > th)
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
                                       T* &outimg,
                                       QString temp_gf,
                                       QString temp_gsdt)
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
    simple_saveimage_wrapper(callback, temp_gf.toStdString().c_str(), (unsigned char *)data1d, in_sz, 1);

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    char* char_temp_gf =  new char[temp_gf.length() + 1];strcpy(char_temp_gf, temp_gf.toStdString().c_str());
    args1.push_back(char_temp_gf); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    string threshold = boost::lexical_cast<string>(th_final); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
    char* char_temp_gsdt =  new char[temp_gsdt.length() + 1];strcpy(char_temp_gsdt, temp_gsdt.toStdString().c_str());
    arg.type = "random";std::vector<char*> args2;args2.push_back(char_temp_gsdt); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_gsdt = 0;
    int datatype;
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    simple_loadimage_wrapper(callback, outimg_file, data1d_gsdt, in_zz, datatype);
    remove(temp_gf.toStdString().c_str());
    remove(temp_gsdt.toStdString().c_str());

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

// stitch 2D block sections to a bigger 2D section image
template <class T> void fusing2D(unsigned int *data, unsigned char *counterImg, T *blockSection2D, V3DLONG xb, V3DLONG xe, V3DLONG yb, V3DLONG ye, V3DLONG *sz_enhanced2d)
{
	long tmp1, tmp2;

	long cnt = 0;
	
	for (long int i=yb; i<=ye; i++)
	{
		tmp1 = i*sz_enhanced2d[0];
											
		for (long int j=xb; j<=xe; j++)
		{
			tmp2 = tmp1+j;
			data[tmp2] = data[tmp2] + (unsigned int)blockSection2D[cnt];
			counterImg[tmp2]++;
			cnt++;
		}			
		
	}
				
	return;
}

// stitch blocks to generate a stack, FL added
template <class T> void fusing(T *data1d, T *block1d, V3DLONG xb, V3DLONG xe, V3DLONG yb, V3DLONG ye, V3DLONG *sz_data1d, unsigned int ws, bool tag_leftmost_block)
{
    
	long cnt = 0;
	long tmp, tmp1, tmp2;
	long sz_data1d_xy = sz_data1d[0]*sz_data1d[1];
	
	long xo = xb+ws/10-1; //overlap area width
	
	for (int k=0; k<sz_data1d[2]; k++)
	{
		tmp = k*sz_data1d_xy;
		
		for (int i=yb; i<=ye; i++)
		{
			tmp1 = tmp + i*sz_data1d[0];
			                                    
			if (tag_leftmost_block == true) // leftmost block do not do averaging between overlapping area
			{
			
				for (int j=xb; j<=xe; j++)
				{
					tmp2 = tmp1+j;
					data1d[tmp2] = block1d[cnt];
					cnt++;
				}			
			}
			else
			{
				for (int j=xb; j<=xo; j++)
				{
					tmp2 = tmp1 + j;
					data1d[tmp2] = (T)(((double)data1d[tmp2] + (double)block1d[cnt])/2);
					cnt++;
				}
				
				// for overlapping area
				for (int j=xo+1; j<=xe; j++)
				{
					tmp2 = tmp1 + j;
					data1d[tmp2] = block1d[cnt];
					cnt++;
				}
				
			}
			
		}
				
	}

	return;
  
}

template <class T> void enhancementWithsoma(V3DPluginCallback2 &callback,
                                            const T* data1d,
                                            T* data1d_enhanced,
                                            V3DLONG *in_sz,
                                            unsigned int c,
                                            T* &outimg,
                                            QString temp_soma,
                                            QString temp_gsdtsoma,
                                            QString temp_ds,
                                            QString temp_gsdtds,
                                            LandmarkList listLandmarks)

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
    int soma_z = 0;

    if(listLandmarks.count() ==0)
        somalocation(callback,data1d,in_sz,1,soma_x,soma_y,soma_z, temp_ds, temp_gsdtds);
    else
    {
        soma_x = listLandmarks.at(0).x;
        soma_y = listLandmarks.at(0).y;
        soma_z = listLandmarks.at(0).z;

    }
    printf("\n soma location is (%d, %d, %d)\n\n", soma_x,soma_y,soma_z);

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;
    V3DLONG offsetc = (c-1)*pagesz;

    double th_soma =  data1d[offsetc + soma_z*M*N + soma_y*N + soma_x] - 5;

    V3DLONG xb = soma_x-200; if(xb<0) xb = 0;
    V3DLONG xe = soma_x+200; if(xe>=N-1) xe = N-1;
    V3DLONG yb = soma_y-200; if(yb<0) yb = 0;
    V3DLONG ye = soma_y+200; if(ye>=M-1) ye = M-1;
    V3DLONG somasz = (xe-xb)*(ye-yb)*P;

  /*  for(V3DLONG iz = 0; iz < P; iz++)
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
    }*/

    void* somaarea = 0;
    soma_detection((unsigned char*)data1d, in_sz, c,soma_x,soma_y,somasz,(unsigned char* &)somaarea);


    V3DLONG soma_sz[4];
    soma_sz[0] = xe-xb; soma_sz[1] = ye-yb; soma_sz[2] = P; soma_sz[3] = 1;
    simple_saveimage_wrapper(callback,temp_soma.toStdString().c_str(), (unsigned char *)somaarea, soma_sz, 1);
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;


    arg.type = "random";std::vector<char*> args1;
    char* char_temp_soma=  new char[temp_soma.length() + 1];
    strcpy(char_temp_soma, temp_soma.toStdString().c_str());
    args1.push_back(char_temp_soma); 
    arg.p = (void *) & args1; 
    input<< arg;
    
    arg.type = "random";
    std::vector<char*> args;
    char channel = '0' + (c-1);
    string threshold = boost::lexical_cast<string>(th_soma); 
    char* threshold2 =  new char[threshold.length() + 1]; 
    strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");
    args.push_back(&channel);args.push_back("1"); 
    arg.p = (void *) & args; 
    input << arg;
    char* char_temp_gsdtsoma=  new char[temp_gsdtsoma.length() + 1];
    strcpy(char_temp_gsdtsoma, temp_gsdtsoma.toStdString().c_str());
    arg.type = "random";std::vector<char*> args2;
    args2.push_back(char_temp_gsdtsoma); 
    arg.p = (void *) & args2; 
    output<< arg;

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

    remove(temp_soma.toStdString().c_str());
    remove(temp_gsdtsoma.toStdString().c_str());

    int Th_gsdt = 50; //was 50

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
                                     int &soma_y,
                                     int &soma_z,
                                     QString temp_ds,
                                     QString temp_gsdtds)
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
    simple_saveimage_wrapper(callback,temp_ds.toStdString().c_str(), (unsigned char *)datald_downsample, in_ds, 1);

    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> args1;
    char* char_temp_ds=  new char[temp_ds.length() + 1];strcpy(char_temp_ds, temp_ds.toStdString().c_str());
    args1.push_back(char_temp_ds); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    string threshold = boost::lexical_cast<string>(th_gsdt); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
    args.push_back(threshold2);args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
    char* char_temp_gsdtds=  new char[temp_gsdtds.length() + 1];strcpy(char_temp_gsdtds, temp_gsdtds.toStdString().c_str());
    arg.type = "random";std::vector<char*> args2;args2.push_back(char_temp_gsdtds); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gsdt";
    QString func_name = "gsdt";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d_gsdt_ds = 0;
    int datatype;
    V3DLONG in_zz[4];

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    simple_loadimage_wrapper(callback,outimg_file, data1d_gsdt_ds, in_zz, datatype);
    remove(temp_ds.toStdString().c_str());
    remove(temp_gsdtds.toStdString().c_str());

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
                    soma_z = dsiz*4;

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

template <class T> void median_filter(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Wx,
                                      unsigned int Wy,
                                      unsigned int Wz,
                                      unsigned int c,
                                      T* &outimg)
{

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M*P;

    T *arr,tmp;
    int ii,jj;
    int size = (2*Wx+1)*(2*Wy+1)*(2*Wz+1);
    arr = new T[size];

    //filtering
    V3DLONG offsetc = (c-1)*pagesz;

    //declare temporary pointer
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

    //Median Filtering
    for(V3DLONG iz = 0; iz < P; iz++)
    {
         printf("\r median filter : %d %% completed ", ((iz + 1)*100) / P);fflush(stdout);
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
                ii = 0;

                for(V3DLONG k=zb; k<=ze; k++)
                {
                    V3DLONG offsetkl = k*M*N;
                    for(V3DLONG j=yb; j<=ye; j++)
                    {
                        V3DLONG offsetjl = j*N;
                        for(V3DLONG i=xb; i<=xe; i++)
                        {
                            T dataval = data1d[ offsetc + offsetkl + offsetjl + i];
                            arr[ii] = dataval;
                            if (ii>0)
                            {
                                jj = ii;
                                while(jj > 0 && arr[jj-1]>arr[jj])
                                {
                                    tmp = arr[jj];
                                    arr[jj] = arr[jj-1];
                                    arr[jj-1] = tmp;
                                    jj--;
                                }
                            }
                            ii++;
                        }
                    }
                }


                //set value
                V3DLONG index_pim = offsetk + offsetj + ix;
                pImage[index_pim] = arr[int(0.5*ii)+1];
            }
        }
    }
printf("\n");

    outimg = pImage;
    delete [] arr;

}
