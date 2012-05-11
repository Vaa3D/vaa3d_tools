/* neuron_dist_func.h
 * The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.
 * 2012-05-04 : by Yinan Wan
 */
 
#ifndef __NEURON_DIST_FUNC_H__
#define __NEURON_DIST_FUNC_H__

#include <v3d_interface.h>

int neuron_dist_io(V3DPluginCallback2 &callback, QWidget *parent);
bool neuron_dist_io(const V3DPluginArgList & input, V3DPluginArgList & output);
bool neuron_dist_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback);
void printHelp();


#endif

