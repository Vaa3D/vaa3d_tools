/* neuron_toolbox_plugin.cpp
 * This is a super plugin that gather all sub-plugins related to neuron structure processing
 * 2012-04-06 : by Yinan Wan
 */
#include <iostream> 
#include "v3d_message.h"

#include "neuron_toolbox_plugin.h"
#include "neuron_toolbox_func.h"
using namespace std;

Q_EXPORT_PLUGIN2(neuron_toolbox, NeuronToolboxPlugin);
 
QStringList NeuronToolboxPlugin::menulist() const
{
	return QStringList() 
		<<tr("Neuron Toolbox")
		<<tr("about");
}

QStringList NeuronToolboxPlugin::funclist() const
{
	return QStringList()
		<<tr("help");
}

void NeuronToolboxPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Neuron Toolbox"))
	{
		neuron_toolbox_func(callback, parent);
	}
	else if (menu_name == tr("help"))
		help(true);
	else
        help(true);
}

bool NeuronToolboxPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
//	cout<<"callback_dofunc="<<(void *)&callback<<endl;
//	callback.getImageWindowList();

	if (func_name == tr("neuron_toolbox"))
	{
		neuron_toolbox_func(callback, parent, input, output);
	}
	else if (func_name == tr("help"))
	{
		help(false);
	}

    return true;
}

