#ifndef GET_TIP_BLOCK_H
#define GET_TIP_BLOCK_H

#include "v3d_interface.h"
//#include "utilities.h"
struct block{
    QString name;
    XYZ small;
    XYZ large;
};
void get_terminal(QString image_file, QString swc_file, QString output_dir, XYZ block_size, V3DPluginCallback2 & callback);
QList<int> get_tips(NeuronTree nt, bool include_root);
block offset_block(block input_block, XYZ offset);
void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format);
void crop_swc(QString input_swc, QString output_swc, block crop_block);
bool my_saveANO(QString ano_dir, QString fileNameHeader, QList<QString> suffix);
void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif // GET_TIP_BLOCK_H
