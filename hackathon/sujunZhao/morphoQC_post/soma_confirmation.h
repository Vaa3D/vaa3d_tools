#ifndef SOMA_CONFIRMATION_H
#define SOMA_CONFIRMATION_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <iterator>
#include <QHash>
#include <morphoqc_func.h>

struct block{
    QString name;
    XYZ small;
    XYZ large;
};

void list_to_apo(QList<int> point_list, QString swc_file, QList<int> color_mark);
NeuronSWC calc_mean_shift_center(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double bkg_thre,int windowradius);
NeuronTree shift_soma(NeuronTree nt, QString image_file,V3DPluginCallback2 & callback);
void Soma_pos_correct(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
void Soma_check(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
bool Soma_correction(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

#endif // SOMA_CONFIRMATION_H
