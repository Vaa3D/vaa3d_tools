#ifndef __KappaSigmaThreshold_H__
#define __KappaSigmaThreshold_H__

#include "V3DITKPluginDefaultHeader.h"

class KappaSigmaThresholdPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(KappaSigmaThreshold);
};

#endif
