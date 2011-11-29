/* ct3d_plugin.cpp
 * This plugin will invoke ct3d program, which track and segment cells from more than two images
 * 2011-06-27 : by Hang Xiao & Axel Mosig
 */
 
#include "v3d_message.h"

#include "ct3d_plugin.h"
#include "ct3d_func.h"
 
Q_EXPORT_PLUGIN2(ct3d, Ct3dPlugin);
 
QStringList Ct3dPlugin::menulist() const
{
	return QStringList()
		<<tr("open ct3d")
		<<tr("about");
}

void Ct3dPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("open ct3d"))
	{
		open_ct3d(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin will invoke ct3d program, which track and segment cells from more than two images. "
			"Developed by Hang Xiao & Axel Mosig, 2011-06-27"));
	}
}

