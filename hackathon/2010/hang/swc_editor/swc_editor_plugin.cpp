/* swc_editor_plugin.cpp
 * This is plugin is used to editor swc file
 * 2011-07-08 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "swc_editor_plugin.h"
#include "swc_editor_func.h"
 
Q_EXPORT_PLUGIN2(swc_editor, SWCEditorPlugin);
 
QStringList SWCEditorPlugin::menulist() const
{
	return QStringList()
		<<tr("open swc editor")
		<<tr("create network swc")
		<<tr("about");
}

void SWCEditorPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("open swc editor"))
	{
		open_sec_editor(callback,parent);
	}
	else if (menu_name == tr("create network swc"))
	{
		create_network_swc(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is plugin is used to editor swc file. "
			"Developed by Hang Xiao, 2011-07-08"));
	}
}

