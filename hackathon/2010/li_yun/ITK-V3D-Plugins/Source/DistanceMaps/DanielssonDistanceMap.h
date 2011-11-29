#ifndef __DanielssonDistanceMap_H__
#define __DanielssonDistanceMap_H__

#include "V3DITKPluginDefaultHeader.h"

class DanielssonDistanceMapPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(DanielssonDistanceMap);
};

#endif
