#ifndef NEURITE_ANALYSIS_MAIN_H
#define NEURITE_ANALYSIS_MAIN_H

#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"
#include "neuron_connector_func.h"
using namespace std;

bool neurite_analysis_main(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp_neurite_analysis();
#endif // NEURITE_ANALYSIS_MAIN_H
