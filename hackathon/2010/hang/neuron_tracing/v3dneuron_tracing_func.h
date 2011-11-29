/* v3dneuron_tracing_func.h
 * This plugin will call v3dneuron_tracing command in dofunc
 * 2011-07-07 : by xiaoh10
 */
 
#ifndef __V3DNEURON_TRACING_FUNC_H__
#define __V3DNEURON_TRACING_FUNC_H__

#include <v3d_interface.h>

int v3dneuron_tracing(V3DPluginCallback2 &callback, QWidget *parent);
bool v3dneuron_tracing(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

