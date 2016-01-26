#ifndef COMMON_H
#define COMMON_H

//#include "v3d_interface.h"
#include <spine_fun.h>

using namespace std;

int calc_nearest_node(NeuronTree neuron,float center_x,float center_y,float center_z);
bool write_marker_file(QString filename,LandmarkList listmarkers);
bool save_project_results(V3DPluginCallback2 *callback, V3DLONG sz_img[4], vector<GOV> label_group, QString folder_output,
      QString input_swc_name, QString input_image_name, bool eswc_flag, NeuronTree neuron, LandmarkList LList, int sel_channel, int bg_thr, int max_dis, int seg_id, int marker_id, QString output_label_name, QString output_marker_name, QString output_csv_name);
void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);
void GetColorRGB(int* rgb, int idx);
vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z);

vector<vector<int> > build_parent_LUT(NeuronTree *neuron_tmp);
vector<vector<int> > neurontree_divide_swc(NeuronTree *neuron_tmp, float distance_thresh);
NeuronTree *check_neuron_tree(NeuronTree neuron, V3DLONG sz_img[4]);
float calc_between_dis(NeuronTree *neuron,int node1_id,int node2_id);

void open_triview_window(QString main_win_name, V3DPluginCallback2 *callback,
                       unsigned char *image, v3dhandle &main_win, V3DLONG sz_img[4]);

#endif // COMMON_H
