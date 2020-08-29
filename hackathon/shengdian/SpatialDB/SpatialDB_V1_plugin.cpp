/* SpatialDB_V1_plugin.cpp
 * Split Neuron_SWC into Branch List
 * 2020-4-24 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include "SpatialDB_V1_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(SpatialDB_V1, SpatialDB);
 
QStringList SpatialDB::menulist() const
{
	return QStringList() 
		<<tr("generateBranchList")
		<<tr("about");
}

QStringList SpatialDB::funclist() const
{
	return QStringList()
		<<tr("Info")
		<<tr("help");
}

void SpatialDB::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("generateBranchList"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("Split Neuron_SWC into Branch List. "
			"Developed by Shengdian, 2020-4-24"));
	}
}

bool SpatialDB::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("Info"))
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

