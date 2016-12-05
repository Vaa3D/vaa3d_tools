/* blastneuron_bjut_plugin.cpp
 * Compare neuron constructions using blastneuron method.
 * 2016-11-30 : by He Yishan
 */
 
#include "v3d_message.h"
#include <vector>
#include "blastneuron_bjut_plugin.h"
#include "blastneuron_bjut_func.h"
using namespace std;
Q_EXPORT_PLUGIN2(blastneuron_bjut, blastneuron_bjutPlugin);
 
QStringList blastneuron_bjutPlugin::menulist() const
{
	return QStringList() 
        <<tr("apply_blastneuron_bjut")
		<<tr("about");
}

QStringList blastneuron_bjutPlugin::funclist() const
{
	return QStringList()
        <<tr("apply_blastneuron_bjut")
		<<tr("help");
}

void blastneuron_bjutPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("apply_blastneuron_bjut"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("Compare neuron constructions using blastneuron method.. "
			"Developed by He Yishan, 2016-11-30"));
	}
}

bool blastneuron_bjutPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("apply_blastneuron_bjut"))
	{
        return blast_bjut_func(input,output);
	}
	else if (func_name == tr("help"))
	{
        printf("This is help!\n");
        v3d_msg("To be implemented.",0);
        return true;
	}
	else return false;

}

