#ifndef PRUNE_ALIGNMENT_H
#define PRUNE_ALIGNMENT_H

#include<v3d_interface.h>

bool export_prune_alignment(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
bool prune_alignment(const V3DPluginArgList & input, V3DPluginArgList & output);
QList<NeuronSWC> prune_long_alignment(QList<NeuronSWC> &neuron,double thres);
#endif // PRUNE_ALIGNMENT_H
