/* [response orientation nms filterBank] = steerableDetector3D(image, filterOrder, sigma);
 *
 * (c) Francois Aguet, 30/08/2012 (last modified 09/02/2012).
 *
 * Compilation:
 * Mac/Linux: mex -I/usr/local/include -I../../mex/include /usr/local/lib/libgsl.a /usr/local/lib/libgslcblas.a steerableDetector3D.cpp
 * Windows: mex COMPFLAGS="$COMPFLAGS /TP /MT" -I"..\..\..\extern\mex\include\gsl-1.14" -I"..\..\mex\include" "..\..\..\extern\mex\lib\gsl.lib" "..\..\..\extern\mex\lib\cblas.lib" -output steerableDetector3D steerableDetector3D.cpp
 */


#include <cstring>
#include <gsl/gsl_poly.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <algorithm>
#include <vector>

#include "convolver3D.h"

#define PI 3.141592653589793

Filter::Filter(const double voxels[], const int nx, const int ny, const int nz, const int M, const double sigma) {
    voxels_ = voxels;
    nx_ = nx;
    ny_ = ny;
    nz_ = nz;
    M_ = M;
    sigma_ = sigma;
    sigmaZ_ = sigma;
    init();
}

Filter::Filter(const double voxels[], const int nx, const int ny, const int nz, const int M, const double sigma, const double zxRatio) {
    voxels_ = voxels;
    nx_ = nx;
    ny_ = ny;
    nz_ = nz;
    M_ = M;
    sigma_ = sigma;
    sigmaZ_ = sigma/zxRatio;
    init();
}


void Filter::init() {
    if (M_==1) {
        alpha_ = 2.0/3.0;
        sign_ = -1.0;
        c_ = 2.0*sqrt(2.0*PI)*sigmaZ_;
    } else if (M_==2) {
        alpha_ = 4.0;
        sign_ = 1.0;
        c_ = 8.0*PI*sqrt(6.0)*sigmaZ_;
    }
    
    N_ = nx_*ny_*nz_;
    response_ = new double[N_];
    gxx_ = new double[N_];
    gxy_ = new double[N_];
    gxz_ = new double[N_];
    gyy_ = new double[N_];
    gyz_ = new double[N_];
    gzz_ = new double[N_];
    
    orientation_ = new double*[N_];
    for (int i=0;i<N_;++i) {
        orientation_[i] = new double[3];
    }
    
    calculateTemplates();
    run();
    
    nms_ = new double[N_];
    memset(nms_, 0, N_*sizeof(double));
    if (M_==1) {
        computeCurveNMS();
    } else if (M_==2) {
        computeSurfaceNMS();
    }    
}


Filter::~Filter() {
    delete[] response_;
    delete[] gxx_;
    delete[] gxy_;
    delete[] gxz_;
    delete[] gyy_;
    delete[] gyz_;
    delete[] gzz_;
    delete[] nms_;
    for (int i=0;i<N_;++i) {
        delete[] orientation_[i];
    }
    delete[] orientation_;
}


void Filter::calculateTemplates() {
    
    double *buffer = new double[N_];
    
    int wWidth = (int)(3.0*sigma_);
    int kLength = wWidth+1;
    double sigma2 = sigma_*sigma_;
    double sigma4 = sigma2*sigma2;
    
    int wWidthZ = (int)(3.0*sigmaZ_);
    int kLengthZ = wWidthZ+1;
    double sigmaZ2 = sigmaZ_*sigmaZ_;
    double sigmaZ4= sigmaZ2*sigmaZ2;
    
    // Compute Gaussian kernels
    double *kernelG = new double[kLength];
    double *kernelGx = new double[kLength];
    double *kernelGxx = new double[kLength];
    
    double *kernelG_z = new double[kLengthZ];
    double *kernelGx_z = new double[kLengthZ];
    double *kernelGxx_z = new double[kLengthZ];
    
    double g;
    for (int i=0;i<=wWidth;++i) {
        g = exp(-(i*i)/(2.0*sigma2)); // normalization by sqrt(2*PI)*sigma_ omitted
        kernelG[i] = g;               // to keep magnitude of response similar to input
        kernelGx[i] = -i/sigma2 * g;
        kernelGxx[i] = (i*i-sigma2)/sigma4 * g;
    }
    
    for (int i=0;i<=wWidthZ;++i) {
        g = exp(-(i*i)/(2.0*sigmaZ2));
        kernelG_z[i] = g;
        kernelGx_z[i] = -i/sigmaZ2 * g;
        kernelGxx_z[i] = (i*i-sigmaZ2)/sigmaZ4 * g;
    }
    
    // Convolve all along x
    convolveEvenX(voxels_, kernelG, kLength, nx_, ny_, nz_, gyy_);
    memcpy(gyz_, gyy_, N_*sizeof(double));
    memcpy(gzz_, gyy_, N_*sizeof(double));
    convolveOddX(voxels_, kernelGx, kLength, nx_, ny_, nz_, gxy_);
    memcpy(gxz_, gxy_, N_*sizeof(double));
    convolveEvenX(voxels_, kernelGxx, kLength, nx_, ny_, nz_, gxx_);
    // gxx
    convolveEvenY(gxx_, kernelG, kLength, nx_, ny_, nz_, buffer);
    convolveEvenZ(buffer, kernelG_z, kLengthZ, nx_, ny_, nz_, gxx_);
    // gxy
    convolveOddY(gxy_, kernelGx, kLength, nx_, ny_, nz_, buffer);
    convolveEvenZ(buffer, kernelG_z, kLengthZ, nx_, ny_, nz_, gxy_);
    // gxz
    convolveEvenY(gxz_, kernelG, kLength, nx_, ny_, nz_, buffer);
    convolveOddZ(buffer, kernelGx_z, kLengthZ, nx_, ny_, nz_, gxz_);
    // gyy
    convolveEvenY(gyy_, kernelGxx, kLength, nx_, ny_, nz_, buffer);
    convolveEvenZ(buffer, kernelG_z, kLengthZ, nx_, ny_, nz_, gyy_);
    // gyz
    convolveOddY(gyz_, kernelGx, kLength, nx_, ny_, nz_, buffer);
    convolveOddZ(buffer, kernelGx_z, kLengthZ, nx_, ny_, nz_, gyz_);
    // gzz
    convolveEvenY(gzz_, kernelG, kLength, nx_, ny_, nz_, buffer);
    convolveEvenZ(buffer, kernelGxx_z, kLengthZ, nx_, ny_, nz_, gzz_);
    
    delete[] kernelG;
    delete[] kernelGx;
    delete[] kernelGxx;
    delete[] kernelG_z;
    delete[] kernelGx_z;
    delete[] kernelGxx_z;
    delete[] buffer;
}


void Filter::run() {
    
    double a, b, c, d, e, f;
    
    for (int i=0;i<N_;++i) {
        
        a = sign_*(gyy_[i] + gzz_[i] - alpha_*gxx_[i]);
        b = sign_*(gxx_[i] + gzz_[i] - alpha_*gyy_[i]);
        c = sign_*(gxx_[i] + gyy_[i] - alpha_*gzz_[i]);
        d = -sign_*(1.0+alpha_)*gxy_[i];
        e = -sign_*(1.0+alpha_)*gxz_[i];
        f = -sign_*(1.0+alpha_)*gyz_[i];
        
        double A[] = {a,d,e,
                      d,b,f,
                      e,f,c};
        
        gsl_matrix_view m = gsl_matrix_view_array(A, 3, 3);
        
        gsl_vector *eval = gsl_vector_alloc(3);
        gsl_matrix *evec = gsl_matrix_alloc(3,3);
        
        gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc(3);
        gsl_eigen_symmv (&m.matrix, eval, evec, w);
        gsl_eigen_symmv_free(w);
        gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_VAL_ASC); // largest eigenvalue
        
        response_[i] = gsl_vector_get(eval, 2) / c_;
        orientation_[i][0] = gsl_matrix_get(evec, 0, 2);
        orientation_[i][1] = gsl_matrix_get(evec, 1, 2);
        orientation_[i][2] = gsl_matrix_get(evec, 2, 2);
    }
}


void Filter::normalize(double v[], const int k) {
    double n = 0.0;
    for (int i=0;i<k;++i) {
        n += v[i]*v[i];
    }
    n = sqrt(n);
    for (int i=0;i<k;++i) {
        v[i] /= n;
    }
}


double* Filter::getResponse() {
    return response_;
}


double* Filter::getNMS() {
    return nms_;
}


double** Filter::getOrientation() {
    return orientation_;
}



// Mirror position for interpolation border conditions
int Filter::mirror(const int x, const int nx) {
    if (x >= 0 && x < nx) {
        return x;
    } else if (x < 0) {
        return -x;
    } else {
        return 2*nx-2-x;
    }
}


double Filter::interpResponse(const double x, const double y, const double z) {
    int xi = (int)x;
    int yi = (int)y;
    int zi = (int)z;
    double dx = x-xi;
    double dy = y-yi;
    double dz = z-zi;
    int x0, x1, y0, y1, z0, z1;
    if (x < 0) { dx = -dx; x1 = mirror(xi-1, nx_); } else { x1 = mirror(xi+1, nx_); }
    if (y < 0) { dy = -dy; y1 = mirror(yi-1, ny_); } else { y1 = mirror(yi+1, ny_); }
    if (z < 0) { dz = -dz; z1 = mirror(zi-1, nz_); } else { z1 = mirror(zi+1, nz_); }
    x0 = mirror(xi, nx_);
    y0 = mirror(yi, ny_);
    z0 = mirror(zi, nz_);
    
    double z00 = (1.0-dy)*((1.0-dx)*response_[x0+y0*nx_+z0*nx_*ny_] + dx*response_[x1+y0*nx_+z0*nx_*ny_])
                     + dy*((1.0-dx)*response_[x0+y1*nx_+z0*nx_*ny_] + dx*response_[x1+y1*nx_+z0*nx_*ny_]);
    double z11 = (1.0-dy)*((1.0-dx)*response_[x0+y0*nx_+z1*nx_*ny_] + dx*response_[x1+y0*nx_+z1*nx_*ny_])
                     + dy*((1.0-dx)*response_[x0+y1*nx_+z1*nx_*ny_] + dx*response_[x1+y1*nx_+z1*nx_*ny_]);
    return (1.0-dz)*z00 + dz*z11;
    return 0.0;
}


// Orientation is stored as a Nx3 (N = nx*ny*nz) array
void Filter::computeSurfaceNMS() {
    double A1, A2;
    int i = 0;
    for (int z=0;z<nz_;++z) {
        for (int y=0;y<ny_;++y) {
            for (int x=0;x<nx_;++x) {
                if (!(orientation_[i][0]==0.0 && orientation_[i][1]==0.0 && orientation_[i][2]==0.0)) {
                    A1 = interpResponse(x+orientation_[i][0], y+orientation_[i][1], z+orientation_[i][2]);
                    A2 = interpResponse(x-orientation_[i][0], y-orientation_[i][1], z-orientation_[i][2]);
                    if (response_[i] > A1 && response_[i] > A2) {
                        nms_[i] = response_[i];
                    }
                }
                ++i;
            }
        }
    }
}


// Interpolate X points uniformly distributed on unit circle perpendicular to orientation
void Filter::computeCurveNMS() {
    double u[3];
    double v[3];
    double iv[3] = {1.0, 0.0, 0.0};
    double jv[3] = {0.0, 1.0, 0.0};
    
    int nt = 10;
    double theta[10];
    double dt = 2.0*PI/nt;
    for (int t=0;t<nt;++t) {
        theta[t] = t*dt;
    }
    
    int i = 0;
    double cosT, sinT, ival;
    for (int z=0;z<nz_;++z) {
        for (int y=0;y<ny_;++y) {
            for (int x=0;x<nx_;++x) {
                if (!(orientation_[i][0]==0.0 && orientation_[i][1]==0.0 && orientation_[i][2]==0.0)) {
                    
                    // vector perpendicular to 'n'
                    if (orientation_[i][0]!=1.0) { // use 'i'
                        cross(iv, orientation_[i], u);
                    } else { // use 'j'
                        cross(jv, orientation_[i], u);
                    }
                    normalize(u, 3);
                    cross(orientation_[i], u, v);
                    // u and v are orthogonal to the orientation vectors
                    
                    // interpolate at values given by (1, theta)
                    for (int t=0;t<nt;++t) {
                        cosT = cos(theta[t]);
                        sinT = sin(theta[t]);
                        ival = interpResponse(x+cosT*u[0]+sinT*v[0], y+cosT*u[1]+sinT*v[1], z+cosT*u[2]+sinT*v[2]);
                        if (ival >= response_[i]) {
                            break;
                        }
                    }
                    if (ival < response_[i]) {
                        nms_[i] = response_[i];
                    }
                }
                i++;
            }
        }
    }
}


void Filter::cross(const double v1[], const double v2[], double r[]) {
    r[0] = v1[1]*v2[2]-v1[2]*v2[1];
    r[1] = v1[2]*v2[0]-v1[0]*v2[2];
    r[2] = v1[0]*v2[1]-v1[1]*v2[0];
}
