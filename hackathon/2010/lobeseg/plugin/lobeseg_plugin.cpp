/* lobeseg_plugin.cpp
 * This plugin sperate the two optic lobes (OLs) and the center brain (CB) of fluit fly brain. Or seperate just one lobe and the center brain with suitable parameters.
 * June 20, 2011 : by Hanchuan Peng and Hang Xiao
 */
 
#include "v3d_message.h"

#include "lobeseg_plugin.h"
#include "lobeseg_func.h"
 
Q_EXPORT_PLUGIN2(lobeseg, LobesegPlugin);
 
QStringList LobesegPlugin::menulist() const
{
	return QStringList()
		<<tr("two sides segmentation")
		<<tr("one side only segmentation")
		<<tr("about");
}

QStringList LobesegPlugin::funclist() const
{
	return QStringList()
		<<tr("lobeseg");
}

void LobesegPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("two sides segmentation"))
	{
		lobeseg_two_sides(callback,parent);
	}
	else if (menu_name == tr("one side only segmentation"))
	{
		lobeseg_one_side_only(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin sperate the two optic lobes (OLs) and the center brain (CB) of fluit fly brain. Or seperate just one lobe and the center brain with suitable parameters.. "
			"Developed by Hanchuan Peng and Hang Xiao, June 20, 2011"));
	}
}

bool LobesegPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("lobeseg"))
	{
		return lobeseg(input, output);
	}
}

