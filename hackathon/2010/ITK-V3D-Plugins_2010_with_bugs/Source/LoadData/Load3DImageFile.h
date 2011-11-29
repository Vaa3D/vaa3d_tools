#ifndef __Load3DImage_H__
#define __Load3DImage_H__

#include "V3DITKPluginDefaultHeader.h"

class Load3DImageFilePlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Load3DImageFile);
};

#endif
