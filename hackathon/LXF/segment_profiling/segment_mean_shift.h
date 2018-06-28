#ifndef SEGMENT_MEAN_SHIFT_H
#define SEGMENT_MEAN_SHIFT_H
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
bool segment_mean_shift(unsigned char* &data1d,LandmarkList &LList,V3DLONG im_cropped_sz[4],int i,vector<MyMarker*> &nt_marker2);
#endif // SEGMENT_MEAN_SHIFT_H
