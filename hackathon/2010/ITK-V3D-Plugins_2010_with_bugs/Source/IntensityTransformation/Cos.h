#ifndef __Cos_H__
#define __Cos_H__

#include "V3DITKPluginDefaultHeader.h"

class CosPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Cos);
};

#endif
