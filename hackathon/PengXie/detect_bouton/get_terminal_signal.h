#ifndef GET_TERMINAL_SIGNAL_H
#define GET_TERMINAL_SIGNAL_H
#include "v3d_interface.h"
#include "utilities.h"
#endif // GET_TERMINAL_SIGNAL_H

void get_terminal_signal(QString input_swc, QString input_image, V3DPluginCallback2 & callback);
int find_tip(NeuronTree nt, XYZ sz_xyz);
double signal_between_markers(unsigned char * data1d, NeuronSWC n1, NeuronSWC n2,
                              long sz0, long sz1, long sz2, long sz3, V3DPluginCallback2 & callback);
bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2);
template<class T> bool save_temp_image(T * &outimg1d, unsigned char * inimg1d, vector<MyMarker> allmarkers,
                                       long sz0, long sz1, long sz2, int datatype, V3DPluginCallback2 & callback);
