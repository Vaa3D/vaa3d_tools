/* branch_pt_detection_func.h
 * This is a test plugin, you can use it as a demo.
 * 2017-12-1 : by YourName
 */
 
#ifndef __BRANCH_PT_DETECTION_FUNC_H__
#define __BRANCH_PT_DETECTION_FUNC_H__


#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

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

    //first get the average value
    double x2, y2, z2;
    double rx2 = double(rs+1)*(rs+1);
    double ry2 = rx2, rz2 = rx2;
    double tmpd;
    double xm=0,ym=0,zm=0, s=0, mv=0, n=0;

    s = 0; n = 0;
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

                s += double(data1d[offsetc+offsetkl + offsetjl + i]);
                n = n+1;
            }
        }
    }
    if (n!=0)
        mv = s/n;
    else
        mv = 0;

    //now get the center of mass
    s = 0; n=0;
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

                w = double(data1d[offsetc+offsetkl + offsetjl + i]) - mv;
                if (w>0)
                {
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;
                n = n+1;
                }
            }
        }
    }

    if(n>0)
    {
        xm /= s; ym /=s; zm /=s;
    }
    else
    {
        xm = x0; ym=y0; zm=z0;
    }

    double spatial_deviation = sqrt(double(xm-x0)*(xm-x0) + double(ym-y0)*(ym-y0) + double(zm-z0)*(zm-z0)) + 1;
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
                w = double(data1d[offsetc+offsetkl + offsetjl + i]) - mv;
                if (w>0)
                {
                cc11 += w*dfx*dfx;
                cc12 += w*dfx*dfy;
                cc13 += w*dfx*dfz;
                cc22 += w*dfy*dfy;
                cc23 += w*dfy*dfz;
                cc33 += w*dfz*dfz;
                }

            }
        }
    }
    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;

    Score = 1; //the Score should at least be 1.
    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix,DD,VV);

        //output the result
        double pc1 = DD(3);
        double pc2 = DD(2);
//        double pc3 = DD(1);
        Score = 1/(pc1/pc2/spatial_deviation);
        //Score = sqrt(pc1)/sqrt(pc2)/spatial_deviation;
    }
    catch (...)
    {
        Score = 0;
    }

    return true;

}

#endif
