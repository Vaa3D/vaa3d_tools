#ifndef CONNNECT_SWC_REDEFINED_H
#define CONNNECT_SWC_REDEFINED_H

#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"
#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"
#include "neuron_connector_func.h"
#include "utilities.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
using namespace std;

NeuronTree connect_swc(NeuronTree nt);

#endif // CONNNECT_SWC_REDEFINED_H
