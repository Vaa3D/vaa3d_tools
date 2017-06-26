#ifndef PRE_PROCESS_H
#define PRE_PROCESS_H
#include "basic_surf_objs.h"
#include "../neuron_utilities/resample_swc.h"
#include "../neuron_utilities/sort_swc.h"
NeuronTree pre_process(NeuronTree nt, double step_size, double prune_size);
int pca1(double *, int, int, double * , double *, double *);
NeuronTree align_axis(NeuronTree input);



#endif // PRE_PROCESS_H
