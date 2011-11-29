#ifndef __Save3DImageFile_H__
#define __Save3DImageFile_H__

#include "V3DITKPluginDefaultHeader.h"

class Save3DImageFilePlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(Save3DImageFile);
};

#endif
