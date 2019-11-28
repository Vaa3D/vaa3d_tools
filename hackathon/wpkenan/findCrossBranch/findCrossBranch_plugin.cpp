/* findCrossBranch_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-11-5 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "findCrossBranch_plugin.h"
#include "iostream"
#include "findCrossBranch.h"
using namespace std;
Q_EXPORT_PLUGIN2(findCrossBranch, TestPlugin);
 
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
		<<tr("func1")
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
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-11-5"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		NeuronTree nt1 = readSWC_file(infiles[0]);
		cout << infiles[0] << endl;
		cout << inparas[0] << endl;

		SwcTree st;
		st.filePath = infiles[0];
		st.nt = nt1;
		st.searchRadius = atoi(inparas[0]);
		cout << "st.searchRadius: " << st.searchRadius << endl;

		int crossType1, crossType2;
		crossType1 = 0;
		crossType2 = 0;
		st.initialize();
		int crossPointsNum = st.crossBranchDetect(crossType1, crossType2);


		NeuronTree ntNoneCrossBranch;
		ntNoneCrossBranch.listNeuron.clear();
		ntNoneCrossBranch.hashNeuron.clear();

		st.crossBranchDeal(ntNoneCrossBranch);
		QString eswcfile = (outfiles.size() >= 1) ? outfiles[0] : "";
		QString markerfile = (outfiles.size() >= 2) ? outfiles[1] : "";
		
		
		markerfile = markerfile + "{" + QString::number(crossType1)+"_" + QString::number(crossType2) + "_" + QString::number(crossPointsNum)+"}" + ".marker";
		//writeESWC_file(eswcfile, st.nt);
		writeESWC_file(eswcfile, st.nt);
		writeESWC_file(eswcfile+"delete.eswc", ntNoneCrossBranch);
		writeMarker_file(markerfile, st.listMarker);

		
		

	}
	else if (func_name == tr("cutSwc2Blocks"))
	{
		/*NeuronTree nt1 = readSWC_file(infiles[0]);

		SwcTree st;
		st.filePath = infiles[0];
		st.nt = nt1;
		st.initialize();*/


		//v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

