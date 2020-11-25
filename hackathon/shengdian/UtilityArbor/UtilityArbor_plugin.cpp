/* UtilityArbor_plugin.cpp
 * Generation of the utility arbors
 * 2020-9-24 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include "UtilityArbor_plugin.h"
#include "Utility_function.h"
using namespace std;
Q_EXPORT_PLUGIN2(UtilityArbor, UtilityArbor);
 
QStringList UtilityArbor::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("about");
}

QStringList UtilityArbor::funclist() const
{
	return QStringList()
		<<tr("UtilityRendering")
		<<tr("UtilityL1")
		<<tr("help");
}

void UtilityArbor::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("Generation of the utility arbors. "
			"Developed by Shengdian, 2020-9-24"));
	}
}

bool UtilityArbor::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("UtilityRendering"))
	{
        qDebug()<<"Utility rendering of Neuron";
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        int maxR = (inparas.size() >= 1) ? atoi(inparas[0]) : 100;
        double axonRatio = (inparas.size()>=2) ? atof(inparas[1]) : 1;
        double otherR = (inparas.size()>=3) ? atof(inparas[2]) : 1;
        double thre = (inparas.size()>=4) ? atof(inparas[3]) : 1;
        getNodeLength(nt,maxR,axonRatio,otherR,thre);
        writeSWC_file(swcfile.split(".").at(0)+"_max_"+QString::number(maxR)+"_UtilityResult.swc",nt);
	}
	else if (func_name == tr("UtilityL1"))
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

