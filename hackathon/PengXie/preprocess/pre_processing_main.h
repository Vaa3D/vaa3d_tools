#ifndef __PRE_PROCESSING_MAIN_H__
#define __PRE_PROCESSING_MAIN_H__

#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"
using namespace std;

bool pre_processing_main(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp_pre_processing();
#endif
