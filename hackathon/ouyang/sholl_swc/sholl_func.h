/* sholl_func.h
 * 2018-06-26: by OuYang Qiang
 */
 
#ifndef __SHOLL_FUNC_H__
#define __SHOLL_FUNC_H__

#include <v3d_interface.h>

void sholl_menu(V3DPluginCallback2 &callback, QWidget *parent);
bool sholl_func(const V3DPluginArgList & input, V3DPluginArgList & output);
void sholl_toolbox(const V3DPluginArgList & input);
void printHelp(V3DPluginCallback2 &callback, QWidget *parent);
void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

