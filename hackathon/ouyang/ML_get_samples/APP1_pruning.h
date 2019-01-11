#ifndef APP1_PRUNING_H
#define APP1_PRUNING_H

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../../vaa3d_tools/v3d_main/basic_c_fun/basic_surf_objs.h"
#include <v3d_interface.h>
#include "get_tip_block.h"

struct trer_and_num{

    NeuronTree tree;
    int delete_num;

};
//V3DLONG pruning_covered_leaf_single_cover(vector<  NeuronTree  >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness);
void pruning_covered_leaf_single_cover(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
void rearrange_and_remove_labeled_deletion_nodes_mmUnit(NeuronTree & mmUnit,QVector<V3DLONG>childs,int delete_nums);
double distL2square(const NeuronSWC & a, const NeuronSWC & b);
double calculate_overlapping_ratio_n1(const NeuronSWC & n1, const NeuronSWC & n2, unsigned char *data1D, unsigned char *data2D,V3DLONG sz[3], double trace_z_thickness,
                                      map<int,float> r_and_index,map<double, V3DLONG> index_map,int mode);
map<int,float> calculate_R(unsigned char *data1D,QList<NeuronSWC> mUnit,double avr_thres,double trace_z_thickness,V3DLONG sz[4],int mode,double thresh,bool aver_or_threhold);
NeuronTree revise_radius(NeuronTree inputtree,map<int,float> radius);
//map<int,float> calculate_R_based_2D(unsigned char *data1d_crop,QList<NeuronSWC> mUnit,double avr_thres,V3DLONG mysz[4]);
unsigned char * get_2d_pixel(unsigned char *data1d_crop,V3DLONG mysz[4]);
double get_aver_signal(vector<MyMarker> allmarkers, unsigned char * data1d,unsigned char * data2d,long sz0, long sz1, long sz2, int mode);
map<int,float> markerRadius_hanchuan_XY(unsigned char *inimg1d, V3DLONG  sz[4], vector<MyMarker> allmarkers, double thresh,bool aver_or_threhold,double aver);
trer_and_num pruning_dark_tip_node(NeuronTree mmUnit,unsigned char *image3d,double threshold,V3DLONG sz[4]);
#endif // APP1_PRUNING_H
