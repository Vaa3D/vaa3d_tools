#ifndef __OtsuMultipleThresholds_H__
#define __OtsuMultipleThresholds_H__

#include "V3DITKPluginDefaultHeader.h"

class OtsuMultipleThresholdsPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(OtsuMultipleThresholds);
};

#endif
