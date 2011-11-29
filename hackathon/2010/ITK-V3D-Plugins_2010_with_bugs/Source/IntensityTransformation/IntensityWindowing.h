#ifndef __IntensityWindowing_H__
#define __IntensityWindowing_H__

#include "V3DITKPluginDefaultHeader.h"

class IntensityWindowingPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(IntensityWindowing);
};

#endif
