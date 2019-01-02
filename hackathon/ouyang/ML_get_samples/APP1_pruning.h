#ifndef APP1_PRUNING_H
#define APP1_PRUNING_H

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../../vaa3d_tools/v3d_main/basic_c_fun/basic_surf_objs.h"
#include <v3d_interface.h>
#include "get_tip_block.h"

//V3DLONG pruning_covered_leaf_single_cover(vector<  NeuronTree  >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness);
void pruning_covered_leaf_single_cover(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
void rearrange_and_remove_labeled_deletion_nodes_mmUnit(NeuronTree & mmUnit,QVector<V3DLONG>childs);
double distL2square(const NeuronSWC & a, const NeuronSWC & b);
double calculate_overlapping_ratio_n1(const NeuronSWC & n1, const NeuronSWC & n2, unsigned char ***imap,  V3DLONG sz[3], double trace_z_thickness,QVector<V3DLONG> childs,map<double, V3DLONG> index_map);
map<int,double> calculate_R(unsigned char ***imap,QList<NeuronSWC> mUnit,double avr_thres,double trace_z_thickness,V3DLONG sz[4]);
#endif // APP1_PRUNING_H
