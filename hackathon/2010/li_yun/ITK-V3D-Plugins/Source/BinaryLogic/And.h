#ifndef __And_H__
#define __And_H__

#include "V3DITKPluginDefaultHeader.h"

class AndPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(And);
};

#endif
