//by Hanchuan Peng
// 2012-Dec

#ifndef __VN_IMGPREPROCESS__
#define __VN_IMGPREPROCESS__

#include <v3d_interface.h>

#include "basic_memory.cpp"
#include "volimg_proc.h"


bool downsampling_img_xyz(unsigned char *inimg, V3DLONG in_sz[4], double dfactor_xy, double dfactor_z, unsigned char *&outimg, V3DLONG out_sz[4]);
void downsampling_marker_xyz(LandmarkList & listLandmarks, double dfactor_xy, double dfactor_z);
bool scaleintensity(Image4DSimple * p4dImage, int channo, double lower_th, double higher_th, double target_min, double target_max);
bool scale_img_and_convert28bit(Image4DSimple * p4dImage, int lb, int ub); //lb, ub: lower bound, upper bound

bool subvolumecopy(Image4DSimple * dstImg,
                   Image4DSimple * srcImg,
                   V3DLONG x0, V3DLONG szx,
                   V3DLONG y0, V3DLONG szy,
                   V3DLONG z0, V3DLONG szz,
                   V3DLONG c0, V3DLONG szc);

bool invertedsubvolumecopy(Image4DSimple * dstImg,
                   Image4DSimple * srcImg,
                   V3DLONG x0, V3DLONG szx,
                   V3DLONG y0, V3DLONG szy,
                   V3DLONG z0, V3DLONG szz,
                   V3DLONG c0, V3DLONG szc);

#endif

