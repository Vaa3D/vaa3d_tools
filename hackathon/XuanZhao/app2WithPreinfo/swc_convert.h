#ifndef SWC_CONVERT_H
#define SWC_CONVERT_H


#include "basic_surf_objs.h"
#include "my_surf_objs.h"

vector<MyMarker*> swc_convert(NeuronTree & nt);
NeuronTree swc_convert(vector<MyMarker*> & inswc);

#endif // SWC_CONVERT_H
