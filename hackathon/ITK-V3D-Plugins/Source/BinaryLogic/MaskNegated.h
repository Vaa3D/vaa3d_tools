#ifndef __MaskNegated_H__
#define __MaskNegated_H__

#include "V3DITKPluginDefaultHeader.h"

class MaskNegatedPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(MaskNegated);
};

#endif
