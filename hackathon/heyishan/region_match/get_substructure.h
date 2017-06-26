#ifndef GET_SUBSTRUCTURE_H
#define GET_SUBSTRUCTURE_H

#include "region_match_plugin.h"
#include "match_swc.h"
#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
void get_substructure(const NeuronTree &s_mk, const vector<NeuronTree>&s_forest,vector<V3DLONG>&selected_cube);

#endif // GET_SUBSTRUCTURE_H
