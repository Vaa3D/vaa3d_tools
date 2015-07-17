/* profile_swc.h
 *  * 2015-07 : by Xiaoxiao Liu
 *   */
 
#ifndef __PROFILE_SWC_H_
#define __PROFILE_SWC_H__

#include <v3d_interface.h>

struct IMAGE_METRICS {
    int type; // segment type: 0-- soma;  1 -- dendrite;2--axon
    double cnr;
    double dy;
    double tubularity;
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


