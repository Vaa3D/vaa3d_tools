#ifndef __Subtract_H__
#define __Subtract_H__

#include "V3DITKPluginDefaultHeader.h"

class SubtractPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Subtract);
};

#endif
