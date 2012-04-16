/* eswc_converter_plugin.cpp
 * This is a plugin to convert between swc and eswc.
 * 2012-02-16 : by Yinan Wan
 */
 
#include "v3d_message.h"

#include "eswc_converter_plugin.h"
#include "eswc_converter_func.h"
 
Q_EXPORT_PLUGIN2(eswc_converter, ESWCPlugin);
 
QStringList ESWCPlugin::menulist() const
{
	return QStringList() 
		<<tr("swc_to_eswc")
		<<tr("eswc_to_swc")
		<<tr("check_eswc_format")
		<<tr("about");
}

QStringList ESWCPlugin::funclist() const
{
	return QStringList()
		<<tr("swc2eswc_io")
		<<tr("eswc2swc_io")
		<<tr("check_eswc_io");
}

void ESWCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("swc_to_eswc"))
	{
		swc2eswc_io(callback,parent);
	}
	else if (menu_name == tr("eswc_to_swc"))
	{
		eswc2swc_io(callback,parent);
	}
	else if (menu_name == tr("check_eswc_format"))
	{
		check_eswc_io(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a plugin to convert between swc and eswc. "
			"Developed by Yinan Wan, 2012-02-16"));
	}
}

bool ESWCPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("swc_to_eswc"))
	{
		return swc2eswc_io(input, output);
	}
	else if (func_name == tr("eswc_to_swc"))
	{
		return eswc2swc_io(input,output);
	}
	else if (func_name == tr("check_eswc_format"))
	{
		return check_eswc_io(input,output);
	}
	else if (func_name == tr("TOOLBOXswc_to_eswc"))
	{
		swc2eswc_toolbox(input);
	}
	else if (func_name == tr("TOOLBOXeswc_to_swc"))
	{
		eswc2swc_toolbox(input);
	}
	else if (func_name == tr("TOOLBOXcheck_eswc_format"))
	{
		check_eswc_toolbox(input);
	}
}

