#ifndef __Xor_H__
#define __Xor_H__

#include "V3DITKPluginDefaultHeader.h"

class XorPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Xor);
};

#endif
