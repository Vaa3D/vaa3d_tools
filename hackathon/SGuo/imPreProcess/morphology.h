#ifndef MINMAXFILTER_H
#define MINMAXFILTER_H

#include <vector>
#include "v3d_basicdatatype.h"
#include "v3d_message.h"
#include "helpFunc.h"

using namespace std;

void grey_erosion(unsigned char* data1d,
                 V3DLONG *in_sz,
                 unsigned int Wx,
                 unsigned int Wy,
                 unsigned int c,
                 unsigned char* &outimg);

void grey_dilation(unsigned char* data1d,
                 V3DLONG *in_sz,
                 unsigned int Wx,
                 unsigned int Wy,
                 unsigned int c,
                 unsigned char* &outimg);

void grey_tophat(unsigned char* data1d,
                 V3DLONG *in_sz,
                 unsigned int Wx,
                 unsigned int Wy,
                 unsigned int c,
                 unsigned char* &outimg);

#endif // MINMAXFILTER_H
