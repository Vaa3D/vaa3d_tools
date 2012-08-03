#ifndef __Sqrt_H__
#define __Sqrt_H__

#include "V3DITKPluginDefaultHeader.h"

class SqrtPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Sqrt);
};

#endif
