/* mouse_monitor_plugin.cpp
 * This is a demo plugin to monitor the mouse event
 * 2011-06-29 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "mouse_monitor_plugin.h"
#include "mouse_monitor_func.h"
 
Q_EXPORT_PLUGIN2(mouse_monitor, MouseMonitorPlugin);
 
QStringList MouseMonitorPlugin::menulist() const
{
	return QStringList()
		<<tr("start monitor")
		<<tr("about");
}

void MouseMonitorPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("start monitor"))
	{
		start_monitor(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a demo plugin to monitor the mouse event. "
			"Developed by Hang Xiao, 2011-06-29"));
	}
}

