//histeq.h
//by Hanchuan Peng
//2006-2011

#ifndef __HISTOGRAM_EQ__
#define __HISTOGRAM_EQ__

#include "../../v3d_main/basic_c_fun/v3d_basicdatatype.h"
//#include "../../basic_c_fun/v3d_basicdatatype.h"

bool hist_eq_uint8(unsigned char * data1d, V3DLONG len); 
bool hist_eq_range_uint8(unsigned char * data1d, V3DLONG len, unsigned char lowerbound, unsigned char upperbound); 

#endif

