/* neuronrecon_func.h
 * a plugin to reconstruct neuron from multiple traced neurons
 * 09/11/2017 : by Yang Yu
 */
 
#ifndef __NEURONRECON_FUNC_H__
#define __NEURONRECON_FUNC_H__

#include <v3d_interface.h>

int neuronrecon_menu(V3DPluginCallback2 &callback,QWidget * parent);
bool neuronrecon_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
void printHelp();

#endif // __NEURONRECON_FUNC_H__

