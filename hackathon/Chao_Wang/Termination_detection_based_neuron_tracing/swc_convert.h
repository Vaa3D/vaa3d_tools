#ifndef __SWC_CONVERT_H__
#define __SWC_CONVERT_H__

#include "basic_surf_objs.h"
#include "my_surf_objs.h"

vector<MyMarker*> swc_convert(NeuronTree & nt);
NeuronTree swc_convert(vector<MyMarker*> & inswc);

#endif
