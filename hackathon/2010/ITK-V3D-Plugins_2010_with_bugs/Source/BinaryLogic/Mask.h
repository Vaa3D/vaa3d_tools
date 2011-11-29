#ifndef __Mask_H__
#define __Mask_H__

#include "V3DITKPluginDefaultHeader.h"

class MaskPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Mask);
};

#endif
