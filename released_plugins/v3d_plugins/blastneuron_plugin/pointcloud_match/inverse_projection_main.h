#ifndef __INVERSE_PROJECTION_MAIN_H___
#define __INVERSE_PROJECTION_MAIN_H___

#include "q_pointcloud_match.h"
#include "basic_surf_objs.h"
#include <string.h>
#include <QtGlobal>
#include <v3d_interface.h>
NeuronTree export_invp_2swc(vector<Coord3D_PCM> & vec_2_invp, NeuronTree & nt_sub);
void printHelp_invp();
int inverse_projection_main(const V3DPluginArgList &input, V3DPluginArgList & output);
#endif

