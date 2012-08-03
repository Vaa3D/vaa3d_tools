#ifndef __ExpNegative_H__
#define __ExpNegative_H__

#include "V3DITKPluginDefaultHeader.h"

class ExpNegativePlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(ExpNegative);
};

#endif
