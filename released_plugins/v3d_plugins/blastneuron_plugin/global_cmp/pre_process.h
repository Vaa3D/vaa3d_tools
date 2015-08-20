#ifndef __PRE_PROCESS_H__
#define __PRE_PROCESS_H__

#include "basic_surf_objs.h"
#include "../pre_processing/prune_short_branch.h"
#include "../pre_processing/resampling.h"
#include "../pre_processing/align_axis.h"


NeuronTree pre_process(NeuronTree nt, double step_size);
#endif


