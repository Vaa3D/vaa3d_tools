/* [response, orientation, nms, filterBank] = steerableDetector(image, filterOrder, sigma);
 *
 * Copyright (C) 2011-2012 Francois Aguet
 * Adapted from the SteerableJ package, Copyright (C) 2005-2008 Francois Aguet, Biomedical Imaging Group, EPFL.
 *
 * Last modified: Nov 29, 2012
 *
 * Compilation:
 * Mac/Linux: mex -I/usr/local/include -I../../mex/include /usr/local/lib/libgsl.a /usr/local/lib/libgslcblas.a steerableDetector.cpp
 * Windows: mex COMPFLAGS="$COMPFLAGS /TP /MT" -I"..\..\..\extern\mex\include\gsl-1.15" -I"..\..\mex\include" "..\..\..\extern\mex\lib\gsl.lib" "..\..\..\extern\mex\lib\cblas.lib" -output steerableDetector steerableDetector.cpp
 */


// This file is part of SteerableDetector.
// 
// SteerableDetector is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SteerableDetector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with SteerableDetector.  If not, see <http://www.gnu.org/licenses/>.


#include <string>
#include <math.h>
#include <gsl/gsl_poly.h>
#include <algorithm>
#include <stdlib.h>
#include <iostream>

// #include "mex.h"
#include "convolver.h"

using namespace std;

#define PI 3.141592653589793
#define TOL 1e-12


static double approxZero(double n) {
    if (fabs(n) < TOL) {
        return 0.0;
    } else {
        return n;
    }
}

static double opposite(double theta) {
    if (theta > 0.0) { // values in (-PI,PI]
        return theta - PI;
    } else {
        return theta + PI;
    }
}

static int getTemplateN(int M) {
    if (M==1) {
        return M*(M+3)/2;
    } else {
        return M*(M+3)/2 - getTemplateN(M-1);
    }
}


double* getWeights(int M, double sigma) {
    double* alpha;
    double s2 = sigma*sigma;
    double s3 = sigma*s2;
    switch (M) {
        case 1:
            alpha = new double[1];
            alpha[0] = sqrt(2.0/PI);
            break;
        case 2: 
            alpha = new double[2];
            // mu = 0
            alpha[0] = -sqrt(3.0/(4.0*PI)) * sigma;
            alpha[1] = -alpha[0] / 3.0;
            // mu = 2
            //alpha[0] = -sqrt(2.0/(3.0*PI))*sigma;
            //alpha[0] = -sigma; // Hessian
            //alpha[1] = 0.0;
            break;
        case 3: // mu = 0
            alpha = new double[3];
            alpha[0] = 0.966;
            alpha[1] = 0.0;
            alpha[2] = 0.256*s2; 
            break;
        case 4: // mu = 0.25
            alpha = new double[5];
            alpha[0] = -0.392*sigma;
            alpha[1] = 0.113*sigma;
            alpha[2] = 0.034*s3;
            alpha[3] = -0.184*s3;
            alpha[4] = 0.025*s3; 
            break;
        case 5:
            alpha = new double[5];
            alpha[0] = 1.1215;
            alpha[1] = 0.018*s2;
            alpha[2] = 0.5576*s2;
            alpha[3] = 0.0038*s2*s2;
            alpha[4] = 0.0415*s2*s2;
            break;
        default:
            alpha = NULL;
    }
    return alpha;
}


void computeBaseTemplates(double* input, int nx, int ny, int M, int borderCondition, double sigma, double** templates) {
    
    int wWidth = (int)(4.0*sigma);
    int nk = wWidth+1;
    
    double* aKernel = new double[nk];
    double* bKernel = new double[nk];
    double* g = new double[nk];
    double* buffer = new double[nx*ny];
    cout<<"In computeBaseTemplates, buffer initialized"<<endl;    


    double d;
    double sigma2 = sigma*sigma;
    double sigma4 = sigma2*sigma2;
    double sigma6 = sigma4*sigma2;
    double sigma8 = sigma4*sigma4;
    double sigma10 = sigma6*sigma4;
    double sigma12 = sigma8*sigma4;
    
    for (int i=0;i<nk;i++) {
        g[i] = exp(-(i*i)/(2.0*sigma2));
        cout<<"g[i] = "<<g[i]<<endl;    
    }
    
    // set up convolver
    Convolver conv = Convolver(input, nx, ny, borderCondition);
    cout<<"Convolver set up"<<endl;    
    
    if (M == 1 || M == 3 || M == 5) {
        
        d = 2.0*PI*sigma4;
        for (int i=0;i<nk;i++) {
            aKernel[i] = -i*g[i] / d;
        }
        
        // g_x
        conv.convolveOddXEvenY(aKernel, nk, g, nk, templates[0]);
        
        // g_y
        conv.convolveEvenXOddY(g, nk, aKernel, nk, templates[1]);
        
        if (M == 3 || M == 5) {
            
            d = 2.0*PI*sigma8;
            for (int i=0;i<nk;i++) {
                aKernel[i] = (3.0*i*sigma2 - i*i*i) * g[i] / d;
            }
            // g_xxx
            conv.convolveOddXEvenY(aKernel, nk, g, nk, templates[2]);
            
            // g_yyy
            conv.convolveEvenXOddY(g, nk, aKernel, nk, templates[5]);
            for (int i=0;i<nk;i++) {
                aKernel[i] = (sigma2 - i*i) * g[i] / d;
                bKernel[i] = i*g[i];
            }
            // gxxy
            conv.convolveEvenXOddY(aKernel, nk, bKernel, nk, templates[3]);
            // gxyy
            conv.convolveOddXEvenY(bKernel, nk, aKernel, nk, templates[4]);
        }
        if (M == 5) {
            
            d = 2.0*PI*sigma12;
            for (int i=0;i<nk;i++) {
                aKernel[i] = -i*(i*i*i*i - 10.0*i*i*sigma2 + 15.0*sigma4) * g[i] / d;
            }
            // gxxxxx
            conv.convolveOddXEvenY(aKernel, nk, g, nk, templates[6]);
            // gyyyyy
            conv.convolveEvenXOddY(g, nk, aKernel, nk, templates[11]);
            for (int i=0;i<nk;i++) {
                aKernel[i] = (i*i*i*i - 6.0*i*i*sigma2 + 3.0*sigma4) * g[i] / d;
                bKernel[i] = -i * g[i];
            }
            // g_xxxxy
            conv.convolveEvenXOddY(aKernel, nk, bKernel, nk, templates[7]);
            // g_xyyyy
            conv.convolveOddXEvenY(bKernel, nk, aKernel, nk, templates[10]);
            for (int i=0;i<nk;i++) {
                aKernel[i] = i*(i*i - 3.0*sigma2) * g[i] / d;
                bKernel[i] = (sigma2 - i*i) * g[i];
            }
            // g_xxxyy
            conv.convolveOddXEvenY(aKernel, nk, bKernel, nk, templates[8]);
            // g_xxyyy
            conv.convolveEvenXOddY(bKernel, nk, aKernel, nk, templates[9]);
        }
    } else { //(M == 2 || M == 4)
        
        d = 2.0*PI*sigma6;
        for (int i=0;i<nk;i++) {
            aKernel[i] = (i*i - sigma2) * g[i] / d;
        }
        // g_xx
        conv.convolveEvenXEvenY(aKernel, nk, g, nk, templates[0]);
        // g_yy
        conv.convolveEvenXEvenY(g, nk, aKernel, nk, templates[2]);
        for (int i=0;i<nk;i++) {
            aKernel[i] = i * g[i];
            bKernel[i] = aKernel[i] / d;
        }
        // g_xy
        conv.convolveOddXOddY(aKernel, nk, bKernel, nk, templates[1]);
        
        if (M == 4) {
            
            d = 2.0*PI*sigma10;
            for (int i=0;i<nk;i++) {
                aKernel[i] = (i*i*i*i - 6.0*i*i*sigma2 + 3.0*sigma4) * g[i] / d;
            }
            // g_xxxx
            conv.convolveEvenXEvenY(aKernel, nk, g, nk, templates[3]);
            // g_yyyy
            conv.convolveEvenXEvenY(g, nk, aKernel, nk, templates[7]);
            for (int i=0;i<nk;i++) {
                aKernel[i] = i * (i*i - 3.0*sigma2) * g[i] / d;
                bKernel[i] = i * g[i];
            }
            // g_xxxy
            conv.convolveOddXOddY(aKernel, nk, bKernel, nk, templates[4]);
            // g_xyyy
            conv.convolveOddXOddY(bKernel, nk, aKernel, nk, templates[6]);
            for (int i=0;i<nk;i++) {
                aKernel[i] = (sigma2 - i*i) * g[i];
                bKernel[i] = aKernel[i] / d;
            }
            // g_xxyy
            conv.convolveEvenXEvenY(aKernel, nk, bKernel, nk, templates[5]);
        }
    }
    
    // free memory
    delete[] aKernel;
    delete[] bKernel;
    delete[] g;
    delete[] buffer;
}




// Point responses
double pointRespM1(int i, double angle, double* alpha, double** templates) {
    
    double cosT = cos(angle);
    double sinT = sin(angle);
    
    double gxi = templates[0][i];
    double gyi = templates[1][i];
    double a11 = alpha[0];
    
    double result = a11 * (cosT*gxi + sinT*gyi);
    return result;
}


double pointRespM2(int i, double angle, double* alpha, double** templates) {
    
    double cosT = cos(angle);
    double sinT = sin(angle);
    
    double gxxi = templates[0][i];
    double gxyi = templates[1][i];
    double gyyi = templates[2][i];
    double a20 = alpha[0];
    double a22 = alpha[1];
    
    double result = sinT*sinT * (a20*gyyi+a22*gxxi)
                  + sinT*cosT*2.0*(a20-a22)*gxyi
                  + cosT*cosT * (a20*gxxi+a22*gyyi);

    return result;
}


double pointRespM3(int i, double angle, double* alpha, double** templates) {
    
    double cosT = cos(angle);
    double sinT = sin(angle);
    double cosT2 = cosT*cosT;
    double sinT2 = sinT*sinT;
    
    double gxi = templates[0][i];
    double gyi = templates[1][i];
    double gxxxi = templates[2][i];
    double gxxyi = templates[3][i];
    double gxyyi = templates[4][i];
    double gyyyi = templates[5][i];
    
    double a10 = alpha[0];
    double a30 = alpha[1];
    double a32 = alpha[2];
    
    double result = a10*(sinT*gyi + cosT*gxi)
            + sinT2*sinT*(a30*gyyyi + a32*gxxyi)
            + cosT*sinT2*(a32*gxxxi + (3.0*a30-2.0*a32)*gxyyi)
            + cosT2*sinT*(a32*gyyyi + (3.0*a30-2.0*a32)*gxxyi)
            + cosT2*cosT*(a32*gxyyi + a30*gxxxi);
    
    return result;
}


double pointRespM4(int i, double angle, double* alpha, double** templates) {
    
    double cosT = cos(angle);
    double sinT = sin(angle);
    double sinTcosT = cosT*sinT;
    double cosT2 = cosT*cosT;
    double sinT2 = sinT*sinT;
    
    double gxxi = templates[0][i];
    double gxyi = templates[1][i];
    double gyyi = templates[2][i];    
    double gxxxxi = templates[3][i];
    double gxxxyi = templates[4][i];
    double gxxyyi = templates[5][i];
    double gxyyyi = templates[6][i];
    double gyyyyi = templates[7][i];
    
    double a20 = alpha[0];
    double a22 = alpha[1];
    double a40 = alpha[2];
    double a42 = alpha[3];
    double a44 = alpha[4];            
       
    double result = sinT2*(a20*gyyi + a22*gxxi)
                  + sinT*cosT*2.0*(a20-a22)*gxyi
                  + cosT2*(a20*gxxi + a22*gyyi)
                  + sinT2*sinT2*(a40*gyyyyi + a42*gxxyyi + a44*gxxxxi)
                  + sinT2*sinTcosT*2.0*((2.0*a40-a42)*gxyyyi + (a42-2.0*a44)*gxxxyi)
                  + sinT2*cosT2*(a42*gyyyyi + (6.0*a40 - 4.0*a42 + 6.0*a44)*gxxyyi + a42*gxxxxi)
                  + sinTcosT*cosT2*2.0*((2.0*a40-a42)*gxxxyi + (a42-2.0*a44)*gxyyyi)
                  + cosT2*cosT2*(a44*gyyyyi + a42*gxxyyi + a40*gxxxxi);
    
    return result;
}



double pointRespM5(int i, double angle, double* alpha, double** templates) {
    
    double cosT = cos(angle);
    double sinT = sin(angle);
    double cosT2 = cosT*cosT;
    double sinT2 = sinT*sinT;
    double cosT3 = cosT2*cosT;
    double sinT3 = sinT2*sinT;
    double sinT4 = sinT2*sinT2;
    double cosT4 = cosT2*cosT2;
    double sinT5 = sinT2*sinT3;
    double cosT5 = cosT2*cosT3;
    
    double gxi = templates[0][i];
    double gyi = templates[1][i];
    double gxxxi = templates[2][i];
    double gxxyi = templates[3][i];
    double gxyyi = templates[4][i];
    double gyyyi = templates[5][i];
    double gxxxxxi = templates[6][i];
    double gxxxxyi = templates[7][i];
    double gxxxyyi = templates[8][i];
    double gxxyyyi = templates[9][i];
    double gxyyyyi = templates[10][i];
    double gyyyyyi = templates[11][i];
    
    double a10 = alpha[0];
    double a30 = alpha[1];
    double a32 = alpha[2];
    double a52 = alpha[3];
    double a54 = alpha[4];
    
    double result = a10*(sinT*gyi + cosT*gxi)
                  + sinT3*(a30*gyyyi + a32*gxxyi)
                  + sinT2*cosT*((3.0*a30-2.0*a32)*gxyyi + a32*gxxxi)
                  + cosT2*sinT*((3.0*a30-2.0*a32)*gxxyi + a32*gyyyi)
                  + cosT3*(a32*gxyyi + a30*gxxxi)
                  + sinT5*(a52*gxxyyyi + a54*gxxxxyi)
                  + sinT4*cosT*(-2.0*a52*gxyyyyi + (3.0*a52-4.0*a54)*gxxxyyi + a54*gxxxxxi)
                  + sinT3*cosT2*(a52*gyyyyyi + 6.0*(a54-a52)*gxxyyyi + (3.0*a52-4.0*a54)*gxxxxyi)
                  + sinT2*cosT3*(a52*gxxxxxi + 6.0*(a54-a52)*gxxxyyi + (3.0*a52-4.0*a54)*gxyyyyi)
                  + cosT4*sinT*(-2.0*a52*gxxxxyi + (3.0*a52-4.0*a54)*gxxyyyi + a54*gyyyyyi)
                  + cosT5*(a54*gxyyyyi + a52*gxxxyyi);
    return result;
}




int getRealRoots(double* z, int nz, double* roots) {
    int nr = nz/2; // total roots

    int nrr = 0; // real roots
    for (int k=0;k<nr;++k) {
        if (z[2*k+1]==0.0)
            nrr++;
    }
    roots = new double[nrr];
    nrr = 0;
    for (int k=0;k<nr;++k) {
        if (z[2*k+1]==0.0)
            roots[nrr++] = z[2*k];
    }
    return nrr;
}



void filterM1(double** templates, int nx, int ny, double* alpha, double* response, double* orientation) {
    
    double* gx = templates[0];
    double* gy = templates[1];
    double a11 = alpha[0];
    
    double* tRoots = new double[2];
    double gxi, gyi;
    
    for (int i=0;i<nx*ny;++i) {
        gxi = approxZero(gx[i]);
        gyi = approxZero(gy[i]);
        
        orientation[i] = atan2(gyi,gxi);
        response[i] = a11*sqrt(gxi*gxi + gyi*gyi);
    }
    delete[] tRoots;
}




// quadratic root solution
void filterM2(double** templates, int nx, int ny, double* alpha, double* response, double* orientation) {
    
    double* gxx = templates[0];
    double* gxy = templates[1];
    double* gyy = templates[2];
    double a20 = alpha[0];
    double a22 = alpha[1];
    
    double A, B, C;
    double a = a22-a20;
    double temp;
   
    for (int i=0;i<nx*ny;++i) {
                
        A = a*gxy[i];
        B = a*(gxx[i]-gyy[i]);
        C = -A;

        if (A == 0.0) { // -> linear
            if (B == 0.0) { // -> null, solve
                orientation[i] = 0.0;
                response[i] = pointRespM2(i, 0.0, alpha, templates);
            } else { // solve linear
                if (C == 0.0) {
                    orientation[i] = 0.0;
                    response[i] = pointRespM2(i, 0.0, alpha, templates);
                    temp = pointRespM2(i, PI/2.0, alpha, templates);
                    if (temp > response[i]) {
                        response[i] = temp;
                        orientation[i] = PI/2.0;
                    }
                } else {
                    orientation[i] = atan(-C/B);
                    response[i] = pointRespM2(i, orientation[i], alpha, templates);
                    temp = pointRespM2(i, orientation[i]+PI/2.0, alpha, templates);
                    if (temp > response[i]) {
                        response[i] = temp;
                        orientation[i] += PI/2.0;
                    }
                }
            }
        } else { // solve quadratic
            double* xRoots = new double[2];
            gsl_poly_solve_quadratic (A, B, C, &xRoots[0], &xRoots[1]);
            
            double* tRoots = new double[2];
            tRoots[0] = atan(xRoots[0]);
            tRoots[1] = atan(xRoots[1]);
            response[i] = pointRespM2(i, tRoots[0], alpha, templates);
            orientation[i] = tRoots[0];
            temp = pointRespM2(i, tRoots[1], alpha, templates);
            if (temp > response[i]) {
                response[i] = temp;
                orientation[i] = tRoots[1];
            }
            delete[] xRoots;
            delete[] tRoots;
        }
    }
}



void filterM3(double** templates, int nx, int ny, double* alpha, double* response, double* orientation) {
    
    double* gx = templates[0];
    double* gy = templates[1];
    double* gxxx = templates[2];
    double* gxxy = templates[3];
    double* gxyy = templates[4];
    double* gyyy = templates[5];
    
    double a10 = alpha[0];
    double a30 = alpha[1];
    double a32 = alpha[2];
    
    double A, B, C, D;
    
    int nr, nt, deg;
    
    for (int i=0;i<nx*ny;++i) {
        
        A = -a10*gx[i] + (2.0*a32-3.0*a30)*gxyy[i] - a32*gxxx[i]; // sin^3
        B =  a10*gy[i] + (3.0*a30-2.0*a32)*gyyy[i] + (7.0*a32-6.0*a30)*gxxy[i]; // sin^2 cos
        C = -a10*gx[i] + (2.0*a32-3.0*a30)*gxxx[i] + (6.0*a30-7.0*a32)*gxyy[i];
        D =  a10*gy[i] + (3.0*a30-2.0*a32)*gxxy[i] + a32*gyyy[i];
        
        A = approxZero(A);
        B = approxZero(B);
        C = approxZero(C);
        D = approxZero(D);
        
        double* roots;
        double* z;
        if (A == 0.0) { // -> quadratic
            if (B == 0.0) { // -> linear
                if (C == 0.0) {// -> null, fixed solution
                    deg = 1;
                    z = new double[2*deg];
                    z[0] = 0.0;
                    z[1] = 0.0;
                } else {
                    deg = 1;
                    double a[2] = {D, C};
                    z = new double[2*deg];
                    gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                    gsl_poly_complex_solve(a, deg+1, w, z);
                    gsl_poly_complex_workspace_free(w);
                }
            } else { // B!=0
                deg = 2;
                double a[3] = {D, C, B};
                z = new double[2*deg];
                gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                gsl_poly_complex_solve(a, deg+1, w, z);
                gsl_poly_complex_workspace_free(w);
            }
        } else { // solve cubic
            deg = 3;
            double a[4] = {D, C, B, A};
            z = new double[2*deg];
            gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
            gsl_poly_complex_solve(a, deg+1, w, z);
            gsl_poly_complex_workspace_free(w);
        }
    
        nr = 0;
        for (int k=0;k<deg;++k) {
            if (z[2*k+1]==0.0)
                nr++;
        }
        roots = new double[nr];
        nr = 0;
        for (int k=0;k<deg;++k) {
            if (z[2*k+1]==0.0) {
                roots[nr++] = z[2*k];
            }
        }
        
        double* tRoots;
        if (nr == 0) {
            nt = 4;
            tRoots = new double[nt];
            tRoots[0] = -PI/2.0;
            tRoots[1] = 0.0;
            tRoots[2] = PI/2.0;
            tRoots[3] = PI;
        } else {
            nt = 2*nr;
            tRoots = new double[nt];
            for (int k=0;k<nr;k++) {
                tRoots[k] = atan(roots[k]);
                tRoots[k+nr] = opposite(tRoots[k]);
            }
        }
        delete[] roots;
        
        
        response[i] = pointRespM3(i, tRoots[0], alpha, templates);
        orientation[i] = tRoots[0];
        
        double temp;
        for (int k=1;k<nt;k++) {
            temp = pointRespM3(i, tRoots[k], alpha, templates);
            if (temp > response[i]) {
                response[i] = temp;
                orientation[i] = tRoots[k];
            }
        }
        delete[] tRoots;
        delete[] z;
    }
}




void filterM4(double** templates, int nx, int ny, double* alpha, double* response, double* orientation) {
    
    double* gxx = templates[0];
    double* gxy = templates[1];
    double* gyy = templates[2];
    double* gxxxx = templates[3];
    double* gxxxy = templates[4];
    double* gxxyy = templates[5];
    double* gxyyy = templates[6];
    double* gyyyy = templates[7];

    double a20 = alpha[0];
    double a22 = alpha[1];
    double a40 = alpha[2];
    double a42 = alpha[3];
    double a44 = alpha[4];
    
    double A, B, C, D, E;
    
    int nr, deg;
    double delta;
    
    cout<<"Fourth order ST before start loop "<<endl;
           
    for (int i=0;i<nx*ny;i++) {
        
        A = (a22-a20)*gxy[i] + (a42-2.0*a40)*gxyyy[i] + (2.0*a44-a42)*gxxxy[i];
        B = (a20-a22)*gyy[i] + (a22-a20)*gxx[i] + (2.0*a40-a42)*gyyyy[i] + 6.0*(a42-a40-a44)*gxxyy[i] + (2.0*a44-a42)*gxxxx[i];
        C = 6.0*((a40-a42+ a44)*gxyyy[i] + (a42-a40-a44)*gxxxy[i]);
        D = (a20-a22)*gyy[i] + (a22-a20)*gxx[i] + (a42-2.0*a44)*gyyyy[i] + 6.0*(a40-a42+a44)*gxxyy[i] + (a42-2.0*a40)*gxxxx[i];
        E = (a20-a22)*gxy[i] + (a42-2.0*a44)*gxyyy[i] + (2.0*a40-a42)*gxxxy[i];
        
        A = approxZero(A);
        C = approxZero(C);
        E = approxZero(E);
        
        double* roots;
        double* z;
        if (A == 0.0) { // -> cubic
            if (B == 0.0) { // -> quadratic
                if (C == 0.0) { // -> linear
                    if (D == 0.0) { // solve null
                        deg = 1;
                        z = new double[2*deg];
                        z[0] = 0.0;
                        z[1] = 0.0;
                    } else { // solve linear
                        deg = 1;
                        double a[2] = {E, D};
                        z = new double[2*deg];
                        gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                        gsl_poly_complex_solve(a, deg+1, w, z);
                        gsl_poly_complex_workspace_free(w);
                    }
                } else { // solve quadratic
                    deg = 2;
                    double a[3] = {E, D, C};
                    z = new double[2*deg];
                    gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                    gsl_poly_complex_solve(a, deg+1, w, z);
                    gsl_poly_complex_workspace_free(w);
                }
            } else { // solve cubic
                if ( (C == 0.0) && (E == 0.0) ) {
                    delta = -D/B;
                    if (delta > 0.0) {
                        deg = 3;
                        delta = sqrt(delta);
                        z = new double[2*deg];
                        z[0] = 0.0;
                        z[1] = 0.0;
                        z[2] = delta;
                        z[3] = 0.0;
                        z[4] = -delta;
                        z[5] = 0.0;
                    } else {
                        deg = 1;
                        z = new double[2*deg];
                        z[0] = 0.0;
                        z[1] = 0.0;
                    }
                } else {
                    deg = 3;
                    double a[4] = {E, D, C, B};
                    z = new double[2*deg];
                    gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                    gsl_poly_complex_solve(a, deg+1, w, z);
                    gsl_poly_complex_workspace_free(w);
                }
            }
        } else { // solve quartic
            deg = 4;
            double a[5] = {E, D, C, B, A};
            z = new double[2*deg];
            gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
            gsl_poly_complex_solve(a, deg+1, w, z);
            gsl_poly_complex_workspace_free(w);
        }
        
        // # real roots
        nr = 0;
        for (int k=0;k<deg;++k) {
            if (z[2*k+1]==0.0)
                nr++;
        }
        roots = new double[nr];
        nr = 0;
        for (int k=0;k<deg;++k) {
            if (z[2*k+1]==0.0) {
                roots[nr++] = z[2*k];
            }
        }

        
        double* tRoots;
        if (nr == 0) { //orientation[i] = 0.0;
            nr = 2;
            tRoots = new double[nr];
            tRoots[0] = 0.0;
            tRoots[1] = PI/2.0;
        } else {
            if (roots[0] == 0.0) {
                nr++;
                tRoots = new double[nr];
                for (int k=0;k<nr-1;k++) {
                    tRoots[k] = atan(roots[k]);
                }
                tRoots[nr-1] = PI/2;
            } else {
                tRoots = new double[nr];
                for (int k=0;k<nr;k++) {
                    tRoots[k] = atan(roots[k]);
                }
            }
        }
        delete[] roots;
              
    
        response[i] = pointRespM4(i, tRoots[0], alpha, templates);
        orientation[i] = tRoots[0];
       
        double temp;
        for (int k=1;k<nr;k++) {
            temp = pointRespM4(i, tRoots[k], alpha, templates);
            if (temp > response[i]) {
                response[i] = temp;
                orientation[i] = tRoots[k];
            }
        }

        //cout<<"i="<<i<<"response[i]: "<<response[i]<<endl;  


        delete[] z;
        delete[] tRoots;
    }
    cout<<"Fourth order ST loop ended "<<endl;
        
}



void filterM5(double** templates, int nx, int ny, double* alpha, double* response, double* orientation) {
    
    double* gx = templates[0];
    double* gy = templates[1];
    double* gxxx = templates[2];
    double* gxxy = templates[3];
    double* gxyy = templates[4];
    double* gyyy = templates[5];
    double* gxxxxx = templates[6];
    double* gxxxxy = templates[7];
    double* gxxxyy = templates[8];
    double* gxxyyy = templates[9];
    double* gxyyyy = templates[10];
    double* gyyyyy = templates[11];
    
    double a10 = alpha[0];
    double a30 = alpha[1];
    double a32 = alpha[2];
    double a52 = alpha[3];
    double a54 = alpha[4];
    
    double A, B, C, D, E, F;
    int nr, nt, deg;
    double delta;
    
    for (int i=0;i<nx*ny;++i) {

        A = -a10*gx[i] + (2.0*a32-3.0*a30)*gxyy[i] - a32*gxxx[i] + (4.0*a54-3.0*a52)*gxxxyy[i] + 2.0*a52*gxyyyy[i] -a54*gxxxxx[i];
        B = a10*gy[i] + (3.0*a30-2.0*a32)*gyyy[i] + (7.0*a32-6.0*a30)*gxxy[i] - 2.0*a52*gyyyyy[i] + (17.0*a52-12.0*a54)*gxxyyy[i] + (13.0*a54-6.0*a52)*gxxxxy[i];
        C = -2.0*a10*gx[i] + (3.0*a30-5.0*a32)*gxyy[i] + (a32-3.0*a30)*gxxx[i] + (12.0*a54-17.0*a52)*gxyyyy[i] + (30.0*a52-34.0*a54)*gxxxyy[i] + (4.0*a54-3.0*a52)*gxxxxx[i];
        D = 2.0*a10*gy[i] + (5.0*a32-3.0*a30)*gxxy[i] + (3.0*a30-a32)*gyyy[i] + (17.0*a52-12.0*a54)*gxxxxy[i] + (34.0*a54-30.0*a52)*gxxyyy[i] + (3.0*a52-4.0*a54)*gyyyyy[i];
        E = -a10*gx[i] + (2.0*a32-3.0*a30)*gxxx[i] + (6.0*a30-7.0*a32)*gxyy[i] + 2.0*a52*gxxxxx[i] + (12.0*a54-17.0*a52)*gxxxyy[i] + (6.0*a52-13.0*a54)*gxyyyy[i];
        F = a10*gy[i] + (3.0*a30-2.0*a32)*gxxy[i] + a32*gyyy[i] + (3.0*a52-4.0*a54)*gxxyyy[i] - 2.0*a52*gxxxxy[i]+ a54*gyyyyy[i];
        
        A = approxZero(A);
        B = approxZero(B);
        C = approxZero(C);
        D = approxZero(D);
        E = approxZero(E);
        F = approxZero(F);
        
        double* roots;
        double* z;
        
        if (A == 0.0) { // quartic
            if (B == 0.0) { // cubic
                if (C == 0.0) { // quadratic
                    if (D == 0.0) { // linear
                        if (E == 0.0) { // null
                            deg = 1;
                            z = new double[2*deg];
                            z[0] = 0.0;
                            z[1] = 0.0;
                        } else {
                            deg = 1;
                            double a[2] = {E, D};
                            z = new double[2*deg];
                            gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                            gsl_poly_complex_solve(a, deg+1, w, z);
                            gsl_poly_complex_workspace_free(w);
                        }
                    } else { // solve quadratic
                        deg = 2;
                        double a[3] = {E, D, C};
                        z = new double[2*deg];
                        gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                        gsl_poly_complex_solve(a, deg+1, w, z);
                        gsl_poly_complex_workspace_free(w);
                    }
                } else { // solve cubic
                    if ( (D == 0.0) && (F == 0.0) ) {
                        delta = -E/C;
                        if (delta > 0.0) {
                            deg = 3;
                            delta = sqrt(delta);
                            z = new double[2*deg];
                            z[0] = 0.0;
                            z[1] = 0.0;
                            z[2] = delta;
                            z[3] = 0.0;
                            z[4] = -delta;
                            z[5] = 0.0;
                        } else {
                            deg = 1;
                            z = new double[2*deg];
                            z[0] = 0.0;
                            z[1] = 0.0;
                        }
                    } else {
                        deg = 3;
                        double a[4] = {F, E, D, C};
                        z = new double[2*deg];
                        gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                        gsl_poly_complex_solve(a, deg+1, w, z);
                        gsl_poly_complex_workspace_free(w);
                    }
                }
            } else { // solve quartic
                deg = 4;
                double a[5] = {F, E, D, C, B};
                z = new double[2*deg];
                gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
                gsl_poly_complex_solve(a, deg+1, w, z);
                gsl_poly_complex_workspace_free(w);
            }
        } else {
            deg = 5;
            double a[6] = {F, E, D, C, B, A};
            z = new double[2*deg];
            gsl_poly_complex_workspace* w = gsl_poly_complex_workspace_alloc(deg+1);
            gsl_poly_complex_solve(a, deg+1, w, z);
            gsl_poly_complex_workspace_free(w);
        }

        
        // # real roots
        nr = 0;
        for (int k=0;k<deg;++k) {
            if (z[2*k+1]==0.0)
                nr++;
        }
        roots = new double[nr];
        nr = 0;
        for (int k=0;k<deg;++k) {
            if (z[2*k+1]==0.0) {
                roots[nr++] = z[2*k];
            }
        }

        
        double* tRoots;
        if (nr == 0) { //orientation[i] = 0.0;
            nt = 4;
            tRoots = new double[nt];
            tRoots[0] = -PI/2.0;
            tRoots[1] = 0.0;
            tRoots[2] = PI/2.0;
            tRoots[3] = PI;
        } else {
            nt = 2*nr;
            tRoots = new double[nt];
            for (int k=0;k<nr;k++) {
                tRoots[k] = atan(roots[k]);
                tRoots[k+nr] = opposite(tRoots[k]);
            }
        }
        delete[] roots;
              
        response[i] = pointRespM5(i, tRoots[0], alpha, templates);
        orientation[i] = tRoots[0];
       
        double temp;
        for (int k=1;k<nt;k++) {
            temp = pointRespM5(i, tRoots[k], alpha, templates);
            if (temp > response[i]) {
                response[i] = temp;
                orientation[i] = tRoots[k];
            }
        }
        delete[] z;
        delete[] tRoots;
    }
}



// Mirror position in image domain for interpolation
int mirror(int x, int nx) {
    if (x >= 0 && x < nx) {
        return x;
    } else if (x < 0) {
        return -x;
    } else {
        return 2*nx-2-x;
    }
}


double interp(double* image, int nx, int ny, double x, double y) {
    int xi = (int)x;
    int yi = (int)y;
    int x0, x1, y0, y1;
    
    double dx = x-xi;
    double dy = y-yi;
    if (x < 0) { dx = -dx; x1 = mirror(xi-1, nx); } else { x1 = mirror(xi+1, nx); }
    if (y < 0) { dy = -dy; y1 = mirror(yi-1, ny); } else { y1 = mirror(yi+1, ny); }
    x0 = mirror(xi, nx);
    y0 = mirror(yi, ny);
    return (1.0-dy)*(dx*image[x1+y0*nx] + (1.0-dx)*image[x0+y0*nx]) + dy*(dx*image[x1+y1*nx] + (1.0-dx)*image[x0+y1*nx]);
}



void computeNMS(double* response, double* orientation, double* nms, int nx, int ny) {
    
    double ux, uy, v1, v2;
    
    div_t divRes;
    for (int i=0;i<nx*ny;++i) {
        divRes = div(i, nx);
        ux = cos(orientation[i]);
        uy = sin(orientation[i]);  

        //cout<<"i="<<i<<", ux: "<<ux<<endl;  
        //cout<<"i="<<i<<", uy: "<<ux<<endl;  


        v1 = interp(response, nx, ny, divRes.rem+ux, divRes.quot+uy);
        v2 = interp(response, nx, ny, divRes.rem-ux, divRes.quot-uy);
        
        //cout<<"i="<<i<<", v1: "<<v1<<endl;  
        //cout<<"i="<<i<<", v2: "<<v2<<endl;  


        if (v1 > response[i] || v2 > response[i]) {
            nms[i] = 0.0;
        } else {
            nms[i] = response[i];
        }  
        //cout<<"i="<<i<<", nms[i]: "<<nms[i]<<endl;  
      
    }
}


void steerablefilter2Dcore(double * input, long* in_sz, int M, double sigma,double* &output_response, double* &output_orientation, double* &output_nms) {
    // Set defaults for options
    int borderCondition = 1;
    int nt = 36;
        
    long nx = in_sz[0];
    long ny = in_sz[1];
    long nz = in_sz[2];
    long pagesz = ny*nx*nz;

    cout<<"core: nx = "<<nx<<endl;
    cout<<"core: ny = "<<ny<<endl;
    cout<<"core: nz = "<<nz<<endl;
    cout<<"core: pagesz = "<<pagesz<<endl;
    
    int L = 2*(int)(4.0*sigma)+1; // support of the Gaussian kernels
         
    cout<<"core: M = "<<M<<endl;
    cout<<"core: sigma = "<<sigma<<endl;

    // number of partial derivative templates
    int nTemplates = getTemplateN(M);
    cout<<"core: nTemplates = "<<nTemplates<<endl;    

    double* response = new double[pagesz];
    double* orientation = new double[pagesz];
    double* nms = new double[pagesz];
    cout<<"core: Output initialized "<<endl;

    // Allocate template memory
    double** templates = new double*[nTemplates];
    for (int i=0;i<nTemplates;++i) {
        templates[i] = new double[pagesz];
    }
    cout<<"core: Templates initialized "<<endl;

    
    // Compute the templates used in the steerable filterbank
    computeBaseTemplates((double *)input, nx, ny, M, borderCondition, sigma, templates);
    double* alpha = getWeights(M, sigma);

    cout<<"core: Templates used in Filterbank ready"<<endl;

    
    // apply filter
    switch (M) {
        case 1:
            cout<<"First order ST starts: "<<endl;
            filterM1(templates, nx, ny, alpha, response, orientation);
            break;
        case 2:
            cout<<"Second order ST starts: "<<endl;
            filterM2(templates, nx, ny, alpha, response, orientation);
            break;
        case 3:
            cout<<"Third order ST starts: "<<endl;
            filterM3(templates, nx, ny, alpha, response, orientation);
            break;
        case 4:
            cout<<"Fourth order ST starts: "<<endl;
            filterM4(templates, nx, ny, alpha, response, orientation);
            break;
        case 5:
            cout<<"Fifth order ST starts: "<<endl;
            filterM5(templates, nx, ny, alpha, response, orientation);
            break;
    }
   

    cout<<"Calculate nms: start "<<endl;
    computeNMS(response, orientation, nms, nx, ny);    
    

    output_response = response;
    output_orientation = orientation;
    output_nms = nms;

    //for(int index = 0; index< nx*ny; index++){
    //   cout<<"index="<<index<<"output_response[index]: "<<output_response[index]<<endl;         
    //}
    /*
    // Free memory
    for (int i=0;i<nTemplates;++i) {
        delete [] templates[i];
    }
    delete[] templates;
    delete[] response;
    delete[] orientation;
    delete[] alpha;
    */
}


// compiled with:
// export DYLD_LIBRARY_PATH=/Applications/MATLAB_R2012b.app/bin/maci64 && g++ -Wall -g -DARRAY_ACCESS_INLINING -I. -L/Applications/MATLAB_R2012b.app/bin/maci64 -I../../mex/include/ -I/Applications/MATLAB_R2012b.app/extern/include steerableDetector.cpp -lmx -lmex -lgsl
// tested with:
// valgrind --tool=memcheck --leak-check=full --show-reachable=yes ./a.out 2>&1 | grep steerable

/*int main(void) {
    int nx = 200;
    int ny = 100;
    int N = nx*ny;
    double* pixels = new double[nx*ny];
    for (int i=0;i<nx*ny;++i) {
        pixels[i] = rand();
    }
    int M = 3;
    double sigma = 3.0;
    
    
    int nTemplates = getTemplateN(M);

    
    // Allocate template memory
    double** templates = new double*[nTemplates];
    for (int i=0;i<nTemplates;++i) {
        templates[i] = new double[N];
    }
    double* response = new double[N];
    double* orientation = new double[N];
    
    // Compute the templates used in the steerable filterbank
    computeBaseTemplates(pixels, nx, ny, M, 3, sigma, templates);
    double* alpha = getWeights(M, sigma);

    
    // apply filter
    switch (M) {
        case 1:
            filterM1(templates, nx, ny, alpha, response, orientation);
            break;
        case 2:
            filterM2(templates, nx, ny, alpha, response, orientation);
            break;
        case 3:
            filterM3(templates, nx, ny, alpha, response, orientation);
            break;
        case 4:
            filterM4(templates, nx, ny, alpha, response, orientation);
            break;
        case 5:
            filterM5(templates, nx, ny, alpha, response, orientation);
            break;
    }
    
    for (int i=0;i<nTemplates;++i) {
        delete [] templates[i];
    }
    delete[] templates;
    delete[] response;
    delete[] orientation;
    delete[] alpha;
    
    delete[] pixels;
}*/
