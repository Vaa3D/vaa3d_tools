#ifndef GET_SUB_BLOCK_H
#define GET_SUB_BLOCK_H
#include "ImageProcessing_plugin.h"

struct PARA
{
    unsigned char* data1d;
    V3DLONG im_cropped_sz[4];
    double original_o[3];
    QString img_name,swc_name;

};


//bool get_sub_block(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
//bool get_sub_block(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback,unsigned char * &out1dData);
bool get_sub_block(V3DPluginCallback2 &callback, QWidget *parent);
bool get_sub_block(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);

#endif // GET_SUB_BLOCK_H
