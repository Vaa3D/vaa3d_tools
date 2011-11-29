#ifndef __RelabelObjects_H__
#define __RelabelObjects_H__

#include "V3DITKPluginDefaultHeader.h"

class RelabelObjectsPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(RelabelObjects);
};

#endif
