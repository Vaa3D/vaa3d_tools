#ifndef BATCH_MAIN_H
#define BATCH_MAIN_H

#include<v3d_interface.h>
#include<vector>
#include "my_surf_objs.h"
#include"v3d_message.h"
#include<basic_surf_objs.h>
#include<QtGui>
bool batch_main(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);
bool GreaterSort (QList<NeuronSWC> a, QList<NeuronSWC> b);
char* num2str(int i);
char* num2str(double i);

QString getAppPath();
#endif // BATCH_MAIN_H
