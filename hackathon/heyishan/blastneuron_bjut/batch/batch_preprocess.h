#ifndef BATCH_PREPROCESS_H
#define BATCH_PREPROCESS_H

#include<v3d_interface.h>
#include<vector>
#include "my_surf_objs.h"
#include"v3d_message.h"
#include<QtGui>

bool batch_preprocess(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif // BATCH_PREPROCESS_H
