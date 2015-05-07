#ifndef __TOOLBOX__
#define __TOOLBOX__

unsigned char quantile(unsigned char *a, int a_len, int ratioNum, int ratioDen);

float zncc(float *v, int v_len, float v_avg, float *tmplt_hat, float tmplt_hat_sum_sqr);

void descending(float * a, int a_len, int * idx);

#endif
