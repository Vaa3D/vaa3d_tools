/* custom_toolbar_plugin.cpp
 * This plugin is used to define the toolbox by yourself
 * 2011-08-04 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "custom_toolbar_plugin.h"
#include "custom_toolbar_func.h"
#include "v3d_custom_toolbar.h"
 
Q_EXPORT_PLUGIN2(custom_toolbar, CustomToolbarPlugin);

QStringList CustomToolbarPlugin::menulist() const
{
	return QStringList()
		<<tr("custom toolbar")
		<<tr("about");
}

void CustomToolbarPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("custom toolbar"))
	{
		custom_toolbar(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin is used to define the toolbar by yourself. "
					"Developed by Hang Xiao, 2011-08-04"));
	}
}

