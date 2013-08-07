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
#include <stdlib.h>
#include <vector>
#include "stackutil.h"
#include <Eigen/Dense>
#include <boost/lexical_cast.hpp>
using namespace Eigen;
using namespace std;
Q_EXPORT_PLUGIN2(local_enhancement, local_enhancement);
 
void processImage(V3DPluginCallback2 &callback, QWidget *parent);
template <class T> void AdpThresholding_adpwindow(T* data1d,
                    V3DLONG *in_sz,
                    unsigned int c,
                    T* &outimg,T* gsdtdata1d);

template <class T> void local_global(T* localEnahancedArea,T* data1d,
                    V3DLONG *in_sz,
                    unsigned int c,
                    V3DLONG xb, V3DLONG xe, V3DLONG yb,V3DLONG ye);


int swapthree(float& dummya, float& dummyb, float& dummyc);

template <class T> void block_detection(T* data1d,
                    V3DLONG *in_sz,
                    unsigned int c,double &th,
                    T* &outimg,V3DLONG xb, V3DLONG xe, V3DLONG yb,V3DLONG ye);
template <class SDATATYPE> int pwi_fusing(SDATATYPE *data1d, SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, V3DLONG *offset, V3DLONG new_sz0, V3DLONG new_sz1, V3DLONG new_sz2);

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
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("localEnhancement"))
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
    ImagePixelType pixeltype = p4DImage->getDatatype();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    V3DLONG i = 0;
    double th_global = 0;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy <  M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

             double PixelVaule = data1d[offsetk + offsetj + ix];
             PixelSum = PixelSum + PixelVaule;
             i++;
            }
        }
        th_global = th_global + PixelSum/(M*N*P);
    }


    int c = 1;
    V3DLONG Ws = 60;
    int county = 0;
    unsigned char* subject1d_y = NULL;
    unsigned char* target1d_y = NULL;

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
        for(V3DLONG ix = 0; ix <N; ix = ix+Ws-Ws/10)
        {
                V3DLONG xb = ix;
                V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;




                 double  th = 0;
                 void* blockarea = 0;
                 switch (pixeltype)
                 {
                 case V3D_UINT8: block_detection(data1d, in_sz, c, th,(unsigned char* &)blockarea,xb,xe,yb,ye); break;
                 case V3D_UINT16: block_detection((unsigned short int *)data1d, in_sz, c, th,(unsigned short int* &)blockarea,xb,xe,yb,ye); break;
                 case V3D_FLOAT32: block_detection((float *)data1d, in_sz, c,th,(float* &)blockarea,xb,xe,yb,ye);break;
                 default: v3d_msg("Invalid data type. Do nothing."); return;
                 }

                 if(th < th_global) th = th_global;
                 V3DLONG block_sz[4];
                 block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = P; block_sz[3] = sc;
                 saveImage("temp.v3draw", (unsigned char *)blockarea, block_sz, pixeltype);
                 V3DPluginArgItem arg;
                 V3DPluginArgList input;
                 V3DPluginArgList output;


                 arg.type = "random";std::vector<char*> args1;
                 args1.push_back("temp.v3draw"); arg.p = (void *) & args1; input<< arg;
                 arg.type = "random";std::vector<char*> args;
                 char channel = '0' + (c-1);
                 string threshold = boost::lexical_cast<string>(th); char* threshold2 =  new char[threshold.length() + 1]; strcpy(threshold2, threshold.c_str());
                 args.push_back(threshold2);args.push_back("1");args.push_back(&channel);args.push_back("1"); arg.p = (void *) & args; input << arg;
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
                 case V3D_UINT8: AdpThresholding_adpwindow((unsigned char *)blockarea, block_sz, c,(unsigned char* &)localEnahancedArea, gsdtblock); break;
                 case V3D_UINT16: AdpThresholding_adpwindow((unsigned short int *)blockarea, block_sz, c, (unsigned short int* &)localEnahancedArea,(unsigned short int *)gsdtblock); break;
                 case V3D_FLOAT32: AdpThresholding_adpwindow((float *)blockarea, block_sz, c, (float* &)localEnahancedArea,(float *)gsdtblock);break;
                 default: v3d_msg("Invalid data type. Do nothing."); return;
                 }


                 if (count==0)
                 {

                     V3DLONG targetsize = block_sz[0]*block_sz[1]*block_sz[2];
                     target1d = new unsigned char [targetsize];
                     for(int i = 0; i<targetsize;i++)
                         target1d[i] = localEnahancedArea[i];
                     szTar[0] = xe-xb+1; szTar[1] = ye-yb+1; szTar[2] = P; szTar[3] = sc;

                 }else
                {

                    V3DLONG subjectsize = block_sz[0]*block_sz[1]*block_sz[2];
                    subject1d = new unsigned char [subjectsize];
                    for(int i = 0; i<subjectsize;i++)
                        subject1d[i] = localEnahancedArea[i];
                    szSub[0] = xe-xb+1; szSub[1] = ye-yb+1; szSub[2] = P; szSub[3] = sc;

                    V3DLONG *offset = new V3DLONG [3];
                    offset[0] = xb;
                    offset[1] = 0;
                    offset[2] = 0;
                    new_sz0 = xe+1;
                    new_sz1 = szSub[1];
                    new_sz2 = szSub[2];

                    V3DLONG totalplxs = sc*new_sz0*new_sz1*new_sz2;
                    unsigned char* data1d_blended = NULL;
                    data1d_blended = new unsigned char [totalplxs];
                    memset(data1d_blended, 0, sizeof(unsigned char)*totalplxs);
                    int success;

                    success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended, (unsigned char *)subject1d, szSub, (unsigned char *)target1d, szTar, offset, new_sz0, new_sz1, new_sz2);

                    V3DLONG targetsize = new_sz0*new_sz1*new_sz2;
                    target1d = new unsigned char [targetsize];
                    for(int i = 0; i<targetsize;i++)
                        target1d[i] = data1d_blended[i];
                    szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = new_sz2; szTar[3] = sc;


                }
                count ++;

        }


            if (county==0)
            {

                V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
                target1d_y = new unsigned char [targetsize_y];
                for(int i = 0; i<targetsize_y;i++)
                    target1d_y[i] = target1d[i];
                szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = P; szTar[3] = sc;

            }else
            {
               V3DLONG subjectsize_y = new_sz0*new_sz1*new_sz2;
               subject1d_y = new unsigned char [subjectsize_y];
               for(int i = 0; i<subjectsize_y;i++)
                   subject1d_y[i] = target1d[i];
               szSub[0] = new_sz0; szSub[1] = new_sz1; szSub[2] = P; szSub[3] = sc;

               Image4DSimple * new4DImage = new Image4DSimple();
               new4DImage->setData((unsigned char *)subject1d_y, new_sz0, new_sz1,new_sz2, 1, pixeltype);
               v3dhandle newwin = callback.newImageWindow();
               callback.setImage(newwin, new4DImage);
               callback.setImageName(newwin, "3D adaptive enhancement result");
               callback.updateImageWindow(newwin);



               V3DLONG *offset = new V3DLONG [3];
               offset[0] = 0;
               offset[1] = yb;
               offset[2] = 0;
               new_sz0 = szSub[0];
               new_sz1 = ye+1;
               new_sz2 = szSub[2];

               V3DLONG totalplxs = sc*new_sz0*new_sz1*new_sz2;
               unsigned char* data1d_blended_y = NULL;
               data1d_blended_y = new unsigned char [totalplxs];
               memset(data1d_blended_y, 0, sizeof(unsigned char)*totalplxs);
               int success;
               success = pwi_fusing<unsigned char>((unsigned char *)data1d_blended_y, (unsigned char *)subject1d_y, szSub, (unsigned char *)target1d_y, szTar, offset, new_sz0, new_sz1, new_sz2);


               V3DLONG targetsize_y = new_sz0*new_sz1*new_sz2;
               target1d_y = new unsigned char [targetsize_y];
               for(int i = 0; i<targetsize_y;i++)
                   target1d_y[i] = data1d_blended_y[i];
               szTar[0] = new_sz0; szTar[1] = new_sz1; szTar[2] = new_sz2; szTar[3] = sc;
            }





        county++;
    }




    //display

    // display
   /* Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)localEnahancedArea, block_sz[0], block_sz[1], block_sz[2], 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "3D adaptive enhancement result");
    callback.updateImageWindow(newwin);
    return;*/

        return;
}

template <class T> void AdpThresholding_adpwindow(T* data1d,
                                                  V3DLONG *in_sz,
                                                  unsigned int c,
                                                  T* &outimg,T* gsdtdatald)
{


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
                    Wx = (int)round((log(GsdtValue)/log(2)));
                    //printf("%d %d\n",PixelValue,Wx);

                    if (Wx > 0 && PixelValue > 0)
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
                             T dataval2 = 255*pImage[offsetk+offsetj+ix]/maxfl;
                            pImage2[offsetk+offsetj+ix] = dataval2;

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

template <class T> void block_detection(T* data1d,
                                       V3DLONG *in_sz,
                                       unsigned int c,double &th,
                                       T* &outimg,V3DLONG xb,V3DLONG xe,V3DLONG yb,V3DLONG ye)

{


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

template <class T> void local_global(T* localdata1d,T *data1d,
                    V3DLONG *in_sz,
                    unsigned int c,
                    V3DLONG xb, V3DLONG xe, V3DLONG yb,V3DLONG ye)

{


         V3DLONG N = in_sz[0];
         V3DLONG M = in_sz[1];
         V3DLONG P = in_sz[2];
         V3DLONG pagesz = N*M*P;
         V3DLONG offsetc = (c-1)*pagesz;

         int i = 0;
          for(V3DLONG iz = 0; iz < P; iz++)
          {
              V3DLONG offsetk = iz*M*N;
              for(V3DLONG iy = yb; iy <  ye+1; iy++)
              {
                  V3DLONG offsetj = iy*N;
                  for(V3DLONG ix = xb; ix < xe+1; ix++)
                  {
                      data1d[offsetc+offsetk + offsetj + ix] = localdata1d[i];
                      i++;
                  }
              }
          }

        //outimg = pImage;

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
                    if(data1d[idx]) //p_mask[idx-offset_c]
                    {
                        data1d[idx] = (SDATATYPE) ( (data1d[idx] + target1d[offsets_j + i - tx_start])/2.0 ); // Avg. Intensity
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

