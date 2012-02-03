/* plugin_creator_plugin.cpp
 * This plugin is used to produce v3d plugin project from a template file
 * 2012-01-27 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "plugin_creator_plugin.h"
#include "plugin_creator_func.h"
 
Q_EXPORT_PLUGIN2(plugin_creator, PluginCreatorPlugin);
 
QStringList PluginCreatorPlugin::menulist() const
{
	return QStringList()
		<<tr("create plugin")
		<<tr("about");
}

void PluginCreatorPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("create plugin"))
	{
		create_plugin(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin is used to produce v3d plugin project from a template file. "
			"Developed by Hang Xiao, 2012-01-27"));
	}
}

