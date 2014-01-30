/* Image_enhancement_Anisotropy_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-01-28 : by Zhi
 */

#include "v3d_message.h"
#include <vector>
#include "Image_enhancement_Anisotropy_plugin.h"
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"


using namespace std;
Q_EXPORT_PLUGIN2(image_anisotropy_map, anisotropy_enhancement);

void processImage(V3DPluginCallback2 &callback, QWidget *parent, int flag);


template <class T> bool compute_Anisotropy_sphere(const T* data1d, V3DLONG N, V3DLONG M, V3DLONG P, V3DLONG c,
                                                  V3DLONG x0, V3DLONG y0, V3DLONG z0,
                                                  V3DLONG rs, double &Score, double & avevalue)
{
    if (!data1d || N<=0 || M<=0 || P<=0 || c<0 || x0<0 || x0>N-1 || y0<0 || y0>M-1 || z0<0 || z0>P-1 || rs<1)
        return false;

        //get the boundary
        V3DLONG offsetc = (c)*(N*M*P);

    V3DLONG xb = x0-rs;if(xb<0) xb = 0;
    V3DLONG xe = x0+rs; if(xe>=N-1) xe = N-1;
    V3DLONG yb = y0-rs;if(yb<0) yb = 0;
    V3DLONG ye = y0+rs; if(ye>=M-1) ye = M-1;
    V3DLONG zb = z0-rs;if(zb<0) zb = 0;
    V3DLONG ze = z0+rs; if(ze>=P-1) ze = P-1;

    V3DLONG i,j,k;
    double w;

    //first get the center of mass
    double x2, y2, z2;
    double rx2 = double(rs+1)*(rs+1);
    double ry2 = rx2, rz2 = rx2;
    double tmpd;
    double xm=0,ym=0,zm=0, s=0, mv=0, n=0;

    for(k=zb; k<=ze; k++)
    {
        V3DLONG offsetkl = k*M*N;
        z2 = k-z0; z2*=z2;
        for(j=yb; j<=ye; j++)
        {
            V3DLONG offsetjl = j*N;
            y2 = j-y0; y2*=y2;
            tmpd = y2/ry2 + z2/rz2;
            if (tmpd>1.0)
                continue;

            for(i=xb; i<=xe; i++)
            {
                x2 = i-x0; x2*=x2;
                if (x2/rx2 + tmpd > 1.0)
                    continue;

                w = double(data1d[offsetc+offsetkl + offsetjl + i]);
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;
                n = n+1;
            }
        }
    }

    if(s>0)
    {
        xm /= s; ym /=s; zm /=s;
        mv = s/n;
    }

    double spatial_deviation = sqrt(double(xm-x0)*(xm-x0) + double(ym-y0)*(ym-y0) + double(zm-z0)*(zm-z0));

    avevalue = mv;

    double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
    double dfx, dfy, dfz;
    for(k=zb; k<=ze; k++)
    {
        z2 = k-z0; z2*=z2;
        dfz = double(k)-zm;
        V3DLONG offsetkl = k*M*N;
        for(j=yb; j<=ye; j++)
        {
            y2 = j-y0; y2*=y2;
            tmpd = y2/ry2 + z2/rz2;
            if (tmpd>1.0)
                continue;

            dfy = double(j)-ym;
            V3DLONG offsetjl = j*N;
            for(i=xb; i<=xe; i++)
            {
                x2 = i-x0; x2*=x2;
                if (x2/rx2 + tmpd > 1.0)
                    continue;

                dfx = double(i)-xm;
                w = double(data1d[offsetc+offsetkl + offsetjl + i]) - mv;  if (w<0) w=0;
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
        // double pc3 = DD(1);
        Score = sqrt(pc1)/sqrt(pc2)/spatial_deviation;
    }
    catch (...)
    {
        Score = 0;
    }

    return true;

}


QStringList anisotropy_enhancement::menulist() const
{
    return QStringList()
            <<tr("fixed window")
           <<tr("adaptive window")
          <<tr("about");
}

QStringList anisotropy_enhancement::funclist() const
{
    return QStringList()
            <<tr("func1")
           <<tr("func2")
          <<tr("help");
}

void anisotropy_enhancement::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("fixed window"))
    {
        processImage(callback,parent,0);
    }
    else if (menu_name == tr("adaptive window"))
    {
        processImage(callback,parent,1);
    }
    else
    {
        v3d_msg(tr("This plugin uses anisotropy property to enhance image "
                   "Developed by Zhi, 2013-01-28"));
    }
}

bool anisotropy_enhancement::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("func1"))
    {
        v3d_msg("To be implemented.");
    }
    else if (func_name == tr("func2"))
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

void processImage(V3DPluginCallback2 &callback, QWidget *parent, int flag)
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

    //input
    bool ok1,ok2;
    int c = 1;
    double rs = 0;

    if(flag ==0)
        rs = QInputDialog::getDouble(parent, "Radius of the window for computing anisotropy map",
                                     "Window radius:",
                                     1, 1, 30, 1, &ok1);
    else
        ok1=true;
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
                                         1, 1, sc, 1, &ok1);
        }
    }

    c = c-1;

    V3DLONG offsetc = (c)*pagesz;
    float* datald_output = 0;
    datald_output = new float[pagesz];
    for(V3DLONG i = 0; i<pagesz;i++)
        datald_output[i] = 0;

    double score_max = 0, ave_last=0;
    double score_each = 0, ave_v=0;

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
                if(flag == 0) //for fixed window
                {
                    if (rs > 0 && PixelValue > 0)
                    {
                        compute_Anisotropy_sphere(data1d, N, M, P, c, ix, iy, iz, rs, score_each, ave_v);
                        if(score_each>0)
                        {
                            datald_output[offsetk + offsetj + ix] = score_each;
                        }
                    }
                }
                else
                {
                    if(PixelValue > 0)
                    {
                        score_max = 0;
                        ave_last = 0;
                        for(rs = 2; rs < 31; rs++)
                        {
                            compute_Anisotropy_sphere(data1d, N, M, P, c, ix, iy, iz, rs, score_each, ave_v);
                            if(rs==2) ave_last = ave_v;

                            if(score_each > score_max && ave_v < ave_last*1.1 && ave_v>ave_last*0.9)
                            {
                                score_max = score_each;
                                ave_last = ave_v;
                            }
                            else
                            {
                                break;
                            }
                        }
                        datald_output[offsetk + offsetj + ix] = score_max;
                    }

                }
            }
        }
    }


    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)datald_output,N, M, P, 1, V3D_FLOAT32);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Local_adaptive_enhancement_result");
    callback.updateImageWindow(newwin);
    return;
}
