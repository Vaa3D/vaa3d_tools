#ifndef __InvertIntensity_H__
#define __InvertIntensity_H__

#include "V3DITKPluginDefaultHeader.h"

class InvertIntensityPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(InvertIntensity);
};

#endif
