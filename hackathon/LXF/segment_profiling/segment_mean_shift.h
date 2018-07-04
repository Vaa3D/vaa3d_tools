#ifndef SEGMENT_MEAN_SHIFT_H
#define SEGMENT_MEAN_SHIFT_H
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_dialog.h"
#include "segment_profiling_main.h"
bool segment_mean_shift(unsigned char* &data1d,LandmarkList &LList,V3DLONG im_cropped_sz[4],int i,vector<MyMarker*> &nt_marker2);
LandmarkList segment_mean_shift_v2(V3DPluginCallback2 &callback,LandmarkList &LList,PARA &PA,int i,QList <NeuronSWC> &nt_marker2);
#endif // SEGMENT_MEAN_SHIFT_H
