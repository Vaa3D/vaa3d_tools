#ifndef TOPO_VALIDATION_H
#define TOPO_VALIDATION_H

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

void Topo_check(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

#endif // TOPO_VALIDATION_H
