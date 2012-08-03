#ifndef __DirectedHausdorffDistance_H__
#define __DirectedHausdorffDistance_H__

#include "V3DITKPluginDefaultHeader.h"

class DirectedHausdorffDistancePlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(DirectedHausdorffDistance);
};

#endif
