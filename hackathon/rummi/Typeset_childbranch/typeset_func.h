#ifndef TYPESET_FUNC_H
#define TYPESET_FUNC_H

#include <v3d_interface.h>

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString FileSWCOpenName);
int typeset_swc(V3DPluginCallback2 &callback, QWidget *parent);
bool typeset_swc(const V3DPluginArgList & input, V3DPluginArgList & output);
bool typeset_swc_toolbox(const V3DPluginArgList & input);


#endif // TYPESET_FUNC_H
