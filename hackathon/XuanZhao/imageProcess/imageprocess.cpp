#include "imageprocess.h"

#include <math.h>


void enhanceImage(unsigned char* data1d, V3DLONG* sz){
    V3DLONG totalSz = sz[0]*sz[1]*sz[2];

    for(int i=0; i<totalSz; i++){
        data1d[i] = sqrt(data1d[i]/255.0)*255;
    }
}
