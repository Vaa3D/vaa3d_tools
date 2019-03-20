#ifndef GET_IMAGE_BLOCK_FROM_APO_H
#define GET_IMAGE_BLOCK_FROM_APO_H

#endif // GET_IMAGE_BLOCK_FROM_APO_H

#include "v3d_interface.h"
#include "my_surf_objs.h"
#include "filter_dialog.h"
#include "math.h"
#include "v3d_basicdatatype.h"
#include "ImgProcessor.h"
#include "bits/stdc++.h"
#include "iostream"
#include "sys/stat.h"
#include "sys/types.h"
#include "dirent.h"
#include "utilities.h"
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
#ifndef RX
#define RX 0.2
#endif
#ifndef RY
#define RY 0.2
#endif
#ifndef RZ
#define RZ 1
#endif

bool crop_by_apo(QString image_file, QString apo_file, QString output_dir, XYZ block_size, V3DPluginCallback2 & callback);
