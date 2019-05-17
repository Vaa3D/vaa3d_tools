#ifndef GMM_ALGORITHM_H
#define GMM_ALGORITHM_H

//#include "v3d_message.h"
//#include <vector>
//#include "basic_surf_objs.h"
//#include "v3d_interface.h"
#include "ImageProcessing_plugin.h"

void gmm(V3DPluginCallback2 &callback,QWidget *parent,input_PARA &PARA);
bool gmm(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);
template <class T> bool gmmFilter(QString fileName,T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg,QWidget *parent);


#endif // GMM_ALGORITHM_H
