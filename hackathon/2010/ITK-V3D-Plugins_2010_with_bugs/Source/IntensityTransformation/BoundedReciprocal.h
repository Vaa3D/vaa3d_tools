#ifndef __BoundedReciprocal_H__
#define __BoundedReciprocal_H__

#include "V3DITKPluginDefaultHeader.h"

class BoundedReciprocalPlugin : public QObject, public V3DPluginInterface
{
  Q_OBJECT
  Q_INTERFACES(V3DPluginInterface)
  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(BoundedReciprocal);
};

#endif
