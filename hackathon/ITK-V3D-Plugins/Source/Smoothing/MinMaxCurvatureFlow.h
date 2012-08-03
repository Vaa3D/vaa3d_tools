#ifndef __MinMaxCurvatureFlow_H__
#define __MinMaxCurvatureFlow_H__

#include "V3DITKPluginDefaultHeader.h"

class MinMaxCurvatureFlowPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(MinMaxCurvatureFlow);
};

#endif
