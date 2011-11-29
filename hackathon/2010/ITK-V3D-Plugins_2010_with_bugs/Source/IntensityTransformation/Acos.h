#ifndef __Acos_H__
#define __Acos_H__

#include "V3DITKPluginDefaultHeader.h"

class AcosPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Acos);
};

#endif
