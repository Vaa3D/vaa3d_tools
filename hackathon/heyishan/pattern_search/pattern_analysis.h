#ifndef PATTERN_ANALYSIS_H
#define PATTERN_ANALYSIS_H

#include "v3d_message.h"
#include "basic_surf_objs.h"

bool pattern_analysis(const NeuronTree &nt,const NeuronTree &boundary,NeuronTree & consensus, int & boundary_length);

#endif // PATTERN_ANALYSIS_H
