#include "toolbox.h"
#include <math.h>

unsigned char quantile(unsigned char *a, int a_len, int ratioNum, int ratioDen) { // V3DLONG in_sz[4], double dfactor_xy, double dfactor_z, unsigned char *&outimg, V3DLONG out_sz[4]

    int n = a_len;//a.length;
    int i, j, l, m, k;
    double x;

    if (ratioNum>=ratioDen) {k = n-1;}
    else if ((ratioNum*n) % ratioDen == 0) k = ((ratioNum*n)/ratioDen)-1;
    else k = (ratioNum*n)/ratioDen;

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
