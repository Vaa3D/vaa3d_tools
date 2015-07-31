/* profile_swc.h
 *  * 2015-07 : by Xiaoxiao Liu
 *   */
 
#ifndef __PROFILE_SWC_H_
#define __PROFILE_SWC_H__

#include <v3d_interface.h>

struct IMAGE_METRICS {
    int type; // segment type: 2-- soma;  2 -- dendrite;3--axon ;4--apical dendrite;  -1 --- not-defined/all types
    double cnr;
    double snr;
    double dy;  //dynamic range
    double tubularity_mean;
    double fg_mean;
    double bg_mean;
    double fg_std;
    double bg_std;
    double tubularity_std;
};


struct ENSEMBLE_METRICS {
    double mean_cnr;
    double mean_snr;
    double mean_dy;
    double mean_tubularity;
    double mean_fg;
    double mean_bg;
    double std_cnr;
    double std_dy;
    double std_tubularity;
    double std_bg;
    double std_fg;
};

bool profile_swc_menu(V3DPluginCallback2 &callback, QWidget *parent);
bool profile_swc_func(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
QList<IMAGE_METRICS> intensity_profile(NeuronTree neuronTree, Image4DSimple * image, float dilate_ratio, int flip,int invert, V3DPluginCallback2 &callback);
void printHelp(const V3DPluginCallback2 &callback, QWidget *parent);
void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);



IMAGE_METRICS compute_metrics(Image4DSimple *image,  QList<NeuronSWC> neuronSegment,float dilate_ratio, V3DPluginCallback2 &callback);

bool flip_y (Image4DSimple * image);
bool invert_intensity(Image4DSimple * image);
bool writeMetrics2CSV(QList<IMAGE_METRICS> result_metrics, QString output_csv_file);


#endif


