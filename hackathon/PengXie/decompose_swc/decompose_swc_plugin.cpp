/* decompose_swc_plugin.cpp
 * decompose an swc file into segments
 * 2018-11-19 : by Peng Xie
 */
 
#include "v3d_message.h"
#include <vector>
#include "decompose_swc_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(decompose_swc, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("to_multiple_swc")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
		v3d_msg(tr("decompose an swc file into segments. "
			"Developed by Peng Xie, 2018-11-19"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("to_multiple_swc"))
	{
        QString input_swc=QString(infiles.at(0));
        QString output_folder=QString(outfiles.at(0));
        NeuronTree nt = readSWC_file(input_swc);
        decompose_to_multiple_swc(nt, output_folder);
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

