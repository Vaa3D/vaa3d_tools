/* neuron_toolbox_func.h
 * This is a super plugin that gather all sub-plugins related to neuron structure processing
 * 2012-04-06 : by Yinan Wan
 */
 
#ifndef __NEURON_TOOLBOX_FUNC_H__
#define __NEURON_TOOLBOX_FUNC_H__

#include <v3d_interface.h>

bool neuron_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent);
bool neuron_toolbox_func(V3DPluginCallback2 & callback, QWidget * parent, const V3DPluginArgList & input, V3DPluginArgList & output);
void help(bool b_useWin);

#endif

