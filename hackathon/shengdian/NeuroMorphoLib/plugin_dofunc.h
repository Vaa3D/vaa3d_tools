#ifndef PLUGIN_DOFUNC_H
#define PLUGIN_DOFUNC_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "neuro_morpho_lib.h"
#include <cmath>
#include <iterator>

using namespace std;

bool lm_statistic_features(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output);
bool write_lm_features(const QString& filename,NeuronTree nt);
#endif // PLUGIN_DOFUNC_H
