#ifndef PRUNE_ALIGNMENT_H
#define PRUNE_ALIGNMENT_H

#include<v3d_interface.h>
#include<vector>
#include<iostream>

bool prune_alignment(const V3DPluginArgList & input, V3DPluginArgList & output);
bool export_list_to_file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
#endif // PRUNE_ALIGNMENT_H
