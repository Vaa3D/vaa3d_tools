/* image_quality_func.h
 * Obtain image quality features
 * 2021-08-04 : by Linus Manubens Gil
 */
 
#ifndef __IMAGEQUALITY_FUNC_H__
#define __IMAGEQUALITY_FUNC_H__

#include <vector>

#include <v3d_interface.h>

int compute(V3DPluginCallback2 &callback, QWidget *parent);
bool compute(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

