#ifndef IMAGE_LABEL_H
#define IMAGE_LABEL_H

#include "ImageProcessing_plugin.h"

bool image_label(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool image_label(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);

#endif // IMAGE_LABEL_H
