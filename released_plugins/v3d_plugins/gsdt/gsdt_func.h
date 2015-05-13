/* gsdt_func.h
 * Perform distance transformation on grayscale image.
 * 2012-03-02 : by Hang Xiao
 */
 
#ifndef __GSDT_FUNC_H__
#define __GSDT_FUNC_H__

#include <v3d_interface.h>

bool gsdt(V3DPluginCallback2 &callback, QWidget *parent);
bool gsdt(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback);

#endif

