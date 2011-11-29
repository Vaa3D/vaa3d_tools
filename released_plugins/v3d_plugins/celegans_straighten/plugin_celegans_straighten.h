//celegans straighten plugin
//by Lei Qu
//2010-08-09

#ifndef __PLUGIN_CELEGANS_STRAIGHTEN_H__
#define __PLUGIN_CELEGANS_STRAIGHTEN_H__

#include <v3d_interface.h>


class CElegansStraightenPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
};



#endif
