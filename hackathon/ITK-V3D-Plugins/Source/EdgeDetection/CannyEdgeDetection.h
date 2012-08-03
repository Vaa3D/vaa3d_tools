#ifndef __CannyEdgeDetection_H__
#define __CannyEdgeDetection_H__

#include "V3DITKPluginDefaultHeader.h"

class CannyEdgeDetectionPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(CannyEdgeDetection);
};

#endif
