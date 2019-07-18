#ifndef GET_TERMINAL_H
#define GET_TERMINAL_H
#include "v3d_interface.h"
#include "utilities.h"
#include "filter_dialog.h"
#include "get_terminal_signal.h"
#include "ImgProcessor.h"
#endif // GET_TERMINAL_H
#ifndef RX
#define RX 0.2
#endif
#ifndef RY
#define RY 0.2
#endif
#ifndef RZ
#define RZ 1
#endif
void get_terminal(QString image_file, QString swc_file, QString output_dir, XYZ block_size, V3DPluginCallback2 & callback);
