#ifndef __EXAMPLE_FUNC_H__
#define __EXAMPLE_FUNC_H__

#include <v3d_message.h>
#include <stackutil.h>
#include <v3d_interface.h>

int image_threshold(V3DPluginCallback2 &callback,QWidget *parent);
int image_threshold(const V3DPluginArgList &input,V3DPluginArgList &output);

void printHelp();

#endif
