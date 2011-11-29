#ifndef __Cast_H__
#define __Cast_H__

#include "V3DITKPluginDefaultHeader.h"

class CastPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Cast);
};

#endif
