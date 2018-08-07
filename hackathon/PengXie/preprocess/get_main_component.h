#ifndef GET_MAIN_COMPONENT_H
#define GET_MAIN_COMPONENT_H

#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
using namespace std;

bool get_main_component(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp_get_main_component();


#endif // GET_MAIN_COMPONENT_H
