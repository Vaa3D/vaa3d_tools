/* sort_func.h
 * 2012-02-01 : by Yinan Wan
 */
 
#ifndef __SORT_FUNC_H__
#define __SORT_FUNC_H__

#include <v3d_interface.h>

void sort_menu(V3DPluginCallback2 &callback, QWidget *parent);
bool sort_func(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp(V3DPluginCallback2 &callback, QWidget *parent);
void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

