/* swcPruning_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-9-4 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "swcPruning_plugin.h"

#include "pruning.h"

using namespace std;
Q_EXPORT_PLUGIN2(swcPruning, swcPruningPlugin);
 
QStringList swcPruningPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList swcPruningPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void swcPruningPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by ZX, 2020-9-4"));
	}
}

bool swcPruningPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("pruningCross"))
	{
        QString swcPath = infiles[0];
        QString swcFalsePath = swcPath + "_false.swc";
        QString swcPrunedPath = swcPath + "_crossPruned.swc";
        NeuronTree nt = readSWC_file(swcPath);

        double bifurcationD = inparas.size()>=1 ? atof(inparas[0]) : 10;

        pruningCross3(nt,bifurcationD);

        writeSWC_file(swcFalsePath,nt);

        NeuronTree outnt = pruningByType(nt,2);
        writeSWC_file(swcPrunedPath,outnt);

	}
    else if (func_name == tr("pruningSoma"))
	{
        QString swcPath = infiles[0];
        QString swcPrunedPath = swcPath + "_somaPruned.swc";
        double times = inparas.size()>=1 ? atof(inparas[0]) : 5;
        NeuronTree nt = readSWC_file(swcPath);
        NeuronTree outnt = pruningSoma(nt,times);
        writeSWC_file(swcPrunedPath,outnt);
    }else if (func_name == tr("pruningByLength")) {
        QString swcPath = infiles[0];
        QString swcPrunedPath = swcPath + "_pruned.swc";
        double length = inparas.size()>=1 ? atof(inparas[0]) : 5;
        NeuronTree nt = readSWC_file(swcPath);
        NeuronTree outnt = pruningByLength(nt,length);
        writeSWC_file(swcPrunedPath,outnt);
    }else if (func_name == tr("pruning")) {
        QString swcPath = infiles[0];
        QString swcPrunedPath = swcPath + "_pruned.swc";

        double bifurcationD = inparas.size()>=1 ? atof(inparas[0]) : 10;
        double somaTimes = inparas.size()>=2 ? atof(inparas[1]) : 6;

        NeuronTree nt = readSWC_file(swcPath);
        if(somaTimes>0){
            NeuronTree tmpnt1 = pruningSoma(nt,somaTimes);
            nt.listNeuron.clear();
            nt.hashNeuron.clear();
            nt.deepCopy(tmpnt1);
        }

        pruningCross3(nt,bifurcationD);
        NeuronTree tmpnt = pruningByType(nt,2);
        NeuronTree outnt = pruningByLength(tmpnt,5);

        writeSWC_file(swcPrunedPath,outnt);

    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

