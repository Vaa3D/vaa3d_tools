/* [response orientation nms filterBank] = steerableDetector3D(image, filterOrder, sigma);
 *
 * (c) Francois Aguet, 30/08/2012 (last modified 09/02/2012).
 *
 * Compilation:
 * Mac/Linux: mex -I/usr/local/include -I../../mex/include /usr/local/lib/libgsl.a /usr/local/lib/libgslcblas.a steerableDetector3D.cpp
 * Windows: mex COMPFLAGS="$COMPFLAGS /TP /MT" -I"..\..\..\extern\mex\include\gsl-1.14" -I"..\..\mex\include" "..\..\..\extern\mex\lib\gsl.lib" "..\..\..\extern\mex\lib\cblas.lib" -output steerableDetector3D steerableDetector3D.cpp
 */


#include <cstring>
#include <gsl_include/gsl_poly.h>
#include <gsl_include/gsl_math.h>
#include <gsl_include/gsl_eigen.h>
//#include <gsl_include/gsl_cblas.h>

#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <iomanip>



#include <fstream>
#include <string>
#include <math.h>
#include "stackutil.h"

#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"


#include "convolver3D.h"
#include "steerableDetector3D.h"

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif


#define PI 3.141592653589793

Filter::Filter(const double voxels[], const int nx, const int ny, const int nz, const int M, const double sigma) {
    voxels_ = voxels;
    nx_ = nx;
    ny_ = ny;
    nz_ = nz;
    M_ = M;
    sigma_ = sigma;
    sigmaZ_ = sigma;
    std::cout<<"core: nx = "<<nx_<<std::endl;
    std::cout<<"core: ny = "<<ny_<<std::endl;
    std::cout<<"core: nz = "<<nz_<<std::endl;
    std::cout<<"core: M = "<<M_<<std::endl;
    std::cout<<"core: sigma = "<<sigma_<<std::endl;
    std::cout<<"core: sigmaZ = "<<sigmaZ_<<std::endl;
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
    std::cout<<"core: nx = "<<nx_<<std::endl;
    std::cout<<"core: ny = "<<ny_<<std::endl;
    std::cout<<"core: nz = "<<nz_<<std::endl;
    std::cout<<"core: M = "<<M_<<std::endl;
    std::cout<<"core: sigma = "<<sigma_<<std::endl;
    std::cout<<"core: sigmaZ = "<<sigmaZ_<<std::endl;
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
    std::cout<<"filter::init, start"<<std::endl;
   

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
  
    std::cout<<"filter::init, output initialization done"<<std::endl;
   

    calculateTemplates();
    run();
    
    nms_ = new double[N_];
    double * pointer_nms=nms_;

    for(int index = 0; index< N_; index++){
        *(pointer_nms++) =0;  
    }

    if (M_==1) {
        computeCurveNMS();
    } else if (M_==2) {
        computeSurfaceNMS();
    }    
}


Filter::~Filter() {
    /*
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
    */
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
    std::cout<<"filter::calculateTemplates, start"<<std::endl;
   


    // Compute Gaussian kernels
    double *kernelG = new double[kLength];
    double *kernelGx = new double[kLength];
    double *kernelGxx = new double[kLength];
    
    double *kernelG_z = new double[kLengthZ];
    double *kernelGx_z = new double[kLengthZ];
    double *kernelGxx_z = new double[kLengthZ];
    std::cout<<"filter::calculateTemplates, gaussian kernels initialization"<<std::endl;
   

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
    std::cout<<"filter::calculateTemplates, gaussian kernels ready"<<std::endl;
   

    // Convolve all along x
    convolveEvenX(voxels_, kernelG, kLength, nx_, ny_, nz_, gyy_);
    std::cout<<"filter::calculateTemplates, convolveEvenX"<<std::endl;
    std::cout<<"N_="<<N_<<std::endl;
        
    double * extIter = gyz_;
    for(int index = 0; index< N_; index++){
        *(extIter++) = gyy_[index];   
        //std::cout<<"copy gyy to gyz, index="<<index<<std::endl;
             
    }
    
    double * extIter2 = gzz_;
    for(int index = 0; index< N_; index++){
        *(extIter2++) = gyy_[index];    
        //std::cout<<"copy gyy to gzz, index="<<index<<std::endl;
                 
    }
     
    std::cout<<"filter::calculateTemplates, gyz,gzz initialized"<<std::endl;
    
   
    convolveOddX(voxels_, kernelGx, kLength, nx_, ny_, nz_, gxy_);
    std::cout<<"filter::calculateTemplates, convolveOddX"<<std::endl;
    
    
    double * extIter3 = gxz_;
    for(int index = 0; index< N_; index++){
        *(extIter3++) = gxy_[index];           
    }
    
    //memcpy(gxz_, gxy_, N_*sizeof(double));
    
    std::cout<<"filter::calculateTemplates, gxz initialized"<<std::endl;
    
    
    convolveEvenX(voxels_, kernelGxx, kLength, nx_, ny_, nz_, gxx_);
    std::cout<<"filter::calculateTemplates, convolveEvenX"<<std::endl;
    
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
    std::cout<<"filter::calculateTemplates, convolve many"<<std::endl;
    
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
    std::cout<<"filter::run, start"<<std::endl;
    
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
        //std::cout<<"filter::run, matrix for eigen"<<i<<std::endl;
    
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << a;
        Cov_Matrix.Row(2) << d << b;
        Cov_Matrix.Row(3) << e << f <<c;

        DiagonalMatrix eigenvalues_matrix;
        Matrix eigenvectors_matrix;
        //std::cout<<"filter::run, Cov_Matrix:"<<std::endl;
        //std::cout<< std::setw(10) << std::setprecision(7) <<Cov_Matrix<<std::endl;
        
        EigenValues(Cov_Matrix,eigenvalues_matrix,eigenvectors_matrix);
        //std::cout<<"filter::run, eigenvalues_matrix:"<<std::endl;
        //std::cout<< std::setw(10) << std::setprecision(7) <<eigenvalues_matrix<<std::endl;
        //std::cout<<"filter::run, eigenvectors_matrix:"<<std::endl;
        //std::cout<< std::setw(10) << std::setprecision(7) <<eigenvectors_matrix<<std::endl;
        
        double a1 = eigenvalues_matrix(1), a2 = eigenvalues_matrix(2), a3 = eigenvalues_matrix(3);
        //std::cout<< "a1:"<<a1<<", a2:"<<a2<<", a3:"<<a3<<std::endl;
        int largest_ind=1;
        largest_ind = swapthree(a1, a2, a3);
                
        response_[i] = a1;
        //std::cout<< "response_[i]:"<<response_[i]<<std::endl;
        
        orientation_[i][0] = eigenvectors_matrix(largest_ind,3);
        orientation_[i][1] = eigenvectors_matrix(largest_ind,3);
        orientation_[i][2] = eigenvectors_matrix(largest_ind,3);
        //std::cout<<"filter::run, orientation_[i][0]="<<orientation_[i][0]<<", [i][1]="<<orientation_[i][1]<<", [i][2]="<<orientation_[i][2]<<std::endl;
    
    }
}

double zhi_abs(double num)
{
    return (num<0) ? -num : num;
}


int swapthree(double& dummya, double& dummyb, double& dummyc)
{
    int largest_ind=1;
    if ( (zhi_abs(dummya) >= zhi_abs(dummyb)) && (zhi_abs(dummyb) >= zhi_abs(dummyc)) )

    {
    }
    else if ( (zhi_abs(dummya) >= zhi_abs(dummyc)) && (zhi_abs(dummyc) >= zhi_abs(dummyb)) )
    {
        double temp = dummyb;
        dummyb = dummyc;
        dummyc = temp;
    }
    else if ( (zhi_abs(dummyb) >= zhi_abs(dummya)) && (zhi_abs(dummya) >= zhi_abs(dummyc)) )
    {
        largest_ind=2;
        double temp = dummya;
        dummya = dummyb;
        dummyb = temp;
    }
    else if ( (zhi_abs(dummyb) >= zhi_abs(dummyc)) && (zhi_abs(dummyc) >= zhi_abs(dummya)) )
    {
        largest_ind=2;
        double temp = dummya;
        dummya = dummyb;
        dummyb = dummyc;
        dummyc = temp;
    }
    else if ( (zhi_abs(dummyc) >= zhi_abs(dummya)) && (zhi_abs(dummya) >= zhi_abs(dummyb)) )
    {
        largest_ind=3;
        double temp = dummya;
        dummya = dummyc;
        dummyc = dummyb;
        dummyb = temp;
    }
    else if ( (zhi_abs(dummyc) >= zhi_abs(dummyb)) && (zhi_abs(dummyb) >= zhi_abs(dummya)) )
    {
        largest_ind=3;
        double temp = dummyc;
        dummyc = dummya;
        dummya = temp;
    }
    else
    {
        return largest_ind;
    }

    return largest_ind;
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
