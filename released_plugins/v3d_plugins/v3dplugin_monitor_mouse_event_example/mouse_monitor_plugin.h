/* mouse_monitor_plugin.h
 * This is a demo plugin to monitor the mouse event
 * 2011-06-29 : by Hang Xiao
 */
 
#ifndef __MOUSE_MONITOR_PLUGIN_H__
#define __MOUSE_MONITOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class MouseMonitorPlugin : public QObject, public V3DPluginInterface2_1
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

