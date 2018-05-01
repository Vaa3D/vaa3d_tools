/* example_func.h
 * This is an example plugin, the header file of plugin functions.
 * 2012-02-10 : by Yinan Wan
 */
 
#ifndef __EXAMPLE_FUNC_H__
#define __EXAMPLE_FUNC_H__

#include <v3d_interface.h>

int tipdetection(V3DPluginCallback2 &callback, QWidget *parent);

void printHelp();

#endif