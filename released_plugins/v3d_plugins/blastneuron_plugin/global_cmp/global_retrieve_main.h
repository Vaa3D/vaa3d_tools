#ifndef __GLOBAL_RETRIEVE_MAIN_H__
#define	 __GLOBAL_RETRIEVE_MAIN_H__

#include <QtGlobal>
#include <vector>
#include "v3d_message.h"
#include <v3d_interface.h>
#include "neuron_retrieve.h"
using namespace std;
void printHelp_global_retrieve();
int global_retrieve_main(const V3DPluginArgList & input, V3DPluginArgList & output);
#endif
