#ifndef __Log10_H__
#define __Log10_H__

#include "V3DITKPluginDefaultHeader.h"

class Log10Plugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Log10);
};

#endif
