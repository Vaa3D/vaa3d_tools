/* ct3d_plugin.h
 * This plugin will invoke ct3d program, which track and segment cells from more than two images
 * 2011-06-27 : by Hang Xiao & Axel Mosig
 */
 
#ifndef __CT3D_PLUGIN_H__
#define __CT3D_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class Ct3dPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
	{return false;}
};

#endif

