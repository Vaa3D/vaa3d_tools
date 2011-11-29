#ifndef __Atan_H__
#define __Atan_H__

#include "V3DITKPluginDefaultHeader.h"

class AtanPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Atan);
};

#endif
