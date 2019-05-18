#ifndef THRESHOLD_SELECT_H
#define THRESHOLD_SELECT_H

//#include "v3d_message.h"
//#include <vector>
//#include "basic_surf_objs.h"
//#include "v3d_interface.h"
#include "ImageProcessing_plugin.h"


void threshold_select(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool threshold_select(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);
template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev);
template <class T> bool statisticalProcess(T* data1d,V3DLONG *in_sz,V3DLONG c);

#endif // THRESHOLD_SELECT_H
