#ifndef ADD_SOMA_H
#define ADD_SOMA_H

#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
using namespace std;

bool add_soma(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp_add_soma();

#endif // ADD_SOMA_H
