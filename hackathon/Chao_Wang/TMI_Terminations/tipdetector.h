#ifndef TIPDETECTOR_H
#define TIPDETECTOR_H
#include <v3d_interface.h>
#include"ray_shooting.h"
class TipDetector
{
private:
    Image4DSimple * p4DImage;
    V3DLONG sz[4];

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
    V3DPluginCallback2* callback;

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

    TipDetector();
    void help();
    float getPluginVersion();
    void getImgData(V3DPluginCallback2& callback2);
    void GUI_input_argu();
    void whole_img();
    bool TipDetect_onePoint(LocationSimple CandiPoint);
    ~TipDetector();
    LandmarkList OutputTipPoint();
};


#endif // TIPDETECTOR_H
