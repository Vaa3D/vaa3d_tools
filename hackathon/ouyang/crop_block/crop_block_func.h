#ifndef CROP_BLOCK_FUNC_H
#define CROP_BLOCK_FUNC_H

#include "basic_surf_objs.h"
#include "v3d_interface.h"



void crop_bt_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QList<int> get_branch(NeuronTree nt);
QList<int> get_tips(NeuronTree nt, bool include_root);

#endif // CROP_BLOCK_FUNC_H


