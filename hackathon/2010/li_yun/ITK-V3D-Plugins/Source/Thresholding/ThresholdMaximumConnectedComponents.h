#ifndef __ThresholdMaximumConnectedComponents_H__
#define __ThresholdMaximumConnectedComponents_H__

#include "V3DITKPluginDefaultHeader.h"

class ThresholdMaximumConnectedComponentsPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(ThresholdMaximumConnectedComponents);
};

#endif
