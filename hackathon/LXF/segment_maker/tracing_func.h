/* sort_func.h
 * 2012-02-01 : by Yinan Wan
 */
 
#ifndef __TRACING_FUNC_H__
#define __TRACING_FUNC_H__

#include <v3d_interface.h>
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
enum tracingMethod {app1, app2, neutube,snake,most,mst, neurogpstree,rivulet2,tremap,gd,advantra,neuronchaser};
bool match_marker(V3DPluginCallback2 &callback,vector<int> &ind,LandmarkList &terafly_landmarks,LocationSimple &t);
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
    int tracing_3D;
    int tracing_comb;
    int grid_trace;
    int global_name;
    double o_x,o_y,o_z;
    double ratio_x,ratio_y,ratio_z;


    V3DLONG in_sz[3];

    int  visible_thresh;//for APP1 use only

    int  seed_win; //for MOST use only
    int  slip_win; //for MOST use only

    tracingMethod  method;

    Image4DSimple* image;
    LandmarkList listLandmarks;
    QString tcfilename,inimg_file,rawfilename,markerfilename,swcfilename,inimg_file_2nd;
};

bool crawler_raw_app(V3DPluginCallback2 &callback, QWidget *parent,TRACE_LS_PARA &p,bool bmenu);

bool grid_raw_all(V3DPluginCallback2 &callback, QWidget *parent,TRACE_LS_PARA &p,bool bmenu);
bool all_tracing_grid(V3DPluginCallback2 &callback,TRACE_LS_PARA &P,V3DLONG ix, V3DLONG iy,V3DLONG iz);


#endif

