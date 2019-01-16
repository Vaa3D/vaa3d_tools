#ifndef AUTO_SOMA_CORRECTION_H
#define AUTO_SOMA_CORRECTION_H
#endif // AUTO_SOMA_CORRECTION_H

#include "pre_processing_main.h"
#include "utilities.h"

#ifndef XSCALE
#define XSCALE 0.2
#endif
#ifndef YSCALE
#define YSCALE 0.2
#endif
#ifndef ZSCALE
#define ZSCALE 1
#endif

bool auto_soma_correction_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool auto_soma_correction(QString qs_file_swc, QString qs_file_apo);
