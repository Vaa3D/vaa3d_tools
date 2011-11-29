#ifndef V3D_PLUGIN_CHANNEL_MANAGER_H_
#define V3D_PLUGIN_CHANNEL_MANAGER_H_

#include "v3d_interface.h"

class ChannelManagerPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
        QStringList menulist() const;
        void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,  QWidget * parent);

        QStringList funclist() const {return QStringList();}
        bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                                 V3DPluginCallback2 & v3d,  QWidget * parent) {return true;}

        float getPluginVersion() const {return 0.1f;}
};

#endif // V3D_PLUGIN_CHANNEL_MANAGER_H_

