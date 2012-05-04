/* hier_label_func.h
 * This plugin heirachically segments the input neuron tree and label the nodes as features in eswc file.
 * 2012-05-04 : by Yinan Wan
 */
 
#ifndef __HIER_LABEL_FUNC_H__
#define __HIER_LABEL_FUNC_H__

#include <v3d_interface.h>

int hierachical_labeling_io(V3DPluginCallback2 &callback, QWidget *parent);
bool hierachical_labeling_io(const V3DPluginArgList & input, V3DPluginArgList & output);

void printHelp();

#endif

