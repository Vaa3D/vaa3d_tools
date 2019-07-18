#ifndef SEGMENT_BLOCK_H
#define SEGMENT_BLOCK_H

#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "v3d_interface.h"

struct Point;
struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
};
typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

struct PARA
{
    unsigned char* data1d;
    V3DLONG im_cropped_sz[4];
    double original_o[3];
    QString img_name,swc_name;

};

bool segment_block(V3DPluginCallback2 &callback,vector<Segment*> &seg_list,PARA &PA);

#endif // SEGMENT_BLOCK_H
