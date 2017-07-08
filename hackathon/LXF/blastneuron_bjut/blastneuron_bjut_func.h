#ifndef BLASTNEURON_BJUT_FUNC_H
#define BLASTNEURON_BJUT_FUNC_H

#include<v3d_interface.h>
#include<vector>
#include "my_surf_objs.h"
#include "neuron_tree_align.h"
#include"v3d_message.h"


bool pre_process_func(const V3DPluginArgList & input, V3DPluginArgList & output);
bool resampling_main(const V3DPluginArgList & input, V3DPluginArgList & output);
bool blastneuron_main(const V3DPluginArgList & input, V3DPluginArgList & output);



void printHelp();

#endif // BLASTNEURON_BJUT_FUNC_H
