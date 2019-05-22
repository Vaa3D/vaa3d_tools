#ifndef TIPDETECTOR_H
#define TIPDETECTOR_H
#include <v3d_interface.h>
#include"ray_shooting.h"
#include"neuron_tools.h"

class TipDetector:public QObject
{
private:
    Image4DSimple * p4DImage;
    V3DLONG sz[4];


    unsigned char * tip_img_datald;

    unsigned char *binar_datald;
    unsigned char *enlarged_datald;
    unsigned char *block;
    unsigned char *xy_mip_datald;
    unsigned char *yz_mip_datald;
    unsigned char *xz_mip_datald;
    vector<vector<float> > ray_x;
    vector<vector<float> > ray_y;
    LandmarkList curlist;
    LocationSimple s;
    int radiu_block;
    V3DLONG new_nx,new_ny,new_nz;
    int length_block;



    //deal swc
//    QList<NeuronTree> * mTreeList;
    bool isSavingTime;
    V3DPluginCallback2* cb;
public slots:
    void ClearTempData();


public:
    int number_of_rays ;
    int background_threshold ;
    float magnitude ;
    int minimum_scale;
    int number_of_scales;
    int step_size;
    int intensity_threshold;
    float angle_threshold;

    unsigned char *datald;
    //img data
    void cluster(LandmarkList &curlistToCluster);
    TipDetector();
    void help();
    float getPluginVersion();
    void getImgData(V3DPluginCallback2& callback2);
    bool GUI_input_argu();
    void whole_img(V3DPluginCallback2& callback);
    bool TipDetect_onePoint(LocationSimple CandiPoint);
    bool TipDetect_onePoint_27neighbor(LocationSimple CandiPoint,LocationSimple &can);
    ~TipDetector();
    LandmarkList OutputTipPoint()
    {
        return curlist;
    }
    void setTipList(LandmarkList tiplist)
    {curlist=tiplist;}
    LocationSimple Nearest_tip(LocationSimple point_a);
    void show_tipdetetor_img(V3DPluginCallback2& callback);
    bool TipDetect_onePoint_neighbor(LocationSimple CandiPoint,LocationSimple &can,unsigned int radius=1);
    bool TipDetect_onePoint_neighbor_no_z(LocationSimple CandiPoint,LocationSimple &can,unsigned int radius=1);
    bool refer_TipDetect_onePoint_(LocationSimple CandiPoint);

    bool save_tp_as_temp(V3DPluginCallback2& callback);

//    void FromNeuronTreeGetTipBranch(V3DPluginCallback2 &callback);
//    void PruningNeuronTree(V3DPluginCallback2 &callback);
    void GetNeuronTreeFrom3dview(V3DPluginCallback2 &callback);
    bool load_tp_temp(V3DPluginCallback2& callback);
    bool isForeground(LocationSimple CandiPoint);
};


#endif // TIPDETECTOR_H
