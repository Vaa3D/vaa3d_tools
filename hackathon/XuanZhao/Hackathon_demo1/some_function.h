#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

#include <QString>
#include <basic_surf_objs.h>
#include <vector>
#include <string>
#include <cstring>
#include <v3d_interface.h>
#include "some_class.h"



block getBlockOfOPoint(NeuronSWC p,int dx,int dy,int dz);

vector<NeuronSWC> getTPointOfBlock(NeuronTree& nt,block b);

blockTree getBlockTree(NeuronTree& nt,int dx,int dy,int dz);


























#endif // SOME_FUNCTION_H
