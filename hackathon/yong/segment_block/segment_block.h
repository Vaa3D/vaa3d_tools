#ifndef SEGMENT_BLOCK_H
#define SEGMENT_BLOCK_H

#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "v3d_interface.h"
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"

struct PARA
{
    unsigned char* data1d;
    V3DLONG im_cropped_sz[4];
    double original_o[3];
    QString img_name,swc_name;

};

bool segment_block(V3DPluginCallback2 &callback,vector<Segment*> &seg_list,PARA &PA);

#endif // SEGMENT_BLOCK_H
