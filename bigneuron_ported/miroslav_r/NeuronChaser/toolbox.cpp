/*
Copyright (C) Erasmus MC. Permission to use this software and corresponding documentation for educational, research, and not-for-profit purposes, without a fee and without a signed licensing agreement, is granted, subject to the following terms and conditions.

IT IS NOT ALLOWED TO REDISTRIBUTE, SELL, OR LEASE THIS SOFTWARE, OR DERIVATIVE WORKS THEREOF, WITHOUT PERMISSION IN WRITING FROM THE COPYRIGHT HOLDER. THE COPYRIGHT HOLDER IS FREE TO MAKE VERSIONS OF THE SOFTWARE AVAILABLE FOR A FEE OR COMMERCIALLY ONLY.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OF ANY KIND WHATSOEVER, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND CORRESPONDING DOCUMENTATION IS PROVIDED "AS IS". THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 * 2015-5-31 : by Miroslav Radojevic
 */

#include "toolbox.h"
#include <math.h>
#include "v3d_message.h"
#include <fstream>
#include <float.h>
#include <iostream>

using namespace std;

unsigned char quantile(unsigned char *a, int a_len, int ratioNum, int ratioDen) {

    int n = a_len; // a.length;
    int i, j, l, m, k;
    double x;

    if (ratioNum>=ratioDen) k = n-1;
    else k = floor(n * ((float)ratioNum/(float)ratioDen));
//    else if ((ratioNum*n) % ratioDen == 0) k = ((ratioNum*n)/ratioDen)-1;
//    else k = (ratioNum*n)/ratioDen;

//    v3d_msg(QString("k=%1\t").arg(k), 0);

    l=0 ; m=n-1;
    while (l < m) {
        x=a[k];
        i = l;
        j = m;
        do {
            while (a[i] < x) i++ ;
            while (x < a[j]) j-- ;
            if (i <= j) {
                float temp = a[i];
                a[i] = a[j];
                a[j] = temp;
                i++ ; j-- ;
            }
        } while (i <= j);
        if (j < k) l = i;
        if (k < i) m = j;
    }

    return a[k];

}

float zncc(float *v, int v_len, float v_avg, float *tmplt_hat, float tmplt_hat_sum_sqr) {

    float num = 0;
    float den = 0;

    for (int i = 0; i < v_len; i++) {
        num += (v[i] - v_avg) * tmplt_hat[i];
        den += pow(v[i] - v_avg, 2);
    }

    return (float) (num / sqrt(den * tmplt_hat_sum_sqr));

}

void descending(float * a, int a_len, int * idx) {

//    int[] idx = new int[a.length];
    for (int i=0; i<a_len; i++) idx[i] = i;

    for (int i = 0; i < a_len-1; i++) {
        for (int j = i+1; j < a_len; j++) {
            if (a[j]>a[i]) { // desc.
                float temp 	= a[i];
                a[i]		= a[j];
                a[j] 		= temp;

                int temp_idx 	= idx[i];
                idx[i] 			= idx[j];
                idx[j]			= temp_idx;
            }
        }
    }

//    return idx;

}

void probability_distribution(float * a, int a_len) {

    float sum = 0;
    for (int i = 0; i < a_len; i++) {
        sum += a[i];
    }

    if (sum<=0.00001) {
//        printf("DIDIT");
        for (int i = 0; i < a_len; i++) {
            a[i] = 1.0 / a_len; // they are all equal weight input has all zeros
        }
    }
    else {
//        printf("DIDITOK");
        for (int i = 0; i < a_len; i++) {
            a[i] /= sum;
        }
    }
}

