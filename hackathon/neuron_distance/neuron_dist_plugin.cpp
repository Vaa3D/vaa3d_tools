/* neuron_dist_plugin.cpp
 * The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.
 * 2012-05-04 : by Yinan Wan
 */
 
#include "v3d_message.h"

#include "neuron_dist_plugin.h"
#include "neuron_dist_func.h"
 
Q_EXPORT_PLUGIN2(neuron_dist, NeuronDistPlugin);
 
QStringList NeuronDistPlugin::menulist() const
{
	return QStringList() 
		<<tr("neuron_dist")
		<<tr("about");
}

QStringList NeuronDistPlugin::funclist() const
{
	return QStringList()
		<<tr("neuron_dist_io")
		<<tr("help");
}

void NeuronDistPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neuron_dist"))
	{
		neuron_dist_io(callback,parent);
	}
	else
	{
		v3d_msg(tr("The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.. "
			"Developed by Yinan Wan, 2012-05-04"));
	}
}

bool NeuronDistPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("neuron_dist_io"))
	{
		return neuron_dist_io(input, output);
	}
	else if (func_name == tr("help"))
	{
		return help(input,output);
	}
}

