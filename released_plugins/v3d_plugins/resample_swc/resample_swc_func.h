/* resample_swc_func.h
 * This is a plugin to resample neuron swc subject to a fixed step length.
 * 2012-03-02 : by Yinan Wan
 */
 
#ifndef __RESAMPLE_SWC_FUNC_H__
#define __RESAMPLE_SWC_FUNC_H__

#include <v3d_interface.h>


#define byte win_byte_override

#include <Windows.h>


#include <gdiplus.h>

#undef byte


int resample_swc(V3DPluginCallback2 &callback, QWidget *parent);
bool resample_swc(const V3DPluginArgList & input, V3DPluginArgList & output);
bool resample_swc_toolbox(const V3DPluginArgList & input);

#endif

