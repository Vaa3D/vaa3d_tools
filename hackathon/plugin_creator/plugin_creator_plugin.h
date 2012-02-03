/* plugin_creator_plugin.h
 * This plugin is used to produce v3d plugin project from a template file
 * 2012-01-27 : by Hang Xiao
 */
 
#ifndef __PLUGIN_CREATOR_PLUGIN_H__
#define __PLUGIN_CREATOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class PluginCreatorPlugin : public QObject, public V3DPluginInterface2_1
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

