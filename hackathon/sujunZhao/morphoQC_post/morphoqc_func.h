#ifndef MORPHOQC_FUNC_H
#define MORPHOQC_FUNC_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <iterator>
#include <QHash>

double computeDist(const NeuronSWC & p1, const NeuronSWC & p2);
void Find_tip(V3DPluginCallback2 & callback, const V3DPluginArgList & input);
bool Type_correction(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
void Tip_pruning(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output);

#endif // MORPHOQC_FUNC_H
