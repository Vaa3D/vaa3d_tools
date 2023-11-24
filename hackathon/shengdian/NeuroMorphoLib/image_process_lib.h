#ifndef IMAGE_PROCESS_LIB_H
#define IMAGE_PROCESS_LIB_H
#include "../../SGuo/imPreProcess/helpFunc.h"
#include "../../SGuo/imPreProcess/bilateral_filter.h"

bool enhanceImage(unsigned char * & data1d,unsigned char * & dst,V3DLONG *mysz,bool biilateral_filter=false);
void adaptiveThresholding(unsigned char * & data1d,unsigned char * & dst,V3DLONG *mysz);

#endif // IMAGE_PROCESS_LIB_H
