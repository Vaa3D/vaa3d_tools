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


// Modified by Liya Ding 2019.05.15

#include <cstring> //memcpy
#include <stdlib.h>
#include <iostream>

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
