/*
 *  sort_func.cpp
 *  io functions for domenu and do func 
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Last change: Wan, Yinan, on 06/23/11.
 *  01/31/12: change the overall plugin organization. enable processing of .ano file, add threshold parameter
 */
 
#include "v3d_message.h"

#include "sort_plugin.h"
#include "sort_func.h"
 
Q_EXPORT_PLUGIN2(sort_neuron_swc, SORTPlugin);
 
QStringList SORTPlugin::menulist() const
{
	return QStringList()
		<<tr("sort_swc")
		<<tr("about");
}

QStringList SORTPlugin::funclist() const
{
	return QStringList()
		<<tr("sort_swc")
		<<tr("help");
}

void SORTPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("sort_swc"))
	{
		sort_menu(callback,parent);
	}
	else if (menu_name == tr("help"))
	{
		printHelp(callback,parent);
	}
	else
	{
		printHelp(callback,parent);
	}
}

bool SORTPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("sort_swc"))
	{
		sort_func(input, output);
	}
	else if (func_name == tr("help"))
	{
		printHelp(input,output);
	}
	else if (func_name == tr("TOOLBOXsort_swc"))
	{
		sort_toolbox(input);
	}
}

