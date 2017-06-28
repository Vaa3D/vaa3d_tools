#include "pre_process.h"

NeuronTree pre_process(NeuronTree nt, double step_size, double prune_size)
{
	NeuronTree result;
	NeuronTree pruned;
    if (!prune_branch(nt, pruned,prune_size))
	{
		fprintf(stderr,"Error in prune_short_branch.\n");
		return result;
	}
	
	printf("Resampling along segments\n");
	NeuronTree resampled = resample(pruned, step_size);


//    printf("Sort \n");
//    NeuronTree sorted = sort(resampled, step_size);


	printf("Aligning PCA axis\n");
    result = align_axis(resampled);
	printf("finish Aligning PCA axis\n");

	return result;
}
