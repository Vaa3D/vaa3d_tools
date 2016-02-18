/* sort_func.h
 * 2012-02-01 : by Yinan Wan
 */
 
#ifndef __TRACING_FUNC_H__
#define __TRACING_FUNC_H__

#include <v3d_interface.h>

struct APP2_LS_PARA
{
    int is_gsdt;
    int is_break_accept;
    int  bkg_thresh;
    double length_thresh;
    int  cnn_type;
    int  channel;
    double SR_ratio;
    int  b_256cube;
    int b_RadiusFrom2D;
    int block_size;
    V3DLONG in_sz[3];


    QString tcfilename,inimg_file,rawfilename,markerfilename;
};

bool crawler_raw(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &p,bool bmenu);
bool app2_tracing(V3DPluginCallback2 &callback,APP2_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList);



#endif

