#ifndef GET_SUB_BLOCK_H
#define GET_SUB_BLOCK_H
#include "segment_profiling_main.h"
bool get_sub_block(V3DPluginCallback2 &callback,int model,vector<MyMarker*> &seg_m,unsigned char * &data1d,V3DLONG im_cropped_sz[4],int n,double original_o[3]);
#endif // GET_SUB_BLOCK_H
