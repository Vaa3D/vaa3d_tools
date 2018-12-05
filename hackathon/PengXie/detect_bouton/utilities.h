#ifndef UTILITIES_H
#define UTILITIES_H

#endif // UTILITIES_H

#include "v3d_interface.h"
#include "NeuronStructExplorer.h"
#include "math.h"
#include "v3d_basicdatatype.h"
#include "bits/stdc++.h"
#include "iostream"
#include "sys/stat.h"
#include "sys/types.h"

using namespace std;

#ifndef NODE_TILE_LENGTH
#define NODE_TILE_LENGTH 100
#endif

#ifndef zRATIO // This is the ratio of z resolution to x and y in IVSCC images.
#define zRATIO (0.28 / 0.1144)
#endif

#ifndef RESIZE_TILE_LENGTH
#define RESIZE_TILE_LENGTH 500
#endif

#ifndef X_TILE_LENGTH
#define X_TILE_LENGTH RESIZE_TILE_LENGTH
#endif
#ifndef Y_TILE_LENGTH
#define Y_TILE_LENGTH RESIZE_TILE_LENGTH
#endif
#ifndef Z_TILE_LENGTH
#define Z_TILE_LENGTH RESIZE_TILE_LENGTH / zRATIO
#endif

// Objects
#ifndef BLOCK
#define BLOCK
struct block{
    QString name;
    XYZ small;
    XYZ large;
};
#endif
// Generic functions
XYZ offset_XYZ(XYZ input, XYZ offset);
QList<CellAPO> offset_apo(QString input_apo, XYZ offset);
block offset_block(block input_block, XYZ offset);
void my_mkdir(QString path);
inline bool exists_file (const std::string& name);
void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format);
void crop_swc(QString input_swc, QString output_swc, block crop_block);
bool my_saveANO(QString ano_dir, QString fileNameHeader, QList<QString> suffix);
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist);

// For bouton detection
QList<block> get_blocks(QString swc);
void run_matlab(QString data_dir, QString block_name);
block string_to_block(QString block_name);
QList<block> nse_to_blocks(NeuronStructExplorer nse);

// For get_terminal
QList<int> get_tips(NeuronTree nt, bool include_root);

// functions for getting blocks







