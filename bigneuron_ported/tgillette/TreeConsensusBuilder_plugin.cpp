/* TreeConsensusBuilder_plugin.cpp
 * Given multiple reconstructions of the same neuron, produces a consensus given a confidence threshold (some proportion of input reconstructions voting for each branch).
 * 2015-11-19 : by Todd Gillette
 */
 
#include "v3d_message.h"
#include <vector>
#include "TreeConsensusBuilder_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(TreeConsensusBuilder, TreeConsensusBuilderPlugin);
 
QStringList TreeConsensusBuilderPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TreeConsensusBuilderPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TreeConsensusBuilderPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("Given multiple reconstructions of the same neuron, produces a consensus given a confidence threshold (some proportion of input reconstructions voting for each branch).. "
			"Developed by Todd Gillette, 2015-11-19"));
	}
}

bool TreeConsensusBuilderPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

