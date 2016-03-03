/* sort_func.h
 * 2012-02-01 : by Yinan Wan
 */
 
#ifndef __TRACING_FUNC_H__
#define __TRACING_FUNC_H__

#include <v3d_interface.h>
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"


struct APP_LS_PARA
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
    bool adap_win;
    V3DLONG in_sz[3];

    int  visible_thresh;//for APP1 use only

    Image4DSimple* image;
    LandmarkList listLandmarks;
    QString tcfilename,inimg_file,rawfilename,markerfilename;
};

struct ALL_LS_PARA
{
    int  channel;
    int  bkg_thresh;
    int  seed_win;
    int  slip_win;

    int  block_size;
    V3DLONG in_sz[3];

    Image4DSimple* image;
    LandmarkList listLandmarks;
    QString tcfilename,inimg_file,rawfilename,markerfilename;
};

bool crawler_raw_app(V3DPluginCallback2 &callback, QWidget *parent,APP_LS_PARA &p,bool bmenu);
bool app_tracing(V3DPluginCallback2 &callback,APP_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList);

bool app_tracing_ada_win(V3DPluginCallback2 &callback,APP_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList);

bool crawler_raw_all(V3DPluginCallback2 &callback, QWidget *parent,ALL_LS_PARA &p,bool bmenu, int method);
bool all_tracing(V3DPluginCallback2 &callback,ALL_LS_PARA &p,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,int method);

NeuronTree sort_eliminate_swc(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage);
LandmarkList eliminate_seed(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage);
bool combine_list2file(QList<NeuronSWC> & lN, QString fileSaveName);

void processSmartScan(V3DPluginCallback2 &callback,list<string> & infostring,QString fileWithData);


#endif

