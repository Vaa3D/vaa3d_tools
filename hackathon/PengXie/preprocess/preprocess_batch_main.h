#ifndef PREPROCESS_BATCH_MAIN_H
#define PREPROCESS_BATCH_MAIN_H

#include "pre_processing_main.h"
#include "qc_main.h"

bool preprocess_batch_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool preprocess_batch_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp_preprocess_batch();

bool preprocess_files_in_dir(QString qs_dir_swc);
#endif // PREPROCESS_BATCH_MAIN_H
