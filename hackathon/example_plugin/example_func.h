/* example_func.h
 * This is an example plugin, the header file of plugin functions.
 * 2012-01-01 : by YourName
 */
 
#ifndef __EXAMPLE_FUNC_H__
#define __EXAMPLE_FUNC_H__

#include <v3d_interface.h>

int image_threshold(V3DPluginCallback2 &callback, QWidget *parent);
int image_threshold(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp();

#endif

