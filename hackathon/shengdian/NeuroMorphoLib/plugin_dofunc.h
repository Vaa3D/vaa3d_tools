#ifndef PLUGIN_DOFUNC_H
#define PLUGIN_DOFUNC_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "neuro_morpho_lib.h"
#include <cmath>
#include <iterator>
//#include "json/json.h"

using namespace std;
bool branch_features(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool lm_statistic_features(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output);
bool bouton_distribution(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool write_lm_features(const QString& filename,NeuronTree nt);
bool write_branches(NeuronTree nt,const QString& outpath,int type=0);
bool write_branch_features(const QString& filename,NeuronTree nt, bool bouton_fea=false);
bool write_bouton_distribution(const QString &filename, NeuronTree nt,int split_times=4);
bool nt_check(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool nt_qc(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool swc2branches(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool neuron_split(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool crop_swc_terafly_image_block(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool crop_local_swc(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool somalist_in_folder(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool bswcTo(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
bool swc_ada_sampling(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
//bool ada_sampling(NeuronTree nt, NeuronTree& nt_sampled,float curvature_ratio=1.0,float thickness_ratio=1.0,float intensity_ratio=1.0);
bool ada_sampling(NeuronTree nt, NeuronTree& nt_sampled,
                  float curvature_ratio=1.0, float dist_thre=2.0,
                  float thickness_ratio=1.0, float r_thre=0.5,
                  float intensity_ratio=1.0, float i_thre=30);
bool swc_parallization(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);
#endif // PLUGIN_DOFUNC_H
