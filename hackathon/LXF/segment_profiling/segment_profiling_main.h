#ifndef SEGMENT_PROFILING_MAIN_H
#define SEGMENT_PROFILING_MAIN_H
#include"basic_surf_objs.h"
#include "../wrong_area_search/my_surf_objs.h"
#include "v3d_interface.h"
#include "v3d_message.h"
#include "../wrong_area_search/find_wrong_area.h"
#include "../../../released_plugins/v3d_plugins/blastneuron_plugin/tree_matching/swc_utils.h"
#include "../../KLS/neuron_image_snr/profile_swc.h"
bool segment_profiling_main(V3DPluginCallback2 &callback,NeuronTree &nt,QString &filename);
bool writeCSV(QList<QList<IMAGE_METRICS> > &CSVlist, QString output_csv_file);
struct PARA
{
    unsigned char* data1d;
    V3DLONG im_cropped_sz[4];
    double original_o[3];
    QString img_name,swc_name,img_name_meanshift;
    NeuronTree nt_meanshift;

};
#endif // SEGMENT_PROFILING_MAIN_H
