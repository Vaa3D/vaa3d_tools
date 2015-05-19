/* Opencv_example_plugin.cpp
 * This plugin will load image and swc in domenu and dofunc
 * 2012-02-21 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "Opencv_example_plugin.h"
#include "Opencv_example_func.h"
 
Q_EXPORT_PLUGIN2(Opencv_example, LoadImageAndSWCPlugin);
 
QStringList LoadImageAndSWCPlugin::menulist() const
{
	return QStringList() 
		<<tr("Opencv_example")
		<<tr("about");
}

QStringList LoadImageAndSWCPlugin::funclist() const
{
	return QStringList()
		<<tr("Opencv_example");
}

void LoadImageAndSWCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Opencv_example"))
	{
		Opencv_example(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin will load image and swc in domenu and dofunc. "
			"Developed by Hang Xiao, 2012-02-21"));
	}
}

bool LoadImageAndSWCPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("Opencv_example"))
	{
		return Opencv_example(input, output);
	}
}

