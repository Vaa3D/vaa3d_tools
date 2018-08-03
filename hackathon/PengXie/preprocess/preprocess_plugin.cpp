/* preprocess_plugin.cpp
 * Preprocess_neuron_for_analysis
 * 2018-7-27 : by PengXie
 */
 
#include "v3d_message.h"
#include <vector>
#include "preprocess_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(preprocess, neuron_analysis);
 
QStringList neuron_analysis::menulist() const
{
    return QStringList()
        <<tr("about");
}

QStringList neuron_analysis::funclist() const
{
	return QStringList()
        <<tr("preprocess")
        <<tr("get_main_component")
        <<tr("add_soma")
        <<tr("help");
}

void neuron_analysis::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
    {
		v3d_msg(tr("Preprocess_neuron_for_analysis. "
			"Developed by PengXie, 2018-7-27"));
	}
}

bool neuron_analysis::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("preprocess"))
	{
        return (pre_processing_main(input, output));
	}
//    else if (func_name == tr("get_main_component"))
//    {
//        return (get_main_component(input, output));
//    }
    else if (func_name == tr("add_soma"))
    {
        return (add_soma(input, output));
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

