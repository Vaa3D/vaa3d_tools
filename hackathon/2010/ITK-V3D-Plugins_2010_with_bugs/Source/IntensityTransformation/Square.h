#ifndef __Square_H__
#define __Square_H__

#include "V3DITKPluginDefaultHeader.h"

class SquarePlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Square);
};

#endif
