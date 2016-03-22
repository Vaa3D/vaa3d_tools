/* sort_func.h
 * 2012-02-01 : by Yinan Wan
 */
 
#ifndef __TRACING_FUNC_H__
#define __TRACING_FUNC_H__

#include <v3d_interface.h>
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"


struct TRACE_LS_PARA
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
    int adap_win;
    V3DLONG in_sz[3];

    int  visible_thresh;//for APP1 use only

    int  seed_win; //for MOST use only
    int  slip_win; //for MOST use only
    int  method; //1:app1, 2:app2, 3: neutube, 4:snake, 5:most.

    Image4DSimple* image;
    LandmarkList listLandmarks;
    QString tcfilename,inimg_file,rawfilename,markerfilename;
};


bool crawler_raw_app(V3DPluginCallback2 &callback, QWidget *parent,TRACE_LS_PARA &p,bool bmenu);
bool app_tracing(V3DPluginCallback2 &callback,TRACE_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList);

bool app_tracing_ada_win(V3DPluginCallback2 &callback,TRACE_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList);

bool crawler_raw_all(V3DPluginCallback2 &callback, QWidget *parent,TRACE_LS_PARA &p,bool bmenu);
bool all_tracing(V3DPluginCallback2 &callback,TRACE_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList);

bool all_tracing_ada_win(V3DPluginCallback2 &callback,TRACE_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList);
bool ada_win_finding(LandmarkList tips,LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,Image4DSimple* total4DImage,int block_size,int direction);

QList<LandmarkList> group_tips(LandmarkList tips,int block_size, int direction);
NeuronTree sort_eliminate_swc(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage);
LandmarkList eliminate_seed(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage);
bool combine_list2file(QList<NeuronSWC> & lN, QString fileSaveName);

void processSmartScan(V3DPluginCallback2 &callback,list<string> & infostring,QString fileWithData);


#endif

