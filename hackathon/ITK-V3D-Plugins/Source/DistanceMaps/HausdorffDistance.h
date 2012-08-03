#ifndef __HausdorffDistance_H__
#define __HausdorffDistance_H__

#include "V3DITKPluginDefaultHeader.h"

class HausdorffDistancePlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(HausdorffDistance);
};

#endif
