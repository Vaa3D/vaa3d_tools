#ifndef __SignedMaurerDistanceMap_H__
#define __SignedMaurerDistanceMap_H__

#include "V3DITKPluginDefaultHeader.h"

class SignedMaurerDistanceMapPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(SignedMaurerDistanceMap);
};

#endif
