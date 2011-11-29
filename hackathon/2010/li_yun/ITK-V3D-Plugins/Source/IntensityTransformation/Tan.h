#ifndef __Tan_H__
#define __Tan_H__

#include "V3DITKPluginDefaultHeader.h"

class TanPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Tan);
};

#endif
