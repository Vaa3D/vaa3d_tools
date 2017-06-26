#ifndef __S2_TRACING_CONNECTOR_FUNC_H__
#define __S2_TRACING_CONNECTOR_FUNC_H__


#include "S2_tracing_connector_plugin.h"


bool generatorcombined4FilesInDir(const V3DPluginArgList & input, V3DPluginArgList & output);
QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code);


#endif