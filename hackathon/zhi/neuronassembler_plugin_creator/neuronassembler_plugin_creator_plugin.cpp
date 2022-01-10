/* plugin_creator_plugin.cpp
 * This plugin is used to produce v3d plugin project from a template file
 * 2012-01-27 : by Hang Xiao
 */
 
#include "v3d_message.h"

#include "neuronassembler_plugin_creator_plugin.h"
#include "plugin_creator_func.h"

Q_EXPORT_PLUGIN2(neuronassembler_plugin_creator, NeuronassemblerPluginCreatorPlugin);

QStringList NeuronassemblerPluginCreatorPlugin::menulist() const
{
	return QStringList()
        <<tr("create neuronassembler plugin")
		<<tr("about");
}

void NeuronassemblerPluginCreatorPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("create neuronassembler plugin"))
	{
        create_plugin(callback,parent);
	}
    else
	{
        v3d_msg(tr("This plugin is used to produce v3d Neuronassembler plugin project from a template file. "
            "Developed by Zhi Zhou, 2014-12-01"));
	}
}
