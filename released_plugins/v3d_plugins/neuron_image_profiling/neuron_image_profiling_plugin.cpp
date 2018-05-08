/* neuron_image_profiling_plugin.cpp
 * This is a plugin for profiling image qualities for neuron tracing, via SNR and tubuliarty measures.
 * Given an input swc with labled neurite types (soma, dendrite and axon), generate dynamic range and SNR
 * 2015-7-6 : by Xiaoxiao Liu and Zhi Zhou
 */

#include "v3d_message.h"
#include <vector>
#include "neuron_image_profiling_plugin.h"
#include "profile_swc.h"
#include <iostream>
#include <fstream>

#include <string>

#include <qdir.h>

using namespace std;

Q_EXPORT_PLUGIN2(neuron_image_profiling, image_profiling);
 
QStringList image_profiling::menulist() const
{
	return QStringList() 
		<<tr("profile_swc")
		<<tr("about");
}

QStringList image_profiling::funclist() const
{
	return QStringList()
		<<tr("profile_swc")
		<< tr("segmentBasedProfile")
		<<tr("help");
}

void image_profiling::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("profile_swc"))
	{
		if (!	profile_swc_menu(callback, parent))
		{
                     std::cout << "error in profile_swc_menu() " << std::endl;
		}  
	}
	else 
	{
		printHelp(callback, parent);
	}
}

bool image_profiling::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("profile_swc"))
	{
                    profile_swc_func(callback,input, output);
	}
	else if (func_name == tr("segmentBasedProfile"))
	{
		vector<char*> infiles, inparas, outfiles;
		if (input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
		if (input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
		if (output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

		QString folderPathQ = infiles.at(0);
		string folderPath = folderPathQ.toStdString();
		QDir directory;
		directory.setPath(folderPathQ);
		QFileInfoList swcList = directory.entryInfoList();

		Image4DSimple* inputImgPtr = new Image4DSimple;
		string caseNum = folderPath.substr(folderPath.length() - 9, 9);
		//cout << caseNum << endl;
		string inputImgName = "Y:\\mips\\";
		inputImgName = inputImgName + caseNum + "_ds4.v3draw";
		const char* inputImgNameC = inputImgName.c_str();
		inputImgPtr->loadImage(inputImgNameC);

		vector<QList<NeuronSWC>> segmentListAxonEst;	 IMAGE_METRICS axonEstMetrics;
		vector<QList<NeuronSWC>> segmentListDendriteEst; IMAGE_METRICS dendriteEstMetrics;
		vector<QList<NeuronSWC>> segmentListAxonMan;     IMAGE_METRICS axonManMetrics;
		vector<QList<NeuronSWC>> segmentListDendriteMan; IMAGE_METRICS dendriteManMetrics;

		segmentListAxonEst.clear();
		segmentListDendriteEst.clear();
		segmentListAxonMan.clear();
		segmentListDendriteMan.clear();
		for (int i = 0; i < swcList.size(); ++i)
		{
			string swcFileBaseName = swcList.at(i).baseName().toStdString();
			string swcFileSuffix = swcList.at(i).suffix().toStdString();

			if (!swcFileBaseName.compare("") || !swcFileBaseName.compare(".") || !swcFileBaseName.compare("..")) continue;
			if (!swcFileBaseName.compare(swcFileBaseName.length() - 3, 3, "ds4") || !swcFileSuffix.compare("v3draw")) continue;

			if (!swcFileBaseName.compare(13, 10, "_estimated_seg"))
			{
				//cout << swcFileBaseName << endl;
				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				if (nt.listNeuron.at(0).type == 2) segmentListAxonEst.push_back(nt.listNeuron);
				else if (nt.listNeuron.at(0).type == 3) segmentListDendriteEst.push_back(nt.listNeuron);
			}
			else if (!swcFileBaseName.compare(14, 6, "manual"))
			{
				//cout << swcFileBaseName << endl;
				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				if (nt.listNeuron.at(0).type == 2) segmentListAxonMan.push_back(nt.listNeuron);
				else if (nt.listNeuron.at(0).type == 3) segmentListDendriteMan.push_back(nt.listNeuron);
			}
		}

		if (segmentListAxonEst.size() > 0)
		{

			
			//axonEstMetrics = compute_metricsSegment()
		}

	}
	else if (func_name == tr("help"))
	{
                printHelp(input, output);
	}
    else
        return false;

	return true;
}

