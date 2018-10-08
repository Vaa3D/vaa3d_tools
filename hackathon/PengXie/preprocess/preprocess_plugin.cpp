/* preprocess_plugin.cpp
 * Preprocess_neuron_for_analysis
 * 2018-7-27 : by PengXie
 */
 
#include "v3d_message.h"
#include "vector"
#include "preprocess_plugin.h"
#include "split_neuron_main.h"
#include "neurite_analysis_main.h"
#include "crop_swc_main.h"
#include "check_connection.h"

using namespace std;
Q_EXPORT_PLUGIN2(preprocess, neuron_analysis);
 
QStringList neuron_analysis::menulist() const
{
    return QStringList()
            <<tr("preprocess")
            <<tr("help")
            <<tr("about");
}

QStringList neuron_analysis::funclist() const
{
	return QStringList()
        <<tr("preprocess")
        <<tr("preprocess_batch")
        <<tr("split_neuron")
        <<tr("qc")
//        <<tr("get_main_component")
        <<tr("crop_swc")
        <<tr("check_connection")
        <<tr("help");
}

void neuron_analysis::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(menu_name == tr("preprocess"))
    {
        pre_processing_domenu(callback, parent);
    }
    if (menu_name == tr("help"))
    {
        v3d_msg(tr("This plugin performs several tasks: \n"
                   "1. Connect and sort the input SWC.\n"
                   "2. Generate statistics for quality control purposes.\n"
                   "3. Split SWC into different components for analysis purposes.\n"
                   "\n"
                   "Please select an SWC file.\n"
                   "Note: an apo file (showing soma location) with the same prefix is required.\n"
                   )
                );
    }
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

    if (func_name == tr("preprocess_batch"))
    {
        return (preprocess_batch_dofunc(input, output));
    }
    if (func_name == tr("preprocess"))
	{
        return (pre_processing_dofunc(input, output));
	}
    if (func_name == tr("split_neuron"))
    {
        return (split_neuron_dofunc(input, output));
    }
    if (func_name == tr("qc"))
    {
        return (qc_dofunc(input, output));
    }
    if (func_name == tr("neurite_analysis"))
    {
        return (neurite_analysis_dofunc(input, output));
    }
    if (func_name == "crop_swc")
    {
        return (crop_swc_dofunc(input, output));
    }
    if (func_name == "check_connection"){
        return (check_connection_dofunc(input, output));
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

