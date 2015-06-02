#ifndef __PRE_PROCESS_H__
#define __PRE_PROCESS_H__

#include "basic_surf_objs.h"
#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"

NeuronTree pre_process(NeuronTree nt, double step_size)
{
	NeuronTree result;
	NeuronTree pruned;
	if (!prune_branch(nt, pruned))
	{
		fprintf(stderr,"Error in prune_short_branch.\n");
		return result;
	}
	
	printf("Resampling along segments\n");
	NeuronTree resampled = resample(pruned, step_size);
	

	printf("Aligning PCA axis\n");
	result = align_axis(resampled);

	return result;
}

#endif


