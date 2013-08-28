/* local_enhancement_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-05 : by Zhi Zhou
 */

#include "v3d_message.h"
#include <vector>
#include "local_enhancement_plugin.h"
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
Q_EXPORT_PLUGIN2(local_enhancement, local_enhancement);

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage1(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);


template <class T> void AdpThresholding_adpwindow(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  T* &outimg, const T* gsdtdata1d);

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


template <class T> void block_detection(T* data1d,
                                        V3DLONG *in_sz,
                                        unsigned int c,double &th,
                                        T* &outimg,V3DLONG xb, V3DLONG xe, V3DLONG yb,V3DLONG ye);
template <class SDATATYPE> int pwi_fusing(SDATATYPE *data1d, SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, V3DLONG *offset, V3DLONG new_sz0, V3DLONG new_sz1, V3DLONG new_sz2);

template <class T> void soma_detection(T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       int x,
                                       int y,
                                       V3DLONG somasize,
                                       T* &outimg);

QStringList local_enhancement::menulist() const
{
    return QStringList()
            <<tr("localEnhancement")
           <<tr("about");
}

QStringList local_enhancement::funclist() const
{
    return QStringList()
            <<tr("localEnhancement")
           <<tr("help");
}

void local_enhancement::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("localEnhancement"))
    {
        processImage(callback,parent);
    }
    else
    {
        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
                   "Developed by Zhi Zhou, 2013-08-05"));
    }
}

bool local_enhancement::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("localEnhancement"))
    {
        return processImage1(input, output,callback);
    }
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f localEnhancement -i <inimg_file> -o <outimg_file> -p <ws> <ch>"<<endl;
        cout<<endl;
        cout<<"ws          block window size (pixel #), default 1000"<<endl;
        cout<<"ch          the input channel value, default 1 and start from 1, default 1"<<endl;
        cout<<endl;
        cout<<endl;
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
    ImagePixelType pixeltype = p4DImage->getDatatype();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;


    //input
    bool ok1,ok4;
    unsigned int Ws=1000, c=1;
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
            ok4=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok4);
        }
    }
    else
        return;
    //soma location
    /*V3DLONG offsetc = (c-1)*pagesz;
    int soma_x,soma_y,soma_z;
    LandmarkList soma_center = callback.getLandmark(curwin);
    if(soma_center.count()== 0)
    {
        QMessageBox::information(0, "", "Please define the soma location using the marker.");
        return;
    }
    LocationSimple tmpLocation(0,0,0);
    tmpLocation = soma_center.at(0);
    tmpLocation.getCoord(soma_x,soma_y,soma_z);


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
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    int datatype;
    V3DLONG * in_zz = 0;

    loadImage(outimg_file, gsdtsoma, in_zz, datatype,0);
    remove("temp.v3draw");
    remove("gsdtImage.v3draw");*/
    saveImage("temp.v3draw", (unsigned char *)data1d, in_sz, pixeltype);
    //invoke gsdt function
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;


    arg.type = "random";std::vector<char*> args1;
    args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
    arg.type = "random";std::vector<char*> args;
    char channel = '0' + c;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back(&channel); args.push_back("1.0"); arg.p = (void *) & args; input << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage.v3draw"); arg.p = (void *) & args2; output<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input,output);

    unsigned char * data1d3 = 0;
    int datatype;
    V3DLONG * in_zz = 0;

    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    loadImage(outimg_file, data1d3, in_zz, datatype);
    remove("temp.v3draw");
    remove("gfImage.v3draw");

    V3DLONG i = 0;
    double th_global = 0;
    V3DLONG offsetc = (c-1)*pagesz;
	unsigned char* data1d2 = 0;
    data1d2 = new unsigned char [pagesz];
	
	double min,max;
	rescale_to_0_255_and_copy((float *)data1d3,pagesz,min,max,data1d2);


    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy <  M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

                double PixelVaule = data1d2[offsetc+offsetk + offsetj + ix];
                PixelSum = PixelSum + PixelVaule;
                i++;
            }
        }
        th_global = th_global + PixelSum/(M*N*P);
    }


    // V3DLONG Ws = 1000;
    int county = 0;
    unsigned char* subject1d_y = NULL;
    unsigned char* target1d_y = NULL;
    V3DLONG szSub_y[4];
    V3DLONG szTar_y[4];

    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
    {
        unsigned char* subject1d = NULL;
        unsigned char* target1d = NULL;

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

            double  th = 0;
            void* blockarea = 0;
            switch (pixeltype)
            {
            case V3D_UINT8: block_detection(data1d2, in_sz, c, th,(unsigned char* &)blockarea,xb,xe,yb,ye); break;
            case V3D_UINT16: block_detection((unsigned short int *)data1d2, in_sz, c, th,(unsigned short int* &)blockarea,xb,xe,yb,ye); break;
            case V3D_FLOAT32: block_detection((float *)data1d2, in_sz, c,th,(float* &)blockarea,xb,xe,yb,ye);break;
            default: v3d_msg("Invalid data type. Do nothing."); return;
            }

            if(th < th_global) th = th_global;
            V3DLONG block_sz[4];
            block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;
            saveImage("temp.v3draw", (unsigned char *)blockarea, block_sz, 1);
            V3DPluginArgItem arg;
            V3DPluginArgList input;
            V3DPluginArgList output;

            arg.type = "random";std::vector<char*> args1;
            args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
            arg.type = "random";std::vector<char*> args;
            string threshold = boost::lexical_cast<string>(th); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
            args.push_back(threshold2);args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
            arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

            QString full_plugin_name = "gsdt";
            QString func_name = "gsdt";

            callback.callPluginFunc(full_plugin_name,func_name, input,output);

            unsigned char * gsdtblock = 0;
            int datatype;
            V3DLONG * in_zz = 0;
            char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
            loadImage(outimg_file, gsdtblock, in_zz, datatype,0);
            remove("temp.v3draw");
            remove("gsdtImage.v3draw");
            unsigned char* localEnahancedArea = NULL;

            switch (pixeltype)
            {
            case V3D_UINT8: AdpThresholding_adpwindow((unsigned char *)blockarea, block_sz, 1,(unsigned char* &)localEnahancedArea, gsdtblock); break;
            case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)blockarea, block_sz, 1, (unsigned short int* &)localEnahancedArea,(unsigned short int *)gsdtblock); break;
            case V3D_FLOAT32: AdpThresholding_adpwindow((float *)blockarea, block_sz, 1, (float* &)localEnahancedArea,(float *)gsdtblock);break;
            default: v3d_msg("Invalid data type. Do nothing."); return;
            }


            if (count==0)
            {

                V3DLONG targetsize = block_sz[0]*block_sz[1]*block_sz[2];
                target1d = new unsigned char [targetsize];
                for(int i = 0; i<targetsize;i++)
                    target1d[i] = localEnahancedArea[i];
                szTar[0] = xe-xb+1; szTar[1] = ye-yb+1; szTar[2] = P; szTar[3] = 1;

              }
            else
            {

                V3DLONG subjectsize = block_sz[0]*block_sz[1]*block_sz[2];
                subject1d = new unsigned char [subjectsize];
                for(int i = 0; i<subjectsize;i++)
                    subject1d[i] = localEnahancedArea[i];
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
                for(int i = 0; i<targetsize;i++)
                    target1d[i] = data1d_blended[i];
                szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = new_sz2; szTar[3] = 1;




            }
            count ++;
        }


        if (county==0)
        {

            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            for(int i = 0; i<targetsize_y;i++)
                target1d_y[i] = target1d[i];
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = P; szTar_y[3] = 1;


        }else
        {
            V3DLONG subjectsize_y = new_sz0*new_sz1*new_sz2;
            subject1d_y = new unsigned char [subjectsize_y];
            for(int i = 0; i<subjectsize_y;i++)
                subject1d_y[i] = target1d[i];
            szSub_y[0] = new_sz0; szSub_y[1] = new_sz1; szSub_y[2] = P; szSub_y[3] = 1;

            V3DLONG *offset = new V3DLONG [3];
            offset[0] = 0;
            offset[1] = yb;
            offset[2] = 0;
            new_sz0 = szSub_y[0];
            new_sz1 = ye+1;
            new_sz2 = szSub_y[2];

            V3DLONG totalplxs = new_sz0*new_sz1*new_sz2;
            unsigned char* data1d_blended_y = NULL;
            data1d_blended_y = new unsigned char [totalplxs];
            memset(data1d_blended_y, 0, sizeof(unsigned char)*totalplxs);
            int success;
            success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended_y, (unsigned char *)subject1d_y, szSub_y, (unsigned char *)target1d_y, szTar_y, offset, new_sz0, new_sz1, new_sz2);


            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            for(int i = 0; i<targetsize_y;i++)
                target1d_y[i] = data1d_blended_y[i];
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = new_sz2; szTar_y[3] = 1;
            /* Image4DSimple * new4DImage = new Image4DSimple();
               new4DImage->setData((unsigned char *)target1d_y, new_sz0, new_sz1,new_sz2, 1, pixeltype);
               v3dhandle newwin = callback.newImageWindow();
               callback.setImage(newwin, new4DImage);
               callback.setImageName(newwin, "3D adaptive enhancement result");
               callback.updateImageWindow(newwin);*/
        }

        /* Image4DSimple * new4DImage = new Image4DSimple();
               new4DImage->setData((unsigned char *)target1d, new_sz0, new_sz1,new_sz2, 1, pixeltype);
               v3dhandle newwin = callback.newImageWindow();
               callback.setImage(newwin, new4DImage);
               callback.setImageName(newwin, "3D adaptive enhancement result");
               callback.updateImageWindow(newwin);*/



        county++;
    }

    V3DLONG output_size = in_sz[0]*in_sz[1]*in_sz[2];
    unsigned char* datald_output = NULL;
    datald_output = new unsigned char [output_size];
    for(int i = 0; i<output_size;i++)
        datald_output[i] = 255*(target1d_y[i]-1)/254;

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)datald_output,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Local_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
}

bool processImage1(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    cout<<"Welcome to local enhancement filter"<<endl;
    if (output.size() != 1) return false;
    unsigned int Ws = 1000, ch=1;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) Ws = atoi(paras.at(0));
        if(paras.size() >= 2) ch = atoi(paras.at(1));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"Ws = "<<Ws<<endl;
    cout<<"ch = "<<ch<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

   // unsigned char * data1d = 0,  * outimg1d = 0;
  //  V3DLONG * in_sz = 0;

    unsigned int c = ch;//-1;


   /* int datatype;
    if(!loadImage(inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }*/

    V3DPluginArgItem arg;
    V3DPluginArgList input2;
    V3DPluginArgList output2;


    arg.type = "random";std::vector<char*> args1;
    args1.push_back(inimg_file); arg.p = (void *) & args1; input2<< arg;
    arg.type = "random";std::vector<char*> args;
    char channel = '0' + ch;
    args.push_back("3");args.push_back("3");args.push_back("3");args.push_back(&channel); args.push_back("1.0"); arg.p = (void *) & args; input2 << arg;
    arg.type = "random";std::vector<char*> args2;args2.push_back("gfImage.v3draw"); arg.p = (void *) & args2; output2<< arg;

    QString full_plugin_name = "gaussian";
    QString func_name = "gf";

    callback.callPluginFunc(full_plugin_name,func_name, input2,output2);

    unsigned char * data1d2 = 0;
    int datatype;
    V3DLONG * in_sz = 0;

    char * outimg_file2 = ((vector<char*> *)(output2.at(0).p))->at(0);
    loadImage(outimg_file2, data1d2, in_sz, datatype);
    remove("gfImage.v3draw");

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;

    unsigned char* data1d = 0;
    data1d = new unsigned char [pagesz];

    double min,max;
    rescale_to_0_255_and_copy((float *)data1d2,pagesz,min,max,data1d);
    datatype = V3D_UINT8;
    V3DLONG i = 0;
    double th_global = 0;
    V3DLONG offsetc = (c-1)*pagesz;
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
                i++;
            }
        }
        th_global = th_global + PixelSum/(M*N*P);
    }

    int county = 0;
    unsigned char* subject1d_y = NULL;
    unsigned char* target1d_y = NULL;
    V3DLONG szSub_y[4];
    V3DLONG szTar_y[4];
    if(Ws>M || Ws>N)
    {
        if(M>N)
            Ws = N;
        else
            Ws = M;

    }


    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
    {
        unsigned char* subject1d = NULL;
        unsigned char* target1d = NULL;

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




            double  th = 0;
            void* blockarea = 0;
            switch (datatype)
            {
            case V3D_UINT8: block_detection(data1d, in_sz, c, th,(unsigned char* &)blockarea,xb,xe,yb,ye); break;
            case V3D_UINT16: block_detection((unsigned short int *)data1d, in_sz, c, th,(unsigned short int* &)blockarea,xb,xe,yb,ye); break;
            case V3D_FLOAT32: block_detection((float *)data1d, in_sz, c,th,(float* &)blockarea,xb,xe,yb,ye);break;
            default: v3d_msg("Invalid data type. Do nothing."); return false;
            }

            if(th < th_global) th = th_global;
            V3DLONG block_sz[4];
            block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = 1;
            saveImage("temp.v3draw", (unsigned char *)blockarea, block_sz, 1);
            V3DPluginArgItem arg;
            V3DPluginArgList input;
            V3DPluginArgList output;


            arg.type = "random";std::vector<char*> args1;
            args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
            arg.type = "random";std::vector<char*> args;
            string threshold = boost::lexical_cast<string>(th); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
            args.push_back(threshold2);args.push_back("1");args.push_back("0");args.push_back("1"); arg.p = (void *) & args; input << arg;
            arg.type = "random";std::vector<char*> args2;args2.push_back("gsdtImage.v3draw"); arg.p = (void *) & args2; output<< arg;

            QString full_plugin_name = "gsdt";
            QString func_name = "gsdt";

            callback.callPluginFunc(full_plugin_name,func_name, input,output);

            unsigned char * gsdtblock = 0;
            int datatype;
            V3DLONG * in_zz = 0;
            char * outimg_file2= ((vector<char*> *)(output.at(0).p))->at(0);
            loadImage(outimg_file2, gsdtblock, in_zz, datatype,0);
            remove("temp.v3draw");
            remove("gsdtImage.v3draw");
            unsigned char* localEnahancedArea = NULL;

            switch (datatype)
            {
            case V3D_UINT8: AdpThresholding_adpwindow((unsigned char *)blockarea, block_sz, 1,(unsigned char* &)localEnahancedArea, gsdtblock); break;
            case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)blockarea, block_sz, 1, (unsigned short int* &)localEnahancedArea,(unsigned short int *)gsdtblock); break;
            case V3D_FLOAT32: AdpThresholding_adpwindow((float *)blockarea, block_sz, 1, (float* &)localEnahancedArea,(float *)gsdtblock);break;
            default: v3d_msg("Invalid data type. Do nothing."); return false;
            }


            if (count==0)
            {

                V3DLONG targetsize = block_sz[0]*block_sz[1]*block_sz[2];
                target1d = new unsigned char [targetsize];
                for(int i = 0; i<targetsize;i++)
                    target1d[i] = localEnahancedArea[i];
                szTar[0] = xe-xb+1; szTar[1] = ye-yb+1; szTar[2] = P; szTar[3] = 1;

            }else
            {

                V3DLONG subjectsize = block_sz[0]*block_sz[1]*block_sz[2];
                subject1d = new unsigned char [subjectsize];
                for(int i = 0; i<subjectsize;i++)
                    subject1d[i] = localEnahancedArea[i];
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
                for(int i = 0; i<targetsize;i++)
                    target1d[i] = data1d_blended[i];
                szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = new_sz2; szTar[3] = 1;


            }
            count ++;

        }


        if (county==0)
        {

            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            for(int i = 0; i<targetsize_y;i++)
                target1d_y[i] = target1d[i];
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = P; szTar_y[3] = 1;

        }else
        {
            V3DLONG subjectsize_y = new_sz0*new_sz1*new_sz2;
            subject1d_y = new unsigned char [subjectsize_y];
            for(int i = 0; i<subjectsize_y;i++)
                subject1d_y[i] = target1d[i];
            szSub_y[0] = new_sz0; szSub_y[1] = new_sz1; szSub_y[2] = P; szSub_y[3] = 1;

            V3DLONG *offset = new V3DLONG [3];
            offset[0] = 0;
            offset[1] = yb;
            offset[2] = 0;
            new_sz0 = szSub_y[0];
            new_sz1 = ye+1;
            new_sz2 = szSub_y[2];

            V3DLONG totalplxs = new_sz0*new_sz1*new_sz2;
            unsigned char* data1d_blended_y = NULL;
            data1d_blended_y = new unsigned char [totalplxs];
            memset(data1d_blended_y, 0, sizeof(unsigned char)*totalplxs);
            int success;
            success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended_y, (unsigned char *)subject1d_y, szSub_y, (unsigned char *)target1d_y, szTar_y, offset, new_sz0, new_sz1, new_sz2);


            V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
            target1d_y = new unsigned char [targetsize_y];
            for(int i = 0; i<targetsize_y;i++)
                target1d_y[i] = data1d_blended_y[i];
            szTar_y[0] = new_sz0; szTar_y[1] = new_sz1; szTar_y[2] = new_sz2; szTar_y[3] = 1;
        }


        county++;
    }

    // save image
    V3DLONG output_size = in_sz[0]*in_sz[1]*in_sz[2];
    unsigned char* datald_output = NULL;
    datald_output = new unsigned char [output_size];
    for(int i = 0; i<output_size;i++)
        datald_output[i] = 255*(target1d_y[i]-1)/254;

    in_sz[3]=1;
    saveImage(outimg_file, (unsigned char *)datald_output, in_sz, 1);

    if(target1d_y) {delete []target1d_y; target1d_y =0;}
    if(subject1d_y) {delete []subject1d_y; subject1d_y =0;}
    if (data1d) {delete []data1d; data1d=0;}
    if (data1d2) {delete []data1d2; data1d2=0;}
    if (in_sz) {delete []in_sz; in_sz=0;}

    return true;
}


template <class T> void AdpThresholding_adpwindow(const T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  T* &outimg, const T* gsdtdatald)
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
                Wx = (int)round((0.1*log(GsdtValue)/log(2)));

                if(Wx == 0) Wx = 1;

                if (Wx > 0 && PixelValue > 0)
                {
                    Wy = Wx;
                    Wz = Wx;

                    V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
                    V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                    V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
                    V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                    V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
                    V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

                    // printf("window size is %d\n",Wx);
                    //Seletive approach
                    double fxx = data1d[offsetk+offsetj+xe]+ data1d[offsetk+offsetj+xb]- 2*data1d[offsetk+offsetj+ix];
                    double fyy = data1d[offsetk+(ye)*N+ix]+data1d[offsetk+(yb)*N+ix]-2*data1d[offsetk+offsetj+ix];
                    double fzz = data1d[(ze)*M*N+offsetj+ix]+data1d[(zb)*M*N+offsetj+ix]- 2*data1d[offsetk+offsetj+ix];

                    double fxy = 0.25*(data1d[offsetk+(ye)*N+xe]+data1d[offsetk+(yb)*N+xb]-data1d[offsetk+(ye)*N+xb]-data1d[offsetk+(yb)*N+xe]);
                    double fxz = 0.25*(data1d[(ze)*M*N+offsetj+xe]+data1d[(zb)*M*N+offsetj+xb]-data1d[(ze)*M*N+offsetj+xb]-data1d[(zb)*M*N+offsetj+xe]);
                    double fyz = 0.25*(data1d[(ze)*M*N+(ye)*N+ix]+data1d[(zb)*M*N+(yb)*N+ix]-data1d[(ze)*M*N+(yb)*N+ix]-data1d[(zb)*M*N+(ye)*N+ix]);

                    SymmetricMatrix Cov_Matrix(3);
                    Cov_Matrix.Row(1) << fxx;
                    Cov_Matrix.Row(2) << fxy << fyy;
                    Cov_Matrix.Row(3) << fxz << fyz <<fzz;

                    DiagonalMatrix DD;
                    //cout << "Matrix" << endl;
                    //cout << Cov_Matrix << endl <<endl;

                    EigenValues(Cov_Matrix,DD);
                    //  cout << "EigenValues" << endl;
                    //   cout <<  DD << endl <<endl;
                    double a1 = DD(1), a2 = DD(2), a3 = DD(3);

                    //  printf("\ncomparison %.4f,%.4f,%.4f\n\n\n",a1,a2,a3);
                      swapthree(a1, a2, a3);
                    if(a1<0 && a2 < 0)
                    {
                        T dataval = zhi_abs(a2)*(zhi_abs(a2)-zhi_abs(a3))/zhi_abs(a1);
                        pImage[offsetk+offsetj+ix] = dataval;
                        if(maxfl<dataval) maxfl = dataval;
                        if(ix ==161 && iy == 44 && iz ==78)
                        printf("zhi zhou is %d %d %d %d\n",PixelValue,GsdtValue,Wx,dataval);
                    }
                }
                else
                    pImage[offsetk+offsetj+ix] = 0;
            }
        }
    }

    printf("\n");

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
                    T dataval2 = 1+254*pImage[offsetk+offsetj+ix]/maxfl;
                    pImage2[offsetk+offsetj+ix] = dataval2;

                }
            }
        }
    }
    outimg = pImage2;
    return;

}


template <class T> void block_detection(T* data1d,
                                        V3DLONG *in_sz,
                                        unsigned int c,double &th,
                                        T* &outimg,V3DLONG xb,V3DLONG xe,V3DLONG yb,V3DLONG ye)

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
    T *pImage = new T [(xe-xb+1)*(ye-yb+1)*P];
    double th1 = 0;
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
            double PixelSum = 0;

            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = yb; iy < ye+1; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = xb; ix < xe+1; ix++)
                {

                    T PixelValue = data1d[offsetc+offsetk + offsetj + ix];
                    pImage[i] = PixelValue;
                    PixelSum = PixelSum + PixelValue;
                    i++;
                }
            }
            th1 = th1 + PixelSum/((xe-xb+1)*(ye-yb+1)*P);


        }
    }
    th = th1;
    outimg = pImage;

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

template <class T> void soma_detection(T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,
                                       int x,
                                       int y,
                                       V3DLONG somasize,
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
}

template <class T> void AdpThresholding_adpwindow_v2(T* data1d,
                                                     V3DLONG *in_sz,
                                                     unsigned int c,
                                                     T* &outimg,
                                                     T* gsdtdatald,T* gsdtsoma,
                                                     int x,
                                                     int y)
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

