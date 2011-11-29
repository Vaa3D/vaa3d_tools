#ifndef __ContourDirectedMeanDistance_H__
#define __ContourDirectedMeanDistance_H__

#include "V3DITKPluginDefaultHeader.h"

class ContourDirectedMeanDistancePlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(ContourDirectedMeanDistance);
};

#endif
