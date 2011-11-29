#ifndef __SignedDanielssonDistanceMap_H__
#define __SignedDanielssonDistanceMap_H__

#include "V3DITKPluginDefaultHeader.h"

class SignedDanielssonDistanceMapPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(SignedDanielssonDistanceMap);
};

#endif
