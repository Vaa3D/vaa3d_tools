#ifndef TYPESET_FUNC_H
#define TYPESET_FUNC_H

#include <v3d_interface.h>

//bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString FileSWCOpenName);
int typeset_swc_func(V3DPluginCallback2 &callback, double settype, QList<ImageMarker> tmp_list_in);
QList<ImageMarker> get_markers(V3DPluginCallback2 &callback);
void final_typeset_save();
void reload_SWC(V3DPluginCallback2 &callback); //to be implimented
bool typeset_swc_toolbox(const V3DPluginArgList & input);


#endif // TYPESET_FUNC_H
