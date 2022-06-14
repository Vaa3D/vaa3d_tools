/* SuperPlugin_plugin.cpp
 * A
data production line.
 * 2022-4-26 : by csz,dlc,ljs
 */
 
#include "v3d_message.h"
#include <vector>
#include "SuperPlugin_plugin.h"
using namespace std;
//Q_EXPORT_PLUGIN2(SuperPlugin, SuperPlugin);
 
QStringList SuperPlugin::menulist() const
{
	return QStringList() 
		<<tr("autoproduce")
		<<tr("preprocess")
		<<tr("computation")
		<<tr("postprocess")
		<<tr("about");
}

QStringList SuperPlugin::funclist() const
{
	return QStringList()
		<<tr("autoproduce")
        <<tr("batchrun")
        <<tr("usrdesigned")
		<<tr("postprocess")
		<<tr("help");
}

void SuperPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("autoproduce"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("preprocess"))
	{
        this->spui=new SuperUI(callback,parent);
        this->spui->show();
        //v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("computation"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("postprocess"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
        v3d_msg(tr("A data production line.. "
			"Developed by csz,dlc,ljs, 2022-4-26"));
	}
}

bool SuperPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("autoproduce"))
	{
        this->spui=new SuperUI(callback,input,output,"autoproduce");
        this->spui->assemblyline();
	}
    else if (func_name == tr("batchrun"))
	{
        this->spui=new SuperUI(callback,input,output,"batchrun");
        this->spui->batchrun();
	}
    else if (func_name == tr("usrdesigned"))
	{
        this->spui=new SuperUI(callback,input,output,"usrdesigned");
        //this->spui->cmd->acceptcmd(input,output);
//        v3d_msg("To be implemented.");
	}
	else if (func_name == tr("postprocess"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
        printf("\n***************************************************************\n");
        printf("**** Usage of SuperPlugin ****\n");
        printf("vaa3d -x SuperPlugin.dll -f autoproduce -i inputfilepath -o outputfilepath -p R//T img//swc func1 para1_1 para1_2 ... func2 para2_1 para2...\n");
        printf("Currently, the following functions are supported:\n");
        printf("gf                          app2            im_sigma_correction     im_subtract_minimum     im_bilateral_filter\n");
        printf("im_fft_filter               im_grey_morph   im_enhancement          gsdt                    cropTerafly\n");
        printf("he(HistogramEqualization)   standardize     dtc                     resample_swc            sort_swc\n");
        printf("The parameters must match their respective functions.\n");
        printf("The first letter of a function must be lowercase.\n");
        printf("For example:\n");
        printf("vaa3d -x SuperPlugin.dll -f autoproduce -i inputfilepath -o outputfilepath -p R img gf 7 7 7 1 3 he app2 NULL 0 AUTO\n");
        printf("*****************************************************************\n");
	}
	else return false;

	return true;
}

