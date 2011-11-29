#ifndef __Multiply_H__
#define __Multiply_H__

#include "V3DITKPluginDefaultHeader.h"

class MultiplyPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Multiply);
};

#endif
