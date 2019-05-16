#include <v3d_interface.h>
#include <app2/my_surf_objs.h>
enum tracingMethod {app1, app2, neutube,snake,most,mst, neurogpstree,rivulet2,tremap,gd,advantra,neuronchaser};


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
    int soma;
    int resume;


    V3DLONG in_sz[3];

    tracingMethod  method;

    int  visible_thresh;//for APP1 use only

    int  seed_win; //for MOST use only
    int  slip_win; //for MOST use only

    Image4DSimple* image;
    LandmarkList listLandmarks;
    QString tcfilename,inimg_file,rawfilename,markerfilename,swcfilename,inimg_file_2nd,output_folder;


};

 bool crawler_raw_app( V3DPluginCallback2 &callback, QWidget *parent, TRACE_LS_PARA &P, bool bmenu,NeuronTree swc);
 bool app_tracing_ada_win_3D(V3DPluginCallback2 &callback,TRACE_LS_PARA &P,LandmarkList inputRootList, LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,NeuronTree swc);
 QList<LandmarkList> group_tips(LandmarkList tips,int block_size, int direction);
 bool ada_win_finding_3D(LandmarkList tips,LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,Image4DSimple* total4DImage,int block_size,int direction);
 void processSmartScan_3D(V3DPluginCallback2 &callback, list<string> & infostring, QString fileWithData);
 NeuronTree smartPrune(NeuronTree nt, double length);
 void smartFuse(V3DPluginCallback2 &callback, QString inputFolder, QString fileSaveName);


