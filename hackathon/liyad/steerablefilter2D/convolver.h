/* Class for 2-D convolutions with even- or odd-symmetric kernels
 * Supported border conditions: mirror, periodic, replicate of border pixels, or zeros
 *
 * Copyright (C) 2012 Francois Aguet
 * 
 * Last modified: Mar 15, 2012
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


#ifndef CONVOLVER_H
#define CONVOLVER_H

#include <cstring> //memcpy

namespace std {

class Convolver {
    
public:
    // define type for border condition function pointers    
    typedef double (Convolver::*BorderConditionFunc)(const int idx, const int i, const int y);
    
    // Border conditions
    static const int ZEROS = 0;
    static const int REPLICATE = 1;
    static const int PERIODIC = 2;
    static const int MIRROR = 3;
    
    Convolver(const double pixels[], const int nx, const int ny);
    Convolver(const double pixels[], const int nx, const int ny, const int borderCondition);
    
    ~Convolver();
    
    void convolveEvenXEvenY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]);
    void convolveEvenXOddY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]);
    void convolveOddXEvenY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]);
    void convolveOddXOddY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]);
    
    int nx_, ny_;
    double *pixels_;
    double *buffer_;
    int borderCondition_;
    
 protected:
    // Border condition function pointers
    BorderConditionFunc leftBCFunc_;
    BorderConditionFunc rightBCFunc_;
    BorderConditionFunc topBCFunc_;
    BorderConditionFunc bottomBCFunc_;
    
    
private:
    // x,y-convolution methods
    void convolveEvenX(const double kernel[], const int k);
    void convolveEvenY(const double kernel[], const int k, double output[]);
    void convolveOddX(const double kernel[], const int k);
    void convolveOddY(const double kernel[], const int k, double output[]);
    
    
    // Border condition methods
    double leftBorderMirror(const int idx, const int i, const int y);
    double rightBorderMirror(const int idx, const int i, const int y);
    double topBorderMirror(const int idx, const int i, const int y);
    double bottomBorderMirror(const int idx, const int i, const int y);
    
    double leftBorderPeriodic(const int idx, const int i, const int y);
    double rightBorderPeriodic(const int idx, const int i, const int y);
    double topBorderPeriodic(const int idx, const int i, const int y);
    double bottomBorderPeriodic(const int idx, const int i, const int y);
    
    double leftBorderReplicate(const int idx, const int i, const int y);
    double rightBorderReplicate(const int idx, const int i, const int y);
    double topBorderReplicate(const int idx, const int i, const int y);
    double bottomBorderReplicate(const int idx, const int i, const int y);
    
    double borderZeros(const int idx, const int i, const int y);
    
};
    
    Convolver::Convolver(const double pixels[], const int nx, const int ny) {
        nx_ = nx;
        ny_ = ny;
        pixels_ = new double[nx_*ny_];
        memcpy(pixels_, pixels, nx_*ny_*sizeof(double));
        buffer_ = new double[nx_*ny_];
        
        borderCondition_ = MIRROR;
        
        leftBCFunc_ = &Convolver::leftBorderMirror;
        rightBCFunc_ = &Convolver::rightBorderMirror;
        topBCFunc_ = &Convolver::topBorderMirror;
        bottomBCFunc_ = &Convolver::bottomBorderMirror;
    }
    
    Convolver::Convolver(const double pixels[], const int nx, const int ny, const int borderCondition) {
        nx_ = nx;
        ny_ = ny;
        pixels_ = new double[nx_*ny_];
        memcpy(pixels_, pixels, nx_*ny_*sizeof(double));
        buffer_ = new double[nx_*ny_];
        
        borderCondition_ = borderCondition;
        
        switch (borderCondition) {
            case ZEROS:
                leftBCFunc_ = &Convolver::borderZeros;
                rightBCFunc_ = &Convolver::borderZeros;
                topBCFunc_ = &Convolver::borderZeros;
                bottomBCFunc_ = &Convolver::borderZeros;
                break;
            case REPLICATE:
                leftBCFunc_ = &Convolver::leftBorderReplicate;
                rightBCFunc_ = &Convolver::rightBorderReplicate;
                topBCFunc_ = &Convolver::topBorderReplicate;
                bottomBCFunc_ = &Convolver::bottomBorderReplicate;
                break;
            case PERIODIC:
                leftBCFunc_ = &Convolver::leftBorderPeriodic;
                rightBCFunc_ = &Convolver::rightBorderPeriodic;
                topBCFunc_ = &Convolver::topBorderPeriodic;
                bottomBCFunc_ = &Convolver::bottomBorderPeriodic;
                break;
            case MIRROR:
                leftBCFunc_ = &Convolver::leftBorderMirror;
                rightBCFunc_ = &Convolver::rightBorderMirror;
                topBCFunc_ = &Convolver::topBorderMirror;
                bottomBCFunc_ = &Convolver::bottomBorderMirror;
                break;
            default:
                break;
        }
    }
    
    Convolver::~Convolver() {
        delete pixels_;
        delete buffer_;
    }


    void Convolver::convolveEvenXEvenY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]) {
        
        convolveEvenX(xkernel, nx);
        convolveEvenY(ykernel, ny, output);
    }
    
    void Convolver::convolveEvenXOddY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]) {
        
        convolveEvenX(xkernel, nx);
        convolveOddY(ykernel, ny, output);
    }
    
    void Convolver::convolveOddXEvenY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]) {
        
        convolveOddX(xkernel, nx);
        convolveEvenY(ykernel, ny, output);
    }
    
    void Convolver::convolveOddXOddY(const double xkernel[], const int nx, const double ykernel[], const int ny, double output[]) {
        
        convolveOddX(xkernel, nx);
        convolveOddY(ykernel, ny, output);
    }
    
    
    
    
    // Convolution along x goes to buffer, along y to output
    void Convolver::convolveEvenX(const double kernel[], const int k) {
        
        int k_1 = k-1;
        
        int idx = 0;
        for (int y=0;y<ny_;++y) {
            for (int x=0;x<k_1;++x) {
                buffer_[idx] = kernel[0]*pixels_[idx];
                for (int i=1;i<=x;++i) {
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]+pixels_[idx+i]);
                }
                for (int i=x+1;i<k;++i) {
                    buffer_[idx] += kernel[i]*((this->*leftBCFunc_)(idx, i, y)+pixels_[idx+i]);
                }
                idx++;
            }
            for (int x=k_1;x<=nx_-k;++x) {
                buffer_[idx] = kernel[0]*pixels_[idx];
                for (int i=1;i<k;++i) {
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]+pixels_[idx+i]);
                }
                idx++;
            }
            for (int x=nx_-k_1;x<nx_;++x) {
                buffer_[idx] = kernel[0]*pixels_[idx];
                for (int i=1;i<nx_-x;++i) {
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]+pixels_[idx+i]);
                }
                for (int i=nx_-x;i<k;++i) {
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]+(this->*rightBCFunc_)(idx, i, y));
                }
                idx++;
            }
        }
    }
    
    
    void Convolver::convolveEvenY(const double *kernel, const int k, double output[]) {
        
        int k_1 = k-1;
        
        int idx, inx;
        for (int x=0;x<nx_;x++) {
            for (int y=0;y<k_1;y++) {
                idx = x+y*nx_;
                output[idx] = kernel[0]*buffer_[idx];
                for (int i=1;i<=y;i++) {
                    inx = i*nx_;
                    output[idx] += kernel[i]*(buffer_[idx-inx]+buffer_[idx+inx]);
                }
                for (int i=y+1;i<k;i++) {
                    output[idx] += kernel[i]*((this->*topBCFunc_)(idx, i, y)+buffer_[idx+i*nx_]);
                }
            }
            for (int y=k_1;y<=ny_-k;y++) {
                idx = x+y*nx_;
                output[idx] = kernel[0]*buffer_[idx];
                for (int i=1;i<k;i++) {
                    inx = i*nx_;
                    output[idx] += kernel[i]*(buffer_[idx-inx]+buffer_[idx+inx]);
                }
            }
            for (int y=ny_-k_1;y<ny_;y++) {
                idx = x+y*nx_;
                output[idx] = kernel[0]*buffer_[idx];
                for (int i=1;i<ny_-y;i++) {
                    inx = i*nx_;
                    output[idx] += kernel[i]*(buffer_[idx-inx]+buffer_[idx+inx]);
                }
                for (int i=ny_-y;i<k;i++) {
                    output[idx] += kernel[i]*(buffer_[idx-i*nx_]+(this->*bottomBCFunc_)(idx, i, y));
                }
            }
        }
    }
    
    
    void Convolver::convolveOddX(const double *kernel, const int k) {
        
        int k_1 = k-1;
        
        int idx = 0;
        for (int y=0;y<ny_;y++) {
            for (int x=0;x<k_1;x++) {
                buffer_[idx] = 0.0; // kernel is symmetric, zero at center
                for (int i=1;i<=x;i++) {
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]-pixels_[idx+i]);
                }
                for (int i=x+1;i<k;i++) {
                    buffer_[idx] += kernel[i]*((this->*leftBCFunc_)(idx, i, y)-pixels_[idx+i]);
                }
                idx++;
            }
            for (int x=k_1;x<=nx_-k;x++) {
                buffer_[idx] = 0.0;
                for (int i=1;i<k;i++) { // value at 0 is 0
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]-pixels_[idx+i]); // conv -> flip
                }
                idx++;
            }
            for (int x=nx_-k_1;x<nx_;x++) {
                buffer_[idx] = 0.0;
                for (int i=1;i<nx_-x;i++) {
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]-pixels_[idx+i]);
                }
                for (int i=nx_-x;i<k;i++) {
                    buffer_[idx] += kernel[i]*(pixels_[idx-i]-(this->*rightBCFunc_)(idx, i, y));
                }
                idx++;
            }
        }
    }
    
    
    void Convolver::convolveOddY(const double *kernel, const int k, double output[]) {
        
        int k_1 = k-1;
        
        int idx, inx;
        for (int x=0;x<nx_;x++) {
            for (int y=0;y<k_1;y++) {
                idx = x+y*nx_;
                output[idx] = 0.0;
                for (int i=1;i<=y;i++) {
                    inx = i*nx_;
                    output[idx] += kernel[i]*(buffer_[idx-inx]-buffer_[idx+inx]);
                }
                for (int i=y+1;i<k;i++) {
                    output[idx] += kernel[i]*((this->*topBCFunc_)(idx, i, y)-buffer_[idx+i*nx_]);
                }
            }
            for (int y=k_1;y<=ny_-k;y++) {
                idx = x+y*nx_;
                output[idx] = 0.0;
                for (int i=1;i<k;i++) {
                    inx = i*nx_;
                    output[idx] += kernel[i]*(buffer_[idx-inx]-buffer_[idx+inx]);
                }
            }
            for (int y=ny_-k_1;y<ny_;y++) {
                idx = x+y*nx_;
                output[idx] = 0.0;
                for (int i=1;i<ny_-y;i++) {
                    inx = i*nx_;
                    output[idx] += kernel[i]*(buffer_[idx-inx]-buffer_[idx+inx]);
                }
                for (int i=ny_-y;i<k;i++) {
                    output[idx] += kernel[i]*(buffer_[idx-i*nx_]-(this->*bottomBCFunc_)(idx, i, y));
                }
            }
        }
    }

    
    
    // Mirror border condition functions
    double Convolver::leftBorderMirror(const int idx, const int i, const int y) {
        return pixels_[i-idx+2*y*nx_];
    }
    double Convolver::rightBorderMirror(const int idx, const int i, const int y) {
        return pixels_[2*(nx_*(y+1)-1)-idx-i]; // right over: idx+i. diff: idx+i-(nx-1)
    }
    double Convolver::topBorderMirror(const int idx, const int i, const int y) {
        return buffer_[idx+(i-2*y)*nx_]; //x+y*nx - i*nx -> x+(i-y)*nx
    }
    double Convolver::bottomBorderMirror(const int idx, const int i, const int y) {
        return buffer_[(2*(ny_-y-1)-i)*nx_+idx];
    }
    
    // Periodic border condition functions
    double Convolver::leftBorderPeriodic(const int idx, const int i, const int y) {
        return pixels_[nx_+idx-i];
    }
    double Convolver::rightBorderPeriodic(const int idx, const int i, const int y) {
        return pixels_[idx+i-nx_];
    }
    double Convolver::topBorderPeriodic(const int idx, const int i, const int y) {
        return buffer_[nx_*(ny_-i)-1+idx]; // idx-i*nx -> nx*ny-1 + idx-i*nx 
    }
    double Convolver::bottomBorderPeriodic(const int idx, const int i, const int y) {
        return buffer_[idx+(i-ny_)*nx_+1]; // idx+i*nx -> idx+i*nx - (nx*ny-1)
    }
    
    // Replicate border condition functions
    double Convolver::leftBorderReplicate(const int idx, const int i, const int y) {
        return pixels_[y*nx_];
    }
    double Convolver::rightBorderReplicate(const int idx, const int i, const int y) {
        return pixels_[nx_-1+y*nx_];
    }
    double Convolver::topBorderReplicate(const int idx, const int i, const int y) {
        return buffer_[idx-y*nx_]; // = x
    }
    double Convolver::bottomBorderReplicate(const int idx, const int i, const int y) {
        return buffer_[(ny_-y-1)*nx_+idx]; // = (ny-1)*nx + x
    }
    
    // Zero border conditions
    double Convolver::borderZeros(const int idx, const int i, const int y) {
        return 0.0;
    }    
    
} // namespace std
#endif // CONVOLVER_H
