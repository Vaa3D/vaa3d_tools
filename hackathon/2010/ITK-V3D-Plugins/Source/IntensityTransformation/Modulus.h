#ifndef __Modulus_H__
#define __Modulus_H__

#include "V3DITKPluginDefaultHeader.h"

class ModulusPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Modulus);
};

#endif
