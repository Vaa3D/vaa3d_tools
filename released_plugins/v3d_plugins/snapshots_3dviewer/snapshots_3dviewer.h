#ifndef __PLUGIN_SNAPSHOTS_3DVIEWER_H__
#define __PLUGIN_SNAPSHOTS_3DVIEWER_H__

#include <QtGui>
#include "v3d_interface.h"

class SnapShots_3Dviewer: public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

    QStringList funclist() const {return QStringList();}
    bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
    {return false;}
    float getPluginVersion() const {return 1.1f;}
};


#endif

