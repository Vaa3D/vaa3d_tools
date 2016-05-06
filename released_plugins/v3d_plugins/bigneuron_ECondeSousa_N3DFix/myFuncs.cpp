/*
* N3DFix - automatic removal of swelling artifacts in neuronal 2D/3D reconstructions
* last update: Mar 2016
* VERSION 2.0
*
* Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>
*          Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>
* Date:    Mar 2016
*
* N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)
* Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016
*
*    Disclaimer
*    ----------
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You can view a copy of the GNU General Public License at
*    <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <iostream>
#include <vector>
#include <math.h>
#include "myHeader.h"
#include "Dend_Section.h"
#include "v3d_message.h"
#include <algorithm>




//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double MEDIAN(std::vector<double> DIAM) {
    double median;
    size_t size = DIAM.size();
    sort(DIAM.begin(), DIAM.end());
    if (size % 2 == 0)	{
        median = (DIAM[size / 2 - 1] + DIAM[size / 2]) / 2;
    }
    else {
        median = DIAM[size / 2];
    }

    return median;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double MEAN(std::vector<double> DIAM) {
    double sum = 0.0;
    for (unsigned int i = 0; i < DIAM.size(); i++) {
        sum += DIAM[i];
    }
    return sum / (1.0 * DIAM.size());
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double STDEV(std::vector<double> DIAM) {
    double temp = 0;
    double mean = MEAN(DIAM);
    for (unsigned int i = 0; i < DIAM.size(); i++) {
        temp += (mean - DIAM[i])*(mean - DIAM[i]);
    }
    return sqrt( temp / (1.0 * DIAM.size()) );
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double MINIMUM(std::vector<double> myvec){
    double up = myvec.at(0)+1;
    for(unsigned it = 0; it<myvec.size();it++){
        if(up > myvec.at(it)){
            up = myvec.at(it);
        }
    }
    return up;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double MAXIMUM(std::vector<double> myvec){
    double up = myvec.at(0)-1;
    for(unsigned it = 0; it<myvec.size();it++){
        if(up < myvec.at(it)){
            up = myvec.at(it);
        }
    }
    return up;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double SUM(std::vector<double> myvec){
    double s = 0;
    for(unsigned it = 0; it<myvec.size();it++){
        s = s + myvec.at(it);
    }
    return s;
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
double RandGen(double A, double B) {
    if (B < A){
        double temp=A;
        A=B;
        B=temp;
    }
    if (abs(B-A)<1e-6){
        return A;
    }else{
        double random = ((double) rand()) / (double) RAND_MAX;
        double diff = B - A;
        double R = random * diff;
        return A + R;
    }
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void Calculate_Baseline( std::vector<double> DIAM,std::vector<double> ARC, std::vector<double> &baseline, std::vector<double> &weights ){
    baseline.clear();
    weights.clear();


    unsigned int Npoints = ARC.size();
    baseline.resize(Npoints, 0.0);
    weights.resize(Npoints, 0.0);
    std::vector<double> r_mad (Npoints, 0.0);
    std::vector<double> DIAM_noise (Npoints, 0.0);

    double MIN_, MAX_;

    //REMOVE OUTLIERS USING MAD (Median Absolute Deviation)
    double outlier_cutoff = 3.0;
    double median_D, mad;

    median_D = MEDIAN(DIAM);
    for(unsigned i = 0; i < Npoints; i = i + 1) {
        r_mad[i] = abs( DIAM[i] - median_D );
    }
    mad = MEDIAN(r_mad);

    // use fitting weights to exclude the outliers
    if( mad > 1.0e-6 ) {
        for(unsigned i = 0; i < Npoints; i = i + 1 ) {
            r_mad[i] = abs( DIAM[i] - median_D ) / mad;
            if( r_mad[i] > outlier_cutoff ) {
                weights[i] = 0.0;
            } else {
                weights[i] = 1.0;
            }
        }
    } else {
        // will end up here if more than 50% of data points had the same value
        // add 1% noise to DIAM profile and recalculate mad
        std::vector<double> DIAM_noise = DIAM;
        MIN_ = MINIMUM(DIAM);
        for (unsigned i=0; i < Npoints; i = i + 1 ) {
            DIAM_noise[i] = DIAM[i] * 0.01 * MIN_ * RandGen((double)-1.0,(double)1.0);
        }
        median_D = MEDIAN(DIAM_noise);
        for(unsigned i = 0; i < Npoints; i = i + 1) {
            r_mad[i] = abs( DIAM_noise[i] - median_D );
        }
        mad = MEDIAN(r_mad);

        if( mad > 1.0e-6 ) {
            for(unsigned i = 0; i < Npoints; i = i + 1 ) {
                r_mad[i] = abs( DIAM_noise[i] - median_D ) / mad;
                if( r_mad[i] > outlier_cutoff ) {
                    weights[i] = 0.0;
                } else {
                    weights[i] = 1.0;
                }
            }
        }else{
            //this is tricky situation; try signalling outliers assuming Normal dist...
            double s = STDEV(DIAM);
            median_D = MEDIAN(DIAM);
            for(unsigned i = 0; i < Npoints; i = i + 1 ) {
                if( abs( DIAM[i] - median_D ) > 2.0 * s ) {
                    weights[i] = 0.0;
                } else {
                    weights[i] = 1.0;
                }
            }
        }
    }


    //PERFORM 2ND ORDER POLYNOMIAL FIT
    double a, b, c;
    std::vector<double> temp_mad;

    double w1 = 0.0;
    double wx = 0.0, wx2 = 0.0, wx3 = 0.0, wx4 = 0.0;
    double wy = 0.0, wyx = 0.0, wyx2 = 0.0;
    double tmpx = 0.0, tmpy = 0.0;
    double den;
    double x, y, w;

    if(DIAM.size()-SUM(weights)>2){
        for(unsigned i = 0; i < Npoints; i = i + 1 ) {
            x = ARC[i];
            y = DIAM[i];
            w = weights[i];
            w1 += w;
            tmpx = w * x;
            wx += tmpx;
            tmpx *= x;
            wx2 += tmpx;
            tmpx *= x;
            wx3 += tmpx;
            tmpx *= x;
            wx4 += tmpx;
            tmpy = w * y;
            wy += tmpy;
            tmpy *= x;
            wyx += tmpy;
            tmpy *= x;
            wyx2 += tmpy;
        }

        den = wx2 * wx2 * wx2 - 2.0 * wx3 * wx2 * wx + wx4 * wx * wx + wx3 * wx3 * w1 - wx4 * wx2 * w1;
        if( den == 0.0 ) {
            a = 0.0;
            b = 0.0;
            c = 0.0;
        } else {
            a = (wx * wx * wyx2 - wx2 * w1 * wyx2 - wx2 * wx * wyx + wx3 * w1 * wyx + wx2 * wx2 * wy - wx3 * wx * wy) / den;
            b = (-wx2 * wx * wyx2 + wx3 * w1 * wyx2 + wx2 * wx2 * wyx - wx4 * w1 * wyx - wx3 * wx2 * wy + wx4 * wx * wy) / den;
            c = (wx2 * wx2 * wyx2 - wx3 * wx * wyx2 - wx3 * wx2 * wyx + wx4 * wx * wyx + wx3 * wx3 * wy - wx4 * wx2 * wy) / den;
        }
        // FILL IN DATA
        double d;
        for(unsigned i = 0; i < Npoints; i = i + 1 ) {
            x = ARC[i];
            d = a * x * x + b * x + c;
            if( d > 1.0e-6 ) {
                baseline[i] = d;
            } else {
                baseline[i] = 1.0e-6;
            }
        }
    }else{
        median_D = MEDIAN(DIAM);
        for(unsigned i = 0; i < Npoints; i = i + 1 ) {
            baseline[i] = median_D;
        }
    }
    // Do not allow the baseline to go bellow the minimal diameter, or above maximal diameter - truncate
    MIN_ = MINIMUM(DIAM);
    MAX_ = MAXIMUM(DIAM);
    for(unsigned i = 0; i < Npoints; i = i + 1 ) {
        if (baseline[i]<MIN_){
            baseline[i] = MIN_;
        }
        if (baseline[i]>MAX_){
            baseline[i] = MAX_;
        }
    }
    return;
}

