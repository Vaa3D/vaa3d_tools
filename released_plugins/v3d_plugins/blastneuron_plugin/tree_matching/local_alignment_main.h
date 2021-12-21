#ifndef __LOCAL_ALIGNMENT_H__
#define __LOCAL_ALIGNMENT_H__
#include <vector>
#include <v3d_interface.h>
#include "neuron_tree_align.h"
#include "my_surf_objs.h"

using namespace std;
void printHelp_local_alignment();
int local_alignment_main(const V3DPluginArgList & input, V3DPluginArgList & output);
#endif
