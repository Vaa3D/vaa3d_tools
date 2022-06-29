/* neuronQC_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2021-5-18 : by zx
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuronQC_plugin.h"
#include "neuronqc_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(neuronQC, neuronQCPlugin);
 
QStringList neuronQCPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList neuronQCPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neuronQCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by ywli, 2022-3-12"));
	}
}

bool neuronQCPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("neuronQC_batch"))
	{
        QString swcDir = infiles.size()>=1 ? infiles[0] : "";
        QString csvPath = outfiles.size()>=1 ? outfiles[0] : "";

        float sLengthThres = inparas.size()>=1 ? atof(inparas[0]) : 10;
        float nodeLengthThres = inparas.size()>=2 ? atof(inparas[1]) : 15;
        bool loopThreeBifurcation = inparas.size()>=3 ? atoi(inparas[2]) : 1;

        ofstream csvFile;
        csvFile.open(csvPath.toStdString().c_str());

        getNeuronFeatureForBatch(swcDir,sLengthThres,nodeLengthThres,loopThreeBifurcation,csvFile);
        csvFile.close();

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

