#ifndef __Asin_H__
#define __Asin_H__

#include "V3DITKPluginDefaultHeader.h"

class AsinPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Asin);
};

#endif
