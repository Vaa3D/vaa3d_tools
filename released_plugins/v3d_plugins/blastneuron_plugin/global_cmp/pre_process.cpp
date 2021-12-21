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
    if(resampled.listNeuron.size()<10) return resampled;    //Maybe PAC will crash if points is less than 8. Add by Yishan He 2017.7.14

	printf("Aligning PCA axis\n");
    result = align_axis(resampled);
	printf("finish Aligning PCA axis\n");

	return result;
}
