#ifndef SHRINK
#define SHRINK

#include "v3d_interface.h"

bool shrink_tracing(unsigned char* input_bw, long input_width, long input_height, long input_layer, const int kernel_sz, NeuronTree *output_tree);

#endif // SHRINK

