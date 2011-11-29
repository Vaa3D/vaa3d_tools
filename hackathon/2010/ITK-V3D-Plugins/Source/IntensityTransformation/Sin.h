#ifndef __Sin_H__
#define __Sin_H__

#include "V3DITKPluginDefaultHeader.h"

class SinPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Sin);
};

#endif
