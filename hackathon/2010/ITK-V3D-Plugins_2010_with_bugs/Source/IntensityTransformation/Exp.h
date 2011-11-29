#ifndef __Exp_H__
#define __Exp_H__

#include "V3DITKPluginDefaultHeader.h"

class ExpPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Exp);
};

#endif
