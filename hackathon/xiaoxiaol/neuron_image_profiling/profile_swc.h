/* profile_swc.h
 *  * 2015-07 : by Xiaoxiao Liu
 *   */
 
#ifndef __PROFILE_SWC_H_
#define __PROFILE_SWC_H__

#include <v3d_interface.h>

bool profile_swc_menu(V3DPluginCallback2 &callback, QWidget *parent);
bool profile_swc_func(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
bool intensity_profile(QList<NeuronSWC> neuron, Image4DSimple * image, float dialate_radius, QString output_csv_file);
void printHelp(const V3DPluginCallback2 &callback, QWidget *parent);
void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif


