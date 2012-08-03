#ifndef __OtsuThreshold_H__
#define __OtsuThreshold_H__

#include "V3DITKPluginDefaultHeader.h"

class OtsuThresholdPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(OtsuThreshold);
};

#endif
