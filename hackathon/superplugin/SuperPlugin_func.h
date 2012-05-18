/* SuperPlugin_func.h
 * This is SuperPlugin which uses multiple other plugins in a pipeline.
 * 2012-05-11 : by J Zhou
 */

#ifndef __SUPERPLUGIN_FUNC_H__
#define __SUPERPLUGIN_FUNC_H__

#include <v3d_interface.h>

int parse_multi_plugin(V3DPluginCallback2 &callback, QWidget *parent);
bool parse_multi_plugin(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

