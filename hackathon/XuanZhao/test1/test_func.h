#ifndef TEST_FUNC_H
#define TEST_FUNC_H

#include<v3d_interface.h>

int image_threshold(V3DPluginCallback2 &callback,QWidget* parent);
int image_threshold(const V3DPluginArgList &input,V3DPluginArgList &output);
void printHelp();

#endif // TEST_FUNC_H
