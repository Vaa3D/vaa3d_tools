/* neuron_stitch_plugin.cpp
 * This plugin is for link neuron segments across stacks.
 * 2014-10-07 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_stitch_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(neuron_stitch, neuron_stitch);
 
QStringList neuron_stitch::menulist() const
{
	return QStringList() 
		<<tr("arrange_stacks")
		<<tr("auto_landmarks")
		<<tr("about");
}

QStringList neuron_stitch::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neuron_stitch::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("arrange_stacks"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("auto_landmarks"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This plugin is for link neuron segments across stacks.. "
			"Developed by Hanbo Chen, 2014-10-07"));
	}
}

bool neuron_stitch::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

