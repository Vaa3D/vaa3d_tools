/* consensus_skeleton_plugin.cpp
 * a plugin to merge multiple neurons by generating a consensus skeleton
 * 2012-05-02 : by Yinan Wan
 */
 
#include "v3d_message.h"

#include "consensus_skeleton_plugin.h"
#include "consensus_skeleton_func.h"
 
Q_EXPORT_PLUGIN2(consensus_skeleton, ConsSkelPlugin);
 
QStringList ConsSkelPlugin::menulist() const
{
	return QStringList() 
        <<tr("consensus_swc")
		<<tr("about");
}

QStringList ConsSkelPlugin::funclist() const
{
	return QStringList()
        <<tr("consensus_swc")
        <<tr("median_swc")
        <<tr("average_node_position")
		<<tr("help");
}

void ConsSkelPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("consensus_swc"))
	{
        consensus_swc_menu(callback,parent);
	}
	else if (menu_name == tr("help"))
	{
		v3d_msg(tr("a plugin to merge multiple neurons by generating a consensus skeleton. "));
	}
	else
	{
		v3d_msg(tr("a plugin to merge multiple neurons by generating a consensus skeleton. "));
	}
}

bool ConsSkelPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("consensus_swc"))
	{
        return consensus_swc_func(input, output, callback);
	}
    if (func_name == tr("median_swc"))
    {
        median_swc_func(input, output);
    }
    if (func_name == tr("average_node_position"))
    {
        average_node_position_func(input, output);
    }
    if (func_name == tr("vote_map"))
    {
        vote_map_func(input, output, callback);
    }
    else
	{
		printHelp();
	}
    return true;
}

