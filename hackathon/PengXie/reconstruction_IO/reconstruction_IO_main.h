#ifndef RECONSTRUCTION_IO_MAIN_H
#define RECONSTRUCTION_IO_MAIN_H
#include "v3d_interface.h"


void reconstruction_IO_main(QString input, QString output,
                            V3DLONG tx=0, V3DLONG ty=0, V3DLONG tz=0,
                            V3DLONG rx=0, V3DLONG ry=0, V3DLONG rz=0,
                            V3DLONG sx=1, V3DLONG sy=1, V3DLONG sz=1
                            );
void convert_format(QString inputfile, QString outputlabel, QString informat, QString outformat);
void scale(QString inputfile, QString outputlabel, double xscale=1, double yscale=1, double zscale=1);

#endif // RECONSTRUCTION_IO_MAIN_H
