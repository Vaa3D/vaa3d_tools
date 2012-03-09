/* histogram_func.h
 * Display histogram of the image
 * 2012-03-01 : by Jianlong Zhou
 */
 
#ifndef __HISTOGRAM_FUNC_H__
#define __HISTOGRAM_FUNC_H__

#include <vector>

#include <v3d_interface.h>

int compute(V3DPluginCallback2 &callback, QWidget *parent);
bool compute(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

