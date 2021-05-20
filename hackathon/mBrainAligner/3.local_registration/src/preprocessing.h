#ifndef __PREPROCESSING_H__
#define __PREPROCESSING_H__



#include "until.h"
using namespace std;

#define WANT_STREAM


bool Calculate_gradient_img(float *&grand_sub, long long * & sz_img_sub, float * & p_img32f_sub255, unsigned char *p_img_sub);
bool Convert_image_datatype(float * & p_img32f_tar, float * & p_img32f_sub_bk, float * &grand_tar, float * &grand_sub, unsigned char *&p_img_sub, long long * & sz_img);
bool Convert_image255(float *&p_img32f_tar255, float * &p_img32f_sub255, unsigned char * & p_img_sub,
	unsigned char * & p_img_tar,  long long & l_npixels);

#endif
