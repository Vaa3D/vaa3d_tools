#ifndef SHIFT_MASK2D_H
#define SHIFT_MASK2D_H
#include "shift_mask_image_plugin.h"
#include "v3d_interface.h"
#include "my_surf_objs.h"
#endif // SHIFT_MASK2D_H
void get_terminal_signal(QString input_swc, QString input_image, V3DPluginCallback2 & callback);
int find_tip(NeuronTree nt, XYZ sz_xyz);
double signal_between_markers(unsigned char * data1d, NeuronSWC n1, NeuronSWC n2,
                              long sz0, long sz1, long sz2, long sz3, V3DPluginCallback2 & callback);
bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2);
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist);
template<class T> bool save_temp_image(T * &outimg1d, unsigned char * inimg1d, vector<MyMarker> allmarkers,
                                       long sz0, long sz1, long sz2, int datatype, V3DPluginCallback2 & callback);
