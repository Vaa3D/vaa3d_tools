/* v3dneuron_tracing_plugin.cpp
 * This plugin will call v3dneuron_tracing command in dofunc
 * 2011-07-07 : by xiaoh10
 */
 
#include "v3d_message.h"

#include "v3dneuron_tracing_plugin.h"
#include "v3dneuron_tracing_func.h"
 
Q_EXPORT_PLUGIN2(v3dneuron_tracing, V3dneuron_tracingPlugin);
 
QStringList V3dneuron_tracingPlugin::menulist() const
{
	return QStringList()
		<<tr("invoke v3dneuron_tracing")
		<<tr("about");
}

QStringList V3dneuron_tracingPlugin::funclist() const
{
	return QStringList()
		<<tr("v3dneuron_tracing");
}

void V3dneuron_tracingPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("invoke v3dneuron_tracing"))
	{
		v3dneuron_tracing(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin will call v3dneuron_tracing command in dofunc. "
			"Developed by xiaoh10, 2011-07-07"));
	}
}

bool V3dneuron_tracingPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("v3dneuron_tracing"))
	{
		return v3dneuron_tracing(input, output);
	}
}

