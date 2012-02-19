#ifndef __ESWC_CORE_H__
#define __ESWC_CORE_H__
#include "basic_surf_objs.h"
#include <vector>
#include <map>
#include <utility>
using namespace std;
bool swc2eswc(NeuronTree input, vector<V3DLONG> & segment_id, vector<V3DLONG> & segment_layer);
int check_eswc(NeuronTree input, vector<V3DLONG> & segment_id, vector<V3DLONG> & segment_layer);
#endif
