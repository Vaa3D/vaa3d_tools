#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

#include <QString>
#include <basic_surf_objs.h>
#include <vector>
#include <string>
#include <cstring>
#include <v3d_interface.h>
#include "some_class.h"

inline bool isin(V3DLONG parent,std::vector<V3DLONG> a);

inline int findIndex(const char* s);

inline QString path(const QString& s);

block getBlockOfOPoint(NeuronSWC p,int dx,int dy,int dz);

vector<NeuronSWC> getTPointOfBlock(NeuronTree& nt,block b);

blockTree getBlockTree(NeuronTree& nt,int dx,int dy,int dz);


























#endif // SOME_FUNCTION_H
