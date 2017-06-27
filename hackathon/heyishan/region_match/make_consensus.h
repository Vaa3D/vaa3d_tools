#ifndef MAKE_CONSENSUS_H
#define MAKE_CONSENSUS_H

#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>

using namespace std;

bool make_consensus(const NeuronTree & nt, NeuronTree & pattern, NeuronTree & mk, V3DPluginCallback2 &callback);

#endif // MAKE_CONSENSUS_H
