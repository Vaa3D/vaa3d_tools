#ifndef __Sigmoid_H__
#define __Sigmoid_H__

#include "V3DITKPluginDefaultHeader.h"

class SigmoidPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Sigmoid);
};

#endif
