/* NeuroMorphoLib_plugin.cpp
 * functions for processing neuron morphology
 * 2022-8-9 : by SD-Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "NeuroMorphoLib_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(NeuroMorphoLib, NMorphoPlugin);
 
QStringList NMorphoPlugin::menulist() const
{
	return QStringList() 
		<<tr("None")
		<<tr("about");
}

QStringList NMorphoPlugin::funclist() const
{
	return QStringList()
            <<tr("lm_feas")
           <<tr("branch_feas")
         <<tr("swc2branches")
        <<tr("crop_swc_terafly_image_block")
        <<tr("bswcTo")
       <<tr("neuron_split")
        <<tr("qc")
       << tr("nt_check")
       <<tr("crop_local_swc")
      <<tr("somalist_in_folder")
       <<tr("help");
}

void NMorphoPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("None"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("functions for processing neuron morphology. "
			"Developed by SD-Jiang, 2022-8-9"));
	}
}

bool NMorphoPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("lm_feas"))
	{
        return lm_statistic_features(callback,input,output);
	}
    else if (func_name == tr("branch_feas"))
    {
        return branch_features(callback,input,output);
    }
    else if (func_name == tr("swc2branches"))
    {
        return swc2branches(callback,input,output);
    }
    else if (func_name == tr("neuron_split"))
    {
        return neuron_split(callback,input,output);
    }
    else if (func_name == tr("bswcTo"))
    {
        return bswcTo(callback,input,output);
    }
    else if (func_name == tr("bouton_distribution"))
    {
        return bouton_distribution(callback,input,output);
    }
    else if (func_name == tr("qc"))
    {
        return nt_qc(callback,input,output);
    }
    else if (func_name == tr("nt_check"))
    {
        return nt_check(callback,input,output);
    }
    else if (func_name == tr("swc_ada_sampling"))
    {
        return swc_ada_sampling(callback,input,output);
    }
    else if(func_name==tr("crop_local_swc"))
    {
        return crop_local_swc(callback,input,output);
    }
    else if (func_name == tr("somalist_in_folder"))
    {
        return somalist_in_folder(callback,input,output);
    }
    else if (func_name == tr("crop_swc_terafly_image_block"))
    {
        return crop_swc_terafly_image_block(callback,input,output);
    }
    else if (func_name == tr("swc_parallization"))
    {
        return swc_parallization(callback,input,output);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

