/* EMEE_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-05 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "EMEE_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <math.h>
#include <vector>
#include "stackutil.h"
#define INF 1E9

#define WANT_STREAM       // include iostream and iomanipulators
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"


using namespace std;
Q_EXPORT_PLUGIN2(EMEE, EMEE);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);
void processImage2(V3DPluginCallback2 &callback, QWidget *parent);

bool processImage2(const V3DPluginArgList & input, V3DPluginArgList & output);
bool processImage3(const V3DPluginArgList & input, V3DPluginArgList & output);

template <class T> bool compute_Anisotropy(const T* data1d, V3DLONG N, V3DLONG M, V3DLONG P, int c,
                       V3DLONG ix, V3DLONG iy, V3DLONG iz,
                       V3DLONG Ws,double &Score)
{

    //get the boundary
    V3DLONG offsetc = (c-1)*(N*M*P);

    V3DLONG xb = ix-Ws;if(xb<0) xb = 0;
    V3DLONG xe = ix+Ws; if(xe>=N-1) xe = N-1;
    V3DLONG yb = iy-Ws;if(yb<0) yb = 0;
    V3DLONG ye = iy+Ws; if(ye>=M-1) ye = M-1;
    V3DLONG zb = iz-Ws;if(zb<0) zb = 0;
    V3DLONG ze = iz+Ws; if(ze>=P-1) ze = P-1;
    double w;
    double xm=0,ym=0,zm=0, s=0;
    double pc1,pc2;

    for(V3DLONG k=zb; k<=ze; k++)
    {
        V3DLONG offsetkl = k*M*N;
        for(V3DLONG j=yb; j<=ye; j++)
        {
            V3DLONG offsetjl = j*N;
            for(V3DLONG i=xb; i<=xe; i++)
            {
                w = double(data1d[offsetc+offsetkl + offsetjl + i]);
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;

            }
        }
    }

    if(s>0)
    {
        xm /= s; ym /=s; zm /=s;
    }
        double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
        double dfx, dfy, dfz;
        for(V3DLONG k=zb; k<=ze; k++)
        {
            dfz = double(k)-zm;
            V3DLONG offsetkl = k*M*N;
            for(V3DLONG j=yb; j<=ye; j++)
            {
                dfy = double(j)-ym;
                V3DLONG offsetjl = j*N;
                for(V3DLONG i=xb; i<=xe; i++)
                {
                    dfx = double(i)-xm;
                    w = double(data1d[offsetc+offsetkl + offsetjl + i]);
                    cc11 += w*dfx*dfx;
                    cc12 += w*dfx*dfy;
                    cc13 += w*dfx*dfz;
                    cc22 += w*dfy*dfy;
                    cc23 += w*dfy*dfz;
                    cc33 += w*dfz*dfz;

                }
            }
        }
        cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;

        try
        {
            //then find the eigen vector
            SymmetricMatrix Cov_Matrix(3);
            Cov_Matrix.Row(1) << cc11;
            Cov_Matrix.Row(2) << cc12 << cc22;
            Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

            DiagonalMatrix DD;
            Matrix VV;
            EigenValues(Cov_Matrix,DD,VV);;

            //output the result
            double pc1 = DD(3);
            double pc2 = DD(2);
           // Score = Score + (pc1-pc2);
            Score = pc1-pc2;
         }
        catch (...)
        {
            Score = 0;
        }

        return true;

}


QStringList EMEE::menulist() const
{
	return QStringList() 
		<<tr("EMEE")
        <<tr("Anisotropy_Evaluation")
		<<tr("about");
}

QStringList EMEE::funclist() const
{
	return QStringList()
		<<tr("EMEE")
		<<tr("help");
}

void EMEE::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("EMEE"))
	{
        processImage(callback,parent);
	}
    else if(menu_name == tr("Anisotropy_Evaluation"))
    {
        processImage2(callback,parent);
    }
    else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-08-05"));
	}
}

bool EMEE::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("Anisotropy_Evaluation"))
    {
        return processImage2(input, output);
    }
    else if (func_name == tr("Anisotropy_Evaluation_sameArea"))
    {
        return processImage3(input, output);
    }
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f Anisotropy_Evaluation -i <inimg_file> -o <outimg_file> -p <ch>"<<endl;
        cout<<endl;
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
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    //define datatype here

    //input
    bool ok1, ok2, ok3, ok4;
    unsigned int Wx=1, Wy=1, Wz=1, c=1;

    Wx = QInputDialog::getInteger(parent, "Window X ",
                                  "Enter radius (window size is 2*radius+1):",
                                  1, 1, N, 1, &ok1);

    if(ok1)
    {
        Wy = QInputDialog::getInteger(parent, "Window Y",
                                      "Enter radius (window size is 2*radius+1):",
                                      1, 1, M, 1, &ok2);
    }
    else
        return;

    if(ok2)
    {
        Wz = QInputDialog::getInteger(parent, "Window Z",
                                      "Enter radius (window size is 2*radius+1):",
                                      1, 1, P, 1, &ok3);
    }
    else
        return;

    if(sc==1)
    {
        c=1;
        ok4=true;
    }
    else
    {
        if(ok3)
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok4);
        }
        else
            return;
    }


    V3DLONG offsetc = (c-1)*pagesz;
    double blockvalue = 0;
    V3DLONG blocknum = 0;
    int maxfl , minfl;
    for(V3DLONG iz = 0; iz < P-Wz; iz = iz+Wz)
    {

        for(V3DLONG iy = 0; iy < M-Wy; iy = iy+Wy)
        {

            for(V3DLONG ix = 0; ix < N-Wx; ix = ix+Wx)
            {
                maxfl = 0;
                minfl = INF;
                V3DLONG xb = ix;
                V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                V3DLONG yb = iy;
                V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                V3DLONG zb = iz;
                V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;
                for(V3DLONG k=zb; k<ze; k++)
                {
                    V3DLONG offsetkl = k*M*N;
                    for(V3DLONG j=yb; j<ye; j++)
                    {
                        V3DLONG offsetjl = j*N;
                        for(V3DLONG i=xb; i<xe; i++)
                        {
                            int dataval = data1d[ offsetc + offsetkl + offsetjl + i];
                            if(dataval+1>maxfl) maxfl = dataval+1;
                            if(dataval+1<minfl) minfl = dataval+1;

                        }
                    }
                }
                if(maxfl>1)
                {
                    blockvalue = blockvalue +  (maxfl/minfl)*log((double)maxfl/minfl);
                    blocknum++;
                }
                // printf("\nEME is %.4f %d %d",blockvalue, maxfl,minfl);
            }
        }
    }
    printf("EMEE is %.5f, %d\n\n",blockvalue/(double)blocknum,blocknum);
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
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    ImagePixelType pixeltype = p4DImage->getDatatype();

    //define datatype here

    //input
    bool ok1;
    unsigned int c=1;

    if(sc==1)
    {
        c=1;
        ok1=true;
    }
    else
    {
       c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok1);
    }


    V3DLONG offsetc = (c-1)*pagesz;
    V3DLONG Score = 0;
    V3DLONG num = 0;
    V3DLONG Ws = 0;
    unsigned char* datald_output = NULL;
    datald_output = new unsigned char [pagesz];
    for(V3DLONG i = 0; i<pagesz;i++)
        datald_output[i] = 0;
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        printf("\r Evaluation : %d %% completed ", int(float(iz)/P*100)); fflush(stdout);
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                V3DLONG PixelValue = data1d[offsetc+offsetk + offsetj + ix];
                Ws = (int)round((log(PixelValue)/log(2)));

               if (Ws > 0 && PixelValue > 0)
                {
                   double Score_each = 0;
                   compute_Anisotropy(data1d, N, M, P,c,ix, iy, iz, Ws,Score_each);
                   if(Score_each>0)
                   {
                    datald_output[offsetk + offsetj + ix] =   Score_each;
                    Score += Score_each;
                    num++;
                   }

                }


            }
        }
    }


    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)datald_output,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Local_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
      printf("\n\nScore is %.5f, %d\n\n",Score/(double)num,num);

    return;
}

bool processImage2(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to anisotropy evaluation filter"<<endl;
    if (output.size() != 1) return false;
    unsigned int ch=1;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) ch = atoi(paras.at(0));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"ch = "<<ch<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

    unsigned char * data1d = 0,  * outimg1d = 0;
    V3DLONG * in_sz = 0;

    unsigned int c = ch;//-1;


    int datatype;
    if(!loadImage(inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;

    V3DLONG offsetc = (c-1)*pagesz;
    V3DLONG Score = 0;
    V3DLONG num = 0;
    V3DLONG Ws = 0;
    unsigned char* datald_output = NULL;
    datald_output = new unsigned char [pagesz];
    for(V3DLONG i = 0; i<pagesz;i++)
        datald_output[i] = 0;
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        printf("\r Evaluation : %d %% completed ", int(float(iz)/P*100)); fflush(stdout);
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                V3DLONG PixelValue = data1d[offsetc+offsetk + offsetj + ix];
                Ws = (int)round((log(PixelValue)/log(2)));

               if (Ws > 0 && PixelValue > 0)
                {


                    V3DLONG xb = ix-Ws;if(xb<0) xb = 0;
                    V3DLONG xe = ix+Ws; if(xe>=N-1) xe = N-1;
                    V3DLONG yb = iy-Ws;if(yb<0) yb = 0;
                    V3DLONG ye = iy+Ws; if(ye>=M-1) ye = M-1;
                    V3DLONG zb = iz-Ws;if(zb<0) zb = 0;
                    V3DLONG ze = iz+Ws; if(ze>=P-1) ze = P-1;
                    double w;
                    double xm=0,ym=0,zm=0, s=0;
                    for(V3DLONG k=zb; k<=ze; k++)
                    {
                        V3DLONG offsetkl = k*M*N;
                        for(V3DLONG j=yb; j<=ye; j++)
                        {
                            V3DLONG offsetjl = j*N;
                            for(V3DLONG i=xb; i<=xe; i++)
                            {
                                w = double(data1d[offsetc+offsetkl + offsetjl + i]);
                                xm += w*i;
                                ym += w*j;
                                zm += w*k;
                                s += w;

                            }
                        }
                    }

                    if(s>0)
                    {
                        xm /= s; ym /=s; zm /=s;

                        double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
                        double dfx, dfy, dfz;
                        for(V3DLONG k=zb; k<=ze; k++)
                        {
                            dfz = double(k)-zm;
                            V3DLONG offsetkl = k*M*N;
                            for(V3DLONG j=yb; j<=ye; j++)
                            {
                                dfy = double(j)-ym;
                                V3DLONG offsetjl = j*N;
                                for(V3DLONG i=xb; i<=xe; i++)
                                {
                                    dfx = double(i)-xm;
                                    w = double(data1d[offsetc+offsetkl + offsetjl + i]);
                                    cc11 += w*dfx*dfx;
                                    cc12 += w*dfx*dfy;
                                    cc13 += w*dfx*dfz;
                                    cc22 += w*dfy*dfy;
                                    cc23 += w*dfy*dfz;
                                    cc33 += w*dfz*dfz;

                                }
                            }
                        }
                        cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;

                        try
                        {
                            //then find the eigen vector
                            SymmetricMatrix Cov_Matrix(3);
                            Cov_Matrix.Row(1) << cc11;
                            Cov_Matrix.Row(2) << cc12 << cc22;
                            Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

                            DiagonalMatrix DD;
                            Matrix VV;
                            EigenValues(Cov_Matrix,DD,VV);;

                            //output the result
                            double pc1 = DD(3);
                            double pc2 = DD(2);
                          //  Score = Score + (pc1-pc2);
                            Score = Score+((pc1+1)/(pc2+1));
                            //Score = Score+((pc1+0.001)/(pc2+0.001));
                                //printf("\n\nScore is %.5f, %d, %.5f, %.5f\n\n",Score/(double)num,num,pc1,pc2);
                            datald_output[offsetk + offsetj + ix] =   (pc1+1)/(pc2+1);
                                   num++;



                         }
                        catch (...)
                        {

                        }
                    }
                }


            }
        }
    }
    in_sz[3] = 1;
    saveImage(outimg_file, (unsigned char *)datald_output, in_sz, datatype);

    if (data1d) {delete []data1d; data1d=0;}
    if (datald_output) {delete []datald_output; datald_output=0;}
    ofstream myfile;
    myfile.open ("/local2/EvaluationScore.txt",ios::out | ios::app );
    myfile << inimg_file  ;
    myfile << "   ";
    myfile << (double)Score/num;
    myfile << "\n";
    myfile.close();
    return true;
}

bool processImage3(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to anisotropy evaluation filter"<<endl;
    if (output.size() != 1) return false;
    unsigned int ch=1;
    if (input.size()>=2)
    {

        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) ch = atoi(paras.at(0));
    }

    char * inimg_file1 = ((vector<char*> *)(input.at(0).p))->at(0);
    char * inimg_file2 = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"ch = "<<ch<<endl;
    cout<<"inimg_file1 = "<<inimg_file1<<endl;
    cout<<"inimg_file2 = "<<inimg_file2<<endl;

    unsigned char * data1d1 = 0,  * data1d2 = 0;
    V3DLONG * in_sz = 0;

    unsigned int c = ch;//-1;


    int datatype1;
    if(!loadImage(inimg_file1, data1d1, in_sz, datatype1))
    {
        cerr<<"load image "<<inimg_file1<<" error!"<<endl;
        return false;
    }

    int datatype2;
    if(!loadImage(inimg_file2, data1d2, in_sz, datatype2))
    {
        cerr<<"load image "<<inimg_file2<<" error!"<<endl;
        return false;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG pagesz = N*M*P;

    V3DLONG offsetc = (c-1)*pagesz;
    V3DLONG Score1 = 0;
    V3DLONG Score2 = 0;
    V3DLONG num = 0;
    V3DLONG Ws = 0;
  //  V3DLONG Ws2 = 0;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        printf("\r Evaluation : %d %% completed ", int(float(iz)/P*100)); fflush(stdout);
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                V3DLONG PixelValue1 = data1d1[offsetc+offsetk + offsetj + ix];
                V3DLONG PixelValue2 = data1d2[offsetc+offsetk + offsetj + ix];
              //  Ws1 = (int)round((log(PixelValue1)/log(2)));
                Ws = 2*(int)round((log(PixelValue2)/log(2)));

               if (Ws > 0 && PixelValue1 > 0 && PixelValue2>0 )
                {
                   double Score_each1 = 0;
                   double Score_each2 = 0;
                   compute_Anisotropy(data1d1, N, M, P,c,ix, iy, iz, Ws,Score_each1);
                   compute_Anisotropy(data1d2, N, M, P,c,ix, iy, iz, Ws,Score_each2);
                   if(Score_each1>0 && Score_each2>0)
                   {

                        Score1 += Score_each1;
                        Score2 += Score_each2;
                        num++;
                   }

                }


            }
        }
    }
     printf("\n");
    if (data1d1) {delete []data1d1; data1d1=0;}
    if (data1d2) {delete []data1d2; data1d2=0;}
    ofstream myfile;
    myfile.open ("/local2/EvaluationScore_v2.txt",ios::out | ios::app );
    myfile << inimg_file1;
    myfile << "   ";
    myfile << (double)Score1/num;
    myfile << "   ";
    myfile << (double)Score2/num;
    myfile << "   ";
    myfile << (double)Score2/Score1;
    myfile << "\n";
    myfile.close();
    return true;
}
