#ifndef __Add_H__
#define __Add_H__

#include "V3DITKPluginDefaultHeader.h"

class AddPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Add);
};

#endif
