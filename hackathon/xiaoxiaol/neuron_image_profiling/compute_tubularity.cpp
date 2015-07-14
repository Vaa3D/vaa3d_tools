#include "compute_tubularity.h"
#include "newmatap.h"
#include "newmatio.h"
#include <iostream>

#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif

using namespace std;

static V3DLONG boundValue(V3DLONG x, V3DLONG m_min, V3DLONG m_max)
{
    x = MAX(x, m_min);
    x = MIN(x, m_max);
    return x;

}



//referred from Image_enhancement_Anisotropy_plugin.cpp
double compute_anisotropy_sphere(const unsigned char * data1d, V3DLONG N, V3DLONG M, V3DLONG P, V3DLONG c,
                                                  V3DLONG x0, V3DLONG y0, V3DLONG z0,
                                                  V3DLONG radius)
{
    if (!data1d || N<=0 || M<=0 || P<=0 || c<0 || x0<0 || x0>N-1 || y0<0 || y0>M-1 || z0<0 || z0>P-1 || radius<1)
        return false;

        //get the boundary
        V3DLONG offsetc = (c)*(N*M*P);

    V3DLONG xb = boundValue(x0-radius,0, N-1);
    V3DLONG xe = boundValue(x0+radius,0, N-1);
    V3DLONG yb = boundValue(y0-radius,0, M-1);
    V3DLONG ye = boundValue(y0+radius,0, M-1);
    V3DLONG zb = boundValue(z0-radius,0, P-1);
    V3DLONG ze = boundValue(z0+radius,0, P-1);

    V3DLONG i,j,k;
    double w;

    //first get the average value
    double x2, y2, z2;
    double rx2 = double(radius+1)*(radius+1);
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
    //std::cout <<"cc = "<<cc11<<" "<<cc12<<" "<<cc13<<" "<<cc22<<" "<<cc23<<" "<<cc33<<std::endl;
    double tubularity = 1; //the tubularity should at least be 1.
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
        // double pc3 = DD(1);
        tubularity = pc1/pc2;

    }
    catch (...)
    {
        tubularity = 0;
    }

    return tubularity;

}
