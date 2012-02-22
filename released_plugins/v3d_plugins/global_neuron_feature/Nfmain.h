#ifndef NFMAIN_H
#define NFMAIN_H
#include "basic_surf_objs.h"

void nf_main(const V3DPluginArgList & input, V3DPluginArgList & output);
void nf_main(V3DPluginCallback2 &callback, QWidget *parent);
void printFeature(double * features);

#endif


