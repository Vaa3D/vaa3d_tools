#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include "v3d_interface.h"

void enhanceImage(unsigned char* data1d, V3DLONG* sz);

void get_2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

#endif // IMAGEPROCESS_H
