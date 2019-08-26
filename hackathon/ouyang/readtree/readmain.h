#ifndef READMAIN_H
#define READMAIN_H
#include <v3d_interface.h>
#include "basic_surf_objs.h"

bool export_list2file(QList<NeuronSWC> & newone, QString fileSaveName, QString fileOpenName);
void readtree(V3DPluginCallback2 &callback, QWidget *parent);
bool downsample25(const V3DPluginArgList & input, V3DPluginArgList & output);
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName);
QStringList importFileList_addnumbersort(const QString & curFilePath);
#endif // READMAIN_H
