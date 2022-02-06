#ifndef SWC_CONVERT_H
#define SWC_CONVERT_H

#include "basic_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

vector<MyMarker*> swc_convert(NeuronTree & nt);
NeuronTree swcConvert(const vector<MyMarker *> &inswc);

#endif // SWC_CONVERT_H
