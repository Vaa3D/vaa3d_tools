#ifndef GET_TIP_BLOCK_H
#define GET_TIP_BLOCK_H

#include <v3d_interface.h>
struct block{
    QString name;
    XYZ small;
    XYZ large;
};
void get_terminal(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QList<int> get_tips(NeuronTree nt, bool include_root);
block offset_block(block input_block, XYZ offset);
void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format,QString input_swc,int tipnum,XYZ tip);
void crop_swc(QString input_swc, QString output_swc, block crop_block);
bool my_saveANO(QString ano_dir, QString fileNameHeader, QList<QString> suffix);
void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp1(const V3DPluginArgList & input, V3DPluginArgList & output);
void get2d_label_image(NeuronTree nt_crop_sorted,V3DLONG mysz,unsigned char * data1d_crop,V3DPluginCallback2 & callback,QString output_format,int tipnum,XYZ tip);
void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

#endif // GET_TIP_BLOCK_H
